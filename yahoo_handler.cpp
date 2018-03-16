#include "util/Base.h"
#include "struct/Rating.h"
#include "util/FileUtil.h"

using namespace mf;

void transform(const string trainFilePath, const string testFilePath, const string outputfolder,
               const string outputMMCTrainPath, const string outputMMCTestPath, const string outputCSRTrainPath,
               const string outputCSRTestPath, const string metaFileName) {

    vector<Rating> trainRatings;
    vector<Rating> testRatings;

    string line;

    int trainUserNum = -1;
    int trainItemNum = -1;
    int currentUserId = -1;

    vector<string> par;

    string flag = "|";
    string flag2 = "\t";

    // All user id's and item id's are consecutive integers (no holes!)
    ifstream train(trainFilePath.c_str());

    while (getline(train, line)){
        if (line.find(flag) != std::string::npos) {
            boost::split(par, line, boost::is_any_of(flag));
            currentUserId = stoi(par[0]);
            trainUserNum = std::max(currentUserId, trainUserNum);
        } else {
            boost::split(par, line, boost::is_any_of(flag2));
            int itemID = stoi(par[0]);
            trainRatings.push_back(Rating(currentUserId, itemID, par[1]));
            trainItemNum = std::max(itemID, trainItemNum);
        }
    }

    train.close();

    int testUserNum = -1;
    int testItemNum = -1;
    currentUserId = -1;

    ifstream test(testFilePath.c_str());

    while (getline(test, line)){
        if (line.find(flag) != std::string::npos) {
            boost::split(par, line, boost::is_any_of(flag));
            currentUserId = stoi(par[0]);
            testUserNum = std::max(currentUserId, testUserNum);
        } else {
            boost::split(par, line, boost::is_any_of(flag2));
            int itemID = stoi(par[0]);
            testRatings.push_back(Rating(currentUserId, itemID, par[1]));
            testItemNum = std::max(itemID, testItemNum);
        }
    }

    test.close();

    trainUserNum++;
    trainItemNum++;
    testUserNum++;
    testItemNum++;

    writeMatrix(trainRatings, trainUserNum, trainItemNum, outputfolder, outputMMCTrainPath, outputCSRTrainPath, "R_train_");

    writeMatrix(testRatings, trainUserNum, trainItemNum, outputfolder, outputMMCTestPath, outputCSRTestPath, "R_test_");

    writeMeta(outputfolder + metaFileName, trainUserNum, trainItemNum, trainRatings.size(), testRatings.size(), outputCSRTrainPath, outputCSRTestPath);
}

int main(int argc, char const *argv[]) {

    string outputFolder;
    string trainPath;
    string testPath;
    string metaPath;
    string outputMMCTrainPath;
    string outputMMCTestPath;
    string outputCSRTrainPath;
    string outputCSRTestPath;

    po::options_description desc("Allowed options");
    desc.add_options()
            ("help", "produce help message")
            ("i_train", po::value<string>(&trainPath)->default_value("../raw_data/yahoo/trainIdx1.txt"), "path to original training file")
            ("i_test", po::value<string>(&testPath)->default_value("../raw_data/yahoo/validationIdx1.txt"), "path to original testing file")
            ("o_folder", po::value<string>(&outputFolder)->default_value("../data/yahoo/"), "path to output folder")
            ("meta_path", po::value<string>(&metaPath)->default_value("meta"), "name of meta file")
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

    transform(trainPath, testPath, outputFolder, outputMMCTrainPath, outputMMCTestPath, outputCSRTrainPath, outputCSRTestPath, metaPath);

    return 0;
}