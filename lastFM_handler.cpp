#include "util/Base.h"
#include "struct/User.h"
#include "struct/Item.h"
#include "struct/Rating.h"
#include "util/FileUtil.h"
#include "util/ThreadUtil.h"
#include <algorithm>


using namespace mf;

//inline double sigmoid(double x) {
//    return 1.0 / (1.0 + exp(-x));
//}

void transformIds(const string inputPath, unordered_map<string, int> &userIdMap, unordered_map<string, int> &itemIdMap,
                  unordered_map<int, string> &r_userIdMap, unordered_map<int, string> &r_itemIdMap) {

    string line;
    ifstream fin;
    fin.open(inputPath.c_str());

    vector<string> par;
    bool first_line = true;
    while (getline(fin, line)) {
        if (first_line) {
            first_line = false;
            continue;
        }
        if (line.length() == 0) {
            continue;
        }
        par.clear();
        boost::split(par, line, boost::is_any_of("\t"));

        auto itr = userIdMap.find(par[0]);
        if (itr == userIdMap.end()) {
            userIdMap.insert(std::make_pair(par[0], userIdMap.size()));
            r_userIdMap.insert(std::make_pair(r_userIdMap.size(), par[0]));
        }

        itr = itemIdMap.find(par[1]);
        if (itr == itemIdMap.end()) {
            itemIdMap.insert(std::make_pair(par[1], itemIdMap.size()));
            r_itemIdMap.insert(std::make_pair(r_itemIdMap.size(), par[1]));
        }
    }
    fin.close();

}

