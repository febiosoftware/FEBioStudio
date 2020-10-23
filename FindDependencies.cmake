# Qt
find_package(Qt5 COMPONENTS Widgets Gui Network OpenGL Charts REQUIRED)
find_package(Qt5 COMPONENTS WebEngineWidgets QUIET)
mark_as_advanced(Qt5Charts_DIR Qt5Core_DIR Qt5Gui_DIR Qt5Network_DIR Qt5OpenGL_DIR Qt5Positioning_DIR 
    Qt5PrintSupport_DIR Qt5QmlModels_DIR Qt5Qml_DIR Qt5Quick_DIR Qt5WebChannel_DIR Qt5WebEngineCore_DIR 
    Qt5WebEngineWidgets_DIR Qt5Widgets_DIR)

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
	find_library(TETGEN_LIB tet 
        PATHS C:/Program\ Files/* $ENV{HOMEPATH}/* $ENV{HOMEPATH}/*/*
        PATH_SUFFIXES "build/lib" "cmbuild/lib" "src/build/lib" "src/cmbuild/lib" "Release" "Debug"
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
	option(USE_NETGEN "Required for meshing CAD objects" ON)
    mark_as_advanced(NETGEN_INC NETGEN_LIB_DIR)
else()
	option(USE_NETGEN "Required for meshing CAD objects" OFF)
    mark_as_advanced(CLEAR NETGEN_INC NETGEN_LIB_DIR)
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
	option(USE_OCCT "Required for importing and meshing CAD objects." ON)
    mark_as_advanced(OCCT_INC OCCT_LIB_DIR)
else()
	option(USE_OCCT "Required for importing and meshing CAD objects." OFF)
    mark_as_advanced(CLEAR OCCT_INC OCCT_LIB_DIR)
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
if(WIN32)
	find_path(QUAZIP_INC quazip.h
        PATHS C:/Program\ Files/* $ENV{HOMEPATH}/* $ENV{HOMEPATH}/*/*
		PATH_SUFFIXES "/quazip5" "include/quazip5" "quazip/include/quazip5" "build/quazip5" "build/include/quazip5"
        DOC "QuaZip include directory")
	find_library(QUAZIP_LIB quazip5 
        PATHS C:/Program\ Files/* $ENV{HOMEPATH}/* $ENV{HOMEPATH}/*/*
        PATH_SUFFIXES "build/lib" "cmbuild/lib" "src/build/lib" "src/cmbuild/lib" "Release" "Debug"
		DOC "QuaZip library path")
elseif(APPLE)
	find_path(QUAZIP_INC quazip.h
        PATHS /usr/include/* /opt/quazip $ENV{HOME}/* $ENV{HOME}/*/*
        PATH_SUFFIXES "/quazip" "include/quazip" "quazip/include/quazip" "build/quazip" "build/include/quazip"
		DOC "QuaZip include directory")
	find_library(QUAZIP_LIB quazip 
        PATHS /opt/quazip $ENV{HOME}/* $ENV{HOME}/*/*
        PATH_SUFFIXES "lib" "quazip/lib" "build" "build/lib" "Release" "Debug"
		DOC "QuaZip library path")
else()
	find_path(QUAZIP_INC quazip.h
        PATHS /opt/quazip $ENV{HOME}/* $ENV{HOME}/*/*
        PATH_SUFFIXES "/quazip5" "include/quazip5" "quazip/include/quazip5" "build/quazip5" "build/include/quazip5"
		DOC "QuaZip include directory")
	find_library(QUAZIP_LIB quazip5 
        PATHS /opt/quazip $ENV{HOME}/* $ENV{HOME}/*/*
        PATH_SUFFIXES "lib" "quazip/lib" "build" "build/lib" "Release" "Debug"
		DOC "QuaZip library path")
endif()

if(QUAZIP_LIB)
    get_filename_component(QUAZIP_TEMP ${QUAZIP_LIB} DIRECTORY)
    set(QUAZIP_LIB_DIR ${QUAZIP_TEMP} CACHE PATH "Path to the QuaZip lib directory (e.g. /opt/quazip/lib)")
    unset(QUAZIP_TEMP)
    unset(QUAZIP_LIB CACHE)
else()
	set(QUAZIP_LIB_DIR "QUAZIP_LIB_DIR-NOTFOUND" CACHE PATH "Path to the QuaZip lib directory (e.g. /opt/quazip/lib)")
    unset(QUAZIP_LIB CACHE)
endif()

if(QUAZIP_INC AND QUAZIP_LIB_DIR)		
	option(USE_QUAZIP "Required for the Model Repository and exporting projects." ON)
    mark_as_advanced(QUAZIP_INC QUAZIP_LIB_DIR)
else()
	option(USE_QUAZIP "Required for the Model Repository and exporting projects." OFF)
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

if(SQLITE_INC AND SQLITE_LIB_DIR AND USE_QUAZIP)		
	option(MODEL_REPO "Build code to connect to the Model Repository." ON)
    mark_as_advanced(SQLITE_INC SQLITE_LIB_DIR)
else()
    SET(MODEL_REPO OFF CACHE BOOL "Build code to connect to the Model Repository." FORCE)
    mark_as_advanced(CLEAR SQLITE_INC SQLITE_LIB_DIR)
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
        PATH_SUFFIXES "include" "ffmpeg/include" "build" "build/include"
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

# ZLIB
find_package(ZLIB REQUIRED)
if(ZLIB_INCLUDE_DIR AND ZLIB_LIBRARY_RELEASE)
    mark_as_advanced(ZLIB_INCLUDE_DIR ZLIB_LIBRARY_RELEASE)
endif()
