cmake_minimum_required(VERSION 2.8)
project(DataHandler)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -std=c++11 -DNDEBUG -pthread -march=native -ftree-vectorize")

find_package(ZLIB REQUIRED)
if(ZLIB_FOUND)
    include_directories(${ZLIB_INCLUDE_DIRS})
    link_libraries(${ZLIB_LIBRARIES})
endif(ZLIB_FOUND)

find_package(Boost COMPONENTS system iostreams)

if(NOT ${Boost_FOUND})
    SET(BOOST_ROOT ~/local) # default
    SET(Boost_NO_SYSTEM_PATHS ON) # force to use own build
    find_package(Boost COMPONENTS system iostreams REQUIRED)
endif(NOT ${Boost_FOUND})

if(Boost_FOUND)
    message(STATUS "Boost_LIBRARIES: ${Boost_LIBRARIES}")
    include_directories(${Boost_INCLUDE_DIR})
    link_libraries(${Boost_LIBRARIES})
endif(Boost_FOUND)

set(util
    util/Base.h
    util/FileUtil.h
    util/ZLibUtil.h
    util/ThreadUtil.h)

set(struct
    struct/Rating.h
    struct/User.h
    struct/Item.h)


add_library(lib SHARED ${util} ${struct})
set_target_properties(lib PROPERTIES LINKER_LANGUAGE CXX)

set(SOURCE_FILES hugewiki_handler.cpp)
add_executable(hugewiki_handler ${SOURCE_FILES})
target_link_libraries(hugewiki_handler lib)
target_link_libraries(hugewiki_handler ${ZLIB_LIBRARIES})
target_link_libraries(hugewiki_handler ${Boost_LIBRARIES})