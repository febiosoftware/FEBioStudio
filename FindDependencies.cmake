# Qt
find_package(Qt6 COMPONENTS Widgets Gui Network OpenGL OpenGLWidgets REQUIRED) 

mark_as_advanced(Qt6_DIR Qt6Core_DIR Qt6CoreTools_DIR Qt6DBusTools_DIR Qt6DBus_DIR Qt6GuiTools_DIR 
    Qt6Gui_DIR Qt6Network_DIR Qt6OpenGLWidgets_DIR Qt6OpenGL_DIR Qt6WidgetsTools_DIR Qt6Widgets_DIR)

# FEBio
# Find FEBio SDK or git repo automatically
if(WIN32)
	set(TEMP_PATHS ${CMAKE_SOURCE_DIR}/.. ${CMAKE_SOURCE_DIR}/../.. $ENV{HOMEPATH}/ $ENV{HOMEPATH}/source/repos $ENV{HOMEPATH}/*)
else()
    set(TEMP_PATHS ${CMAKE_SOURCE_DIR}/.. ${CMAKE_SOURCE_DIR}/../.. $ENV{HOME}/ $ENV{HOME}/*)
endif()

find_path(FEBio_SDK FECore/stdafx.h
    PATHS ${TEMP_PATHS}
    PATH_SUFFIXES FEBio
    DOC "Path to the FEBio SDK, or git repo.")

if(NOT FEBio_SDK)
    if(WIN32)
        set(TEMP_PATHS $ENV{PROGRAMFILES}/* ${CMAKE_SOURCE_DIR}/.. $ENV{HOMEPATH}/* )
    elseif(APPLE)
        set(TEMP_PATHS /Applications/* ${CMAKE_SOURCE_DIR}/.. $ENV{HOME}/*)
    else()
        set(TEMP_PATHS ${CMAKE_SOURCE_DIR}/.. $ENV{HOME}/*)
    endif()

    find_path(FEBio_SDK "include/FECore/stdafx.h"
        PATHS ${TEMP_PATHS}
        PATH_SUFFIXES sdk
        DOC "Path to the FEBio SDK, or git repo.")
endif()

if(NOT FEBio_SDK)
    set(FEBio_SDK "" CACHE PATH "Path to the FEBio SDK, or git repo.")
    message(FATAL_ERROR "Unable to find path to FEBio SDK or git repo automatically. Please set FEBio_SDK to the path to your FEBio SDK or git repo.")
endif()

# Only update the include and lib directories if the FEBio_SDK path has been changed.
if(NOT OLD_SDK)
    set(NEWPATH TRUE)
else()
    #cmake_path(CONVERT ${OLD_SDK} TO_CMAKE_PATH_LIST STD_OLD_SDK)
    #cmake_path(CONVERT ${FEBio_SDK} TO_CMAKE_PATH_LIST STD_FEBIO_SDK)
    #string(COMPARE NOTEQUAL ${STD_FEBIO_SDK} ${STD_OLD_SDK} NEWPATH)
    string(COMPARE NOTEQUAL ${FEBio_SDK} ${OLD_SDK} NEWPATH)
endif()

if(NEWPATH)
    # Is this the SDK?
    string(REGEX MATCH "sdk" IS_SDK ${FEBio_SDK})

    set(LIB_SUFFIXES "")
    if(IS_SDK)
        set(FEBio_INC "${FEBio_SDK}/include" CACHE PATH "Path to FEBio include directory." FORCE)

        if(WIN32)
            list(APPEND LIB_SUFFIXES "lib" "lib/Release"  "vs2017/Release" "vs2017/Debug")
        else()
            list(APPEND LIB_SUFFIXES "lib")
        endif()
    else()
        set(FEBio_INC ${FEBio_SDK} CACHE PATH "Path to FEBio include directory." FORCE)

        if(WIN32)
            list(APPEND LIB_SUFFIXES "lib" "lib/Release" "cmbuild/lib/Release" "cmbuild/lib/Debug" "cbuild/lib/Release" "cbuild/lib/Debug" "build/lib/Release" "build/lib/Debug")
        else()
            list(APPEND LIB_SUFFIXES "cbuild/lib" "cmbuild/lib" "build/lib" "cbuild/Release/lib" "cmbuild/Release/lib" "build/Release/lib" "cbuild/Debug/lib" "cmbuild/Debug/lib" "build/Debug/lib")
        endif()
    endif()

    mark_as_advanced(FEBio_INC)

    # Find lib path
    find_library(FECORE
        NAMES FECore fecore fecore_gcc64 fecore_lnx64
        PATHS ${FEBio_SDK}
        PATH_SUFFIXES ${LIB_SUFFIXES}
        DOC "FEBio library path")

    if(FECORE)
        get_filename_component(FECORE_TEMP ${FECORE} DIRECTORY)
        set(FEBio_LIB_DIR ${FECORE_TEMP} CACHE PATH "Path to the FEBio lib directory." FORCE)
        mark_as_advanced(FEBio_LIB_DIR)
        unset(FECORE_TEMP)
        unset(FECORE CACHE)
    else()
        set(FEBio_LIB_DIR CACHE PATH "Path to the FEBio lib directory." FORCE)
        message(SEND_ERROR "Unable to find FEBio Library path automatically. Set FEBio_LIB_DIR.")
        unset(FECORE CACHE)
    endif()
endif()

set(OLD_SDK ${FEBio_SDK} CACHE INTERNAL "Old SDK path.")
mark_as_advanced(OLD_SDK)

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
        PATH_SUFFIXES "build/lib" "cbuild/lib" "src/build/lib" "src/cbuild/lib" "Release" "Debug" "lib"
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

set(MMG_DBG_LIB_DIR CACHE PATH "Path to the MMG debug lib directory")
mark_as_advanced(MMG_DBG_LIB_DIR)

# TETGEN
if(WIN32)
  find_path(TETGEN_INC tetgen.h
        PATHS C:/Program\ Files/* $ENV{HOMEPATH}/* $ENV{HOMEPATH}/*/*
        PATH_SUFFIXES "include" "include/tetgen*" "src" "build" "build/include"
        DOC "TetGen include directory")
  find_library(TETGEN_LIB tet tetgen
        PATHS C:/Program\ Files/* $ENV{HOMEPATH}/* $ENV{HOMEPATH}/*/*
        PATH_SUFFIXES "lib" "build/lib" "cmbuild/lib" "src/build/lib" "src/cmbuild/lib" "x64/Release" "x64/Debug"
        DOC "TetGen library path")
