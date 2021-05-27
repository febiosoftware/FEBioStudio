# Qt
find_package(${Qt_Version} COMPONENTS Widgets Gui Network OpenGL REQUIRED) 

if(Qt_Ver VERSION_EQUAL 6)
  find_package(${Qt_Version} COMPONENTS Core5Compat OpenGLWidgets REQUIRED)
endif()
find_package(${Qt_Version} COMPONENTS WebEngineWidgets QUIET)
mark_as_advanced(${Qt_Version}_DIR ${Qt_Version}Charts_DIR ${Qt_Version}Core_DIR ${Qt_Version}Gui_DIR ${Qt_Version}Network_DIR ${Qt_Version}OpenGL_DIR 
    ${Qt_Version}Positioning_DIR ${Qt_Version}PrintSupport_DIR ${Qt_Version}QmlModels_DIR ${Qt_Version}Qml_DIR ${Qt_Version}Quick_DIR
    ${Qt_Version}Widgets_DIR ${Qt_Version}Core5Compat_DIR ${Qt_Version}GuiTools_DIR ${Qt_Version}CoreTools_DIR ${Qt_Version}OpenGLWidgets_DIR
    ${Qt_Version}WidgetsTools_DIR ${Qt_Version}WebEngineWidgets_DIR XKB_INCLUDE_DIR XKB_LIBRARY)
	
#~ if(Qt_Ver VERSION_EQUAL 5)
  mark_as_advanced(Qt5Charts_DIR Qt5WebEngineWidgets_DIR Qt5WebChannel_DIR Qt5WebEngineCore_DIR)
#~ endif()

#Teem
if(WIN32)
	find_path(TEEM_INC teem/nrrd.h
        PATHS C:/Program\ Files/* $ENV{HOMEPATH}/* $ENV{HOMEPATH}/*/*
		PATH_SUFFIXES "include" "include/teem*" "src" "build" "build/include"
        DOC "Teem include directory")
	find_library(TEEM_LIB teem 
        PATHS C:/Program\ Files/* $ENV{HOMEPATH}/* $ENV{HOMEPATH}/*/*
        PATH_SUFFIXES "build/bin" "src/build/bin" "Release" "Debug"
		DOC "Teem library path")
else()
	find_path(TEEM_INC teem/nrrd.h
        PATHS /opt/hypre* $ENV{HOME}/* $ENV{HOME}/*/*
        PATH_SUFFIXES "include" "include/teem" "build" "build/include" "src" 
		DOC "Teem include directory")
	find_library(TEEM_LIB teem
        PATHS /opt/teem* $ENV{HOME}/* $ENV{HOME}/*/*
        PATH_SUFFIXES "build/bin" "build/lib" "src/build/bin" "src/build/lib" "Release" "Debug"
		DOC "Teem library path")
endif()

if(TEEM_LIB)
    get_filename_component(TEEM_TEMP ${TEEM_LIB} DIRECTORY)
    set(TEEM_LIB_DIR ${TEEM_TEMP} CACHE PATH "Path to the Teem lib directory (e.g. /opt/teem/bin)")
    unset(TEEM_TEMP)
    unset(TEEM_LIB CACHE)
else()
	set(TEEM_LIB_DIR  CACHE PATH "Path to the Teem lib directory (e.g. /opt/teem/bin)")
    unset(TEEM_LIB CACHE)
endif()

#Libtiff
if(WIN32)
	find_path(LIBTIFF_INC tiffio.h
        PATHS C:/Program\ Files/* $ENV{HOMEPATH}/* $ENV{HOMEPATH}/*/*
		PATH_SUFFIXES "include" "src" "build" "build/include"
        DOC "LibTiff include directory")
	find_library(LIBTIFF_LIB tiff 
        PATHS C:/Program\ Files/* $ENV{HOMEPATH}/* $ENV{HOMEPATH}/*/*
        PATH_SUFFIXES "build/lib" "src/build/lib" "Release" "Debug"
		DOC "LibTiff library path")
