cmake_minimum_required(VERSION 3.18.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(fbs)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_FILES_DIRECTORY ${CMAKE_BINARY_DIR}/CMakeFiles)

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

if(NOT WIN32)
    find_package(OpenMP QUIET)
endif()

if(WIN32)
    add_definitions(-DWIN32)
    add_compile_options(/MP /openmp)
elseif(APPLE)
    add_definitions(-D__APPLE__)
    set(CMAKE_OSX_DEPLOYMENT_TARGET 10.13)
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

find_package(ZLIB)

target_compile_definitions(XPLTLib PRIVATE HAVE_ZLIB)
target_include_directories(XPLTLib PRIVATE ${ZLIB_INCLUDE_DIR})
target_link_libraries(XPLTLib PRIVATE ${ZLIB_LIBRARY_RELEASE})

pybind11_add_module(fbs PyFBS.cpp PyFBSCore.cpp PyFBSPost.cpp PyFBSMesh.cpp PyFBSGeom.cpp)

if(WIN32)
    target_link_libraries(fbs PRIVATE ${FEBIOSTUDIO_LIBS} ${FEBIO_LIBS})
    target_link_libraries(fbs PRIVATE psapi.lib ws2_32.lib)
elseif(APPLE)
    target_link_libraries(fbs PRIVATE ${FEBIOSTUDIO_LIBS} ${FEBIO_LIBS})
else()
	target_link_libraries(fbs PRIVATE -Wl,--start-group ${FEBIOSTUDIO_LIBS} ${FEBIO_LIBS} -Wl,--end-group)
endif()
