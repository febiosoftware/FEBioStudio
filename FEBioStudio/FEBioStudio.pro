DESTDIR = .
TEMPLATE = app
TARGET = FEBioStudio
DESTDIR = ../build/bin
CONFIG += debug c++11
CONFIG += qt opengl qtwidgets qtcharts warn_off
DEFINES += LINUX HAS_NETGEN HAS_OCC TETLIBRARY
INCLUDEPATH += ../
QT += widgets opengl gui charts
QMAKE_LFLAGS_RELEASE += -static-libstdc++ -static-libgcc
QMAKE_LFLAGS_RPATH = 
QMAKE_LFLAGS += '-Wl,-rpath,\'\$$ORIGIN/../lib/\',-z,origin'

LIBS += -L/home/sci/mherron/Resources/Qt/5.12.0/gcc_64/lib
LIBS += -L/home/sci/mherron/Resources/tetgen1.5.0/lib -ltet64
LIBS += -L/opt/netgen/lib 
LIBS += -L/usr/local/lib 
LIBS += -L../build/lib
LIBS += -Wl,--start-group

#Netgen Libs
LIBS += -lnglib -lcsg -linterface -lmesh -locc

#OpenCascade Libs
LIBS += -lTKernel -lTKGeomBase -lTKTopAlgo -lTKPrim -lTKMesh -lTKMath -lTKBRep -lTKSTL -lTKFillet -lTKBO -lTKIGES -lTKSTEP -lTKSTEPBase -lTKXSBase -lTKG3d -lTKLCAF -lTKVCAF

#PreView Libs
LIBS += -labaqus -lansys -lgeomlib -lmathlib -lxml -lmeshio -lfebio -lcomsol -llsdyna -lmeshtools -lfemlib -lmeshlib -limagelib -lfscore

#PostView Libs
LIBS += -lglwlib -lpostgl -lxpltlib -lpostlib

LIBS += -Wl,--end-group

LIBS += -L/usr/lib64 -lGLU -lGL -lz

RESOURCES = ../febiostudio.qrc

SOURCES += *.cpp
HEADERS += *.h
