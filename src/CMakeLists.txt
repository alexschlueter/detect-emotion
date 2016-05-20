project(playone)
find_package( OpenCV REQUIRED )
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(SOURCE_FILES playone.cpp)
include_directories ("${CMAKE_SOURCE_DIR}/header")
add_executable(playone ${SOURCE_FILES})
target_link_libraries(playone ${OpenCV_LIBS})


project(playall)
find_package( OpenCV REQUIRED )
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(SOURCE_FILES playall.cpp)
include_directories ("${CMAKE_SOURCE_DIR}/header")
add_executable(playall ${SOURCE_FILES})
target_link_libraries(playall ${OpenCV_LIBS})