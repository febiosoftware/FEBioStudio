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

##### Search for library and include directories #####

include(FindDependencies.cmake)

#### Check if paths are valid and find libs ####

function(findLib libName libDir libOut)
    find_library(TEMP ${libName} PATHS ${libDir} NO_DEFAULT_PATH)

    # Optional argument ARGV3 can be used to pass an alternate name for the library
    if(NOT TEMP AND ${ARGC} GREATER 3)
        find_library(TEMP ${ARGV3} PATHS ${libDir} NO_DEFAULT_PATH)
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

##### Set appropriate defines and includes #####

if(WIN32)
    add_definitions(-DWIN32)
elseif(APPLE)
    add_definitions(-D__APPLE__)
    set(CMAKE_OSX_DEPLOYMENT_TARGET 10.13)
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

macro(findHdrSrc name)
	file(GLOB HDR_${name} "${name}/*.h")
	file(GLOB SRC_${name} "${name}/*.cpp")
	list(FILTER SRC_${name} EXCLUDE REGEX "qrc_")
	list(FILTER SRC_${name} EXCLUDE REGEX "moc_")
endmacro()

findHdrSrc(FEBioStudio)

set(FEBIOSTUDIO_LIBS Abaqus Ansys Comsol CUILib FEBio FEMLib FSCore GeomLib GLLib GLWLib ImageLib LSDyna MathLib MeshIO MeshLib MeshTools Nike3D PostGL PostLib XML XPLTLib)

foreach(name IN LISTS FEBIOSTUDIO_LIBS)
	findHdrSrc(${name})
endforeach(name)

##### Add binaries #####

# Linking the Qt libraries also pulls in the include files
link_libraries(Qt5::Core Qt5::Widgets Qt5::Gui Qt5::Network Qt5::OpenGL Qt5::Charts)

if(Qt5WebEngineWidgets_FOUND)
	link_libraries(Qt5::WebEngineWidgets)
endif()

if(APPLE)
	# set bundle icon
	set(FBS_ICON ${CMAKE_CURRENT_SOURCE_DIR}/icons/FEBioStudio.icns)
	set(MACOSX_BUNDLE_ICON_FILE FEBioStudio.icns)
	set_source_files_properties(${FBS_ICON} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
	
	add_executable(FEBioStudio MACOSX_BUNDLE ${FBS_ICON} ${HDR_FEBioStudio} ${SRC_FEBioStudio} febiostudio.qrc)
	set_property(TARGET FEBioStudio PROPERTY AUTOGEN_BUILD_DIR ${CMAKE_BINARY_DIR}/CMakeFiles/AutoGen/FEBioStudio_autogen)
else()
	add_executable(FEBioStudio ${HDR_FEBioStudio} ${SRC_FEBioStudio} febiostudio.qrc)
	set_property(TARGET FEBioStudio PROPERTY AUTOGEN_BUILD_DIR ${CMAKE_BINARY_DIR}/CMakeFiles/AutoGen/FEBioStudio_autogen)
endif()

macro(addLib name)
	add_library(${name} ${HDR_${name}} ${SRC_${name}})
	set_property(TARGET ${name} PROPERTY AUTOGEN_BUILD_DIR ${CMAKE_BINARY_DIR}/CMakeFiles/AutoGen/${name}_autogen)
endmacro()

foreach(name IN LISTS FEBIOSTUDIO_LIBS)
	addLib(${name})
endforeach(name)

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
target_link_libraries(FEBioStudio ${OPENGL_LIBRARY})

target_link_libraries(FEBioStudio ${FEBIOSTUDIO_LIBS})

if(WIN32)
elseif(APPLE)
else()
    target_link_libraries(FEBioStudio -Wl,--end-group)
    target_link_libraries(FEBioStudio -Wl,-rpath,\'\$ORIGIN/../lib/\')
endif()


