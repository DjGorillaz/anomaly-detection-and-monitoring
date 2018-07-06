QT += core
QT += network
QT += widgets
QT += gui

QMAKE_CXXFLAGS += /std:c++17

TARGET = client

TEMPLATE = app

LIBS += -luser32    \ #User32.lib \
        -lgdi32     \#Gdi32.lib \
        -lgdiplus   \#Gdiplus.lib \
        ../includes/libs/libtins/lib/tins.lib \
        ../includes/libs/pcap/Lib/x64/wpcap.lib \
        Ws2_32.lib \
        Iphlpapi.lib

INCLUDEPATH += ../includes/ \
               ../includes/libs/libtins/include \
               ../includes/libs/pcap/Include \

SOURCES += \
        main.cpp \
        client.cpp \
        ../includes/config.cpp \
        ../includes/fileclient.cpp \
        ../includes/fileserver.cpp \
        ../includes/klog.cpp \
        ../includes/sniffer.cpp \
        ../includes/mousehook.cpp

HEADERS += \
        client.h \
        ../includes/config.h \
        ../includes/fileclient.h \
        ../includes/fileserver.h \
        ../includes/klog.h \
        ../includes/enums.h \
        ../includes/sniffer.h \
    ../includes/mousehook.h

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
