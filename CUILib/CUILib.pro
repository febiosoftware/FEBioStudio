TEMPLATE = lib
TARGET = cuilib
DESTDIR = ../build/lib
CONFIG += debug c++14 staticlib warn_off 
QMAKE_CXX = g++
QMAKE_CXXFLAGS += -std=c++14 -O0
DEFINES += LINUX HAS_QUAZIP HAS_SSH HAS_OCC TETLIBRARY HAS_NETGEN FFMPEG
INCLUDEPATH += ../
QT += widgets

# Input
HEADERS += *.h
SOURCES = $$files(*.cpp)
SOURCES -= $$files(moc_*.cpp)
