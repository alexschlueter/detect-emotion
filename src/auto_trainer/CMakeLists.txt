project(autotrainer2)
find_package(OpenCV REQUIRED )
find_package(Qt5 COMPONENTS Core REQUIRED)
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
cmake_policy(SET CMP0020 NEW)
file(GLOB autotrainer_src
    "*.cpp"
    "*.h"
)

include_directories ("${CMAKE_SOURCE_DIR}/header")
add_executable(autotrainer2 ${autotrainer_src})
target_include_directories(autotrainer2 PUBLIC ${QtCore_INCLUDES}  ${OpenCV_INCLUDE_DIRS} )
set(LIBS Qt5::Core ${OpenCV_LIBS})
target_link_libraries(autotrainer2 ${LIBS} EmoLib)
