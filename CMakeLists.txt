cmake_minimum_required(VERSION 3.16)
project(PlaceYF)

set(CMAKE_CXX_STANDARD 17)

include_directories(proto)
find_package(absl REQUIRED)

add_library(grpc_proto
        proto/PlaceYF.grpc.pb.h
        proto/PlaceYF.grpc.pb.cc
        proto/PlaceYF.pb.h
        proto/PlaceYF.pb.cc
        )
target_link_libraries(grpc_proto
        grpc++_reflection
        grpc++
        libprotobuf
        absl::synchronization
        )

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lpthread")

add_library(HouseMemory SHARED HouseMemory.cpp Utils.cpp Utils.h)
target_link_libraries(HouseMemory grpc_proto)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mwindows")

add_executable(PlaceYF main.cpp PlaceYF.cpp main.rc)
target_link_libraries(PlaceYF grpc_proto nana png jpeg)
