cmake_minimum_required(VERSION 3.8)

project(anomaly-detection-monitoring-software)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})

set(PROJECT_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/includes)
set(IMAGES_DIR ${PROJECT_SOURCE_DIR}/images)
set(LIBTINS_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/includes/libs/libtins/include)
set(PCAP_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/includes/libs/WpdPack/Include)
set(PCAP_LIB_DIR ${PROJECT_SOURCE_DIR}/includes/libs/WpdPack/Lib)
set(LIBTINS_LIB_DIR ${PROJECT_SOURCE_DIR}/includes/libs/libtins/lib)
 
include_directories("${PROJECT_INCLUDE_DIR}"
					"${LIBTINS_INCLUDE_DIR}"
					"${PCAP_INCLUDE_DIR}"
					)
					
link_directories("${PCAP_LIB_DIR}"
				 "${LIBTINS_LIB_DIR}"
				 )

#Find Eigen3 library
find_package(Eigen3 CONFIG REQUIRED)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed
set(CMAKE_AUTOMOC ON)
# Create code from a list of Qt designer ui files
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find the Qt libraries
find_package(Qt5Core CONFIG REQUIRED)
find_package(Qt5Network CONFIG REQUIRED)
find_package(Qt5Widgets CONFIG REQUIRED)
find_package(Qt5Gui CONFIG REQUIRED)

add_subdirectory(includes)
add_subdirectory(server)
add_subdirectory(client)
