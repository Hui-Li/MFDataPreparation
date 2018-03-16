#ifndef FILEUTIL_H
#define FILEUTIL_H

#include <zlib.h>
#include "../util/Base.h"
#include "../struct/Rating.h"

namespace mf {

    void writeMeta(const string metaFilePath, const int userNum, const int itemNum, const int trainRatingNum,
                   const int testRatingNum, const string mmcTrainFileName, const string mmcTestFileName) {
        // create meta file
        ofstream fout;
        fout.open(metaFilePath);
        fout << userNum << " " << itemNum << endl;
        fout << trainRatingNum << " " << mmcTrainFileName << endl;
        fout << testRatingNum << " " << mmcTestFileName << endl;
        fout.close();
    }

    void writeBinaryFile(const string path, char *content, const int size) {

        std::ofstream ValueOut(path);
        if (!ValueOut.is_open()){
            cerr << "Cannot open file " + path << endl;
            exit(1);
        }

        ValueOut.write(content, size);
        ValueOut.close();
    }

    struct less_than_key {
        inline bool operator()(const Rating &r1, const Rating &r2) {
            if (r1.itemID < r2.itemID) {
                return true;
            } else if (r1.itemID == r2.itemID) {
                return r1.userID < r2.userID;
            } else {
                return false;
            }
        }
    };

    void writeCUMF_ALSFiles(vector<Rating> &ratings, const int userNum, const int itemNum, const string outputFolder, const string cumf_prefix){

        boost::filesystem::create_directories(outputFolder + "cumf_als/");
        string cumf_path = "cumf_als/" + cumf_prefix;

        // Since we remap ids and sort ratings, the order of COO output for cumf_als is different to the output from their python code
        // sort by column
        sort(ratings.begin(), ratings.end(), less_than_key());

        const int size_of_float = sizeof(float);
        const int size_of_int = sizeof(int);

        // csr, csc and coo for cumf are processed on (item,user,rating)
        int *coo_row_idx = new int [ratings.size()];
        int *coo_col_idx = nullptr;
        float *coo_score_idx = nullptr;

        float *csr_score_float = new float[ratings.size()];
        int *csr_row_ptr = new int[itemNum+1];
        int *csr_col_idx = new int[ratings.size()];

        csr_row_ptr[0] = 0;
        int currentItemID = 0;
        int ratingNumForCurrentRow = 0;

        for (int ratingIndex = 0; ratingIndex < ratings.size(); ratingIndex++) {
            Rating &rating = ratings[ratingIndex];
            csr_score_float[ratingIndex] = stod(rating.rating);
            csr_col_idx[ratingIndex] = rating.userID;
            coo_row_idx[ratingIndex] = rating.itemID;

            while(currentItemID!=rating.itemID){
                csr_row_ptr[currentItemID + 1] = csr_row_ptr[currentItemID] + ratingNumForCurrentRow;
                currentItemID++;
                ratingNumForCurrentRow = 0;
            }

            ratingNumForCurrentRow++;
        }

        csr_row_ptr[itemNum] = csr_row_ptr[itemNum-1] + ratingNumForCurrentRow;

        // Transpose CSR into CCS matrix
        float *ccs_rating_scores = new float[ratings.size()];
        int *ccs_row_idx = new int[ratings.size()];
        int *ccs_col_ptr = new int[userNum + 1];
        std::fill(ccs_col_ptr, ccs_col_ptr + userNum + 1, 0);

        int k = 0;
        int *reverse_row_index = new int[ratings.size()]; // each value belongs to which row
        for (int i = 0; i < itemNum; i++) {
            for (int j = 0; j < csr_row_ptr[i + 1] - csr_row_ptr[i]; j++) {
                reverse_row_index[k] = i;
                k++;
            }
        }

        for (int i = 0; i < ratings.size(); i++) {
            ccs_col_ptr[csr_col_idx[i] + 1]++;
        }
        for (int i = 1; i <= userNum; i++) {
            ccs_col_ptr[i] += ccs_col_ptr[i - 1];
        }

        int *nn = new int[userNum + 1];
        std::copy(ccs_col_ptr, ccs_col_ptr + userNum + 1, nn);

        for (int i = 0; i < ratings.size(); i++) {
            int x = nn[csr_col_idx[i]];
            nn[csr_col_idx[i]] += 1;
            ccs_rating_scores[x] = csr_score_float[i];
            ccs_row_idx[x] = reverse_row_index[i];
        }

        delete[] reverse_row_index;
        delete[] nn;

        if (cumf_prefix.compare("R_test_") != 0) {
            writeBinaryFile(outputFolder + cumf_path + "csr.data.bin", (char *) csr_score_float,
                            size_of_float * ratings.size());
            writeBinaryFile(outputFolder + cumf_path + "csr.indices.bin", (char *) csr_col_idx,
                            size_of_int * ratings.size());
            writeBinaryFile(outputFolder + cumf_path + "csr.indptr.bin", (char *) csr_row_ptr,
                            size_of_int * (itemNum + 1));

            writeBinaryFile(outputFolder + cumf_path + "csc.data.bin", (char *) ccs_rating_scores,
                            size_of_float * ratings.size());
            writeBinaryFile(outputFolder + cumf_path + "csc.indices.bin", (char *) ccs_row_idx,
                            size_of_int * ratings.size());
            writeBinaryFile(outputFolder + cumf_path + "csc.indptr.bin", (char *) ccs_col_ptr,
                            size_of_int * (userNum + 1));
        }

        delete[] csr_score_float;
        delete[] csr_row_ptr;
        delete[] csr_col_idx;

        delete[] ccs_rating_scores;
        delete[] ccs_row_idx;
        delete[] ccs_col_ptr;

        // COO
        writeBinaryFile(outputFolder + cumf_path + "coo.row.bin", (char *) coo_row_idx, sizeof(int) * ratings.size());

        delete[] coo_row_idx;

        if(cumf_prefix.compare("R_test_")==0){

            coo_col_idx = new int[ratings.size()];
            coo_score_idx = new float[ratings.size()];

            int i =0;
            for(auto rating:ratings){
                coo_col_idx[i] = rating.userID;
                coo_score_idx[i] = stod(rating.rating);
                i++;
            }

            writeBinaryFile(outputFolder + cumf_path + "coo.data.bin", (char *) coo_score_idx, sizeof(float) * ratings.size());
            writeBinaryFile(outputFolder + cumf_path + "coo.col.bin", (char *) coo_col_idx, sizeof(int) * ratings.size());

            delete[] coo_col_idx;
            delete[] coo_score_idx;
        }

    }