void split(const int num_of_thread, const string dataPath, const string outputFolder, const string metaPath, const string userIDMapPath, const string itemIDMapPath, const string outputMMCTrainPath,
           const string outputMMCTestPath, const string outputCSRTrainPath,
           const string outputCSRTestPath, const double percentage, unordered_map<string, int> &userIdMap, unordered_map<string, int> &itemIdMap, unordered_map<int, string> &r_userIdMap, unordered_map<int, string> &r_itemIdMap) {

    string line;
    vector<User> users(userIdMap.size());
    vector<Item> items(itemIdMap.size());

    vector<string> par;

    int ratingNum = 0;

    bool first_line = true;
    ifstream fin(dataPath.c_str());
    while (getline(fin, line)) {
        if (first_line) {
            first_line = false;
            continue;
        }
        if (line.length() == 0) {
            continue;
        }
        par.clear();
        boost::split(par, line, boost::is_any_of("\t"));

        int userID = userIdMap.find(par[0])->second;
        int itemID = itemIdMap.find(par[1])->second;

        string rating = par[2];

        users[userID].userID = userID;
        users[userID].ratedItemIDs.push_back(itemID);
        users[userID].ratings[itemID] = rating;

        items[itemID].itemID = itemID;
        items[itemID].raterIDs.push_back(userID);
        items[itemID].ratings[userID] = rating;

        ratingNum++;
    }
    fin.close();

    for (auto user_iter = users.begin(); user_iter != users.end(); user_iter++) {
        if (user_iter->ratedItemIDs.size() == 0) {
            cout << "No interaction user: " << r_userIdMap[user_iter->userID] << endl;
        }
    }

    for (auto item_iter = items.begin(); item_iter != items.end(); item_iter++) {
        if (item_iter->raterIDs.size() == 0) {
            cout << "No interaction item: " << r_itemIdMap[item_iter->itemID] << endl;
        }
    }

    cout << "rating number: " << ratingNum << endl;

    vector<vector<Rating> > trainRatings(num_of_thread);
    vector<vector<Rating> > testRatings(num_of_thread);

    const int u_workload = users.size() / num_of_thread + ((users.size() % num_of_thread == 0) ? 0 : 1);
    const int i_workload = items.size() / num_of_thread + ((items.size() % num_of_thread == 0) ? 0 : 1);

    // first guarantee that each item at least appears in train set once
    std::function<void(int)> func1 = [&](int thread_index) -> void {

        const int i_start = i_workload * thread_index;
        const int i_end = std::min(i_workload + i_start, (int) items.size());
        const int local_workload = i_end - i_start;
        int count = 0;

        vector<Rating> &localTrainRatings = trainRatings[thread_index];

        for (int i = i_start; i < i_end; i++) {

            count++;

            if(thread_index==0 && (count % 100==0)){
                cout << "i: " << count << " of " << local_workload << endl;
            }

            Item &item = items[i];
            unordered_map<int, string> &ratings = item.ratings;
            vector<int> &raterIDs = item.raterIDs;

            random_shuffle(raterIDs.begin(), raterIDs.end());

            int itemID = item.itemID;
            int userID = raterIDs[0];
            string score = ratings[userID];

            localTrainRatings.push_back(Rating(userID, itemID, score));

            //remove from users
            User &user = users[userID];
            user.locker.lock();
            user.ratedItemIDs.erase(std::remove(user.ratedItemIDs.begin(), user.ratedItemIDs.end(), itemID),
                                    user.ratedItemIDs.end());
            user.ratings.erase(itemID);
            user.locker.unlock();
        }
    };

    ThreadUtil::execute_threads(func1, num_of_thread);

    // then guarantee that each user at least appears in train set once
    std::function<void(int)> func2 = [&](int thread_index) -> void {
        const int u_start = u_workload * thread_index;
        const int u_end = std::min(u_workload + u_start, (int) users.size());
        const int local_workload = u_end - u_start;
        int count = 0;

        vector<Rating> &localTrainRatings = trainRatings[thread_index];

        for (int i = u_start; i < u_end; i++) {

            count++;

            if(thread_index==0 && (count % 100==0)){
                cout << "u1: " << count << " of " << local_workload << endl;
            }

            User &user = users[i];
            unordered_map<int, string> &ratings = user.ratings;
            vector<int> &ratedIDs = user.ratedItemIDs;

            random_shuffle(ratedIDs.begin(), ratedIDs.end());

            int userID = user.userID;
            int itemID = ratedIDs[0];
            string score = ratings[itemID];

            localTrainRatings.push_back(Rating(userID, itemID, score));

            //remove from items
            Item &item = items[itemID];
            item.locker.lock();
            item.raterIDs.erase(std::remove(item.raterIDs.begin(), item.raterIDs.end(), userID),
                                item.raterIDs.end());
            item.ratings.erase(userID);
            item.locker.unlock();
        }
    };

    ThreadUtil::execute_threads(func2, num_of_thread);

    int trainNum = 0;

    for(auto localTrainRatings:trainRatings) {
        trainNum += localTrainRatings.size();
    }

    double newPercentage = (ceil(ratingNum * percentage) - trainNum) / (ratingNum - trainNum + 0.0);

    std::function<void(int)> func3 = [&](int thread_index) -> void {
        const int u_start = u_workload * thread_index;
        const int u_end = std::min(u_workload + u_start, (int) users.size());
        const int local_workload = u_end - u_start;

        vector<Rating> &localTrainRatings = trainRatings[thread_index];
        vector<Rating> &localTestRatings = testRatings[thread_index];

        int count = 0;

        for (int u = u_start; u < u_end; u++) {

            count++;

            if(thread_index==0 && (count % 100==0)){
                cout << "u2: " << count << " of " << local_workload << endl;
            }

            User &user = users[u];
            vector<int> &ratedItemIDs = user.ratedItemIDs;
            unordered_map<int, string> &ratings = user.ratings;
            random_shuffle(ratedItemIDs.begin(), ratedItemIDs.end());
            const int flag = ceil(ratedItemIDs.size() * newPercentage);

            for (int i = 0; i < flag; i++) {
                localTrainRatings.push_back(Rating(user.userID, ratedItemIDs[i], ratings[ratedItemIDs[i]]));
            }

            for (int i = flag; i < user.ratings.size(); i++) {
                localTestRatings.push_back(Rating(user.userID, ratedItemIDs[i], ratings[ratedItemIDs[i]]));
            }
        }
    };

    ThreadUtil::execute_threads(func3, num_of_thread);

    trainNum = 0;
    int testNum = 0;

    for(auto localTrainRatings:trainRatings) {
        trainNum += localTrainRatings.size();
    }

    for(auto localTestRatings:testRatings) {
        testNum += localTestRatings.size();
    }

    vector<Rating> globalTrainRatings;
    globalTrainRatings.reserve(trainNum); // preallocate memory

    for(auto localTrainRatings:trainRatings) {
        globalTrainRatings.insert(globalTrainRatings.end(), localTrainRatings.begin(), localTrainRatings.end());
    }

    vector<Rating> globalTestRatings;
    globalTestRatings.reserve(testNum); // preallocate memory

    for(auto localTestRatings:testRatings) {
        globalTestRatings.insert(globalTestRatings.end(), localTestRatings.begin(), localTestRatings.end());
    }

    writeMatrix(globalTrainRatings, userIdMap.size(), itemIdMap.size(), outputFolder, outputMMCTrainPath, outputCSRTrainPath, "R_train_");

    writeMatrix(globalTestRatings, userIdMap.size(), itemIdMap.size(), outputFolder, outputMMCTestPath, outputCSRTestPath, "R_test_");

    writeMeta(outputFolder + metaPath, userIdMap.size(), itemIdMap.size(), trainNum, testNum, outputCSRTrainPath, outputCSRTestPath);

    writeKeyMap(outputFolder + userIDMapPath, userIdMap);

    writeKeyMap(outputFolder + itemIDMapPath, itemIdMap);
}


