project(keytest)
find_package( OpenCV REQUIRED )
add_executable(keytest keytest.cpp)
target_include_directories(keytest PUBLIC ${OpenCV_INCLUDE_DIRS})
target_link_libraries(keytest ${OpenCV_LIBS})

project(playone)
find_package( OpenCV REQUIRED )
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(SOURCE_FILES playone.cpp controls.cpp)
include_directories ("${CMAKE_SOURCE_DIR}/header")
add_executable(playone ${SOURCE_FILES})
target_include_directories(playone PUBLIC ${OpenCV_INCLUDE_DIRS})
target_link_libraries(playone ${OpenCV_LIBS})


project(playall)
find_package( OpenCV REQUIRED )
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(SOURCE_FILES playall.cpp controls.cpp)
include_directories ("${CMAKE_SOURCE_DIR}/header")
add_executable(playall ${SOURCE_FILES})
target_include_directories(playall PUBLIC ${OpenCV_INCLUDE_DIRS})
target_link_libraries(playall ${OpenCV_LIBS})

add_subdirectory(qviewer)