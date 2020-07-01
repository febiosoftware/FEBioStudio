TEMPLATE = lib
TARGET = fscore
DESTDIR = ../build/lib
CONFIG += debug qt qwidgets opengl staticlib warn_off c++14
QMAKE_CXX = g++
QMAKE_CXXFLAGS += -std=c++14 -O0
DEFINES += LINUX HAS_QUAZIP HAS_SSH #HAS_OCC TETLIBRARY HAS_NETGEN FFMPEG
INCLUDEPATH += ../

#QT += widgets opengl


# Input
HEADERS += *.h
SOURCES = $$files(*.cpp)
SOURCES -= $$files(moc_*.cpp)
