TEMPLATE = lib
DESTDIR = ../build/lib
TARGET = postgl
CONFIG += debug qt qwidgets opengl staticlib warn_off 
INCLUDEPATH += ../
QMAKE_CXXFLAGS += -DLINUX -DTETLIBRARY -DNDEBUG


# Input
HEADERS += *.h
SOURCES += *.cpp
