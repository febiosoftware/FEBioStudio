TEMPLATE = lib
TARGET = meshtools
DESTDIR = ../build/lib
CONFIG += release c++14 staticlib warn_off 
DEFINES += LINUX HAS_NETGEN HAS_OCC
INCLUDEPATH += ../
INCLUDEPATH += /opt/netgen/include
INCLUDEPATH += /opt/netgen/include/include
INCLUDEPATH += /home/sci/mherron/Resources/opencascade-7.3.0/include/opencascade

HEADERS += *.h
SOURCES += *.cpp
