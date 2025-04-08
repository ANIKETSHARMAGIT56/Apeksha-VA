cmake_minimum_required(VERSION 3.11) # FetchContent is available in 3.11+
project(apeksha-gui)
# Dependencies
set(CMAKE_CXX_STANDARD 20)

# Make sure the compiler supports the specified standard
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(RAYLIB_VERSION 5.5)
find_package(raylib ${RAYLIB_VERSION} QUIET) # QUIET or REQUIRED
if (NOT raylib_FOUND) # If there's none, fetch and build raylib
include(FetchContent)
FetchContent_Declare(
  raylib
  DOWNLOAD_EXTRACT_TIMESTAMP OFF
  URL https://github.com/raysan5/raylib/archive/refs/tags/${RAYLIB_VERSION}.tar.gz
  )
  FetchContent_GetProperties(raylib)
  if (NOT raylib_POPULATED) # Have we downloaded raylib yet?
  set(FETCHCONTENT_QUIET NO)
  FetchContent_MakeAvailable(raylib)
  set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # don't build the supplied examples
  endif()
  endif()
  
  
  
  add_subdirectory(traypp)
  add_executable(${PROJECT_NAME} main.cpp)
  
  target_link_libraries(${PROJECT_NAME} raylib m sioclient ixwebsocket tray)
  
  install(TARGETS ${PROJECT_NAME} DESTINATION bin)

if (APPLE)
    target_link_libraries(${PROJECT_NAME} "-framework IOKit")
    target_link_libraries(${PROJECT_NAME} "-framework Cocoa")
    target_link_libraries(${PROJECT_NAME} "-framework OpenGL")
endif()