int main(int argc, char const *argv[]){

    string outputFolder;
    string filePath;
    string metaPath;
    string userIDMapPath;
    string itemIDMapPath;
    string outputMMCTrainPath;
    string outputMMCTestPath;
    string outputCSRTrainPath;
    string outputCSRTestPath;
    int num_of_thread;

    // 80% of each user's ratings are used for training
    double percentage;

    po::options_description desc("Allowed options");
    desc.add_options()
            ("help", "produce help message")
            ("percentage", po::value<double>(&percentage)->default_value(0.8), "percentage of training ratings")
            ("num_of_thread", po::value<int>(&num_of_thread)->default_value(8), "number of threads")
            ("rating_path", po::value<string>(&filePath)->default_value("../raw_data/lastFM/user_artists.dat"), "path to original rating file")
            ("o_folder", po::value<string>(&outputFolder)->default_value("../data/lastFM/"), "path to output folder")
            ("meta_path", po::value<string>(&metaPath)->default_value("meta"), "name of meta file")
            ("user_id_map_path", po::value<string>(&userIDMapPath)->default_value("user_id_map.dat"), "name of user id map file")
            ("item_id_map_path", po::value<string>(&itemIDMapPath)->default_value("item_id_map.dat"), "name of item id map file")
            ("o_mmc_train", po::value<string>(&outputMMCTrainPath)->default_value("train.mmc"), "name of output MMC training file")
            ("o_mmc_test", po::value<string>(&outputMMCTestPath)->default_value("test.mmc"), "name of output MMC testing file")
            ("o_csr_train", po::value<string>(&outputCSRTrainPath)->default_value("train.csr"), "name of output CSR training file")
            ("o_csr_test", po::value<string>(&outputCSRTestPath)->default_value("test.csr"), "name of output CSR testing file");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help")) {
        cout << desc << endl;
        return 0;
    }

    srand(time(NULL));

    unordered_map<string, int> userIdMap;
    unordered_map<string, int> itemIdMap;
    unordered_map<int, string> r_userIdMap;
    unordered_map<int, string> r_itemIdMap;
    transformIds(filePath, userIdMap, itemIdMap, r_userIdMap, r_itemIdMap);

    split(num_of_thread, filePath, outputFolder, metaPath, userIDMapPath, itemIDMapPath, outputMMCTrainPath, outputMMCTestPath, outputCSRTrainPath,
          outputCSRTestPath, percentage, userIdMap, itemIdMap, r_userIdMap, r_itemIdMap);

    return 0;
}
