TEMPLATE = lib
TARGET = postlib
DESTDIR = ../build/lib
CONFIG += debug c++11 staticlib warn_off
QMAKE_CXX = g++
QMAKE_CXXFLAGS += -DLINUX -DTETLIBRARY -DNDEBUG
QMAKE_CXXFLAGS += -MMD
INCLUDEPATH += ../

# Input
HEADERS += *.h

SOURCES += *.cpp