else()
	find_path(TETGEN_INC tetgen.h
        PATHS /opt/tetgen* $ENV{HOME}/* $ENV{HOME}/*/*
        PATH_SUFFIXES "include" "include/tetgen*" "src" "build" "build/include"
        DOC "TetGen include directory")
	find_library(TETGEN_LIB tet
        PATHS /opt/tetgen* $ENV{HOME}/* $ENV{HOME}/*/*
        PATH_SUFFIXES "build" "cbuild" "build/lib" "cmbuild/lib" "src/build/lib" "src/cmbuild/lib" "Release" "Debug" "lib"
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

set(TETGEN_DBG_LIB_DIR CACHE PATH "Path to the TET debug lib directory")
mark_as_advanced(TETGEN_DBG_LIB_DIR)

# OpenCascade
if(WIN32)
    find_path(OCCT_INC gp_Pnt.hxx
        PATHS C:/Program\ Files/* $ENV{HOMEPATH}/* $ENV{HOMEPATH}/*/*
        PATH_SUFFIXES "/opencascade" "include/opencascade" "opencascade/include/opencascade" "build/opencascade" "build/include/opencascade" "inc"
        DOC "OpenCascade include directory")
    find_library(OCCT_LIB TKernel
        PATHS C:/Program\ Files/* $ENV{HOMEPATH}/* $ENV{HOMEPATH}/*/*
        PATH_SUFFIXES "build/lib" "cmbuild/lib" "src/build/lib" "src/cmbuild/lib" "Release" "Debug" "win64/vc14/lib"
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

set(OCCT_DBG_LIB_DIR CACHE PATH "Path to the OpenCascade debug lib directory")
mark_as_advanced(OCCT_DBG_LIB_DIR)

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
        PATHS $ENV{HOME}/* $ENV{HOME}/*/* /usr/local/x86_64/Contents/Resources/include /opt/netgen* /Applications/Netgen.app /usr/include/*
        PATH_SUFFIXES "include" "netgen/include" "build" "build/include" "occ" "Contents/Resources/include"
    DOC "Netgen include directory")
	find_library(NETGEN_LIB nglib
        PATHS /opt/netgen* $ENV{HOME}/* $ENV{HOME}/*/* /Applications/Netgen.app /usr/lib/*
        PATH_SUFFIXES "lib" "netgen/lib" "build" "build/lib" "Release" "Debug" "MacOS" "Contents/MacOS"
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

set(NETGEN_DBG_LIB_DIR CACHE PATH "Path to the Netgen debug lib directory")
mark_as_advanced(NETGEN_DBG_LIB_DIR)

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
    set(SSH_LIB_DIR ${SSH_TEMP} CACHE PATH "Path to the LibSSH lib directory (e.g. /opt/libssh/lib)" FORCE)
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

set(SSH_DBG_LIB_DIR CACHE PATH "Path to the LibSSH debug lib directory")
mark_as_advanced(SSH_DBG_LIB_DIR)

# OpenSSL
if(WIN32)
  find_path(SSL_INC openssl/aes.h
    PATHS C:/Program\ Files/* $ENV{HOMEPATH}/* $ENV{HOMEPATH}/*/*
    PATH_SUFFIXES "include" "openssl/include" "build" "build/include"
    DOC "OpenSSL include directory")

  find_library(SSL_LIB libssl ssl
    PATHS C:/Program\ Files/* $ENV{HOMEPATH}/* $ENV{HOMEPATH}/*/*
    PATH_SUFFIXES "lib" "build/lib" "cmbuild/lib" "src/build/lib" "src/cmbuild/lib" "Release" "Debug"
    DOC "OpenSSL library path")
elseif(APPLE)
	find_path(SSL_INC openssl/aes.h
        PATHS /opt/openssl /usr/local/opt/ $ENV{HOME}/* $ENV{HOME}/*/*
        PATH_SUFFIXES "include" "openssl/include" "build" "build/include"
		DOC "OpenSSL include directory")
	find_library(SSL_LIB ssl
        PATHS /opt/openssl /usr/local/opt/ $ENV{HOME}/* $ENV{HOME}/*/*
        PATH_SUFFIXES "lib" "openssl/lib" "build" "build/lib" "Release" "Debug" "homebrew/lib"
		DOC "OpenSSL library path")
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
    set(SSL_LIB_DIR ${SSL_TEMP} CACHE PATH "Path to the OpenSSL lib directory (e.g. /opt/openssl/lib)" FORCE)
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

set(SSL_DBG_LIB_DIR CACHE PATH "Path to the OpenSSL debug lib directory")
mark_as_advanced(SSL_DBG_LIB_DIR)

# LibZip
if(WIN32)
  find_path(LIBZIP_INC zip.h
    PATHS C:/Program\ Files/* $ENV{HOMEPATH}/* $ENV{HOMEPATH}/*/*
    PATH_SUFFIXES "include" "/libzip" "include/libzip" "libzip/include/libzip" "build/libzip" "build/include/libzip"
    DOC "LibZip include directory")
  find_library(LIBZIP_LIB zip
    PATHS C:/Program\ Files/* $ENV{HOMEPATH}/* $ENV{HOMEPATH}/*/*
    PATH_SUFFIXES "lib" "build/lib" "cmbuild/lib" "src/build/lib" "src/cmbuild/lib" "Release" "Debug"
    DOC "LibZip library path")
elseif(APPLE)
	find_path(LIBZIP_INC zip.h
        PATHS /usr/include/* /opt/libzip $ENV{HOME}/* $ENV{HOME}/*/*
        PATH_SUFFIXES "/libzip" "include/libzip" "libzip/include/libzip" "build/libzip" "build/include/libzip" "include"
		DOC "LibZip include directory")
	find_library(LIBZIP_LIB zip
        PATHS /opt/libzip $ENV{HOME}/* $ENV{HOME}/*/*
        PATH_SUFFIXES "lib" "libzip/lib" "build" "build/lib" "Release" "Debug"
		DOC "LibZip library path")
else()
	find_path(LIBZIP_INC zip.h
        PATHS /opt/libzip $ENV{HOME}/* $ENV{HOME}/*/* /usr/local/include/*
        PATH_SUFFIXES "/libzip" "include/libzip" "libzip/include/libzip" "build/libzip" "build/include/libzip"
		DOC "LibZip include directory")
	find_library(LIBZIP_LIB zip
        PATHS /opt/libzip $ENV{HOME}/* $ENV{HOME}/*/* /usr/local/ /usr/local/* 
        PATH_SUFFIXES "lib" "lib64" "libzip/lib" "libzip/build/libzip" "build" "build/lib" "Release" "Debug"
		DOC "LibZip library path")
endif()

if(LIBZIP_LIB)
    get_filename_component(LIBZIP_NAME ${LIBZIP_LIB} NAME)
    get_filename_component(LIBZIP_TEMP ${LIBZIP_LIB} DIRECTORY)
    set(LIBZIP_LIB_DIR ${LIBZIP_TEMP} CACHE PATH "Path to the LibZip lib directory (e.g. /opt/libzip/lib)")
    unset(LIBZIP_TEMP)
    unset(LIBZIP_LIB CACHE)
else()
	set(LIBZIP_LIB_DIR "LIBZIP_LIB_DIR-NOTFOUND" CACHE PATH "Path to the LibZip lib directory (e.g. /opt/libzip/lib)")
    unset(LIBZIP_LIB CACHE)
endif()

if(LIBZIP_INC AND LIBZIP_LIB_DIR)		
    mark_as_advanced(LIBZIP_INC LIBZIP_LIB_DIR)
else()
    mark_as_advanced(CLEAR LIBZIP_INC LIBZIP_LIB_DIR)
endif()

set(LIBZIP_DBG_LIB_DIR CACHE PATH "Path to the LibZip debug lib directory")
mark_as_advanced(LIBZIP_DBG_LIB_DIR)

# SQLite
if(WIN32)
  find_path(SQLITE_INC sqlite3.h
    PATHS C:/Program\ Files/* $ENV{HOMEPATH}/* $ENV{HOMEPATH}/*/*
    PATH_SUFFIXES "include" "sqlite/include" "build" "build/include"
    DOC "SQLite include directory")
  find_library(SQLITE_LIB sqlite3
    PATHS C:/Program\ Files/* $ENV{HOMEPATH}/* $ENV{HOMEPATH}/*/*
    PATH_SUFFIXES "build/lib" "cmbuild/lib" "src/build/lib" "src/cmbuild/lib" "Release" "Debug" "lib"
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
    set(SQLITE_LIB_DIR ${SQLITE_TEMP} CACHE PATH "Path to the SQLite lib directory (e.g. /opt/sqlite/lib)" FORCE)
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

if(SQLITE_INC AND SQLITE_LIB_DIR AND LIBZIP_INC AND LIBZIP_LIB_DIR)		
	option(MODEL_REPO "Build code to connect to the Model Repository." ON)
else()
    SET(MODEL_REPO OFF CACHE BOOL "Build code to connect to the Model Repository." FORCE)
endif()

set(SQLITE_DBG_LIB_DIR CACHE PATH "Path to the SQLite debug lib directory")
mark_as_advanced(SQLITE_DBG_LIB_DIR)

# FFMPEG
if(WIN32)
  find_path(FFMPEG_INC libavformat/libavformat.h libavformat/avformat.h
    PATHS C:/Program\ Files/* $ENV{HOMEPATH}/* $ENV{HOMEPATH}/*/*
    PATH_SUFFIXES "include" "ffmpeg/include" "build" "build/include"
    DOC "FFMPEG include directory")
  find_library(FFMPEG_LIB avformat
    PATHS C:/Program\ Files/* $ENV{HOMEPATH}/* $ENV{HOMEPATH}/*/*
    PATH_SUFFIXES "build/lib" "cmbuild/lib" "src/build/lib" "src/cmbuild/lib" "Release" "Debug"
    DOC "FFMPEG library path")
elseif(APPLE)
	find_path(FFMPEG_INC libavformat/avformat.h
        PATHS /usr/local/homebrew /usr/local/opt/ /opt/ffmpeg $ENV{HOME}/* $ENV{HOME}/*/*
        PATH_SUFFIXES "include" "ffmpeg/include" "build" "build/include" "homebrew/include"
		DOC "FFMPEG include directory")
	find_library(FFMPEG_LIB avformat
        PATHS /usr/local/homebrew /usr/local/opt/ /opt/ffmpeg $ENV{HOME}/* $ENV{HOME}/*/*
        PATH_SUFFIXES "lib" "ffmpeg/lib" "build" "build/lib" "Release" "Debug" "homebrew/lib"
		DOC "FFMPEG library path")
else()
	find_path(FFMPEG_INC libavformat/avformat.h
        PATHS /usr/local/x86_64 /usr/include/ /opt/ffmpeg $ENV{HOME}/* $ENV{HOME}/*/*
        PATH_SUFFIXES "ffmpeg" "ffmpeg/include" "build" "build/include"
		DOC "FFMPEG include directory")
	find_library(FFMPEG_LIB avformat
        PATHS /usr/local/x86_64 /opt/ffmpeg $ENV{HOME}/* $ENV{HOME}/*/*
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

set(FFMPEG_DBG_LIB_DIR CACHE PATH "Path to the FFMPEG debug lib directory")
mark_as_advanced(FFMPEG_DBG_LIB_DIR)

# OpenGL
find_package(OpenGL REQUIRED)
# Use non-standard PATH_SUFFIXES on HOMEBREW macs
option(HOMEBREW "Did you use HOMEBREW to install dependencies" OFF)
if(HOMEBREW AND APPLE)
  find_package(GLEW PATH_SUFFIXES glew REQUIRED)
else()
  find_package(GLEW REQUIRED)
endif()

# ZLIB
find_package(ZLIB)
if(ZLIB_INCLUDE_DIR AND ZLIB_LIBRARY_RELEASE)
	option(USE_ZLIB "Required for reading compressed xplt files" ON)
    mark_as_advanced(ZLIB_INCLUDE_DIR ZLIB_LIBRARY_RELEASE)
else()
	option(USE_ZLIB "Required for reading compressed xplt files" OFF)
    mark_as_advanced(CLEAR ZLIB_INCLUDE_DIR ZLIB_LIBRARY_RELEASE)
endif()

# OpenMP
if(NOT WIN32)
    find_package(OpenMP QUIET)
endif()

# SITK
find_package(SimpleITK QUIET)

if(DEFINED SimpleITK_FOUND)
	option(USE_ITK "Required to import most image files." ON)
else()
	option(USE_ITK "Required to import most image files." OFF)
endif()