else()
	find_path(LIBTIFF_INC tiffio.h
        PATHS /opt/hypre* $ENV{HOME}/* $ENV{HOME}/*/*
        PATH_SUFFIXES "include" "build" "build/include" "src" 
		DOC "LibTiff include directory")
	find_library(LIBTIFF_LIB tiff
        PATHS /opt/libtiff* $ENV{HOME}/* $ENV{HOME}/*/*
        PATH_SUFFIXES "build/bin" "build/lib" "src/build/bin" "src/build/lib" "Release" "Debug"
		DOC "LibTiff library path")
endif()

if(LIBTIFF_LIB)
    get_filename_component(LIBTIFF_TEMP ${LIBTIFF_LIB} DIRECTORY)
    set(LIBTIFF_LIB_DIR ${LIBTIFF_TEMP} CACHE PATH "Path to the LibTiff lib directory (e.g. /opt/libtiff/lib)")
    unset(LIBTIFF_TEMP)
    unset(LIBTIFF_LIB CACHE)
else()
	set(LIBTIFF_LIB_DIR  CACHE PATH "Path to the LibTiff lib directory (e.g. /opt/libtiff/lib)")
    unset(LIBTIFF_LIB CACHE)
endif()


if(LIBTIFF_INC AND LIBTIFF_LIB_DIR AND TEEM_INC AND TEEM_LIB_DIR)		
	option(USE_TEEM "Required for Teem use" ON)
    mark_as_advanced(TEEM_INC TEEM_LIB_DIR)
else()
	option(USE_TEEM "Required for Teem use" OFF)
    mark_as_advanced(CLEAR TEEM_INC TEEM_LIB_DIR)
endif()

# Dicom
if(WIN32)
	find_path(DICOM_INC dcmtk/dcmimgle/dcmimage.h
        PATHS C:/Program\ Files/* $ENV{HOMEPATH}/* $ENV{HOMEPATH}/*/*
		PATH_SUFFIXES "include" "include/dcmtk*" "src" "build" "build/dcmtk"
        DOC "Dicom include directory")
	find_library(DICOM_LIB dcmimgle 
        PATHS C:/Program\ Files/* $ENV{HOMEPATH}/* $ENV{HOMEPATH}/*/*
        PATH_SUFFIXES "build/lib" "src/build/lib" "Release" "Debug"
		DOC "Dicom library path")
else()
	find_path(DICOM_INC dcmtk/dcmimgle/dcmimage.h
        PATHS /opt/hypre* $ENV{HOME}/* $ENV{HOME}/*/*
        PATH_SUFFIXES "include" "include/dcmtk*" "build" "build/include/dcmtk" "src" 
		DOC "Dicom include directory")
	find_library(DICOM_LIB dcmimgle
        PATHS /opt/teem* $ENV{HOME}/* $ENV{HOME}/*/*
        PATH_SUFFIXES "build/bin" "build/lib" "src/build/bin" "src/build/lib" "Release" "Debug"
		DOC "Dicom library path")
endif()

if(DICOM_LIB)
    get_filename_component(DICOM_TEMP ${DICOM_LIB} DIRECTORY)
    set(DICOM_LIB_DIR ${DICOM_TEMP} CACHE PATH "Path to the Dicom lib directory (e.g. /opt/dcmtk/lib)")
    unset(DICOM_TEMP)
    unset(DICOM_LIB CACHE)
else()
	set(DICOM_LIB_DIR  CACHE PATH "Path to the Dicom lib directory (e.g. /opt/dcmtk/lib)")
    unset(DICOM_LIB CACHE)
endif()

if(DICOM_INC AND DICOM_LIB_DIR)		
	option(USE_DICOM "Required for Dicom use" ON)
    mark_as_advanced(DICOM_INC DICOM_LIB_DIR)
else()
	option(USE_DICOM "Required for Dicom use" OFF)
    mark_as_advanced(CLEAR DICOM_INC DICOM_LIB_DIR)
endif()

# MMG
if(WIN32)
	find_path(MMG_INC mmg/mmg3d/libmmg3d.h
        PATHS C:/Program\ Files/* $ENV{HOMEPATH}/* $ENV{HOMEPATH}/*/*
		PATH_SUFFIXES "include" "include/mmg*" "src" "build" "build/include"
        DOC "MMG include directory")
	find_library(MMG_LIB mmg3d 
        PATHS C:/Program\ Files/* $ENV{HOMEPATH}/* $ENV{HOMEPATH}/*/*
        PATH_SUFFIXES "build/lib" "cmbuild/lib" "src/build/lib" "src/cmbuild/lib" "Release" "Debug"
		DOC "MMG library path")
