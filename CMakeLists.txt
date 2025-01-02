cmake_minimum_required(VERSION 3.5.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(FEBioStudio)

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

if(NOT Qt_Root)
    set(Qt_Root "" CACHE PATH "Path to the root Qt directory (e.g. /opt/Qt)")
endif()

if(NOT Qt6_DIR OR NOT CMAKE_PREFIX_PATH)
    if(WIN32)
        find_path(Qt_TEMP Qt6Config.cmake
            PATHS ${Qt_Root}/ ${Qt_Root}/* ${Qt_Root}/*/* C:/Qt/* C:/Qt*/* $ENV{HOME}/Qt/* $ENV{HOME}/*/Qt/* /Qt/* /Qt*/*
            PATH_SUFFIXES "lib/cmake/Qt6" "msvc2019_64/lib/cmake/Qt6"
            DOC "Qt CMake Directory"
            NO_DEFAULT_PATH)
    elseif(APPLE)
        find_path(Qt_TEMP Qt6Config.cmake
            PATHS ${Qt_Root}/ ${Qt_Root}/* ${Qt_Root}/*/* /opt/Qt/* /opt/Qt*/* /usr/local/Qt/* /usr/local/Qt*/*
              $ENV{HOME}/Qt/* $ENV{HOME}/*/Qt/* /Qt/* /Qt*/*
            PATH_SUFFIXES "lib/cmake/Qt6" "clang_64/lib/cmake/Qt6"
            DOC "Qt6 CMake Directory"
            NO_DEFAULT_PATH)
    else()
        find_path(Qt_TEMP Qt6Config.cmake
            PATHS ${Qt_Root}/ ${Qt_Root}/* ${Qt_Root}/*/* /opt/Qt/* /opt/Qt*/* /usr/local/Qt/* /usr/local/Qt*/* 
                $ENV{HOME}/Qt/* $ENV{HOME}/*/Qt6 $ENV{HOME}/*/Qt/* /Qt/* /Qt*/* 
            PATH_SUFFIXES "gcc_64/lib/cmake/Qt6" "lib/cmake/Qt6"
            DOC "Qt CMake Directory"
            NO_DEFAULT_PATH)
    endif()
    
    if(Qt_TEMP)
        set(Qt6_DIR ${Qt_TEMP} CACHE INTERNAL "Path to Qt6 installation prefix.")

        set(CMAKE_PREFIX_PATH ${Qt_TEMP}/../../.. CACHE INTERNAL "Qt Prefix.")

        unset(Qt_TEMP CACHE)
    else()
        unset(Qt_TEMP CACHE)
    endif()
    
    if(NOT Qt6_DIR OR NOT CMAKE_PREFIX_PATH)
        message(FATAL_ERROR "Unable to locate Qt6Config.cmake. Please check the value of Qt6_Root and set it to the root directory of your Qt6 installation (e.g. /opt/Qt).")
    endif()
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

##### Search for library and include directories #####

if(WIN32)
  list(APPEND CMAKE_PREFIX_PATH ${DEPENDENCIES})
endif()

include(FindDependencies.cmake)

#### Check if paths are valid and find libs ####

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

if(USE_MMG)
    set(MMG_LIB_NAMES mmg3d mmg2d mmgs)
    foreach(name IN LISTS MMG_LIB_NAMES)
        findLib(MMG_LIB_DIR MMG_LIB ${name})
        list(APPEND MMG_LIBS ${MMG_LIB})
        unset(MMG_LIB)
    endforeach(name)
    
    if(MMG_DBG_LIB_DIR STREQUAL "")
        unset(MMG_DBG_LIBS)
    else()
        foreach(name IN LISTS MMG_LIB_NAMES)
            findLib(MMG_DBG_LIB_DIR MMG_LIB ${name})
            list(APPEND MMG_DBG_LIBS ${MMG_LIB})
            unset(MMG_LIB)
        endforeach(name)    
    endif()
endif()

if(USE_TETGEN)
    findLib(TETGEN_LIB_DIR TETGEN_LIB tet tetgen)
    
    if(TETGEN_DBG_LIB_DIR STREQUAL "")
        unset(TETGEN_DBG_LIB)
    else()
        findLib(TETGEN_DBG_LIB_DIR TETGEN_DBG_LIB tet tetgen)
    endif()
endif()

if(CAD_FEATURES)
    set(NETGEN_LIB_NAMES nglib)
    
    foreach(name IN LISTS NETGEN_LIB_NAMES)
        findLib(NETGEN_LIB_DIR NETGEN_LIB ${name})
        list(APPEND NETGEN_LIBS ${NETGEN_LIB})
        unset(NETGEN_LIB)
    endforeach(name)
    
    if(NETGEN_DBG_LIB_DIR STREQUAL "")
        unset(NETGEN_DBG_LIBS)
    else()
        foreach(name IN LISTS NETGEN_LIB_NAMES)
            findLib(NETGEN_DBG_LIB_DIR NETGEN_LIB ${name})
            list(APPEND NETGEN_DBG_LIBS ${NETGEN_LIB})
            unset(NETGEN_LIB)
        endforeach(name)    
    endif()

    set(OCCT_LIB_NAMES TKernel TKGeomBase TKTopAlgo TKPrim TKMesh TKMath TKBRep TKSTL TKFillet TKBO TKIGES TKSTEP TKSTEPBase TKXSBase TKG3d TKLCAF TKVCAF)
    foreach(name IN LISTS OCCT_LIB_NAMES)
        findLib(OCCT_LIB_DIR OCCT_LIB ${name})
        list(APPEND OCCT_LIBS ${OCCT_LIB})
        unset(OCCT_LIB)
    endforeach(name)
    
    if(OCCT_DBG_LIB_DIR STREQUAL "")
        unset(OCCT_DBG_LIBS)
    else()
        foreach(name IN LISTS OCCT_LIB_NAMES)
            findLib(OCCT_DBG_LIB_DIR OCCT_LIB ${name})
            list(APPEND OCCT_DBG_LIBS ${OCCT_LIB})
            unset(OCCT_LIB)
        endforeach(name)    
    endif()
    
endif()

if(USE_SSH)
    findLib(SSH_LIB_DIR SSH_LIB ssh)
    
    if(SSH_DBG_LIB_DIR STREQUAL "")
        unset(SSH_DBG_LIB)
    else()
        findLib(SSH_DBG_LIB_DIR SSH_DBG_LIB ssh)
    endif()
    
    findLib(SSL_LIB_DIR CRYPTO_LIB crypto libcrypto)
    list(APPEND SSL_LIBS ${CRYPTO_LIB})
    unset(CRYPTO_LIB)
    
    findLib(SSL_LIB_DIR SSL_LIB ssl libssl)
    list(APPEND SSL_LIBS ${SSL_LIB})
    unset(SSL_LIB)
    
    if(SSL_DBG_LIB_DIR STREQUAL "")
        unset(SSL_DBG_LIBS)
    else()
        findLib(SSL_DBG_LIB_DIR CRYPTO_LIB crypto libcrypto)
        list(APPEND SSL_DBG_LIBS ${CRYPTO_LIB})
        unset(CRYPTO_LIB)
        
        findLib(SSL_DBG_LIB_DIR SSL_LIB ssl libssl)
        list(APPEND SSL_DBG_LIBS ${SSL_LIB})
        unset(SSL_LIB)  
    endif()
endif()

if(MODEL_REPO)
    findLib(LIBZIP_LIB_DIR LIBZIP_LIB zip)
    
    if(LIBZIP_DBG_LIB_DIR STREQUAL "")
        unset(LIBZIP_DBG_LIB)
    else()
        findLib(LIBZIP_DBG_LIB_DIR LIBZIP_DBG_LIB zip)
    endif()
	
    findLib(SQLITE_LIB_DIR SQLITE_LIB sqlite3)
    
    if(SQLITE_DBG_LIB_DIR STREQUAL "")
        unset(SQLITE_DBG_LIB)
    else()
        findLib(SQLITE_DBG_LIB_DIR SQLITE_DBG_LIB sqlite3)
    endif()
endif()

if(USE_FFMPEG)
    set(FFMPEG_LIB_NAMES avformat avcodec avutil swresample swscale)
    
    foreach(name IN LISTS FFMPEG_LIB_NAMES)
        findLib(FFMPEG_LIB_DIR FFMPEG_LIB ${name})
        list(APPEND FFMPEG_LIBS ${FFMPEG_LIB})
        unset(FFMPEG_LIB)
    endforeach(name)
    
    if(FFMPEG_DBG_LIB_DIR STREQUAL "")
        unset(FFMPEG_DBG_LIBS)
    else()
        foreach(name IN LISTS FFMPEG_LIB_NAMES)
            findLib(FFMPEG_DBG_LIB_DIR FFMPEG_LIB ${name})
            list(APPEND FFMPEG_DBG_LIBS ${FFMPEG_LIB})
            unset(FFMPEG_LIB)
        endforeach(name)    
    endif()
    
endif()

if(USE_ZLIB)
	if(NOT EXISTS ${ZLIB_LIBRARY_RELEASE})
		message(SEND_ERROR "Could not find ZLIB. Check ZLIB_LIBRARY_RELEASE.")
	endif()
	if(NOT EXISTS ${ZLIB_INCLUDE_DIR}/zlib.h)
		message(SEND_ERROR "Could not find zlib.h. Check ZLIB_INCLUDE_DIR.")
	endif()
endif()

# Find FEBio Libs
set(FEBio_LIB_NAMES fecore febiolib febiomech xml febioplot febiofluid febiomix feamr febioopt)

if (WIN32)
    set(FEBio_DEBUG_LIBS "")
    set(FEBio_RELEASE_LIBS "")

    foreach(name IN LISTS FEBio_LIB_NAMES)
        find_library(TEMP NAMES ${name} 
            PATHS ${FEBio_LIB_DIR} PATH_SUFFIXES Debug NO_DEFAULT_PATH)
        list(APPEND FEBio_DEBUG_LIBS ${TEMP})
        unset(TEMP CACHE)
    endforeach(name)

    foreach(name IN LISTS FEBio_LIB_NAMES)
        find_library(TEMP NAMES ${name} 
            PATHS ${FEBio_LIB_DIR} PATH_SUFFIXES Release NO_DEFAULT_PATH)
        list(APPEND FEBio_RELEASE_LIBS ${TEMP})
        unset(TEMP CACHE)
    endforeach(name)

    unset(FEBio_LIB_NAMES)
else()
    set(FEBio_LIBS "")

    foreach(name IN LISTS FEBio_LIB_NAMES)
        findLib(FEBio_LIB_DIR FEBio_LIB ${name})
        list(APPEND FEBio_LIBS ${FEBio_LIB})
        unset(FEBio_LIB)
    endforeach(name)

    unset(FEBio_LIB_NAMES)
endif()

##### Stupid OMP stuff for stupid macOS #####

if(UNIX)
    option(USE_MKL_OMP "Link to MKL OMP instead of System OMP" OFF)
    set(MKL_OMP "" CACHE PATH "Path to the MKL OMP library")
    mark_as_advanced(USE_MKL_OMP MKL_OMP)
endif()

##### Set appropriate defines and includes #####

if(WIN32)
    add_definitions(-DWIN32 -D_CRT_SECURE_NO_WARNINGS)
    add_compile_options(/MP /openmp)
elseif(APPLE)
    add_definitions(-D__APPLE__)
    set(CMAKE_OSX_DEPLOYMENT_TARGET 10.15)
    set(CMAKE_OSX_DEPLOYMENT_TARGET 10.15)
    
    if(${OpenMP_C_FOUND})
        add_compile_options(-Xpreprocessor -fopenmp -w)
    endif()
    add_compile_options(-w)
    
else()
	add_definitions(-DLINUX)
    add_compile_options(-w -Wall)
    
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libstdc++")
    
    if(${OpenMP_C_FOUND})
        add_compile_options(-fopenmp)
    endif()
    
    set(CMAKE_BUILD_RPATH_USE_LINK_PATH FALSE)
    set(CMAKE_BUILD_RPATH $ORIGIN/../lib/)
endif()

include_directories(.)

if(USE_MMG)
	add_definitions(-DHAS_MMG)
	include_directories(${MMG_INC})
endif()

if(USE_TETGEN)
	add_definitions(-DTETLIBRARY)
	include_directories(${TETGEN_INC})
endif()

if(CAD_FEATURES)
	add_definitions(-DHAS_NETGEN)
	include_directories(${NETGEN_INC})
    include_directories(${NETGEN_INC}/include)

	add_definitions(-DHAS_OCC)
	include_directories(${OCCT_INC})
endif()

if(USE_SSH)
	add_definitions(-DHAS_SSH)
	include_directories(${SSH_INC} ${SSL_INC})
endif()

if(MODEL_REPO)
	add_definitions(-DHAS_LIBZIP)
	include_directories(${LIBZIP_INC})

	add_definitions(-DMODEL_REPO)
	include_directories(${SQLITE_INC})
endif()

if(USE_FFMPEG)
	add_definitions(-DFFMPEG)
	include_directories(${FFMPEG_INC})
endif()

if(APPLE)
    include_directories(${GLEW_INCLUDE_DIR})
else()
    include_directories(${GLEW_INCLUDE_DIRS})
endif()

if(USE_PYTHON)
    include_directories(${PYBIND11_INC})
endif()

# Link ZLIB
if(USE_ZLIB)
    add_definitions(-DHAVE_ZLIB)
    include_directories(${ZLIB_INCLUDE_DIR})
endif()

if(USE_ITK)
    add_definitions(-DHAS_ITK)
    include_directories(${SimpleITK_INCLUDE_DIRS})
endif()

include_directories(${FEBio_INC})

##### Find Source Files #####

macro(findHdrSrc name)
	file(GLOB HDR_${name} "${name}/*.h")
	file(GLOB SRC_${name} "${name}/*.cpp")
endmacro()

findHdrSrc(FEBioStudio)

set(FEBIOSTUDIO_LIBS Abaqus Ansys Comsol CUILib FEBio FEMLib FSCore GeomLib GLLib OGLLib GLWLib RTLib ImageLib LSDyna MeshIO MeshLib MeshTools Nike3D PostGL PostLib XPLTLib FEBioLink FEBioApp FEBioMonitor VTKLib PyLib)

foreach(name IN LISTS FEBIOSTUDIO_LIBS)
	findHdrSrc(${name})
endforeach(name)

##### Add binaries #####

# set the name of the executable
if (WIN32)
    set(FBS_BIN_NAME FEBioStudio2)
else()
    set(FBS_BIN_NAME FEBioStudio)
endif()

if(USE_PYTHON)
    add_definitions(-DHAS_PYTHON)
    link_libraries(Python3::Python)
endif()

if(WIN32)
	add_executable(${FBS_BIN_NAME} WIN32 ${HDR_FEBioStudio} ${SRC_FEBioStudio} febiostudio.qrc)
	set_property(TARGET ${FBS_BIN_NAME} PROPERTY AUTOGEN_BUILD_DIR ${CMAKE_BINARY_DIR}/CMakeFiles/AutoGen/FEBioStudio_autogen)
    include(set_icon.cmake)
    set_icon(${FBS_BIN_NAME} icons/febiostudio.ico)

elseif(APPLE)
	# set bundle icon
	set(FBS_ICON ${CMAKE_CURRENT_SOURCE_DIR}/icons/FEBioStudio.icns)
	set(MACOSX_BUNDLE_ICON_FILE FEBioStudio.icns)
	set_source_files_properties(${FBS_ICON} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
	
	add_executable(${FBS_BIN_NAME} MACOSX_BUNDLE ${FBS_ICON} ${HDR_FEBioStudio} ${SRC_FEBioStudio} febiostudio.qrc)
	set_property(TARGET ${FBS_BIN_NAME} PROPERTY AUTOGEN_BUILD_DIR ${CMAKE_BINARY_DIR}/CMakeFiles/AutoGen/FEBioStudio_autogen)

else()
	add_executable(${FBS_BIN_NAME} ${HDR_FEBioStudio} ${SRC_FEBioStudio} febiostudio.qrc)
	set_property(TARGET ${FBS_BIN_NAME} PROPERTY AUTOGEN_BUILD_DIR ${CMAKE_BINARY_DIR}/CMakeFiles/AutoGen/FEBioStudio_autogen)
endif()

macro(addLib name)
	add_library(${name} ${HDR_${name}} ${SRC_${name}})
	set_property(TARGET ${name} PROPERTY AUTOGEN_BUILD_DIR ${CMAKE_BINARY_DIR}/CMakeFiles/AutoGen/${name}_autogen)
endmacro()

foreach(name IN LISTS FEBIOSTUDIO_LIBS)
	addLib(${name})
endforeach(name)


##### Set dev commit information #####

# Cross platform execute_process
macro(crossExecProcess OUT)
	if(WIN32)
		execute_process(COMMAND cmd.exe /c ${CMD}
			WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
			RESULT_VARIABLE SUCCESS
			OUTPUT_VARIABLE ${OUT}
			OUTPUT_STRIP_TRAILING_WHITESPACE)
	else()
		execute_process(COMMAND ${CMD}
			WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
			RESULT_VARIABLE SUCCESS
			OUTPUT_VARIABLE ${OUT}
			OUTPUT_STRIP_TRAILING_WHITESPACE)
	endif()
endmacro()

# Check for git and give warning if not available
if(EXISTS ${CMAKE_SOURCE_DIR}/.git)
    if(WIN32)
        set(CMD where git)
    else()
        set(CMD which git)
    endif()
    crossExecProcess(NULL)

    if(SUCCESS EQUAL 0)
        # Get branch name and check if we're on the master branch
        set(CMD git rev-parse --abbrev-ref HEAD)
        crossExecProcess(BRANCH_NAME)

        string(COMPARE EQUAL ${BRANCH_NAME} "master" ISMASTER)
        
        # Get the commit info and set the DEVCOMMIT macro
        if(SUCCESS EQUAL 0 AND NOT ISMASTER)
            
            set(CMD git rev-parse --short=9 HEAD)
            crossExecProcess(DEVCOMMIT)
            
            if(${SUCCESS} EQUAL 0)
                target_compile_definitions(${FBS_BIN_NAME} PUBLIC "DEVCOMMIT=\"${DEVCOMMIT}\"")
            endif()
        endif()
    else()
        message(WARNING "Could not find git in system PATH. Development version info will not be addded.")
    endif()
endif()


##### Link Libraries #####

# Linking the Qt libraries also pulls in the include files
target_link_libraries(${FBS_BIN_NAME} Qt6::Core Qt6::Widgets Qt6::Gui Qt6::Network Qt6::OpenGL Qt6::OpenGLWidgets)
target_link_libraries(CUILib Qt6::Widgets)
target_link_libraries(GLWLib Qt6::Gui Qt6::OpenGLWidgets)
target_link_libraries(PostGL Qt6::Gui)
target_link_libraries(PostLib Qt6::Gui)
target_link_libraries(GLLib Qt6::OpenGL Qt6::OpenGLWidgets)
target_link_libraries(OGLLib Qt6::OpenGL Qt6::OpenGLWidgets)
target_link_libraries(RTLib)
target_link_libraries(FSCore Qt6::Core)
target_link_libraries(MeshIO Qt6::Core)
target_link_libraries(MeshTools Qt6::Core)
target_link_libraries(ImageLib Qt6::Core Qt6::Gui)
target_link_libraries(PyLib Qt6::Core Qt6::Gui Qt6::OpenGLWidgets)
target_link_libraries(FEBioApp Qt6::Core Qt6::Widgets Qt6::Gui Qt6::OpenGLWidgets)
target_link_libraries(FEBioMonitor Qt6::Core Qt6::Widgets Qt6::Gui Qt6::OpenGLWidgets)

if(WIN32)
elseif(APPLE)
else()
    set_property(TARGET MeshTools PROPERTY CXX_STANDARD 17)
    
    target_link_libraries(${FBS_BIN_NAME} -static-libstdc++ -static-libgcc)
    
	target_link_libraries(${FBS_BIN_NAME} -Wl,--start-group)
endif()

if(UNIX)
    if(${USE_MKL_OMP})
        target_link_libraries(${FBS_BIN_NAME} ${MKL_OMP})
    else()
        target_link_libraries(${FBS_BIN_NAME} ${OpenMP_C_LIBRARIES})
    endif()
endif()

if(USE_MMG)
	target_link_libraries(${FBS_BIN_NAME} optimized ${MMG_LIBS})
    
    if(DEFINED MMG_DBG_LIBS)
        target_link_libraries(${FBS_BIN_NAME} debug ${MMG_DBG_LIBS})
    else()
        target_link_libraries(${FBS_BIN_NAME} debug ${MMG_LIBS})
    endif()
endif()

if(USE_TETGEN)
	target_link_libraries(${FBS_BIN_NAME} optimized ${TETGEN_LIB})
    
    if(DEFINED TETGEN_DBG_LIBS)
        target_link_libraries(${FBS_BIN_NAME} debug ${TETGEN_DBG_LIB})
    else()
        target_link_libraries(${FBS_BIN_NAME} debug ${TETGEN_LIB})
    endif()
endif()

if(CAD_FEATURES)
	target_link_libraries(${FBS_BIN_NAME} optimized ${NETGEN_LIBS})
    
    if(DEFINED NETGEN_DBG_LIBS)
        target_link_libraries(${FBS_BIN_NAME} debug ${NETGEN_DBG_LIBS})
    else()
        target_link_libraries(${FBS_BIN_NAME} debug ${NETGEN_LIBS})
    endif()

    target_link_libraries(${FBS_BIN_NAME} optimized ${OCCT_LIBS})
    
    if(DEFINED OCCT_DBG_LIBS)
        target_link_libraries(${FBS_BIN_NAME} debug ${OCCT_DBG_LIBS})
    else()
        target_link_libraries(${FBS_BIN_NAME} debug ${OCCT_LIBS})
    endif()
endif()

if(USE_SSH)
	target_link_libraries(${FBS_BIN_NAME} optimized ${SSH_LIB})
    
    if(DEFINED SSH_DBG_LIB)
        target_link_libraries(${FBS_BIN_NAME} debug ${SSH_DBG_LIB})
    else()
        target_link_libraries(${FBS_BIN_NAME} debug ${SSH_LIB})
    endif()
    
    target_link_libraries(${FBS_BIN_NAME} optimized ${SSL_LIBS})
    
    if(DEFINED SSL_DBG_LIBS)
        target_link_libraries(${FBS_BIN_NAME} debug ${SSL_DBG_LIBS})
    else()
        target_link_libraries(${FBS_BIN_NAME} debug ${SSL_LIBS})
    endif()
endif()

if(MODEL_REPO)
	target_link_libraries(${FBS_BIN_NAME} optimized ${LIBZIP_LIB})
    
    if(DEFINED LIBZIP_DBG_LIB)
        target_link_libraries(${FBS_BIN_NAME} debug ${LIBZIP_DBG_LIB})
    else()
        target_link_libraries(${FBS_BIN_NAME} debug ${LIBZIP_LIB})
    endif()
    
    target_link_libraries(${FBS_BIN_NAME} optimized ${SQLITE_LIB})

    if(DEFINED SQLITE_DBG_LIB)
        target_link_libraries(${FBS_BIN_NAME} debug ${SQLITE_DBG_LIB})
    else()
        target_link_libraries(${FBS_BIN_NAME} debug ${SQLITE_LIB})
    endif()
endif()

if(USE_FFMPEG)
    target_link_libraries(${FBS_BIN_NAME} optimized ${FFMPEG_LIBS})
    
    if(DEFINED FFMPEG_DBG_LIBS)
        target_link_libraries(${FBS_BIN_NAME} debug ${FFMPEG_DBG_LIBS})
    else()
        target_link_libraries(${FBS_BIN_NAME} debug ${FFMPEG_LIBS})
    endif()
endif()

if(USE_ITK)
    target_link_libraries(${FBS_BIN_NAME} ${SimpleITK_LIBRARIES})
endif()

if(USE_ZLIB)
    target_link_libraries(${FBS_BIN_NAME} ${ZLIB_LIBRARY_RELEASE})
endif()

target_link_libraries(${FBS_BIN_NAME} ${OPENGL_LIBRARY})


target_link_libraries(${FBS_BIN_NAME} ${GLEW_LIBRARIES} GLEW::glew)

target_link_libraries(${FBS_BIN_NAME} ${FEBIOSTUDIO_LIBS})

if (WIN32)
    foreach(name IN LISTS FEBio_RELEASE_LIBS)
        target_link_libraries(${FBS_BIN_NAME} optimized ${name})
    endforeach()

    foreach(name IN LISTS FEBio_DEBUG_LIBS)
        target_link_libraries(${FBS_BIN_NAME} debug ${name})
    endforeach()
else()
    target_link_libraries(${FBS_BIN_NAME} ${FEBio_LIBS})
endif()

if(WIN32)
    target_link_libraries(${FBS_BIN_NAME} vfw32.lib)
elseif(APPLE)
else()
    target_link_libraries(${FBS_BIN_NAME} -Wl,--end-group)
endif()

option(BUILD_UPDATER "Build updater" OFF)
if(BUILD_UPDATER)
    if(WIN32)
        add_definitions(-DWIN32)
    elseif(APPLE)
        add_definitions(-D__APPLE__)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")
    else()
        add_definitions(-DLINUX)

        set(CMAKE_BUILD_RPATH_USE_LINK_PATH FALSE)
        set(CMAKE_BUILD_RPATH $ORIGIN/../lib/)
    endif()

    set(UPDATER_SOURCES Updater/FEBioStudioUpdater.cpp Updater/MainWindow.cpp Updater/ZipThread.cpp
        Updater/getapppath.cpp Updater/FEBioStudioUpdater.qrc FEBioStudio/UpdateChecker.cpp 
        FEBioStudio/ServerSettings.cpp)

    add_executable(FEBioStudioUpdater ${UPDATER_SOURCES})
    set_property(TARGET FEBioStudioUpdater PROPERTY AUTOGEN_BUILD_DIR ${CMAKE_BINARY_DIR}/CMakeFiles/AutoGen/FEBioStudioUpdater_autogen)

    add_executable(mvUtil "Updater/mvUtil.cpp")
    set_property(TARGET mvUtil PROPERTY AUTOGEN_BUILD_DIR ${CMAKE_BINARY_DIR}/CMakeFiles/AutoGen/mvUtil_autogen)

    if(WIN32)
        SET_TARGET_PROPERTIES(FEBioStudioUpdater PROPERTIES LINK_FLAGS "/MANIFESTUAC:\"level='requireAdministrator' uiAccess='false'\" /SUBSYSTEM:WINDOWS")
        SET_TARGET_PROPERTIES(mvUtil PROPERTIES LINK_FLAGS "/MANIFESTUAC:\"level='requireAdministrator'\"")
        target_link_options(FEBioStudioUpdater PRIVATE "/entry:mainCRTStartup")
    endif()

    target_include_directories(FEBioStudioUpdater PRIVATE "Updater")

    findLib(FEBio_LIB_DIR XML_LIB xml)

    target_link_libraries(FEBioStudioUpdater Qt6::Widgets Qt6::Network ${XML_LIB} ${LIBZIP_LIB})
endif()
