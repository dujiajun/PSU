cmake_minimum_required(VERSION 3.19)
project(common)

set(CMAKE_CXX_STANDARD 20)

if (MSVC)
    set(OS "win")
    set(Boost_INCLUDE_DIR libOTe/cryptoTools/thirdparty/${OS}/include/boost-1_76)
else ()
    set(OS "unix")
    set(Boost_INCLUDE_DIR libOTe/cryptoTools/thirdparty/${OS}/include/)
endif ()

include_directories(libOTe/out/install/${CMAKE_BUILD_TYPE}/include)
include_directories(libOTe/cryptoTools/thirdparty/${OS}/include/)
include_directories(${Boost_INCLUDE_DIR})

link_directories(libOTe/out/install/${CMAKE_BUILD_TYPE}/lib)
link_directories(libOTe/cryptoTools/thirdparty/${OS}/lib/)

find_library(cryptoTools cryptoTools)
find_library(libOTe libOTe)
find_library(relic relic_s)

find_package(Boost)

include_directories(osn)
include_directories(mp_oprf)
include_directories(common)

add_library(osn STATIC
        osn/benes.cpp
        osn/benes.h
        osn/OSNReceiver.cpp
        osn/OSNReceiver.h
        osn/OSNSender.cpp
        osn/OSNSender.h)
add_library(mp_oprf STATIC
        mp_oprf/MPOPRFReceiver.cpp
        mp_oprf/MPOPRFReceiver.h
        mp_oprf/MPOPRFSender.cpp
        mp_oprf/MPOPRFSender.h
        )
add_library(common STATIC
        common/PSUSender.h
        common/PSUReceiver.h
        common/utils.cpp
        common/utils.h
        common/MP-OPRF-Parameters.h
        common/SimpleIndexParameters.h
        common/CuckooIndex.cpp
        common/CuckooIndex.h
        common/SimpleIndex.cpp
        common/SimpleIndex.h
        common/sha1.cpp
        common/sha1.h)

add_executable(ShuffleReceiver_Cuckoo
        shuffle_receiver_cuckoo/SRCReceiver.cpp
        shuffle_receiver_cuckoo/SRCReceiver.h
        shuffle_receiver_cuckoo/SRCSender.cpp
        shuffle_receiver_cuckoo/SRCSender.h
        shuffle_receiver_cuckoo/ShuffleReceiver_Cuckoo.cpp)

add_executable(ShuffleReceiver_Simple
        shuffle_receiver_simple/SRSReceiver.cpp
        shuffle_receiver_simple/SRSReceiver.h
        shuffle_receiver_simple/SRSSender.cpp
        shuffle_receiver_simple/SRSSender.h
        shuffle_receiver_simple/ShuffleReceiver_Simple.cpp)

add_executable(ShuffleReceiver
        "shuffle_receiver/SRReceiver.cpp"
        "shuffle_receiver/SRReceiver.h"
        "shuffle_receiver/SRSender.cpp"
        "shuffle_receiver/SRSender.h"
        shuffle_receiver/ShuffleReceiver.cpp)

add_executable(ShuffleSender
        shuffle_sender/SSSender.cpp
        shuffle_sender/SSSender.h
        shuffle_sender/SSReceiver.cpp
        shuffle_sender/SSReceiver.h
        shuffle_sender/ShuffleSender.cpp)

add_executable(ShuffleSender_Cuckoo
        shuffle_sender_cuckoo/SSCSender.cpp
        shuffle_sender_cuckoo/SSCSender.h
        shuffle_sender_cuckoo/SSCReceiver.cpp
        shuffle_sender_cuckoo/SSCReceiver.h
        shuffle_sender_cuckoo/ShuffleSender_Cuckoo.cpp)

add_executable(test_osn
        test/test_osn.cpp)

target_link_libraries(ShuffleReceiver common osn mp_oprf libOTe cryptoTools relic_s)
target_link_libraries(ShuffleReceiver_Cuckoo common osn mp_oprf libOTe cryptoTools relic_s)
target_link_libraries(ShuffleReceiver_Simple common osn mp_oprf libOTe cryptoTools relic_s)
target_link_libraries(ShuffleSender common osn mp_oprf libOTe cryptoTools relic_s)
target_link_libraries(ShuffleSender_Cuckoo common osn mp_oprf libOTe cryptoTools relic_s)
target_link_libraries(test_osn osn libOTe cryptoTools relic_s)