    void writeMMC(vector<Rating> &ratings, const int userNum, const int itemNum, const string outputFolder,
                  const string MMCOutputPath, const bool addOneForID=true) {

        sort(ratings.begin(), ratings.end());

        std::ofstream MMCOut(outputFolder + MMCOutputPath);
        if (!MMCOut.is_open()){
            cerr << "Cannot open file " + MMCOutputPath << endl;
            exit(1);
        }

        MMCOut.precision(std::numeric_limits<double>::digits10 + 1);

        // write header
        MMCOut << "%%MatrixMarket matrix coordinate real general" << std::endl;
        MMCOut << "%=================================================================================" << std::endl;
        MMCOut << "%" << std::endl;
        MMCOut << "% This ASCII file represents a sparse MxN matrix with L" << std::endl;
        MMCOut << "% nonzeros in the following Matrix Market format:" << std::endl;
        MMCOut << "%" << std::endl;
        MMCOut << "% +----------------------------------------------+" << std::endl;
        MMCOut << "% |%%MatrixMarket matrix coordinate real general | <--- header line" << std::endl;
        MMCOut << "% |%                                             | <--+" << std::endl;
        MMCOut << "% |% comments                                    |    |-- 0 or more comment lines" << std::endl;
        MMCOut << "% |%                                             | <--+" << std::endl;
        MMCOut << "% |    M  N  L                                   | <--- rows, columns, entries" << std::endl;
        MMCOut << "% |    I1  J1  A(I1, J1)                         | <--+" << std::endl;
        MMCOut << "% |    I2  J2  A(I2, J2)                         |    |" << std::endl;
        MMCOut << "% |    I3  J3  A(I3, J3)                         |    |-- L lines" << std::endl;
        MMCOut << "% |        . . .                                 |    |" << std::endl;
        MMCOut << "% |    IL JL  A(IL, JL)                          | <--+" << std::endl;
        MMCOut << "% +----------------------------------------------+   " << std::endl;
        MMCOut << "%" << std::endl;
        MMCOut << "% Indices are 1-based, i.e. A(1,1) is the first element." << std::endl;
        MMCOut << "%" << std::endl;
        MMCOut << "%=================================================================================" << std::endl;

        // write dimension line
        MMCOut << userNum << " " << itemNum << " " << ratings.size() << std::endl;

        // write matrix
        for (auto rating:ratings) {
            if(addOneForID){
                MMCOut << (rating.userID+1) << " " << (rating.itemID+1) << " " << rating.rating << std::endl;
            } else {
                MMCOut << rating.userID << " " << rating.itemID << " " << rating.rating << std::endl;
            }

        }

        // done
        MMCOut.close();

    }

