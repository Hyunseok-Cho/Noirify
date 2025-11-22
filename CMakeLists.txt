cmake_minimum_required(VERSION 3.26)
project(Noirify LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

qt_add_executable(Noirify
        src/main.cpp
        src/MainWindow.cpp
        src/MainWindow.h
        processors/cpp/noirify_cpp.cpp
        processors/cpp/noirify_cpp.h
        resources/resources.qrc
)

target_link_libraries(Noirify PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets)


