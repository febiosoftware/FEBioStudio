DESTDIR = .
TEMPLATE = app
TARGET = FEBioStudio
DESTDIR = ../build/bin
CONFIG += debug c++14
CONFIG += qt opengl qtwidgets qtcharts warn_off
DEFINES += LINUX HAS_OCC TETLIBRARY HAS_NETGEN HAS_QUAZIP HAS_SSH HAS_MMG FFMPEG MODEL_REPO
INCLUDEPATH += ../
INCLUDEPATH += /usr/local/include/
INCLUDEPATH += /home/sci/mherron/Resources/libssh/include/
INCLUDEPATH += /home/sci/mherron/Resources/openssl-OpenSSL_1_1_1d/include/
INCLUDEPATH += /usr/include/ffmpeg
QT += widgets opengl gui charts network
QMAKE_CXX = g++
QMAKE_CXXFLAGS += -std=c++14 -O0
QMAKE_LFLAGS += -static-libstdc++ -static-libgcc
#QMAKE_LFLAGS_RPATH = 
QMAKE_LFLAGS += '-Wl,-rpath,\'\$$ORIGIN/../lib/\',-z,origin'

LIBS += -L/home/sci/mherron/Resources/Qt/5.13.2/gcc_64/lib
LIBS += -L/home/sci/mherron/Resources/tetgen1.5.0/
LIBS += -L/opt/netgen/lib 
LIBS += -L/home/sci/mherron/Resources/libssh/build2/lib -lssh 
LIBS += -L/home/sci/mherron/Resources/openssl-OpenSSL_1_1_1d -lcrypto
LIBS += -L/usr/local/lib 
LIBS += -L../build/lib
LIBS += -Wl,--start-group

#Tetgen
LIBS += -ltet

#Netgen Libs
LIBS += -lnglib -lcsg -linterface -lmesh -locc

#OpenCascade Libs
LIBS += -lTKernel -lTKGeomBase -lTKTopAlgo -lTKPrim -lTKMesh -lTKMath -lTKBRep -lTKSTL -lTKFillet -lTKBO -lTKIGES -lTKSTEP -lTKSTEPBase -lTKXSBase -lTKG3d -lTKLCAF -lTKVCAF

#QuaZip Lib
LIBS += -lquazip5

#PreView Libs
LIBS += -labaqus -lansys -lgeomlib -lmathlib -lxml -lmeshio -lfebio -lcomsol -llsdyna -lmeshtools -lfemlib -lmeshlib -lnike3d -limagelib -lfscore

#PostView Libs
LIBS += -lgllib -lglwlib -lpostgl -lxpltlib -lpostlib -lcuilib

LIBS += -Wl,--end-group

LIBS += -L/usr/lib64 -lGLU -lGL -lz

LIBS += -lavformat -lavcodec -lavresample -lavutil -lswresample -lswscale

LIBS += /home/sci/mherron/Resources/mmg/build/lib/libmmg3d.a

RESOURCES = ../febiostudio.qrc

SOURCES += *.cpp
HEADERS += *.h