else()
	find_path(MMG_INC mmg/mmg3d/libmmg3d.h
        PATHS /opt/hypre* $ENV{HOME}/* $ENV{HOME}/*/*
        PATH_SUFFIXES "include" "include/mmg" "build" "build/include" "cbuild" "cbuild/include" "src" 
		DOC "MMG include directory")
	find_library(MMG_LIB mmg3d 
        PATHS /opt/mmg* $ENV{HOME}/* $ENV{HOME}/*/*
        PATH_SUFFIXES "build/lib" "cbuild/lib" "src/build/lib" "src/cbuild/lib" "Release" "Debug"
		DOC "MMG library path")
endif()

if(MMG_LIB)
    get_filename_component(MMG_TEMP ${MMG_LIB} DIRECTORY)
    set(MMG_LIB_DIR ${MMG_TEMP} CACHE PATH "Path to the MMG lib directory (e.g. /opt/mmg/lib)")
    unset(MMG_TEMP)
    unset(MMG_LIB CACHE)
else()
	set(MMG_LIB_DIR  CACHE PATH "Path to the MMG lib directory (e.g. /opt/mmg/lib)")
    unset(MMG_LIB CACHE)
endif()


if(MMG_INC AND MMG_LIB_DIR)		
	option(USE_MMG "Required for MMG use" ON)
    mark_as_advanced(MMG_INC MMG_LIB_DIR)
else()
	option(USE_MMG "Required for MMG use" OFF)
    mark_as_advanced(CLEAR MMG_INC MMG_LIB_DIR)
endif()

# TETGEN
if(WIN32)
	find_path(TETGEN_INC tetgen.h
        PATHS C:/Program\ Files/* $ENV{HOMEPATH}/* $ENV{HOMEPATH}/*/*
		PATH_SUFFIXES "include" "include/tetgen*" "src" "build" "build/include"
        DOC "TetGen include directory")
	find_library(TETGEN_LIB tetgen 
        PATHS C:/Program\ Files/* $ENV{HOMEPATH}/* $ENV{HOMEPATH}/*/*
        PATH_SUFFIXES "build/lib" "cmbuild/lib" "src/build/lib" "src/cmbuild/lib" "x64/Release" "x64/Debug"
		DOC "TetGen library path")
else()
	find_path(TETGEN_INC tetgen.h
        PATHS /opt/tetgen* $ENV{HOME}/* $ENV{HOME}/*/*
        PATH_SUFFIXES "include" "include/tetgen*" "src" "build" "build/include"
		DOC "TetGen include directory")
	find_library(TETGEN_LIB tet 
        PATHS /opt/tetgen* $ENV{HOME}/* $ENV{HOME}/*/*
        PATH_SUFFIXES "build" "cbuild" "build/lib" "cmbuild/lib" "src/build/lib" "src/cmbuild/lib" "Release" "Debug"
		DOC "TetGen library path")
endif()

if(TETGEN_LIB)
    get_filename_component(TETGEN_TEMP ${TETGEN_LIB} DIRECTORY)
    set(TETGEN_LIB_DIR ${TETGEN_TEMP} CACHE PATH "Path to the TET lib directory (e.g. /opt/tetgen/lib)")
    unset(TETGEN_TEMP)
    unset(TETGEN_LIB CACHE)
else()
	set(TETGEN_LIB_DIR "TETGEN_LIB_DIR-NOTFOUND" CACHE PATH "Path to the TET lib directory (e.g. /opt/tetgen/lib)")
    unset(TETGEN_LIB CACHE)
endif()

if(TETGEN_INC AND TETGEN_LIB_DIR)		
	option(USE_TETGEN "Required for adaptive remeshing" ON)
    mark_as_advanced(TETGEN_INC TETGEN_LIB_DIR)
else()
	option(USE_TETGEN "Required for adaptive remeshing" OFF)
    mark_as_advanced(CLEAR TETGEN_INC TETGEN_LIB_DIR)
endif()

