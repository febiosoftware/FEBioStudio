DESTDIR = .
TEMPLATE = app
TARGET = FEBioStudio
DESTDIR = ../build/bin
CONFIG += debug c++14
CONFIG += qt opengl qtwidgets qtcharts warn_off
DEFINES += LINUX MODEL_REPO HAS_SSH HAS_QUAZIP HAS_OCC TETLIBRARY HAS_NETGEN HAS_MMG FFMPEG WEBHELP
INCLUDEPATH += ../
INCLUDEPATH += /home/mherron/Projects/FEBioStudio
INCLUDEPATH += /usr/local/include/
INCLUDEPATH += /home/mherron/Resources/libssh/include/
INCLUDEPATH += /home/mherron/Resources/openssl/include/
INCLUDEPATH += /usr/include/ffmpeg
INCLUDEPATH += /home/mherron/Resources/quazip/quazip5
INCLUDEPATH += /home/mherron/Resources/sqlite/build
QT += widgets opengl gui charts network webenginewidgets
QMAKE_CXX = g++
QMAKE_CXXFLAGS += -std=c++14 -O0
QMAKE_LFLAGS += -static-libstdc++ -static-libgcc
#QMAKE_LFLAGS_RPATH = \$$ORIGIN/../lib/
QMAKE_RPATHDIR += $ORIGIN/../lib
#QMAKE_LFLAGS += '-Wl,-rpath,\'\$$ORIGIN/../lib/\',-z,origin'

LIBS += -L/home/mherron/Resources/Qt/5.13.2/gcc_64/lib
LIBS += -L/home/mherron/Resources/tetgen1.5.1/build
LIBS += -L/opt/netgen/lib 
LIBS += -L/home/mherron/Resources/libssh/build/lib -lssh 
LIBS += -L/home/mherron/Resources/openssl -lcrypto -lssl
LIBS += -L/home/mherron/Resources/quazip/build
LIBS += -L/home/mherron/Resources/sqlite/build/libs -lsqlite3
#LIBS += -L/home/mherron/Resources/scotch_6.0.9/lib -lscotch
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

LIBS += /home/mherron/Resources/mmg/build/lib/libmmg3d.a

RESOURCES = ../febiostudio.qrc

HEADERS += *.h
SOURCES = $$files(*.cpp)
SOURCES -= $$files(moc_*.cpp)
SOURCES -= $$files(qrc_*)


