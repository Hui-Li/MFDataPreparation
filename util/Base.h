#ifndef BASE_H
#define BASE_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <map>
#include <mutex>
#include <thread>

using std::cout;
using std::endl;
using std::cerr;

using std::make_pair;
using std::pair;
using std::mutex;
using std::string;
using std::vector;
using std::map;
using std::ifstream;
using std::ofstream;
using std::getline;
using std::stringstream;
using std::unordered_map;
using std::unordered_set;

////////////////// Boost //////////////////////

#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>
#include <boost/format.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/matrix_sparse.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <boost/numeric/ublas/matrix_proxy.hpp>
#include <boost/math/constants/constants.hpp>

#include <boost/filesystem.hpp>

namespace po = boost::program_options;

////////////////// Boost //////////////////////

namespace mf {
// common matrix types
    typedef boost::numeric::ublas::coordinate_matrix<double, boost::numeric::ublas::row_major>
            SparseMatrix;
    typedef boost::numeric::ublas::coordinate_matrix<double, boost::numeric::ublas::column_major>
            SparseMatrixCM;
    typedef boost::numeric::ublas::matrix<double, boost::numeric::ublas::row_major>
            DenseMatrix;
    typedef boost::numeric::ublas::matrix<double, boost::numeric::ublas::column_major>
            DenseMatrixCM;

    typedef SparseMatrix::size_type mf_size_type;
}

#endif //BASE_H
