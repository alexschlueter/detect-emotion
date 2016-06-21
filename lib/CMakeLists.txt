project(EmotionLib)


find_package( OpenCV REQUIRED )
set(CMAKE_INCLUDE_CURRENT_DIR ON)
include_directories ("${CMAKE_SOURCE_DIR}/header")

file(GLOB lib_src
    "*.cpp"
    )

add_library(EmoLib STATIC ${lib_src})

target_link_libraries(EmoLib ${OpenCV_LIBS})