# OpenCascade
if(WIN32)
	find_path(OCCT_INC gp_Pnt.hxx
        PATHS C:/Program\ Files/* $ENV{HOMEPATH}/* $ENV{HOMEPATH}/*/*
		PATH_SUFFIXES "/opencascade" "include/opencascade" "opencascade/include/opencascade" "build/opencascade" "build/include/opencascade"
        DOC "OpenCascade include directory")
	find_library(OCCT_LIB TKernel 
        PATHS C:/Program\ Files/* $ENV{HOMEPATH}/* $ENV{HOMEPATH}/*/*
        PATH_SUFFIXES "build/lib" "cmbuild/lib" "src/build/lib" "src/cmbuild/lib" "Release" "Debug"
		DOC "OpenCascade library path")
else()
	find_path(OCCT_INC gp_Pnt.hxx
        PATHS /opt/opencascade $ENV{HOME}/* $ENV{HOME}/*/*
        PATH_SUFFIXES "/opencascade" "include/opencascade" "opencascade/include/opencascade" "build/opencascade" "build/include/opencascade"
		DOC "OpenCascade include directory")
	find_library(OCCT_LIB TKernel 
        PATHS /opt/opencascade $ENV{HOME}/* $ENV{HOME}/*/*
        PATH_SUFFIXES "lib" "opencascade/lib" "build" "build/lib" "Release" "Debug"
		DOC "OpenCascade library path")
endif()

if(OCCT_LIB)
    get_filename_component(OCCT_TEMP ${OCCT_LIB} DIRECTORY)
    set(OCCT_LIB_DIR ${OCCT_TEMP} CACHE PATH "Path to the OpenCascade lib directory (e.g. /opt/opencascade/lib)")
    unset(OCCT_TEMP)
    unset(OCCT_LIB CACHE)
else()
	set(OCCT_LIB_DIR "OCCT_LIB_DIR-NOTFOUND" CACHE PATH "Path to the OpenCascade lib directory (e.g. /opt/opencascade/lib)")
    unset(OCCT_LIB CACHE)
endif()

if(OCCT_INC AND OCCT_LIB_DIR)		
    mark_as_advanced(OCCT_INC OCCT_LIB_DIR)
else()
    mark_as_advanced(CLEAR OCCT_INC OCCT_LIB_DIR)
endif()

# NETGEN
if(WIN32)
	find_path(NETGEN_INC include/occgeom.hpp
        PATHS C:/Program\ Files/* $ENV{HOMEPATH}/* $ENV{HOMEPATH}/*/*
		PATH_SUFFIXES "include" "netgen/include" "build" "build/include"
        DOC "Netgen include directory")
	find_library(NETGEN_LIB nglib 
        PATHS C:/Program\ Files/* $ENV{HOMEPATH}/* $ENV{HOMEPATH}/*/*
        PATH_SUFFIXES "build/lib" "cmbuild/lib" "src/build/lib" "src/cmbuild/lib" "Release" "Debug"
		DOC "Netgen library path")
else()
	find_path(NETGEN_INC include/occgeom.hpp
        PATHS /opt/netgen* $ENV{HOME}/* $ENV{HOME}/*/* /Applications/Netgen.app/Contents/Resources/include
        PATH_SUFFIXES "include" "netgen/include" "build" "build/include"
		DOC "Netgen include directory")
	find_library(NETGEN_LIB nglib 
        PATHS /opt/netgen* $ENV{HOME}/* $ENV{HOME}/*/* /Applications/Netgen.app/Contents/MacOS
        PATH_SUFFIXES "lib" "netgen/lib" "build" "build/lib" "Release" "Debug"
		DOC "Netgen library path")
endif()

if(NETGEN_LIB)
    get_filename_component(NETGEN_TEMP ${NETGEN_LIB} DIRECTORY)
    set(NETGEN_LIB_DIR ${NETGEN_TEMP} CACHE PATH "Path to the Netgen lib directory (e.g. /opt/netgen/lib)")
    unset(NETGEN_TEMP)
    unset(NETGEN_LIB CACHE)
else()
	set(NETGEN_LIB_DIR "NETGEN_LIB_DIR-NOTFOUND" CACHE PATH "Path to the Netgen lib directory (e.g. /opt/netgen/lib)")
    unset(NETGEN_LIB CACHE)
endif()

if(NETGEN_INC AND NETGEN_LIB_DIR)		
    mark_as_advanced(NETGEN_INC NETGEN_LIB_DIR)
