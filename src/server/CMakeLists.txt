project(server)

# Populate a CMake variable with the sources
set(server_SRCS
    filedialog.cpp
    filedialog.ui
	server.cpp
	server.ui
    main.cpp
	user.cpp
	${IMAGES_DIR}/images.qrc
)

#Create the executable
add_executable(${PROJECT_NAME} WIN32 ${server_SRCS})

set_target_properties(server PROPERTIES
	CXX_STANDARD 17
	CXX_STANDARTD_REQUIRED ON
)

# Use libraries
target_link_libraries(server
									Qt5::Core
									Qt5::Gui
									Qt5::Widgets
									Qt5::Network
									Eigen3::Eigen
									filelib
									configlib
)