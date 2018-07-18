project(libraries)

# Add a library with the above sources
add_library(filelib 
						fileclient.cpp
						fileserver.cpp
						data.cpp
)

add_library(configlib 	config.cpp)
							
target_link_libraries(filelib
								Qt5::Core
								Qt5::Network
)

target_link_libraries(configlib Qt5::Core)