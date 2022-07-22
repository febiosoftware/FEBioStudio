cmake_minimum_required(VERSION 3.1.0)

set(CMAKE_CXX_STANDARD 14)
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

# Change to 6 if looking to build with Qt6.
set(Qt_Ver 6 CACHE STRING "Major version number of Qt.")
set(Qt_Version Qt${Qt_Ver})

if(NOT Qt_Root)
    set(Qt_Root "" CACHE PATH "Path to the root Qt directory (e.g. /opt/Qt)")
endif()

if(NOT NOT_FIRST)
    set(NOT_FIRST true CACHE INTERNAL "")
    return()
endif()

if(Qt_Ver VERSION_EQUAL 6)
    if(NOT CMAKE_PREFIX_PATH)
    
        if(WIN32)
            find_path(Qt_TEMP lib/cmake/Qt6/Qt6Config.cmake
                PATHS ${Qt_Root}/ ${Qt_Root}/* ${Qt_Root}/*/* C:/Qt/* C:/Qt*/* $ENV{HOME}/Qt/* $ENV{HOME}/*/Qt/* /Qt/* /Qt*/* 
                PATH_SUFFIXES "msvc2015_64" "msvc2017_64" "msvc2019_64"
                DOC "Qt CMake Directory"
                NO_DEFAULT_PATH)
        elseif(APPLE)
            find_path(Qt_TEMP lib/cmake/Qt6/Qt6Config.cmake
                PATHS ${Qt_Root}/ ${Qt_Root}/* ${Qt_Root}/*/* /opt/Qt/* /opt/Qt*/* /usr/local/Qt/* /usr/local/Qt*/* 
                    $ENV{HOME}/Qt/* $ENV{HOME}/*/Qt/* /Qt/* /Qt*/* 
                PATH_SUFFIXES "clang_64"
                DOC "${Qt_Version} CMake Directory"
                NO_DEFAULT_PATH)
        else()
            find_path(Qt_TEMP lib/cmake/Qt6/Qt6Config.cmake
                PATHS ${Qt_Root}/ ${Qt_Root}/* ${Qt_Root}/*/* /opt/Qt/* /opt/Qt*/* /usr/local/Qt/* /usr/local/Qt*/* 
                    $ENV{HOME}/Qt/* $ENV{HOME}/*/Qt6 $ENV{HOME}/*/Qt/* /Qt/* /Qt*/* 
                PATH_SUFFIXES "gcc_64"
                DOC "Qt CMake Directory"
                NO_DEFAULT_PATH)
        endif()
        
        if(Qt_TEMP)
            set(CMAKE_PREFIX_PATH ${Qt_TEMP} CACHE INTERNAL "Path to Qt6 installation prefix.")
            unset(Qt_TEMP CACHE)
        else()
            unset(Qt_TEMP CACHE)
        endif()
        
        if(NOT CMAKE_PREFIX_PATH)
            message(FATAL_ERROR "Unable to locate ${Qt_Version}Config.cmake. Please check the value of ${Qt_Version}_Root and set it to the root directory of your ${Qt_Version} installation (e.g. /opt/Qt).")
        endif()
    endif()
elseif(Qt_Ver VERSION_EQUAL 5)
    if(NOT Qt5_DIR)
        if(WIN32)
            find_file(Qt_CMAKE cmake/${Qt_Version}/${Qt_Version}Config.cmake
                PATHS ${Qt_Root}/* ${Qt_Root}/*/* C:/Qt/* C:/Qt*/* $ENV{HOME}/Qt/* $ENV{HOME}/*/Qt/* /Qt/* /Qt*/* 
                PATH_SUFFIXES "msvc2015_64/lib" "msvc2017_64/lib" "msvc2019_64/lib" "lib"
                DOC "Qt CMake Directory")
        elseif(APPLE)
            find_file(Qt_CMAKE cmake/${Qt_Version}/${Qt_Version}Config.cmake
                PATHS ${Qt_Root}/* ${Qt_Root}/*/* /opt/Qt/* /opt/Qt*/* /usr/local/Qt/* /usr/local/Qt*/* 
                    $ENV{HOME}/Qt/* $ENV{HOME}/*/Qt/* /Qt/* /Qt*/* 
                PATH_SUFFIXES "clang_64/lib" "lib"
                DOC "${Qt_Version} CMake Directory")
        else()
            find_file(Qt_CMAKE cmake/${Qt_Version}/${Qt_Version}Config.cmake
                PATHS ${Qt_Root}/* ${Qt_Root}/*/* /opt/Qt/* /opt/Qt*/* /usr/local/Qt/* /usr/local/Qt*/* 
                    $ENV{HOME}/Qt/* $ENV{HOME}/*/${Qt_Version} $ENV{HOME}/*/Qt/* /Qt/* /Qt*/* 
                PATH_SUFFIXES "gcc_64/lib" "lib"
                DOC "Qt CMake Directory")
        endif()
            
        if(Qt_CMAKE)
            get_filename_component(Qt_TEMP ${Qt_CMAKE} DIRECTORY)
            set(Qt5_DIR ${Qt_TEMP} CACHE INTERNAL "Path to Qt5 CMake directory.")
            
            unset(Qt_TEMP)
            unset(Qt_CMAKE CACHE)
        
        else()
            unset(Qt_CMAKE CACHE)
        endif()
        
        if(NOT Qt5_DIR)
            message(FATAL_ERROR "Unable to locate ${Qt_Version}Config.cmake. Please check the value of ${Qt_Version}_Root and set it to the root directory of your ${Qt_Version} installation (e.g. /opt/Qt).")
        endif()    
    endif()
