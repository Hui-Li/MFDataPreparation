#include <math.h>
#include "util/Base.h"
#include "struct/User.h"
#include "struct/Item.h"
#include "struct/Rating.h"
#include "util/FileUtil.h"
#include "util/ZLibUtil.h"
#include "util/ThreadUtil.h"

#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/algorithm/string.hpp>

using namespace mf;
using namespace ZLibUtil;

#define use_boost

void transformIds(const string inputPath, unordered_map<string, int> &userIdMap, unordered_map<string, int> &itemIdMap) {

    cout << "read from: " << inputPath << endl;

#ifdef use_boost
    vector<string> par;

    std::ifstream file(inputPath, std::ios_base::in | std::ios_base::binary);

    if (!file.is_open()){
        cerr << "Cannot open file " + inputPath << endl;
        exit(1);
    }

    boost::iostreams::filtering_istream in;
    in.push(boost::iostreams::gzip_decompressor());
    in.push(file);
    unsigned long long line_num = 0;
    for (std::string line; std::getline(in, line);) {
        line_num++;
        if (line_num <= 2){
            continue;
        }

        if (line_num % 10000000 == 0) {
            cout << "transformIds: " << line_num << endl;
        }

        boost::split(par, line, boost::is_any_of("\t"));

        auto itr = userIdMap.find(par[1]);
        if (itr == userIdMap.end()) {
            userIdMap.insert(std::make_pair(par[1], userIdMap.size()));
        }

        itr = itemIdMap.find(par[2]);
        if (itr == itemIdMap.end()) {
            itemIdMap.insert(std::make_pair(par[2], itemIdMap.size()));
        }
    }
    file.close();

#else

    string fileData;
    if (!loadBinaryFile(inputPath, fileData)) {
        printf("Error loading input file.");
        return;
    }

    cout << "finish loading " << inputPath << endl;

    string data;
    if (!gzipInflate(fileData, data)) {
        printf("Error decompressing file.");
        return;
    }

    cout << "finish decompressing " << inputPath << endl;

    stringstream ss(data.c_str());
    string line;

    unsigned long long line_num = 0;
    string user_id;
    string item_id;

    while (std::getline(ss, line, '\n')) {
        line_num++;
        if (line_num <= 2){
            continue;
        }

        if (line_num % 1000000 == 0) {
            cout << "transformIds: " << line_num << endl;
        }

        std::istringstream is(line);
        is >> user_id >> item_id;

        auto itr = userIdMap.find(user_id);
        if (itr == userIdMap.end()) {
            userIdMap.insert(std::make_pair(user_id, userIdMap.size()));
        }

        itr = itemIdMap.find(item_id);
        if (itr == itemIdMap.end()) {
            itemIdMap.insert(std::make_pair(item_id, itemIdMap.size()));
        }
    }
#endif

    cout << "line number: " << line_num << endl;
}

