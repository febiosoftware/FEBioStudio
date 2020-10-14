cmake_minimum_required(VERSION 3.1.0)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(FEBioStudio)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_FILES_DIRECTORY ${CMAKE_BINARY_DIR}/CMakeFiles)

# Set a default build type if none was specified
set(default_build_type "Release")
 
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
  set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE
      STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
    "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

if(NOT Qt5_DIR)
    if(NOT Qt5_ROOT)
        set(Qt5_ROOT "" CACHE PATH "Path to the root Qt directory (e.g. /opt/Qt)")
    endif()
    
    if(WIN32)
    
    elseif(APPLE)
		find_file(Qt5_CMAKE cmake/Qt5/Qt5Config.cmake
			PATHS ${Qt5_ROOT}/* ${Qt5_ROOT}/*/* /opt/Qt/* /opt/Qt*/* /usr/local/Qt/* /usr/local/Qt*/* $ENV{HOME}/Qt/* $ENV{HOME}/*/Qt/* /Qt/* /Qt*/* 
			PATH_SUFFIXES "clang_64/lib"
			DOC "Qt5 CMake Directory")
    else()
		find_file(Qt5_CMAKE cmake/Qt5/Qt5Config.cmake
			PATHS ${Qt5_ROOT}/* ${Qt5_ROOT}/*/* /opt/Qt/* /opt/Qt*/* /usr/local/Qt/* /usr/local/Qt*/* $ENV{HOME}/Qt/* $ENV{HOME}/*/Qt/* /Qt/* /Qt*/* 
			PATH_SUFFIXES "gcc_64/lib"
			DOC "Qt5 CMake Directory")
    endif()
        
    if(Qt5_CMAKE)
        get_filename_component(Qt5_TEMP ${Qt5_CMAKE} DIRECTORY)
        set(Qt5_DIR ${Qt5_TEMP} CACHE PATH "Path to the root Qt directory (e.g. /opt/Qt)")
        unset(Qt5_TEMP)
        unset(Qt5_CMAKE CACHE)
    endif()
    
    if(NOT Qt5_DIR)
        message(FATAL_ERROR "Unable to locate Qt5Config.cmake. Please check the value of Qt5_ROOT and set it to the root directory of your Qt5 installation (e.g. /opt/Qt).")
    endif()
endif()


set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

#### Check if paths are valid and find libs ####

function(findLib libName libDir libOut)
    find_library(TEMP ${libName} PATHS ${libDir})

    # Optional argument ARGV3 can be used to pass an alternate name for the library
    if(NOT TEMP AND ${ARGC} GREATER 3)
        find_library(TEMP ${ARGV3} PATHS ${libDir})
    endif()
    
    if(TEMP)
        set(${libOut} ${TEMP} PARENT_SCOPE)
        unset(TEMP CACHE)
    else()
        if(WIN32)
            message(SEND_ERROR "Could not find ${libName}.lib. Check MMG_LIB_DIR.")
        elseif(APPLE)
            message(SEND_ERROR "Could not find ${libName}.a, ${libName}.so, or ${libName}.dylib Check MMG_LIB_DIR.")
        else()
            message(SEND_ERROR "Could not find ${libName}.a, or ${libName}.so. Check MMG_LIB_DIR.")
        endif()
        unset(TEMP CACHE)
    endif()
endfunction()

if(USE_MMG)
    findLib(mmg3d ${MMG_LIB_DIR} MMG_LIB)
endif()

if(USE_TETGEN)
    findLib(tet ${TET_LIB_DIR} TET_LIB)
endif()

if(USE_NETGEN)
    set(NETGEN_LIBS "")
    
    set(NETGEN_LIB_NAMES nglib csg interface mesh occ)
    foreach(name IN LISTS NETGEN_LIB_NAMES)
        findLib(${name} ${NETGEN_LIB_DIR} NETGEN_LIB)
        list(APPEND NETGEN_LIBS ${NETGEN_LIB})
        unset(NETGEN_LIB)
    endforeach(name)
    
    unset(NETGEN_LIB_NAMES)
endif()