else()
    message(FATAL_ERROR "FEBio Studio cannot be compiled with Qt version ${Qt_Ver}")
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

##### Search for library and include directories #####

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

if(USE_TEEM)
  findLib(TEEM_LIB_DIR TEEM_LIB teem)
  findLib(LIBTIFF_LIB_DIR LIBTIFF_LIB tiff)
  #find_library(teem NAMES teem PATH TEEM_LIB_DIR TEEM_LIB)
endif()

if(USE_DICOM)
  set(DICOM_LIB_NAMES dcmdata dcmimage dcmimgle i2d oflog ofstd)
  foreach(name IN LISTS DICOM_LIB_NAMES)
    findLib(DICOM_LIB_DIR DICOM_LIB ${name})
	list(APPEND DICOM_LIBS ${DICOM_LIB})
	unset(DICOM_LIB)
  endforeach(name)
endif()

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
    set(NETGEN_LIB_NAMES nglib csg interface occ)
    
    # This NetGen library is called "meshing" on Windows and "mesh" elsewhere.
    if(WIN32)
        list(APPEND NETGEN_LIB_NAMES meshing)
    else()
        list(APPEND NETGEN_LIB_NAMES mesh)
    endif()
    
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
    findLib(QUAZIP_LIB_DIR QUAZIP_LIB ${QUAZIP_NAMES})
    
    if(QUAZIP_DBG_LIB_DIR STREQUAL "")
        unset(QUAZIP_DBG_LIB)
    else()
        findLib(QUAZIP_DBG_LIB_DIR QUAZIP_DBG_LIB ${QUAZIP_NAMES})
    endif()
	
    findLib(SQLITE_LIB_DIR SQLITE_LIB sqlite3)
    
    if(SQLITE_DBG_LIB_DIR STREQUAL "")
        unset(SQLITE_DBG_LIB)
    else()
        findLib(SQLITE_DBG_LIB_DIR SQLITE_DBG_LIB sqlite3)
    endif()
endif()

if(USE_FFMPEG)
    if(WIN32)
        set(FFMPEG_LIB_NAMES avformat avcodec avutil swresample swscale)
    else()
        set(FFMPEG_LIB_NAMES avformat avcodec avresample avutil swresample swscale)
    endif()
    
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

# Find FEBio Libs
if (WIN32)
    set(FEBio_DEBUG_LIBS "")
    set(FEBio_RELEASE_LIBS "")

    set(FEBio_LIB_NAMES fecore febiolib febiomech xml febioplot)

    set(FEBio_DEBUG_DIR ${FEBio_LIB_DIR}Debug)
    set(FEBio_RELEASE_DIR ${FEBio_LIB_DIR}Release)

    foreach(name IN LISTS FEBio_LIB_NAMES)
        findLib(FEBio_DEBUG_DIR FEBio_LIB ${name})
        list(APPEND FEBio_DEBUG_LIBS ${FEBio_LIB})
        unset(FEBio_LIB)
    endforeach(name)

    foreach(name IN LISTS FEBio_LIB_NAMES)
        findLib(FEBio_RELEASE_DIR FEBio_LIB ${name})
        list(APPEND FEBio_RELEASE_LIBS ${FEBio_LIB})
        unset(FEBio_LIB)
    endforeach(name)

    unset(FEBio_LIB_NAMES)
else()
    set(FEBio_LIBS "")

    set(FEBio_LIB_NAMES fecore febiolib febiomech xml febioplot)

    foreach(name IN LISTS FEBio_LIB_NAMES)
        findLib(FEBio_LIB_DIR FEBio_LIB ${name})
        list(APPEND FEBio_LIBS ${FEBio_LIB})
        unset(FEBio_LIB)
    endforeach(name)

    unset(FEBio_LIB_NAMES)
