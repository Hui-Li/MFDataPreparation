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

    void writeMMC_GZ(vector<Rating> &ratings, const int userNum, const int itemNum, const string outputFolder,
                  const string MMCOutputPath, const bool addOneForID=true) {

        sort(ratings.begin(), ratings.end());

        string path = outputFolder + MMCOutputPath + ".gz";
        gzFile fi = gzopen(path.c_str(), "wb9");

        // write header
        gzprintf(fi, "%%MatrixMarket matrix coordinate real general\n");
        gzprintf(fi, "%=================================================================================\n");
        gzprintf(fi, "%\n");
        gzprintf(fi, "% This ASCII file represents a sparse MxN matrix with L\n");
        gzprintf(fi, "% nonzeros in the following Matrix Market format:\n");
        gzprintf(fi, "%\n");
        gzprintf(fi, "% +----------------------------------------------+\n");
        gzprintf(fi, "% |%%MatrixMarket matrix coordinate real general | <--- header line\n");
        gzprintf(fi, "% |%                                             | <--+\n");
        gzprintf(fi, "% |% comments                                    |    |-- 0 or more comment lines\n");
        gzprintf(fi, "% |%                                             | <--+\n");
        gzprintf(fi, "% |    M  N  L                                   | <--- rows, columns, entries\n");
        gzprintf(fi, "% |    I1  J1  A(I1, J1)                         | <--+\n");
        gzprintf(fi, "% |    I2  J2  A(I2, J2)                         |    |\n");
        gzprintf(fi, "% |    I3  J3  A(I3, J3)                         |    |-- L lines\n");
        gzprintf(fi, "% |        . . .                                 |    |\n");
        gzprintf(fi, "% |    IL JL  A(IL, JL)                          | <--+\n");
        gzprintf(fi, "% +----------------------------------------------+   \n");
        gzprintf(fi, "%\n");
        gzprintf(fi, "% Indices are 1-based, i.e. A(1,1) is the first element.\n");
        gzprintf(fi, "%\n");
        gzprintf(fi, "%=================================================================================\n");

        // write dimension line
        gzprintf(fi, "%d %d %d\n", userNum, itemNum, ratings.size());

        // write matrix
        for (auto rating:ratings) {
            if(addOneForID){
                gzprintf(fi, "%d %d %.1f\n", rating.userID + 1, rating.itemID + 1, rating.rating);
            } else {
                gzprintf(fi, "%d %d %.1f\n", rating.userID, rating.itemID, rating.rating);
            }
        }

        // done
        gzclose(fi);

    }

    void writeCSR_GZ(vector<Rating> &ratings, const int userNum, const int itemNum, const string outputFolder,
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
            csr_score_double[ratingIndex] = rating.rating;
            csr_col_idx[ratingIndex] = rating.itemID;

            while(currentUserID!=rating.userID){
                csr_row_ptr[currentUserID + 1] = csr_row_ptr[currentUserID] + ratingNumForCurrentRow;
                currentUserID++;
                ratingNumForCurrentRow = 0;
            }

            ratingNumForCurrentRow++;
        }

        csr_row_ptr[userNum] = csr_row_ptr[userNum-1] + ratingNumForCurrentRow;

        string path = outputFolder + CSROutputPath + ".gz";
        gzFile fi = gzopen(path.c_str(), "wb9");

        const int size_of_double = sizeof(double);
        const int size_of_int = sizeof(int);

        gzwrite(fi, csr_score_double, size_of_double * ratings.size());
        gzwrite(fi, csr_row_ptr, size_of_int * (userNum + 1));
        gzwrite(fi, csr_col_idx, size_of_int * ratings.size());

        gzclose(fi);

        delete[] csr_score_double;
        delete[] csr_row_ptr;
        delete[] csr_col_idx;

    }

}
#endif //FILEUTIL_H