    void writeCSR(vector<Rating> &ratings, const int userNum, const int itemNum, const string outputFolder,
                  const string CSROutputPath){

        sort(ratings.begin(), ratings.end());

        // write CSR
        // https://en.wikipedia.org/wiki/Sparse_matrix#Compressed_sparse_row_.28CSR.2C_CRS_or_Yale_format.29
        double *csr_score_double = new double[ratings.size()];
        int *csr_row_ptr = new int[userNum+1];
        int *csr_col_idx = new int[ratings.size()];

        csr_row_ptr[0] = 0;
        int currentUserID = 0;
        int ratingNumForCurrentRow = 0;

        for (int ratingIndex = 0; ratingIndex < ratings.size(); ratingIndex++) {
            Rating &rating = ratings[ratingIndex];
            csr_score_double[ratingIndex] = stod(rating.rating);
            csr_col_idx[ratingIndex] = rating.itemID;

            while(currentUserID!=rating.userID){
                csr_row_ptr[currentUserID + 1] = csr_row_ptr[currentUserID] + ratingNumForCurrentRow;
                currentUserID++;
                ratingNumForCurrentRow = 0;
            }

            ratingNumForCurrentRow++;
        }

        csr_row_ptr[userNum] = csr_row_ptr[userNum-1] + ratingNumForCurrentRow;

        std::ofstream CSROut(outputFolder + CSROutputPath);
        if (!CSROut.is_open()){
            cerr << "Cannot open file " + CSROutputPath << endl;
            exit(1);
        }

        const int size_of_double = sizeof(double);
        const int size_of_int = sizeof(int);

        CSROut.write((char *) csr_score_double, size_of_double * ratings.size());
        CSROut.write((char *) csr_row_ptr, size_of_int * (userNum + 1));
        CSROut.write((char *) csr_col_idx, size_of_int * ratings.size());

        CSROut.close();

        delete[] csr_score_double;
        delete[] csr_row_ptr;
        delete[] csr_col_idx;

    }

    void writeMatrix(vector<Rating> &ratings, const int userNum, const int itemNum, const string outputFolder,
                     const string MMCOutputPath, const string CSROutputPath, const string cumf_prefix, const bool addOneForID=true) {
        boost::filesystem::create_directories(outputFolder);

        // test
//        if(cumf_prefix.compare("R_test_")==0){
//            int *coo_row_idx = new int[itemNum];
//            int *coo_col_idx = new int[userNum];
//            float *coo_score_idx = new float[ratings.size()];
//
//            int i =0;
//            for(auto rating:ratings){
//                coo_row_idx[i] = rating.itemID-1;
//                coo_col_idx[i] = rating.userID-1;
//                coo_score_idx[i] = stod(rating.rating);
//                i++;
//            }
//
//            // COO
//            writeBinaryFile(outputFolder + cumf_prefix + "coo.row.bin", (char *) coo_row_idx, sizeof(int) * ratings.size());
//            writeBinaryFile(outputFolder + cumf_prefix + "coo.data.bin", (char *) coo_score_idx, sizeof(float) * ratings.size());
//            writeBinaryFile(outputFolder + cumf_prefix + "coo.col.bin", (char *) coo_col_idx, sizeof(int) * ratings.size());
//            delete[] coo_row_idx;
//            delete[] coo_col_idx;
//            delete[] coo_score_idx;
//            exit(123);
//        } else {
//
//            return;
//        }

        writeMMC(ratings, userNum, itemNum, outputFolder, MMCOutputPath, addOneForID);
        writeCSR(ratings, userNum, itemNum, outputFolder, CSROutputPath);

        writeCUMF_ALSFiles(ratings, userNum, itemNum, outputFolder, cumf_prefix);
    }
}
#endif //FILEUTIL_H