if(USE_OCC)
    set(OCC_LIBS "")
    
    set(OCC_LIB_NAMES TKernel TKGeomBase TKTopAlgo TKPrim TKMesh TKMath TKBRep TKSTL TKFillet TKBO TKIGES TKSTEP TKSTEPBase TKXSBase TKG3d TKLCAF TKVCAF)
    foreach(name IN LISTS OCC_LIB_NAMES)
        findLib(${name} ${OCC_LIB_DIR} OCC_LIB)
        list(APPEND OCC_LIBS ${OCC_LIB})
        unset(OCC_LIB)
    endforeach(name)
    
    unset(OCC_LIB_NAMES)
endif()

if(USE_SSH)
    findLib(ssh ${SSH_LIB_DIR} SSH_LIB)
    
    set(SSL_LIBS "")
    
    findLib(crypto ${SSL_LIB_DIR} CRYPTO_LIB)
    list(APPEND SSL_LIBS ${CRYPTO_LIB})
    unset(CRYPTO_LIB)
    
    findLib(ssl ${SSL_LIB_DIR} SSL_LIB)
    list(APPEND SSL_LIBS ${SSL_LIB})
    unset(SSL_LIB)
endif()

if(USE_QUAZIP)
    # Passing quazip5 as a possible alternate name for the library
    findLib(quazip ${QUAZIP_LIB_DIR} QUAZIP_LIB quazip5)
endif()

if(MODEL_REPO)
    findLib(sqlite3 ${SQLITE_LIB_DIR} SQLITE_LIB)
endif()

if(USE_FFMPEG)
    set(FFMPEG_LIBS "")
    
    set(FFMPEG_LIB_NAMES avformat avcodec avresample avutil swresample swscale)
    foreach(name IN LISTS FFMPEG_LIB_NAMES)
        findLib(${name} ${FFMPEG_LIB_DIR} FFMPEG_LIB)
        list(APPEND FFMPEG_LIBS ${FFMPEG_LIB})
        unset(FFMPEG_LIB)
    endforeach(name)
    
    unset(FFMPEG_LIB_NAMES)
endif()


##### Search for library and include directories #####

# Qt
find_package(Qt5 COMPONENTS Widgets Gui Network OpenGL Charts REQUIRED)
find_package(Qt5 COMPONENTS WebEngineWidgets QUIET)
mark_as_advanced(Qt5Charts_DIR Qt5Core_DIR Qt5Gui_DIR Qt5Network_DIR Qt5OpenGL_DIR Qt5Positioning_DIR 
    Qt5PrintSupport_DIR Qt5QmlModels_DIR Qt5Qml_DIR Qt5Quick_DIR Qt5WebChannel_DIR Qt5WebEngineCore_DIR 
    Qt5WebEngineWidgets_DIR Qt5Widgets_DIR)