else()
    mark_as_advanced(CLEAR NETGEN_INC NETGEN_LIB_DIR)
endif()

if(NETGEN_INC AND NETGEN_LIB_DIR AND OCCT_INC AND OCCT_LIB_DIR)		
	option(CAD_FEATURES "Required for importing and meshing CAD objects" ON)
else()
	SET(CAD_FEATURES OFF CACHE BOOL "Required for importing and meshing CAD objects" FORCE)
endif()

# LibSSH
if(WIN32)
	find_path(SSH_INC libssh/libssh.h
        PATHS C:/Program\ Files/* $ENV{HOMEPATH}/* $ENV{HOMEPATH}/*/*
		PATH_SUFFIXES "include" "libssh/include" "build" "build/include"
        DOC "LibSSH include directory")
	find_library(SSH_LIB ssh 
        PATHS C:/Program\ Files/* $ENV{HOMEPATH}/* $ENV{HOMEPATH}/*/*
        PATH_SUFFIXES "build/lib" "cmbuild/lib" "src/build/lib" "src/cmbuild/lib" "Release" "Debug"
		DOC "LibSSH library path")
else()
	find_path(SSH_INC libssh/libssh.h
        PATHS /opt/libssh /usr/local/opt/ $ENV{HOME}/* $ENV{HOME}/*/*
        PATH_SUFFIXES "include" "libssh/include" "build" "build/include"
		DOC "LibSSH include directory")
	find_library(SSH_LIB ssh
        PATHS /opt/libssh /usr/local/opt/ $ENV{HOME}/* $ENV{HOME}/*/*
        PATH_SUFFIXES "lib" "libssh/lib" "build" "build/lib" "Release" "Debug"
		DOC "LibSSH library path")
endif()

if(SSH_LIB)
    get_filename_component(SSH_TEMP ${SSH_LIB} DIRECTORY)
    set(SSH_LIB_DIR ${SSH_TEMP} CACHE PATH "Path to the LibSSH lib directory (e.g. /opt/libssh/lib)")
    unset(SSH_TEMP)
    unset(SSH_LIB CACHE)
else()
	set(SSH_LIB_DIR "SSH_LIB_DIR-NOTFOUND" CACHE PATH "Path to the LibSSH lib directory (e.g. /opt/libssh/lib)")
    unset(SSH_LIB CACHE)
endif()

if(SSH_INC AND SSH_LIB_DIR)		
	option(USE_SSH "Required for running jobs on remote machines." ON)
    mark_as_advanced(SSH_INC SSH_LIB_DIR)
else()
	option(USE_SSH "Required for running jobs on remote machines." OFF)
    mark_as_advanced(CLEAR SSH_INC SSH_LIB_DIR)
endif()

# OpenSSL
if(WIN32)
	find_path(SSL_INC openssl/aes.h
        PATHS C:/Program\ Files/* $ENV{HOMEPATH}/* $ENV{HOMEPATH}/*/*
		PATH_SUFFIXES "include" "openssl/include" "build" "build/include"
        DOC "OpenSSL include directory")
	find_library(SSL_LIB ssl 
        PATHS C:/Program\ Files/* $ENV{HOMEPATH}/* $ENV{HOMEPATH}/*/*
        PATH_SUFFIXES "build/lib" "cmbuild/lib" "src/build/lib" "src/cmbuild/lib" "Release" "Debug"
		DOC "OpenSSL library path")
elseif(APPLE)
	find_path(SSL_INC openssl/aes.h
        PATHS /opt/openssl /usr/local/opt/ $ENV{HOME}/* $ENV{HOME}/*/*
        PATH_SUFFIXES "include" "openssl/include" "build" "build/include"
		DOC "OpenSSL include directory")
	find_library(SSL_LIB ssl 
        PATHS /opt/openssl /usr/local/opt/ $ENV{HOME}/* $ENV{HOME}/*/*
        PATH_SUFFIXES "lib" "openssl/lib" "build" "build/lib" "Release" "Debug"
		DOC "OpenSSL library path"
		NO_DEFAULT_PATH)
