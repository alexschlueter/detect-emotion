project(playone)
find_package( OpenCV REQUIRED )
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(SOURCE_FILES playone.cpp)
add_executable(playone ${SOURCE_FILES})
target_link_libraries(playone ${OpenCV_LIBS})


project(playall)
find_package( OpenCV REQUIRED )
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(SOURCE_FILES playall.cpp)
add_executable(playall ${SOURCE_FILES})
target_link_libraries(playall ${OpenCV_LIBS})