# MMG
if(WIN32)
	find_path(MMG_INC mmg\mmg3d\libmmg3d.h
        PATHS C::/Program\ Files/* $ENV{HOMEPATH}/* $ENV{HOMEPATH}/*/*
		PATH_SUFFIXES "include" "include/mmg*" "src" "build" "build/include"
        DOC "MMG include directory")
	find_library(MMG_LIB mmg3d 
        PATHS C::/Program\ Files/* $ENV{HOMEPATH}/* $ENV{HOMEPATH}/*/*
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
	find_path(TET_INC tetgen.h
        PATHS C::/Program\ Files/* $ENV{HOMEPATH}/* $ENV{HOMEPATH}/*/*
		PATH_SUFFIXES "include" "include/tetgen*" "src" "build" "build/include"
        DOC "TetGen include directory")
	find_library(TET_LIB tet 
        PATHS C::/Program\ Files/* $ENV{HOMEPATH}/* $ENV{HOMEPATH}/*/*
        PATH_SUFFIXES "build/lib" "cmbuild/lib" "src/build/lib" "src/cmbuild/lib" "Release" "Debug"
		DOC "TetGen library path")
else()
	find_path(TET_INC tetgen.h
        PATHS /opt/tetgen* $ENV{HOME}/* $ENV{HOME}/*/*
        PATH_SUFFIXES "include" "include/tetgen*" "src" "build" "build/include"
		DOC "TetGen include directory")
	find_library(TET_LIB tet 
        PATHS /opt/tetgen* $ENV{HOME}/* $ENV{HOME}/*/*
        PATH_SUFFIXES "build" "cbuild" "build/lib" "cmbuild/lib" "src/build/lib" "src/cmbuild/lib" "Release" "Debug"
		DOC "TetGen library path")
endif()

if(TET_LIB)
    get_filename_component(TET_TEMP ${TET_LIB} DIRECTORY)
    set(TET_LIB_DIR ${TET_TEMP} CACHE PATH "Path to the TET lib directory (e.g. /opt/tetgen/lib)")
    unset(TET_TEMP)
    unset(TET_LIB CACHE)
else()
	set(TET_LIB_DIR "TET_LIB_DIR-NOTFOUND" CACHE PATH "Path to the TET lib directory (e.g. /opt/tetgen/lib)")
    unset(TET_LIB CACHE)
endif()

if(TET_INC AND TET_LIB_DIR)		
	option(USE_TETGEN "Required for adaptive remeshing" ON)
    mark_as_advanced(TET_INC TET_LIB_DIR)
else()
	option(USE_TETGEN "Required for adaptive remeshing" OFF)
    mark_as_advanced(CLEAR TET_INC TET_LIB_DIR)
endif()

# NETGEN
if(WIN32)
	find_path(NETGEN_INC include/occgeom.hpp
        PATHS C::/Program\ Files/* $ENV{HOMEPATH}/* $ENV{HOMEPATH}/*/*
		PATH_SUFFIXES "include" "netgen/include" "build" "build/include"
        DOC "Netgen include directory")
	find_library(NETGEN_LIB nglib 
        PATHS C::/Program\ Files/* $ENV{HOMEPATH}/* $ENV{HOMEPATH}/*/*
        PATH_SUFFIXES "build/lib" "cmbuild/lib" "src/build/lib" "src/cmbuild/lib" "Release" "Debug"
		DOC "Netgen library path")
else()
	find_path(NETGEN_INC include/occgeom.hpp
        PATHS /opt/netgen* $ENV{HOME}/* $ENV{HOME}/*/*
        PATH_SUFFIXES "include" "netgen/include" "build" "build/include"
		DOC "Netgen include directory")
	find_library(NETGEN_LIB nglib 
        PATHS /opt/netgen* $ENV{HOME}/* $ENV{HOME}/*/*
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
	find_path(OCC_INC gp_Pnt.hxx
        PATHS C::/Program\ Files/* $ENV{HOMEPATH}/* $ENV{HOMEPATH}/*/*
		PATH_SUFFIXES "/opencascade" "include/opencascade" "opencascade/include/opencascade" "build/opencascade" "build/include/opencascade"
        DOC "OpenCascade include directory")
	find_library(OCC_LIB TKernel 
        PATHS C::/Program\ Files/* $ENV{HOMEPATH}/* $ENV{HOMEPATH}/*/*
        PATH_SUFFIXES "build/lib" "cmbuild/lib" "src/build/lib" "src/cmbuild/lib" "Release" "Debug"
		DOC "OpenCascade library path")
else()
	find_path(OCC_INC gp_Pnt.hxx
        PATHS /opt/opencascade $ENV{HOME}/* $ENV{HOME}/*/*
        PATH_SUFFIXES "/opencascade" "include/opencascade" "opencascade/include/opencascade" "build/opencascade" "build/include/opencascade"
		DOC "OpenCascade include directory")
	find_library(OCC_LIB TKernel 
        PATHS /opt/opencascade $ENV{HOME}/* $ENV{HOME}/*/*
        PATH_SUFFIXES "lib" "opencascade/lib" "build" "build/lib" "Release" "Debug"
		DOC "OpenCascade library path")
endif()

if(OCC_LIB)
    get_filename_component(OCC_TEMP ${OCC_LIB} DIRECTORY)
    set(OCC_LIB_DIR ${OCC_TEMP} CACHE PATH "Path to the OpenCascade lib directory (e.g. /opt/opencascade/lib)")
    unset(OCC_TEMP)
    unset(OCC_LIB CACHE)
else()
	set(OCC_LIB_DIR "OCC_LIB_DIR-NOTFOUND" CACHE PATH "Path to the OpenCascade lib directory (e.g. /opt/opencascade/lib)")
    unset(OCC_LIB CACHE)
endif()

if(OCC_INC AND OCC_LIB_DIR)		
	option(USE_OCC "Required for importing and meshing CAD objects." ON)
    mark_as_advanced(OCC_INC OCC_LIB_DIR)
else()
	option(USE_OCC "Required for importing and meshing CAD objects." OFF)
    mark_as_advanced(CLEAR OCC_INC OCC_LIB_DIR)
endif()

# LibSSH
if(WIN32)
	find_path(SSH_INC libssh/libssh.h
        PATHS C::/Program\ Files/* $ENV{HOMEPATH}/* $ENV{HOMEPATH}/*/*
		PATH_SUFFIXES "include" "libssh/include" "build" "build/include"
        DOC "LibSSH include directory")
	find_library(SSH_LIB ssh 
        PATHS C::/Program\ Files/* $ENV{HOMEPATH}/* $ENV{HOMEPATH}/*/*
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
        PATHS C::/Program\ Files/* $ENV{HOMEPATH}/* $ENV{HOMEPATH}/*/*
		PATH_SUFFIXES "include" "openssl/include" "build" "build/include"
        DOC "OpenSSL include directory")
	find_library(SSL_LIB ssl 
        PATHS C::/Program\ Files/* $ENV{HOMEPATH}/* $ENV{HOMEPATH}/*/*
        PATH_SUFFIXES "build/lib" "cmbuild/lib" "src/build/lib" "src/cmbuild/lib" "Release" "Debug"
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
        PATHS C::/Program\ Files/* $ENV{HOMEPATH}/* $ENV{HOMEPATH}/*/*
		PATH_SUFFIXES "/quazip5" "include/quazip5" "quazip/include/quazip5" "build/quazip5" "build/include/quazip5"
        DOC "QuaZip include directory")
	find_library(QUAZIP_LIB quazip5 
        PATHS C::/Program\ Files/* $ENV{HOMEPATH}/* $ENV{HOMEPATH}/*/*
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
        PATHS C::/Program\ Files/* $ENV{HOMEPATH}/* $ENV{HOMEPATH}/*/*
		PATH_SUFFIXES "include" "sqlite/include" "build" "build/include"
        DOC "SQLite include directory")
	find_library(SQLITE_LIB sqlite3 
        PATHS C::/Program\ Files/* $ENV{HOMEPATH}/* $ENV{HOMEPATH}/*/*
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
        PATHS C::/Program\ Files/* $ENV{HOMEPATH}/* $ENV{HOMEPATH}/*/*
		PATH_SUFFIXES "include" "ffmpeg/include" "build" "build/include"
        DOC "FFMPEG include directory")
	find_library(FFMPEG_LIB avformat 
        PATHS C::/Program\ Files/* $ENV{HOMEPATH}/* $ENV{HOMEPATH}/*/*
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
include(FindOpenGL)

# ZLIB
include(FindZLIB)
if(ZLIB_INCLUDE_DIR AND ZLIB_LIBRARY_RELEASE)
    mark_as_advanced(ZLIB_INCLUDE_DIR ZLIB_LIBRARY_RELEASE)
endif()

##### Set appropriate defines and includes #####

if(WIN32)
    add_definitions(-DWIN32)
elseif(APPLE)
    add_definitions(-D__APPLE__)
else()
	add_definitions(-DLINUX)
    add_compile_options(-static-libstdc++ -static-libgcc -w -Wall)
endif()

include_directories(.)

if(USE_MMG)
	add_definitions(-DHAS_MMG)
	include_directories(${MMG_INC})
endif()

if(USE_TETGEN)
	add_definitions(-DTETLIBRARY)
	include_directories(${TET_INC})
endif()

if(USE_NETGEN)
	add_definitions(-DHAS_NETGEN)
	include_directories(${NETGEN_INC})
    include_directories(${NETGEN_INC}/include)
endif()

if(USE_OCC)
	add_definitions(-DHAS_OCC)
	include_directories(${OCC_INC})
endif()

if(USE_SSH)
	add_definitions(-DHAS_SSH)
	include_directories(${SSH_INC} ${SSL_INC})
endif()

if(USE_QUAZIP)
	add_definitions(-DHAS_QUAZIP)
	include_directories(${QUAZIP_INC})
endif()

if(MODEL_REPO)
	add_definitions(-DMODEL_REPO)
	include_directories(${SQLITE_INC})
endif()

if(USE_FFMPEG)
	add_definitions(-DFFMPEG)
	include_directories(${FFMPEG_INC})
endif()

if(Qt5WebEngineWidgets_FOUND)
    add_definitions(-DWEBHELP)
endif()

include_directories(${ZLIB_INCLUDE_DIR})

##### Find Source Files #####

file(GLOB SRC_FEBioStudio "FEBioStudio/*.cpp")
file(GLOB HDR_FEBioStudio "FEBioStudio/*.h")
file(GLOB SRC_Abaqus "Abaqus/*.cpp")
file(GLOB SRC_Ansys "Ansys/*.cpp")
file(GLOB SRC_Comsol "Comsol/*.cpp")
file(GLOB SRC_CUILib "CUILib/*.cpp")
file(GLOB HDR_CUILib "CUILib/*.h")
file(GLOB SRC_FEBio "FEBio/*.cpp")
file(GLOB SRC_FEMLib "FEMLib/*.cpp")
file(GLOB SRC_FSCore "FSCore/*.cpp")
file(GLOB SRC_GeomLib "GeomLib/*.cpp")
file(GLOB SRC_GLLib "GLLib/*.cpp")
file(GLOB SRC_GLWLib "GLWLib/*.cpp")
file(GLOB SRC_ImageLib "ImageLib/*.cpp")
file(GLOB SRC_LSDyna "LSDyna/*.cpp")
file(GLOB SRC_MathLib "MathLib/*.cpp")
file(GLOB SRC_MeshIO "MeshIO/*.cpp")
file(GLOB SRC_MeshLib "MeshLib/*.cpp")
file(GLOB SRC_MeshTools "MeshTools/*.cpp")
file(GLOB SRC_Nike3D "Nike3D/*.cpp")
file(GLOB SRC_PostGL "PostGL/*.cpp")
file(GLOB SRC_PostLib "PostLib/*.cpp")
file(GLOB SRC_XML "XML/*.cpp")
file(GLOB SRC_XPLTLib "XPLTLib/*.cpp")

##### Filter out existing moc and qrc files #####

list(FILTER SRC_FEBioStudio EXCLUDE REGEX "qrc_")
list(FILTER SRC_FEBioStudio EXCLUDE REGEX "moc_")
list(FILTER SRC_Abaqus EXCLUDE REGEX "moc_")
list(FILTER SRC_Ansys EXCLUDE REGEX "moc_")
list(FILTER SRC_Comsol EXCLUDE REGEX "moc_")
list(FILTER SRC_CUILib EXCLUDE REGEX "moc_")
list(FILTER SRC_FEBio EXCLUDE REGEX "moc_")
list(FILTER SRC_FEMLib EXCLUDE REGEX "moc_")
list(FILTER SRC_FSCore EXCLUDE REGEX "moc_")
list(FILTER SRC_GeomLib EXCLUDE REGEX "moc_")
list(FILTER SRC_GLLib EXCLUDE REGEX "moc_")
list(FILTER SRC_GLWLib EXCLUDE REGEX "moc_")
list(FILTER SRC_ImageLib EXCLUDE REGEX "moc_")
list(FILTER SRC_LSDyna EXCLUDE REGEX "moc_")
list(FILTER SRC_MathLib EXCLUDE REGEX "moc_")
list(FILTER SRC_MeshIO EXCLUDE REGEX "moc_")
list(FILTER SRC_MeshLib EXCLUDE REGEX "moc_")
list(FILTER SRC_MeshTools EXCLUDE REGEX "moc_")
list(FILTER SRC_Nike3D EXCLUDE REGEX "moc_")
list(FILTER SRC_PostGL EXCLUDE REGEX "moc_")
list(FILTER SRC_PostLib EXCLUDE REGEX "moc_")
list(FILTER SRC_XML EXCLUDE REGEX "moc_")
list(FILTER SRC_XPLTLib EXCLUDE REGEX "moc_")

##### Add binaries #####

# Linking the Qt libraries also pulls in the include files
link_libraries(Qt5::Core Qt5::Widgets Qt5::Gui Qt5::Network Qt5::OpenGL Qt5::Charts)

if(Qt5WebEngineWidgets_FOUND)
	link_libraries(Qt5::WebEngineWidgets)
endif()

add_executable(FEBioStudio ${SRC_FEBioStudio} ${HDR_FEBioStudio} febiostudio.qrc)
add_library(Abaqus ${SRC_Abaqus})
add_library(Ansys ${SRC_Ansys})
add_library(Comsol ${SRC_Comsol})
add_library(CUILib ${SRC_CUILib} ${HDR_CUILib})
add_library(FEBio ${SRC_FEBio})
add_library(FEMLib ${SRC_FEMLib})
add_library(FSCore ${SRC_FSCore})
add_library(GeomLib ${SRC_GeomLib})
add_library(GLLib ${SRC_GLLib})
add_library(GLWLib ${SRC_GLWLib})
add_library(ImageLib ${SRC_ImageLib})
add_library(LSDyna ${SRC_LSDyna})
add_library(MathLib ${SRC_MathLib})
add_library(MeshIO ${SRC_MeshIO})
add_library(MeshLib ${SRC_MeshLib})
add_library(MeshTools ${SRC_MeshTools})
add_library(Nike3D ${SRC_Nike3D})
add_library(PostGL ${SRC_PostGL})
add_library(PostLib ${SRC_PostLib})
add_library(XML ${SRC_XML})
add_library(XPLTLib ${SRC_XPLTLib})

##### Link Libraries #####

if(WIN32)
elseif(APPLE)
else()
    set_property(TARGET MeshTools PROPERTY CXX_STANDARD 17)
    
	target_link_libraries(FEBioStudio -Wl,--start-group)
endif()

if(USE_MMG)
	target_link_libraries(FEBioStudio ${MMG_LIB})
endif()

if(USE_TETGEN)
	target_link_libraries(FEBioStudio ${TET_LIB})
endif()

if(USE_NETGEN)
	target_link_libraries(FEBioStudio ${NETGEN_LIBS})
endif()

if(USE_OCC)
    target_link_libraries(FEBioStudio ${OCC_LIBS})
endif()

if(USE_SSH)
	target_link_libraries(FEBioStudio ${SSH_LIB})
    target_link_libraries(FEBioStudio ${SSL_LIBS})
endif()

if(USE_QUAZIP)
    target_link_libraries(FEBioStudio ${QUAZIP_LIB})
endif()

if(MODEL_REPO)
	target_link_libraries(FEBioStudio ${SQLITE_LIB})
endif()

if(USE_FFMPEG)
    target_link_libraries(FEBioStudio ${FFMPEG_LIBS})
endif()

target_link_libraries(FEBioStudio ${ZLIB_LIBRARY_RELEASE})
target_link_libraries(FEBioStudio ${OPENGL_gl_LIBRARY} ${OPENGL_glu_LIBRARY})

target_link_libraries(FEBioStudio Abaqus Ansys GeomLib MathLib XML MeshIO FEBio Comsol LSDyna MeshTools FEMLib MeshLib Nike3D ImageLib  GLLib GLWLib PostGL XPLTLib PostLib CUILib FSCore)

if(WIN32)
elseif(APPLE)
else()
    target_link_libraries(FEBioStudio -Wl,--end-group)
    target_link_libraries(FEBioStudio -Wl,-rpath,\'\$ORIGIN/../lib/\')
    
    ##### Create qt.conf #####
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/qt.conf "[Paths]
    Plugins = ../lib")
endif()