else()
	find_path(SSL_INC openssl/aes.h
        PATHS /opt/openssl /usr/local/opt/ $ENV{HOME}/* $ENV{HOME}/*/*
        PATH_SUFFIXES "include" "openssl/include" "build" "build/include"
		DOC "OpenSSL include directory")
	find_library(SSL_LIB ssl 
        PATHS /opt/openssl /usr/local/opt/ $ENV{HOME}/* $ENV{HOME}/*/*
        PATH_SUFFIXES "lib" "openssl/lib" "build" "build/lib" "Release" "Debug"
		DOC "OpenSSL library path")
endif()

if(SSL_LIB)
    get_filename_component(SSL_TEMP ${SSL_LIB} DIRECTORY)
    set(SSL_LIB_DIR ${SSL_TEMP} CACHE PATH "Path to the OpenSSL lib directory (e.g. /opt/openssl/lib)")
    unset(SSL_TEMP)
    unset(SSL_LIB CACHE)
else()
	set(SSL_LIB_DIR "SSL_LIB_DIR-NOTFOUND" CACHE PATH "Path to the OpenSSL lib directory (e.g. /opt/openssl/lib)")
    unset(SSL_LIB CACHE)
endif()

if(SSL_INC AND SSL_LIB_DIR AND USE_SSH)		
	option(USE_SSH "Required for running jobs on remote machines." ON)
    mark_as_advanced(SSL_INC SSL_LIB_DIR)
else()
	option(USE_SSH "Required for running jobs on remote machines." OFF)
    mark_as_advanced(CLEAR SSL_INC SSL_LIB_DIR)
endif()

# QuaZip
if(Qt_Ver VERSION_EQUAL 6)
    set(QUAZIP_NAMES quazip1-qt6 quazip6 quazip)
else()
    set(QUAZIP_NAMES quazip5 quazip)
endif()

if(WIN32)
	find_path(QUAZIP_INC quazip.h
        PATHS C:/Program\ Files/* $ENV{HOMEPATH}/* $ENV{HOMEPATH}/*/*
		PATH_SUFFIXES "/quazip5" "include/quazip5" "quazip/include/quazip5" "build/quazip5" "build/include/quazip5"
        DOC "QuaZip include directory")
	find_library(QUAZIP_LIB 
        NAMES ${QUAZIP_NAMES} 
        PATHS C:/Program\ Files/* $ENV{HOMEPATH}/* $ENV{HOMEPATH}/*/*
        PATH_SUFFIXES "build/lib" "cmbuild/lib" "src/build/lib" "src/cmbuild/lib" "Release" "Debug"
		DOC "QuaZip library path")
elseif(APPLE)
	find_path(QUAZIP_INC quazip.h
        PATHS /usr/include/* /opt/quazip $ENV{HOME}/* $ENV{HOME}/*/*
        PATH_SUFFIXES "quazip" "include/quazip" "quazip/include/quazip" "build/quazip" "build/include/quazip"
		DOC "QuaZip include directory")
	find_library(QUAZIP_LIB 
        NAMES ${QUAZIP_NAMES}  
        PATHS /opt/quazip $ENV{HOME}/* $ENV{HOME}/*/*
        PATH_SUFFIXES "lib" "quazip/lib" "build" "build/lib" "Release" "Debug"
		DOC "QuaZip library path")
