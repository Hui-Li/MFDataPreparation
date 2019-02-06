# MFDataPreparation

Tools which transforms public recommender system data to MMC file (i.e., <user_id, item_id, rating> triple file) and [CSR file](https://en.wikipedia.org/wiki/Sparse_matrix#Compressed_sparse_row_.28CSR.2C_CRS_or_Yale_format.29) used in several matrix factorization libraries. For MMC files, id starts with 1. For CSR file, id starts with 0. In addition, DataHandler will generate data under folder "cumf_als" for [cumf_als](https://github.com/cuMF/cumf_als) library (except for hugewiki). 

In order to make ids consecutive and start with 1/0 in MMC/CSR file, we have remapped user/item ids except for Yahoo! Music where original ids are consecutive integers (no holes) starting from 0. The mapped user/item ids will be outputed to two separate files with each line containing <original id, new id>. To find the mapped id for CSR file, add 1 to the id in MMC file and then search for the mapped pair. For id in MMC file, just use the new id in MMC file to search. For instance, search `?,1` in user id mapping file to find the original user id for user id `0` in the user CSR file, and search `?,1` in user id mapping file to find the original user id for user id `1` in the user MMC file.

## Environment

- Ubuntu 16.04
- CMake 2.8
- GCC 5.4
- Boost 1.63 
- Zlib 1.2.11  (only for HugeWiki)

To install CMake, Boost and Zlib, try:

```
sudo apt install cmake
sudo apt install libboost-all-dev
sudo apt install zlib1g-dev
```

## How to run
Type:
```
mkdir build
cd build
cmake ../
make
```
and then you can find the executables in the build folder. For parameters, type:
```
program --help
```

## Public Data Sets for Recemmender Systems
Some of the links may become invalid due to privacy issue or the shutdown of the website, but it is easy to find other download links in internet. 

1. MovieLens 100K: [http://grouplens.org/datasets/movielens/](http://grouplens.org/datasets/movielens/) 
2. MovieLens 10M: [http://grouplens.org/datasets/movielens/](http://grouplens.org/datasets/movielens/) 
3. MovieLens 20M: [http://grouplens.org/datasets/movielens/](http://grouplens.org/datasets/movielens/) 
4. Netflix: [http://www.select.cs.cmu.edu/code/graphlab/datasets/](http://www.select.cs.cmu.edu/code/graphlab/datasets/) 
5. Yahoo! Music: [https://webscope.sandbox.yahoo.com/catalog.php?datatype=r](https://webscope.sandbox.yahoo.com/catalog.php?datatype=r)
6. HugeWiki: [http://www.select.cs.cmu.edu/code/graphlab/datasets/](http://www.select.cs.cmu.edu/code/graphlab/datasets/)
7. Last.FM: [https://grouplens.org/datasets/hetrec-2011/](https://grouplens.org/datasets/hetrec-2011/)
8. Yelp: [https://www.kaggle.com/c/yelp-recsys-2013](https://www.kaggle.com/c/yelp-recsys-2013) or [https://www.yelp.com/dataset/challenge](https://www.yelp.com/dataset/challenge)
