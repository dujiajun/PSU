cmake_minimum_required(VERSION 3.19)
project(PSU)

set(CMAKE_CXX_STANDARD 20)

if (MSVC)
    set(OS "win")
    set(BOOST_INCLUDEDIR libOTe/cryptoTools/thirdparty/${OS}/include/boost-1_76)
    find_package(Boost)
else ()
    set(OS "unix")
    set(BOOST_INCLUDEDIR libOTe/cryptoTools/thirdparty/${OS}/include/)
    set(BOOST_COMPOENTS filesystem system thread atomic regex)
    add_compile_options(-maes -msse2 -msse3 -mssse3 -msse4.1 -mpclmul)
    find_package(Boost COMPONENTS REQUIRED ${BOOST_COMPOENTS})
endif ()

set(MY_LIBS libOTe cryptoTools relic_s)

include_directories(libOTe/out/install/${PRESET_NAME}/include)
include_directories(libOTe/cryptoTools/thirdparty/${OS}/include/)
include_directories(${BOOST_INCLUDEDIR})

link_directories(libOTe/out/install/${PRESET_NAME}/lib)
link_directories(libOTe/cryptoTools/thirdparty/${OS}/lib/)

include_directories(osn)
include_directories(mp_oprf)
include_directories(common)
include_directories(shuffle_receiver)
include_directories(shuffle_receiver_cuckoo)
# include_directories(shuffle_receiver_simple)
include_directories(shuffle_sender)
include_directories(shuffle_sender_cuckoo)

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
        common/cmdline.h
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
add_library(src STATIC 
        shuffle_receiver_cuckoo/SRCReceiver.cpp
        shuffle_receiver_cuckoo/SRCReceiver.h
        shuffle_receiver_cuckoo/SRCSender.cpp
        shuffle_receiver_cuckoo/SRCSender.h)
# add_library(srs STATIC 
#         shuffle_receiver_simple/SRSReceiver.cpp
#         shuffle_receiver_simple/SRSReceiver.h
#         shuffle_receiver_simple/SRSSender.cpp
#         shuffle_receiver_simple/SRSSender.h)
add_library(sr STATIC
        shuffle_receiver/SRReceiver.cpp
        shuffle_receiver/SRReceiver.h
        shuffle_receiver/SRSender.cpp
        shuffle_receiver/SRSender.h)
add_library(ss STATIC
        shuffle_sender/SSSender.cpp
        shuffle_sender/SSSender.h
        shuffle_sender/SSReceiver.cpp
        shuffle_sender/SSReceiver.h)
add_library(ssc STATIC
        shuffle_sender_cuckoo/SSCSender.cpp
        shuffle_sender_cuckoo/SSCSender.h
        shuffle_sender_cuckoo/SSCReceiver.cpp
        shuffle_sender_cuckoo/SSCReceiver.h)

add_executable(test_osn
        test/test_osn.cpp)

add_executable(test_psu
        test/test_psu.cpp)

target_link_libraries(test_psu ss ssc sr src common osn mp_oprf ${MY_LIBS} ${Boost_LIBRARIES})
target_link_libraries(test_osn osn ${MY_LIBS} ${Boost_LIBRARIES})