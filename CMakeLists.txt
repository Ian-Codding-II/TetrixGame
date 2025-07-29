#Specify the minimum CMake version required
cmake_minimum_required(VERSION 3.16)

#Define the project name and language
project(TetrixGame LANGUAGES CXX)

#Set C++ standard to 17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#Enable Qt's automatic MOC, RCC, and UIC processing
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

#Set Qt6 path (adjust if installed elsewhere, e.g., /opt/Qt/6.4.2/gcc_64)
#This path is typical for Ubuntu's Qt6 installation
list(APPEND CMAKE_PREFIX_PATH "/usr/lib/x86_64-linux-gnu/cmake/Qt6")

#Find Qt6 Widgets and Multimedia modules
find_package(Qt6 COMPONENTS Widgets Multimedia REQUIRED)

#Define source files
set(SOURCES
src/main.cpp
src/TetrixWindow.cpp
src/TetrixBoard.cpp
src/TetrixPiece.cpp
)

#Define header files
set(HEADERS
src/TetrixWindow.h
src/TetrixBoard.h
src/TetrixPiece.h
)

#Create the executable
add_executable(TetrixGame ${SOURCES} ${HEADERS})

#Specify include directories
target_include_directories(TetrixGame PRIVATE ${CMAKE_SOURCE_DIR}/src)

#Link Qt6 Widgets and Multimedia libraries
target_link_libraries(TetrixGame PRIVATE Qt6::Widgets Qt6::Multimedia)