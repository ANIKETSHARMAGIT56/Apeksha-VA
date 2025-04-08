project(libapeksha-test)

add_executable(libapeksha-test main.cpp ../libapeksha/libapeksha.cpp)
target_link_libraries(libapeksha-test PUBLIC ixwebsocket)