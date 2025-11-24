cmake_minimum_required(VERSION 3.18.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(fbs)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY AUTOGEN_TARGETS_FOLDER Autogen)

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

# OpenMP
if(NOT WIN32)
    find_package(OpenMP QUIET)
endif()

# Find pybind11
if(WIN32)
	find_path(PYBIND11_DIR include/pybind11/pybind11.h
        PATHS C:/Program\ Files/* $ENV{HOMEPATH}/* $ENV{HOMEPATH}/*/*
		PATH_SUFFIXES "pybind11"
        DOC "pybind11 directory")
elseif(APPLE)
	find_path(PYBIND11_DIR include/pybind11/pybind11.h
        PATHS /usr/include/ /opt/pybind11 $ENV{HOME}/* $ENV{HOME}/*/*
        PATH_SUFFIXES "pybind11"
		DOC "pybind11 directory"
		NO_DEFAULT_PATH)
else()
	find_path(PYBIND11_DIR include/pybind11/pybind11.h
        PATHS /usr/include/ /opt/pybind11 $ENV{HOME}/* $ENV{HOME}/*/*
        PATH_SUFFIXES "pybind11" 
		DOC "pybind11 directory")
endif()

if(NOT PYBIND11_DIR)
    message(FATAL_ERROR "Could not automatically locate pybind11. Please specify pybind11 directory in PYBIND11_DIR")
endif()

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

function(findLib libDir libOut libName)
    # Optional arguments ARGV3-ARVG6 can be used as alernative names for the library
    find_library(TEMP NAMES ${libName} ${ARGV3} ${ARGV4} ${ARGV5} ${ARGV6}
        PATHS ${${libDir}} NO_DEFAULT_PATH)
    
    if(TEMP)
        set(${libOut} ${TEMP} PARENT_SCOPE)
        unset(TEMP CACHE)
    else()
        if(WIN32)
            message(SEND_ERROR "Could not find ${libName}.lib. Check ${libDir}.")
        elseif(APPLE)
            message(SEND_ERROR "Could not find lib${libName}.so, lib${libName}.a, or lib${libName}.dylib Check ${libDir}.")
        else()
            message(SEND_ERROR "Could not find lib${libName}.so, or lib${libName}.a. Check ${libDir}")
        endif()
        unset(TEMP CACHE)
    endif()
endfunction()

if(USE_TETGEN)
    findLib(TETGEN_LIB_DIR TETGEN_LIB tet tetgen)
    
    if(TETGEN_DBG_LIB_DIR STREQUAL "")
        unset(TETGEN_DBG_LIB)
    else()
        findLib(TETGEN_DBG_LIB_DIR TETGEN_DBG_LIB tet tetgen)
    endif()
endif()

if(WIN32)
    add_definitions(-DWIN32)
    add_compile_options(/MP /openmp)
elseif(APPLE)
    add_definitions(-D__APPLE__)
    set(CMAKE_OSX_DEPLOYMENT_TARGET 11.0)
    add_compile_options(-fPIC)

    if(${OpenMP_CXX_FOUND})
        add_compile_options(-Xpreprocessor -fopenmp -w)
        link_libraries(${OpenMP_CXX_LIBRARIES})
    endif()
else()
	add_definitions(-DLINUX)
    add_compile_options(-w -Wall -fPIC)
    
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libstdc++")
    
    if(${OpenMP_CXX_FOUND})
        add_compile_options(-fopenmp)
        link_libraries(${OpenMP_CXX_LIBRARIES})
    endif()
endif()

set(FEBioDir "" CACHE PATH "Path to the FEBio repo")

include_directories(.)
include_directories(..)
include_directories(${FEBioDir})

add_definitions(-DHAS_PYTHON -DPY_EXTERNAL)

macro(findHdrSrc name)
	file(GLOB HDR_${name} "../${name}/*.h")
	file(GLOB SRC_${name} "../${name}/*.cpp")
endmacro()

macro(findHdrSrcFEBio name)
	file(GLOB HDR_${name} "${FEBioDir}/${name}/*.h")
	file(GLOB SRC_${name} "${FEBioDir}/${name}/*.cpp")
endmacro()

set(FEBIOSTUDIO_LIBS GeomLib MeshIO MeshTools MeshLib FEMLib FSCore PostLib XPLTLib FEBioLink GLLib)
set(FEBIO_LIBS FECore FEBioLib FEBioMech FEBioXML FEBioPlot XML NumCore FEBioTest FEBioFluid FEBioMix)

foreach(name IN LISTS FEBIOSTUDIO_LIBS)
	findHdrSrc(${name})
endforeach(name)

foreach(name IN LISTS FEBIO_LIBS)
	findHdrSrcFEBio(${name})
endforeach(name)

macro(addLib name)
	add_library(${name} ${HDR_${name}} ${SRC_${name}})
	set_property(TARGET ${name} PROPERTY AUTOGEN_BUILD_DIR ${CMAKE_BINARY_DIR}/CMakeFiles/AutoGen/${name}_autogen)
endmacro()

foreach(name IN LISTS FEBIOSTUDIO_LIBS)
	addLib(${name})
endforeach(name)

foreach(name IN LISTS FEBIO_LIBS)
	addLib(${name})
endforeach(name)

add_subdirectory(${PYBIND11_DIR} ./pybind11)

# Manually pull in include dirs for OpenMP on macOS
if(APPLE)
    target_include_directories(FEBioMech PRIVATE ${OpenMP_C_INCLUDE_DIRS} ${OMP_INC})
    target_include_directories(GLLib PRIVATE ${OpenMP_C_INCLUDE_DIRS} ${OMP_INC})
endif()

find_package(ZLIB)

target_compile_definitions(XPLTLib PRIVATE HAVE_ZLIB)
target_include_directories(XPLTLib PRIVATE ${ZLIB_INCLUDE_DIR})
target_link_libraries(XPLTLib PRIVATE ${ZLIB_LIBRARY_RELEASE})

pybind11_add_module(fbs PyFBS.cpp PyFBSCore.cpp PyFBSPost.cpp PyFBSMesh.cpp PyFBSGeom.cpp)

if(USE_TETGEN)
	target_include_directories(MeshTools PRIVATE ${TETGEN_INC})
    target_compile_definitions(MeshTools PRIVATE TETLIBRARY)

	target_link_libraries(fbs PRIVATE optimized ${TETGEN_LIB})
    
    if(DEFINED TETGEN_DBG_LIBS)
        target_link_libraries(fbs PRIVATE debug ${TETGEN_DBG_LIB})
    else()
        target_link_libraries(fbs PRIVATE debug ${TETGEN_LIB})
    endif()
endif()

if(WIN32)
    target_link_libraries(fbs PRIVATE ${FEBIOSTUDIO_LIBS} ${FEBIO_LIBS})
    target_link_libraries(fbs PRIVATE psapi.lib ws2_32.lib)
elseif(APPLE)
    target_link_libraries(fbs PRIVATE ${FEBIOSTUDIO_LIBS} ${FEBIO_LIBS})
else()
	target_link_libraries(fbs PRIVATE -Wl,--start-group ${FEBIOSTUDIO_LIBS} ${FEBIO_LIBS} -Wl,--end-group)
endif()
