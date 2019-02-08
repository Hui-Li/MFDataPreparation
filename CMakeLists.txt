cmake_minimum_required(VERSION 2.8)
project(MFDataPreparation)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -std=c++11 -DNDEBUG -pthread -march=native -ftree-vectorize")

find_package(Boost COMPONENTS serialization program_options system filesystem iostreams)

if(NOT ${Boost_FOUND})
    SET(BOOST_ROOT ~/local) # default
    SET(Boost_NO_SYSTEM_PATHS ON) # force to use own build
    find_package(Boost COMPONENTS serialization program_options system filesystem iostreams REQUIRED)
endif(NOT ${Boost_FOUND})

if(Boost_FOUND)
    message(STATUS "Boost found")
    include_directories(${Boost_INCLUDE_DIR})
    link_libraries(${Boost_LIBRARIES})
endif(Boost_FOUND)

set(util
    util/Base.h
    util/FileUtil.h
    util/ThreadUtil.h)

set(struct
    struct/Rating.h
    struct/User.h
    struct/Item.h)

add_library(lib SHARED ${util} ${struct})
set_target_properties(lib PROPERTIES LINKER_LANGUAGE CXX)

link_libraries(lib ${Boost_LIBRARIES})

add_executable(lastFM_handler lastFM_handler.cpp)
add_executable(ml100k_handler ml100k_handler.cpp)
add_executable(ml10m_handler ml10m_handler.cpp)
add_executable(ml20m_handler ml20m_handler.cpp)
add_executable(netflix_handler netflix_handler.cpp)
add_executable(yahoo_handler yahoo_handler.cpp)
add_executable(yelp_handler yelp_handler.cpp)