else()
	find_path(QUAZIP_INC quazip.h
        PATHS /usr/local/include/* /opt/quazip $ENV{HOME}/* $ENV{HOME}/*/* /usr/local/include/*
        PATH_SUFFIXES "quazip" "quazip5" "include/quazip5" "quazip/include/quazip5" "build/quazip5" "build/include/quazip5"
		DOC "QuaZip include directory")
	find_library(QUAZIP_LIB
        NAMES ${QUAZIP_NAMES}
        PATHS /opt/quazip $ENV{HOME}/* $ENV{HOME}/*/* /usr/local/ /usr/local/* 
        PATH_SUFFIXES "lib" "lib64" "quazip/lib" "quazip/build/quazip" "build" "build/lib" "Release" "Debug"
		DOC "QuaZip library path")
endif()

if(QUAZIP_LIB)
    get_filename_component(QUAZIP_NAME ${QUAZIP_LIB} NAME)
    get_filename_component(QUAZIP_TEMP ${QUAZIP_LIB} DIRECTORY)
    set(QUAZIP_LIB_DIR ${QUAZIP_TEMP} CACHE PATH "Path to the QuaZip lib directory (e.g. /opt/quazip/lib)")
    unset(QUAZIP_TEMP)
    unset(QUAZIP_LIB CACHE)
else()
	set(QUAZIP_LIB_DIR "QUAZIP_LIB_DIR-NOTFOUND" CACHE PATH "Path to the QuaZip lib directory (e.g. /opt/quazip/lib)")
    unset(QUAZIP_LIB CACHE)
endif()

if(QUAZIP_INC AND QUAZIP_LIB_DIR)		
    mark_as_advanced(QUAZIP_INC QUAZIP_LIB_DIR)
else()
    mark_as_advanced(CLEAR QUAZIP_INC QUAZIP_LIB_DIR)
endif()

# SQLite
if(WIN32)
	find_path(SQLITE_INC sqlite3.h
        PATHS C:/Program\ Files/* $ENV{HOMEPATH}/* $ENV{HOMEPATH}/*/*
		PATH_SUFFIXES "include" "sqlite/include" "build" "build/include"
        DOC "SQLite include directory")
	find_library(SQLITE_LIB sqlite3 
        PATHS C:/Program\ Files/* $ENV{HOMEPATH}/* $ENV{HOMEPATH}/*/*
        PATH_SUFFIXES "build/lib" "cmbuild/lib" "src/build/lib" "src/cmbuild/lib" "Release" "Debug"
		DOC "SQLite library path")
else()
	find_path(SQLITE_INC sqlite3.h
        PATHS /opt/sqlite $ENV{HOME}/* $ENV{HOME}/*/*
        PATH_SUFFIXES "include" "sqlite/include" "build" "build/include"
		DOC "SQLite include directory")
	find_library(SQLITE_LIB sqlite3
        PATHS /opt/sqlite $ENV{HOME}/* $ENV{HOME}/*/*
        PATH_SUFFIXES "lib" "sqlite/lib" "build" "build/lib" "libs" "sqlite/libs" "build" "build/libs" "Release" "Debug"
		DOC "SQLite library path")
endif()

if(SQLITE_LIB)
    get_filename_component(SQLITE_TEMP ${SQLITE_LIB} DIRECTORY)
    set(SQLITE_LIB_DIR ${SQLITE_TEMP} CACHE PATH "Path to the SQLite lib directory (e.g. /opt/sqlite/lib)")
    unset(SQLITE_TEMP)
    unset(SQLITE_LIB CACHE)
else()
	set(SQLITE_LIB_DIR "SQLITE_LIB_DIR-NOTFOUND" CACHE PATH "Path to the SQLite lib directory (e.g. /opt/sqlite/lib)")
    unset(SQLITE_LIB CACHE)
endif()

if(SQLITE_INC AND SQLITE_LIB_DIR)		
    mark_as_advanced(SQLITE_INC SQLITE_LIB_DIR)
else()
    mark_as_advanced(CLEAR SQLITE_INC SQLITE_LIB_DIR)
endif()

if(SQLITE_INC AND SQLITE_LIB_DIR AND QUAZIP_INC AND QUAZIP_LIB_DIR)		
	option(MODEL_REPO "Build code to connect to the Model Repository." ON)
else()
    SET(MODEL_REPO OFF CACHE BOOL "Build code to connect to the Model Repository." FORCE)
endif()

# FFMPEG
if(WIN32)
	find_path(FFMPEG_INC libavformat/libavformat.h
        PATHS C:/Program\ Files/* $ENV{HOMEPATH}/* $ENV{HOMEPATH}/*/*
		PATH_SUFFIXES "include" "ffmpeg/include" "build" "build/include"
        DOC "FFMPEG include directory")
	find_library(FFMPEG_LIB avformat 
        PATHS C:/Program\ Files/* $ENV{HOMEPATH}/* $ENV{HOMEPATH}/*/*
        PATH_SUFFIXES "build/lib" "cmbuild/lib" "src/build/lib" "src/cmbuild/lib" "Release" "Debug"
		DOC "FFMPEG library path")
