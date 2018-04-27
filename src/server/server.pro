QT += core gui
QT += network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = server
TEMPLATE = app

CONFIG += c++14

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += ../includes/ \
               ../includes/libs/eigen \

SOURCES += main.cpp \
        server.cpp  \
        ../includes/config.cpp \
        ../includes/fileclient.cpp \
        ../includes/fileserver.cpp \
        ../includes/user.cpp \
        filedialog.cpp \

HEADERS  += server.h    \
        ../includes/config.h \
        ../includes/fileclient.h \
        ../includes/fileserver.h \
        ../includes/user.h \
        filedialog.h \

FORMS    += server.ui \
        filedialog.ui

RESOURCES += \
        ../images/images.qrc
