#include "util/Base.h"
#include "struct/Rating.h"
#include "util/FileUtil.h"

using namespace mf;

void transformIds(const string &input_path, unordered_map<int, int> &uid_map, unordered_map<int, int> &mid_map,
                  const int skip_line) {

    ifstream fin(input_path.c_str());
    fin.tie(nullptr);
    if (fin.fail()) {
        cout << "fail to open the dir" << endl;
        exit(0);
    }

    int line_counter = 0;
    string row;
    while (line_counter < skip_line) {
        getline(fin, row);
        line_counter++;
    }

    int uid, mid;
    float rate;

    //read from file
    while (fin >> uid >> mid >> rate) {

        //update line counter
        line_counter++;

        //print the progress
        if (line_counter % 1000000 == 0) {
            cout << "read " << line_counter << " lines...\n";
            cout.flush();
        }

        //remap the ids
        if (uid_map.find(uid) == uid_map.end()) {
            uid_map.insert(std::pair<int, int>(uid, uid_map.size()));
        }

        if (mid_map.find(mid) == mid_map.end()) {
            mid_map.insert(std::pair<int, int>(mid, mid_map.size()));
        }
    }
    fin.close();
}

int convertFile(const string &input_path, const string &outputFolder, const string &outputMMCFileName,
                const string &outputCSRFileName, unordered_map<int, int> &uid_map,
                unordered_map<int, int> &mid_map, const int skip_line, const string &cumf_prefix) {

    vector<Rating> ratings;

    ifstream fin(input_path.c_str());

    fin.tie(nullptr);

    int line_counter = 0;
    string row;
    while (line_counter < skip_line) {
        getline(fin, row);
        line_counter++;
    }

    int uid, mid;
    string rate;

    while (fin >> uid >> mid >> rate) {

        line_counter++;

        if (line_counter % 1000000 == 0) {
            cout << "load " << line_counter << " lines...\n";
            cout.flush();
        }

        ratings.push_back(Rating(uid_map[uid], mid_map[mid], rate));

    }

    fin.close();

    writeMatrix(ratings, uid_map.size(), mid_map.size(), outputFolder, outputMMCFileName, outputCSRFileName, cumf_prefix);

    return line_counter - skip_line;
}

int main(int argc, char const *argv[]) {

    string outputFolder;
    string trainPath;
    string testPath;
    string metaPath;
    string userIDMapPath;
    string itemIDMapPath;
    string outputMMCTrainPath;
    string outputMMCTestPath;
    string outputCSRTrainPath;
    string outputCSRTestPath;

    po::options_description desc("Allowed options");
    desc.add_options()
            ("help", "produce help message")
            ("i_train", po::value<string>(&trainPath)->default_value("../raw_data/netflix/netflix_mm"), "path to original training file")
            ("i_test", po::value<string>(&testPath)->default_value("../raw_data/netflix/netflix_mme"), "path to original testing file")
            ("o_folder", po::value<string>(&outputFolder)->default_value("../data/netflix/"), "path to output folder")
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

    unordered_map<int, int> uid_map;
    unordered_map<int, int> mid_map;
    transformIds(trainPath, uid_map, mid_map, 3);

    int num_train, num_test;

    num_train = convertFile(trainPath, outputFolder, outputMMCTrainPath, outputCSRTrainPath, uid_map, mid_map, 3, "R_train_");
    num_test = convertFile(testPath, outputFolder, outputMMCTestPath, outputCSRTestPath, uid_map, mid_map, 3, "R_test_");

    writeMeta(outputFolder + metaPath, uid_map.size(), mid_map.size(), num_train, num_test, outputCSRTrainPath, outputCSRTestPath);

    writeKeyMap(outputFolder + userIDMapPath, uid_map);
    writeKeyMap(outputFolder + itemIDMapPath, mid_map);

    return 0;
}