void split(const string dataPath, const string outputFolder, const string metaPath, const string outputMMCTrainPath,
           const string outputMMCTestPath, const string outputCSRTrainPath,
           const string outputCSRTestPath, const double percentage, unordered_map<string, int> &userIdMap, unordered_map<string, int> &itemIdMap, bool output_mmc, bool output_csr, const int num_of_thread) {

    vector<User> users(userIdMap.size());
    vector<Item> items(itemIdMap.size());

    cout << "read from: " << dataPath << endl;

#ifdef use_boost

    vector<string> par;

    std::ifstream file(dataPath, std::ios_base::in | std::ios_base::binary);
    boost::iostreams::filtering_istream in;
    in.push(boost::iostreams::gzip_decompressor());
    in.push(file);

    unsigned long long ratingNum = 0;
    unsigned long long line_num = 0;

    for (std::string line; std::getline(in, line);) {
        line_num++;
        if (line_num <= 2){
            continue;
        }

        if (line_num % 10000000 == 0) {
            cout << "split: " << line_num << endl;
        }

        boost::split(par, line, boost::is_any_of("\t"));

        int userID = userIdMap.find(par[1])->second;
        int itemID = itemIdMap.find(par[2])->second;
        users[userID].userID = userID;
        users[userID].ratedItemIDs.push_back(itemID);
        users[userID].ratings[itemID] = std::stof(par[3]);

        items[itemID].itemID = itemID;
        items[itemID].raterIDs.push_back(userID);
        items[itemID].ratings[userID] = std::stof(par[3]);

        ratingNum++;
    }
    file.close();


#else
    string fileData;
    if (!loadBinaryFile(dataPath, fileData)) {
        printf("Error loading input file.");
        return;
    }

    cout << "finish loading " << dataPath << endl;

    string data;
    if (!gzipInflate(fileData, data)) {
        printf("Error decompressing file.");
        return;
    }

    cout << "finish decompressing " << dataPath << endl;

    stringstream ss(data.c_str());
    string line;

    unsigned long long line_num = 0;
    unsigned long long ratingNum = 0;

    string user_id_str;
    string item_id_str;
    string rating_str;

    while (std::getline(ss, line, '\n')) {
        line_num++;
        if (line_num <= 2){
            continue;
        }

        if (line_num % 10000000 == 0) {
            cout << "split: " << line_num << endl;
        }

        std::istringstream is(line);
        is >> user_id_str >> item_id_str >> rating_str;

        int userID = userIdMap.find(user_id_str)->second;
        int itemID = itemIdMap.find(item_id_str)->second;

        users[userID].userID = userID;
        users[userID].ratedItemIDs.push_back(itemID);
        users[userID].ratings[itemID] = rating_str;

        items[itemID].itemID = itemID;
        items[itemID].raterIDs.push_back(userID);
        items[itemID].ratings[userID] = rating_str;

        ratingNum++;
    }

#endif

    cout << "rating number: " << ratingNum << endl;

    vector<vector<Rating> > trainRatings(num_of_thread);
    vector<vector<Rating> > testRatings(num_of_thread);

    const unsigned long long u_workload = users.size() / num_of_thread + ((users.size() % num_of_thread == 0) ? 0 : 1);
    const unsigned long long i_workload = items.size() / num_of_thread + ((items.size() % num_of_thread == 0) ? 0 : 1);

    // first guarantee that each item at least appears in train set once
    std::function<void(int)> func1 = [&](int thread_index) -> void {

        const int i_start = i_workload * thread_index;
        const int i_end = (i_workload + i_start) < items.size() ? (i_workload + i_start) : items.size();
        const int local_workload = i_end - i_start;
        int count = 0;

        vector<Rating> &localTrainRatings = trainRatings[thread_index];

        for (int i = i_start; i < i_end; i++) {

            count++;

            if(thread_index==0 && (count % 100==0)){
                cout << "i: " << count << " of " << local_workload << endl;
            }

            Item &item = items[i];
            unordered_map<int, float> &ratings = item.ratings;
            vector<int> &raterIDs = item.raterIDs;

            random_shuffle(raterIDs.begin(), raterIDs.end());

            int itemID = item.itemID;
            int userID = raterIDs[0];
            float score = ratings[userID];

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
        const int u_end = (u_workload + u_start) < users.size()?(u_workload + u_start):users.size();
        const int local_workload = u_end - u_start;
        int count = 0;

        vector<Rating> &localTrainRatings = trainRatings[thread_index];

        for (int i = u_start; i < u_end; i++) {

            count++;

            if(thread_index==0 && (count % 100==0)){
                cout << "u1: " << count << " of " << local_workload << endl;
            }

            User &user = users[i];
            unordered_map<int, float> &ratings = user.ratings;
            vector<int> &ratedIDs = user.ratedItemIDs;

            random_shuffle(ratedIDs.begin(), ratedIDs.end());

            int userID = user.userID;
            int itemID = ratedIDs[0];
            float score = ratings[itemID];

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

    unsigned long long trainNum = 0;

    for(auto localTrainRatings:trainRatings) {
        trainNum += localTrainRatings.size();
    }

    double newPercentage = (ceil(ratingNum * percentage) - trainNum) / (ratingNum - trainNum + 0.0);

    std::function<void(int)> func3 = [&](int thread_index) -> void {
        const int u_start = u_workload * thread_index;
        const int u_end = (u_workload + u_start) < users.size() ? (u_workload + u_start) : users.size();
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
            unordered_map<int, float> &ratings = user.ratings;
            random_shuffle(ratedItemIDs.begin(), ratedItemIDs.end());
            unsigned long long flag = ceil(ratedItemIDs.size() * newPercentage);

            for (unsigned long long i = 0; i < flag; i++) {
                localTrainRatings.push_back(Rating(user.userID, ratedItemIDs[i], ratings[ratedItemIDs[i]]));
            }

            for (unsigned long long i = flag; i < user.ratings.size(); i++) {
                localTestRatings.push_back(Rating(user.userID, ratedItemIDs[i], ratings[ratedItemIDs[i]]));
            }
        }
    };

    ThreadUtil::execute_threads(func3, num_of_thread);

    trainNum = 0;
    unsigned long long testNum = 0;

    for(auto localTrainRatings:trainRatings) {
        trainNum += localTrainRatings.size();
    }

    for(auto localTestRatings:testRatings) {
        testNum += localTestRatings.size();
    }

    vector<Rating> globalTrainRatings;
    globalTrainRatings.reserve(trainNum); // preallocate memory

    for(auto localTrainRatings:trainRatings) {
        globalTrainRatings.insert( globalTrainRatings.end(), localTrainRatings.begin(), localTrainRatings.end() );
    }

    vector<Rating> globalTestRatings;
    globalTestRatings.reserve(testNum); // preallocate memory

    for(auto localTestRatings:testRatings) {
        globalTestRatings.insert( globalTestRatings.end(), localTestRatings.begin(), localTestRatings.end() );
    }

    if(output_mmc) {
        cout << "output_mmc" << endl;
        writeMMC_GZ(globalTrainRatings, userIdMap.size(), itemIdMap.size(), outputFolder, outputMMCTrainPath);
        writeMMC_GZ(globalTestRatings, userIdMap.size(), itemIdMap.size(), outputFolder, outputMMCTestPath);
    }

    if(output_csr) {
        cout << "output_csr" << endl;
        writeMeta(outputFolder + metaPath, userIdMap.size(), itemIdMap.size(), trainRatings.size(), testRatings.size(), outputCSRTrainPath, outputCSRTestPath);
        writeCSR_GZ(globalTrainRatings, userIdMap.size(), itemIdMap.size(), outputFolder, outputCSRTrainPath);
        writeCSR_GZ(globalTestRatings, userIdMap.size(), itemIdMap.size(), outputFolder, outputCSRTestPath);
    }
}

int main(int argc, char const *argv[]){

    string outputFolder = "/data/huilee/mf_data/hugewiki/";
    string filePath = "/data/huilee/raw_data/hugewiki.gz";
    string metaPath = "meta";
    string outputMMCTrainPath = "train.mmc";
    string outputMMCTestPath = "test.mmc";
    string outputCSRTrainPath = "train.csr";
    string outputCSRTestPath = "test.csr";
    int num_thread = 40;
    bool output_mmc = true;
    bool output_csr = true;

    // 80% of each user's ratings are used for training
    double percentage = 0.8;

    unordered_map<string, int> userIdMap;
    unordered_map<string, int> itemIdMap;

    cout << "transformIds" << endl;

    transformIds(filePath, userIdMap, itemIdMap);

    cout << "split" << endl;

    split(filePath, outputFolder, metaPath, outputMMCTrainPath, outputMMCTestPath, outputCSRTrainPath,
          outputCSRTestPath, percentage, userIdMap, itemIdMap, output_mmc, output_csr, num_thread);

    return 0;
}