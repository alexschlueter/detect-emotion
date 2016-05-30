project(qviewer)
find_package(OpenCV REQUIRED )
find_package(Qt5 COMPONENTS Gui Quick Qml Widgets)
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
file(GLOB qviewer_src
    "*.cpp"
    "*.h"
)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}")
include_directories ("${CMAKE_SOURCE_DIR}/header")
qt5_add_resources(UI_RESOURCES qml.qrc)
add_executable(qviewer ${qviewer_src} ${UI_RESOURCES})
target_include_directories(qviewer PUBLIC ${QtCore_INCLUDES} ${QtQml_INCLUDES} ${QtGui_INCLUDES} ${QtWidgets_INCLUDES}  ${OpenCV_INCLUDE_DIRS} )
target_link_libraries(qviewer ${OpenCV_LIBS} Qt5::Core Qt5::Quick Qt5::Qml Qt5::Gui Qt5::Widgets)
