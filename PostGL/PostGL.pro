TEMPLATE = lib
CONFIG += debug qt qwidgets opengl staticlib warn_off
DESTDIR = ../build/lib
TARGET = postgl
INCLUDEPATH += ../
QMAKE_CXXFLAGS += -DLINUX -DTETLIBRARY -DNDEBUG


# Input
HEADERS += *.h
SOURCES += *.cpp
