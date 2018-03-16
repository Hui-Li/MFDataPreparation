cmake_minimum_required(VERSION 2.8)
project(DataHandler)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -std=c++11 -DNDEBUG -march=native -ftree-vectorize")

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
    util/FileUtil.h)

set(struct
    struct/Rating.h
    struct/User.h
    struct/Item.h)

add_library(lib SHARED ${util} ${struct})
set_target_properties(lib PROPERTIES LINKER_LANGUAGE CXX)

set(SOURCE_FILES ml10m_handler.cpp)
add_executable(ml10m_handler ${SOURCE_FILES})
target_link_libraries(ml10m_handler lib)
target_link_libraries(ml10m_handler ${Boost_LIBRARIES})
target_link_libraries(ml10m_handler ${ZLIB_LIBRARIES} )

set(SOURCE_FILES ml20m_handler.cpp)
add_executable(ml20m_handler ${SOURCE_FILES})
target_link_libraries(ml20m_handler lib)
target_link_libraries(ml20m_handler ${Boost_LIBRARIES})
target_link_libraries(ml20m_handler ${ZLIB_LIBRARIES} )

set(SOURCE_FILES netflix_handler.cpp)
add_executable(netflix_handler ${SOURCE_FILES})
target_link_libraries(netflix_handler lib)
target_link_libraries(netflix_handler ${Boost_LIBRARIES})
target_link_libraries(netflix_handler ${ZLIB_LIBRARIES} )

set(SOURCE_FILES yahoo_handler.cpp)
add_executable(yahoo_handler ${SOURCE_FILES})
target_link_libraries(yahoo_handler lib)
target_link_libraries(yahoo_handler ${Boost_LIBRARIES})
target_link_libraries(yahoo_handler ${ZLIB_LIBRARIES} )