endif()

##### Set appropriate defines and includes #####

if(WIN32)
    add_definitions(-DWIN32 -D_CRT_SECURE_NO_WARNINGS)
    add_compile_options(/MP /openmp)
elseif(APPLE)
    add_definitions(-D__APPLE__)
    set(CMAKE_OSX_DEPLOYMENT_TARGET 10.13)
    
    if(${OpenMP_C_FOUND})
        add_compile_options(-Xpreprocessor -fopenmp)
    endif()
    
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

if(USE_TEEM)
  add_definitions(-DHAS_TEEM)
  include_directories(${TEEM_INC})
  include_directories(${LIBTIFF_INC})
endif()

if(USE_DICOM)
  add_definitions(-DHAS_DICOM)
  include_directories(${DICOM_INC})
endif()

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
	add_definitions(-DHAS_QUAZIP)
	include_directories(${QUAZIP_INC})

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

include_directories(${ZLIB_INCLUDE_DIR})

if(USE_ITK)
    add_definitions(-DHAS_ITK)
    #~ include(${ITK_USE_FILE})
    include_directories(${SimpleITK_INCLUDE_DIRS})
endif()

include_directories(${FEBio_INC})

##### Find Source Files #####

macro(findHdrSrc name)
	file(GLOB HDR_${name} "${name}/*.h")
	file(GLOB SRC_${name} "${name}/*.cpp")
endmacro()

findHdrSrc(FEBioStudio)

set(FEBIOSTUDIO_LIBS Abaqus Ansys Comsol CUILib FEBio FEMLib FSCore GeomLib GLLib GLWLib ImageLib LSDyna MeshIO MeshLib MeshTools Nike3D PostGL PostLib XPLTLib FEBioLink)

foreach(name IN LISTS FEBIOSTUDIO_LIBS)
	findHdrSrc(${name})
endforeach(name)

##### Add binaries #####

# Linking the Qt libraries also pulls in the include files
link_libraries(${Qt_Version}::Core ${Qt_Version}::Widgets ${Qt_Version}::Gui ${Qt_Version}::Network ${Qt_Version}::OpenGL)

if(Qt_Ver VERSION_EQUAL 6)
  link_libraries(Qt6::Core5Compat Qt6::OpenGLWidgets)
endif()

# set the name of the executable
if (WIN32)
    set(FBS_BIN_NAME FEBioStudio2)
else()
    set(FBS_BIN_NAME FEBioStudio)
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
        # Get branch name and check if we're on the develop branch
        set(CMD git rev-parse --abbrev-ref HEAD)
        crossExecProcess(BRANCH_NAME)

        string(COMPARE EQUAL ${BRANCH_NAME} "develop" ISDEVELOP)
        string(COMPARE EQUAL ${BRANCH_NAME} "febio4"  ISFEBIO4)
        
        # Get the commit info and set the DEVCOMMIT macro
        if(SUCCESS EQUAL 0 AND (ISDEVELOP OR ISFEBIO4))
            
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

if(WIN32)
elseif(APPLE)
else()
    set_property(TARGET MeshTools PROPERTY CXX_STANDARD 17)
    
    target_link_libraries(${FBS_BIN_NAME} -static-libstdc++ -static-libgcc)
    
	target_link_libraries(${FBS_BIN_NAME} -Wl,--start-group)
endif()

if(USE_TEEM)
  target_link_libraries(${FBS_BIN_NAME} ${TEEM_LIB})
  target_link_libraries(${FBS_BIN_NAME} ${LIBTIFF_LIB})
endif()

if(USE_DICOM)
  target_link_libraries(${FBS_BIN_NAME} ${DICOM_LIBS})
endif()

if(NOT WIN32)
    if(${OpenMP_C_FOUND})
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
	target_link_libraries(${FBS_BIN_NAME} optimized ${QUAZIP_LIB})
    
    if(DEFINED QUAZIP_DBG_LIB)
        target_link_libraries(${FBS_BIN_NAME} debug ${QUAZIP_DBG_LIB})
    else()
        target_link_libraries(${FBS_BIN_NAME} debug ${QUAZIP_LIB})
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

target_link_libraries(${FBS_BIN_NAME} ${ZLIB_LIBRARY_RELEASE})
target_link_libraries(${FBS_BIN_NAME} ${OPENGL_LIBRARY})

if(APPLE)
    target_link_libraries(${FBS_BIN_NAME} ${GLEW_SHARED_LIBRARY_RELEASE} ${GLEW_LIBRARIES})
else()
    target_link_libraries(${FBS_BIN_NAME} ${GLEW_LIBRARIES})
endif()

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

