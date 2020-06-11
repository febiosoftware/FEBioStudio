TEMPLATE = lib
TARGET = meshtools
DESTDIR = ../build/lib
CONFIG += debug c++17 staticlib warn_off
QMAKE_CXX = g++
QMAKE_CXXFLAGS += -std=c++17 -O0
DEFINES += LINUX HAS_QUAZIP HAS_SSH HAS_OCC TETLIBRARY HAS_NETGEN FFMPEG HAS_MMG
INCLUDEPATH += ../
INCLUDEPATH += /opt/netgen/include
INCLUDEPATH += /opt/netgen/include/include
INCLUDEPATH += /home/mherron/Resources/opencascade-7.4.0/build/include/opencascade
INCLUDEPATH += /home/mherron/Resources/tetgen1.5.1/
INCLUDEPATH += /home/mherron/Resources/mmg/build/include

HEADERS += *.h
SOURCES = $$files(*.cpp)
SOURCES -= $$files(moc_*.cpp)
