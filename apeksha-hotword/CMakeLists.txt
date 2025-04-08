cmake_minimum_required(VERSION 3.10)

# Set the project name
project(
    apeksha-hotword 
    DESCRIPTION "howord trigger for Apeksha-VA"
    LANGUAGES CXX)


add_executable(apeksha-hotword main.cpp)
target_include_directories(apeksha-hotword PUBLIC pv_porcupine/include/)
target_include_directories(apeksha-hotword PUBLIC IXWebSocket)

target_link_directories(apeksha-hotword PUBLIC pv_porcupine/lib/linux/x86_64/)
target_link_libraries(apeksha-hotword pv_porcupine ixwebsocket)
