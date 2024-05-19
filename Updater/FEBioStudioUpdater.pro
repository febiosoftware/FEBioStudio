DESTDIR = .
TEMPLATE = app
TARGET = FEBioStudioUpdater
DESTDIR = ./build/bin
CONFIG += debug c++14
CONFIG += qt opengl qtwidgets qtcharts warn_off
DEFINES += LINUX
INCLUDEPATH += .
INCLUDEPATH += /home/mherron/Projects/FEBioStudio
QT += widgets opengl gui charts network webenginewidgets
QMAKE_CXX = g++
QMAKE_CXXFLAGS += -std=c++14 -O0
QMAKE_LFLAGS += -static-libstdc++ -static-libgcc
#QMAKE_LFLAGS_RPATH = \$$ORIGIN/../lib/
QMAKE_RPATHDIR += $ORIGIN/../lib
#QMAKE_LFLAGS += '-Wl,-rpath,\'\$$ORIGIN/../lib/\',-z,origin'

LIBS += -L/home/mherron/Projects/FEBioStudio/build/Release/lib
LIBS += -Wl,--start-group

LIBS += -lMathLib -lFSCore -lXML

LIBS += -Wl,--end-group



#RESOURCES = ../febiostudio.qrc

HEADERS += *.h
SOURCES = $$files(*.cpp)
SOURCES -= $$files(moc_*.cpp)
SOURCES -= $$files(qrc_*)


