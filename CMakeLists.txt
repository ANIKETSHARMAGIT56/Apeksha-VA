cmake_minimum_required(VERSION 3.31)

project(Apeksha-VA)
# target_include_directories(Apeksha-VA PUBLIC CROW_INCLUDE_DIR)


# add_subdirectory(lib/raylib)
# add_subdirectory(lib/Crow)
# add_subdirectory(lib/IXWebSocket)
# # add_subdirectory(lib/clay)
add_subdirectory(libapeksha/IXWebSocket)
add_subdirectory(apeksha-server)
add_subdirectory(apeksha-gui)
add_subdirectory(apeksha-hotword)
add_subdirectory(libapeksha-test)