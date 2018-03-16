# MFDataTransfer

Tools which transforms public recommender system data to MMC file (i.e., <user_id, item_id, rating> triple file) and [CSR file](https://en.wikipedia.org/wiki/Sparse_matrix#Compressed_sparse_row_.28CSR.2C_CRS_or_Yale_format.29) used in several matrix factorization libraries. For MMC files, id starts with 1. For CSR file, id starts with 0. In addition, DataHandler will generate data under folder "cumf_als" for [cumf_als](https://github.com/cuMF/cumf_als) library (except for hugewiki).

## Environment

- Ubuntu 16.04
- CMake 2.8
- GCC 5.4
- Boost 1.63 
- Zlib 1.2.11

## Public Datasets for Recemmender Systems
Some of the links may become invalid due to privacy issue or the shutdown of the website, but it is easy to find other download links in internet. 
 
1. MovieLens 10M: http://grouplens.org/datasets/movielens/
2. MovieLens 20M: http://grouplens.org/datasets/movielens/
2. Netflix: http://www.select.cs.cmu.edu/code/graphlab/datasets/
3. Yahoo! Music: https://webscope.sandbox.yahoo.com/catalog.php?datatype=r
4. HugeWiki: http://www.select.cs.cmu.edu/code/graphlab/datasets/