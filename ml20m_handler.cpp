#include "util/Base.h"
#include "struct/User.h"
#include "struct/Item.h"
#include "struct/Rating.h"
#include "util/FileUtil.h"

using namespace mf;

void transformIds(const string inputPath, unordered_map<string, int> &userIdMap, unordered_map<string, int> &itemIdMap) {

	string line;
	ifstream fin;
	fin.open(inputPath.c_str());

    // skip fisrt line
    getline(fin, line);

	vector<string> par;

	while (getline(fin, line)) {

		if (line.length() == 0) {
			continue;
		}
		par.clear();
		boost::split(par, line, boost::is_any_of(","));


		auto itr = userIdMap.find(par[0]);
		if (itr == userIdMap.end()) {
			userIdMap.insert(std::make_pair(par[0], userIdMap.size()));
		}

		itr = itemIdMap.find(par[1]);
		if (itr == itemIdMap.end()) {
			itemIdMap.insert(std::make_pair(par[1], itemIdMap.size()));
		}
	}
	fin.close();

}

void split(const string dataPath, const string outputFolder, const string metaPath, const string userIDMapPath, const string itemIDMapPath, const string outputMMCTrainPath,
		   const string outputMMCTestPath, const string outputCSRTrainPath,
		   const string outputCSRTestPath,
		   const double percentage, unordered_map<string, int> &userIdMap, unordered_map<string, int> &itemIdMap) {

	string line;
	vector<User> users(userIdMap.size());
	vector<Item> items(itemIdMap.size());

	vector<string> par;

	int ratingNum = 0;

	ifstream fin(dataPath.c_str());

    // skip fisrt line
    getline(fin, line);

	while (getline(fin, line)) {
		if (line.length() == 0) {
			continue;
		}
		par.clear();
		boost::split(par, line, boost::is_any_of(","));

		int userID = userIdMap.find(par[0])->second;
		int itemID = itemIdMap.find(par[1])->second;
		users[userID].userID = userID;
		users[userID].ratedItemIDs.push_back(itemID);
		users[userID].ratings[itemID] = par[2];

		items[itemID].itemID = itemID;
		items[itemID].raterIDs.push_back(userID);
		items[itemID].ratings[userID] = par[2];

		ratingNum++;
	}
	fin.close();

	int trainNum = 0;
	int testNum = 0;

    vector<Rating> trainRatings;
    vector<Rating> testRatings;

	// each user has at least rated 20 items in the original data
	// first guarantee that each item at least appears in train set once
	for (int i = 0; i < items.size(); i++) {
		Item &item = items[i];
		unordered_map<int, string> &ratings = item.ratings;
		vector<int> &raterIDs = item.raterIDs;
		random_shuffle(raterIDs.begin(), raterIDs.end());

		int itemID = item.itemID;
		int userID = raterIDs[0];
		string score = ratings[userID];

        trainRatings.push_back(Rating(userID, itemID, ratings[userID]));

		trainNum++;

		//remove from users
		User &user = users[userID];
		user.ratedItemIDs.erase(std::remove(user.ratedItemIDs.begin(), user.ratedItemIDs.end(), itemID),
								user.ratedItemIDs.end());
		user.ratings.erase(itemID);

	}

	double newPercentage = (ceil(ratingNum * percentage) - trainNum) / (ratingNum - trainNum + 0.0);

	for (User &user:users) {
		vector<int> &ratedItemIDs = user.ratedItemIDs;
		unordered_map<int, string> &ratings = user.ratings;
		random_shuffle(ratedItemIDs.begin(), ratedItemIDs.end());
		int flag = ceil(ratedItemIDs.size() * newPercentage);

		for (int i = 0; i < flag; i++) {

            trainRatings.push_back(Rating(user.userID, ratedItemIDs[i], ratings[ratedItemIDs[i]]));
			trainNum++;
		}

		for (int i = flag; i < user.ratings.size(); i++) {

            testRatings.push_back(Rating(user.userID, ratedItemIDs[i], ratings[ratedItemIDs[i]]));
			testNum++;
		}
	}

	writeMatrix(trainRatings, userIdMap.size(), itemIdMap.size(), outputFolder, outputMMCTrainPath, outputCSRTrainPath, "R_train_");

	writeMatrix(testRatings, userIdMap.size(), itemIdMap.size(), outputFolder, outputMMCTestPath, outputCSRTestPath, "R_test_");

	writeMeta(outputFolder + metaPath, userIdMap.size(), itemIdMap.size(), trainRatings.size(), testRatings.size(), outputCSRTrainPath, outputCSRTestPath);

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

	// 80% of each user's ratings are used for training
	double percentage;

	po::options_description desc("Allowed options");
	desc.add_options()
			("help", "produce help message")
			("percentage", po::value<double>(&percentage)->default_value(0.8), "percentage of training ratings")
			("rating_path", po::value<string>(&filePath)->default_value("../raw_data/ml20m/ratings.csv"), "path to original rating file")
			("o_folder", po::value<string>(&outputFolder)->default_value("../data/ml20m/"), "path to output folder")
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
	transformIds(filePath, userIdMap, itemIdMap);

	split(filePath, outputFolder, metaPath, userIDMapPath, itemIDMapPath, outputMMCTrainPath, outputMMCTestPath, outputCSRTrainPath,
		  outputCSRTestPath, percentage, userIdMap, itemIdMap);

	return 0;
}