elseif(APPLE)
	find_path(FFMPEG_INC libavformat/avformat.h
        PATHS /usr/local/opt/ /opt/ffmpeg $ENV{HOME}/* $ENV{HOME}/*/*
        PATH_SUFFIXES "include" "ffmpeg/include" "build" "build/include"
		DOC "FFMPEG include directory"
		NO_DEFAULT_PATH)
	find_library(FFMPEG_LIB avformat
        PATHS  /usr/local/opt/ /opt/ffmpeg $ENV{HOME}/* $ENV{HOME}/*/*
        PATH_SUFFIXES "lib" "ffmpeg/lib" "build" "build/lib" "Release" "Debug"
		DOC "FFMPEG library path"
		NO_DEFAULT_PATH)
else()
	find_path(FFMPEG_INC libavformat/avformat.h
        PATHS /usr/include/ /opt/ffmpeg $ENV{HOME}/* $ENV{HOME}/*/*
        PATH_SUFFIXES "ffmpeg" "ffmpeg/include" "build" "build/include"
		DOC "FFMPEG include directory")
	find_library(FFMPEG_LIB avformat
        PATHS /opt/ffmpeg $ENV{HOME}/* $ENV{HOME}/*/*
        PATH_SUFFIXES "lib" "ffmpeg/lib" "build" "build/lib" "Release" "Debug"
		DOC "FFMPEG library path")
endif()

if(FFMPEG_LIB)
    get_filename_component(FFMPEG_TEMP ${FFMPEG_LIB} DIRECTORY)
    set(FFMPEG_LIB_DIR ${FFMPEG_TEMP} CACHE PATH "Path to the FFMPEG lib directory (e.g. /opt/ffmpeg/lib)")
    unset(FFMPEG_TEMP)
    unset(FFMPEG_LIB CACHE)
else()
	set(FFMPEG_LIB_DIR "FFMPEG_LIB_DIR-NOTFOUND" CACHE PATH "Path to the FFMPEG lib directory (e.g. /opt/ffmpeg/lib)")
    unset(FFMPEG_LIB CACHE)
endif()

if(FFMPEG_INC AND FFMPEG_LIB_DIR)		
	option(USE_FFMPEG "Required to export mpeg videos." ON)
    mark_as_advanced(FFMPEG_INC FFMPEG_LIB_DIR)
else()
	option(USE_FFMPEG "Required to export mpeg videos." OFF)
    mark_as_advanced(CLEAR FFMPEG_INC FFMPEG_LIB_DIR)
endif()

# OpenGL
find_package(OpenGL REQUIRED)

# Python
find_package(Python3 COMPONENTS Development)

if(WIN32)
	find_path(PYBIND11_INC pybind11/pybind11.h
        PATHS C:/Program\ Files/* $ENV{HOMEPATH}/* $ENV{HOMEPATH}/*/*
		PATH_SUFFIXES "include" "pybind11/include"
        DOC "pybind11 include directory")
elseif(APPLE)
	find_path(PYBIND11_INC pybind11/pybind11.h
        PATHS /usr/include/ /opt/pybind11 $ENV{HOME}/* $ENV{HOME}/*/*
        PATH_SUFFIXES "include" "pybind11/include"
		DOC "pybind11 include directory"
		NO_DEFAULT_PATH)
else()
	find_path(PYBIND11_INC pybind11/pybind11.h
        PATHS /usr/include/ /opt/pybind11 $ENV{HOME}/* $ENV{HOME}/*/*
        PATH_SUFFIXES "include" "pybind11/include" 
		DOC "pybind11 include directory")
endif()

if(PYBIND11_INC)
    mark_as_advanced(PYBIND11_INC)
else()
    option(USE_PYTHON "Required for Python plugins." OFF)
endif()

if(Python3_Development_FOUND)
    if(PYBIND11_INC)
        option(USE_PYTHON "Required for Python plugins." ON)
    endif()
else()
    option(USE_PYTHON "Required for Python plugins." OFF)
    set(Python_ROOT_DIR "" CACHE PATH "Path to the root of a Python installation")
endif()

# ZLIB
find_package(ZLIB REQUIRED)
if(ZLIB_INCLUDE_DIR AND ZLIB_LIBRARY_RELEASE)
    mark_as_advanced(ZLIB_INCLUDE_DIR ZLIB_LIBRARY_RELEASE)
endif()

# OpenMP
if(NOT WIN32)
    find_package(OpenMP QUIET)
endif()
