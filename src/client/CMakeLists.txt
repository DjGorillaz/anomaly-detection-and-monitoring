project(client)

# Populate a CMake variable with the sources
set(client_SRCS
    client.cpp
	klog.cpp
    main.cpp
	mousehook.cpp
	sniffer.cpp
	)

#Create the executable
add_executable(${PROJECT_NAME} WIN32 ${client_SRCS})
#WIN32
set_target_properties(client PROPERTIES
	CXX_STANDARD 17
	CXX_STANDARTD_REQUIRED ON
#	COMPILE_OPTIONS -Wpedantic -Wall -Wextra
)


#find_library(TINS_LIBRARY tins)
#find_library(WPCAP_LIBRARY wpcap)

# Use libraries
target_link_libraries(client
									Qt5::Core
									Qt5::Network
									Qt5::Widgets
#Qt5::Gui
									filelib
									configlib
									user32
									gdi32
									gdiplus
									#${TINS_LIBRARY}
									#${WPCAP_LIBRARY}
									tins
									wpcap
									Ws2_32
									Iphlpapi
)

#install(TARGETS client RUNTIME DESTINATION bin)