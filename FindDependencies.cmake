# Qt
find_package(Qt5 COMPONENTS Widgets Gui Network OpenGL Charts REQUIRED)
find_package(Qt5 COMPONENTS WebEngineWidgets QUIET)
mark_as_advanced(Qt5Charts_DIR Qt5Core_DIR Qt5Gui_DIR Qt5Network_DIR Qt5OpenGL_DIR Qt5Positioning_DIR 
    Qt5PrintSupport_DIR Qt5QmlModels_DIR Qt5Qml_DIR Qt5Quick_DIR Qt5WebChannel_DIR Qt5WebEngineCore_DIR 
    Qt5WebEngineWidgets_DIR Qt5Widgets_DIR)

macro(findComponent name headername libname incPaths libPaths incSuffixes libSuffixes incOptions libOptions)
    if(WIN32)
        find_path(${name}_INC ${headername}
            PATHS ${incpaths} C::/Program\ Files/* $ENV{HOMEPATH}/* $ENV{HOMEPATH}/*/*
            PATH_SUFFIXES ${incSuffixes} "include" "src" "build" "build/include"
            DOC "${name} include directory"
            ${incOptions})
        find_library(${name}_LIB ${libname} 
            PATHS ${libpaths} C::/Program\ Files/* $ENV{HOMEPATH}/* $ENV{HOMEPATH}/*/*
            PATH_SUFFIXES ${libSuffixes} "build" "build/lib" "cbuild/lib" "src/build/lib" "src/cbuild/lib" "Release" "Debug" 
            DOC "${name} library path"
            ${libOptions})
    else()
        find_path(${name}_INC ${headername}
            PATHS ${incpaths} $ENV{HOME}/* $ENV{HOME}/*/*
            PATH_SUFFIXES ${incSuffixes} "include" "build" "build/include" "cbuild" "cbuild/include" "src" 
            DOC "${name} include directory"
            ${incOptions})
        find_library(${name}_LIB ${libname} 
            PATHS ${libpaths} $ENV{HOME}/* $ENV{HOME}/*/*
            PATH_SUFFIXES ${libSuffixes} "build" "build/lib" "cbuild" "cbuild/lib" "src/build/lib" "src/cbuild/lib" "Release" "Debug" 
            DOC "${name} library path"
            ${libOptions})
    endif()

    if(${name}_LIB)
        get_filename_component(${name}_TEMP ${${name}_LIB} DIRECTORY)
        set(${name}_LIB_DIR ${${name}_TEMP} CACHE PATH "Path to the ${name} lib directory.")
        unset(${name}_TEMP)
        unset(${name}_LIB CACHE)
    else()
        set(${name}_LIB_DIR  CACHE PATH "Path to the ${name} lib directory.")
        unset(${name}_LIB CACHE)
    endif()
endmacro()

macro(setComponent name optionText)
    if(${name}_INC AND ${name}_LIB_DIR)		
        option(USE_${name} ${optionText} ON)
        mark_as_advanced(${name}_INC ${name}_LIB_DIR)
    else()
        option(USE_${name} ${optionText} OFF)
        mark_as_advanced(CLEAR ${name}_INC ${name}_LIB_DIR)
    endif()
endmacro()

#findComponent name headername libname incpaths libpaths incSuffixes libSuffixes incOptions libOptions)

# MMG
findComponent(MMG mmg/mmg3d/libmmg3d.h mmg3d "/opt/mmg*" "/opt/mmg*" "include/mmg" "" "" "")
setComponent(MMG "Required for MMG use")

# TETGEN
findComponent(TET tetgen.h tet "/opt/tetgen*" "/opt/tetgen*" "include/tetgen" "" "" "")
setComponent(TET "Required for adaptive remeshing")

# NETGEN
set(NETGEN_incPaths "/opt/netgen*" "/Applications/Netgen.app/Contents/Resources/include")
set(NETGEN_libPaths "/opt/netgen*" "/Applications/Netgen.app/Contents/MacOS" )

findComponent(NETGEN include/occgeom.hpp nglib ${NETGEN_incPaths} ${NETGEN_libPaths} "netgen/include" "" "" "")
setComponent(NETGEN "Required for meshing CAD objects")
    
# OpenCascade
set(OCC_incSuffixes "opencascade" "include/opencascade" "opencascade/include/opencascade" "build/opencascade" "build/include/opencascade")

findComponent(OCC gp_Pnt.hxx TKernel "/opt/opencascade" "/opt/opencascade" ${OCC_incSuffixes} "opencascade/lib" "" "")
setComponent(OCC "Required for importing and meshing CAD objects")
    
# LibSSH
findComponent(SSH libssh/libssh.h ssh "/opt/libssh" "/opt/libssh" "libssh/include" "libssh/lib" "" "")
setComponent(SSH "Required for running jobs on remote machines")
    
# OpenSSL
if(APPLE)
    findComponent(SSL openssl/aes.h ssl "/opt/openssl" "/opt/openssl" "openssl/include" "openssl/lib" "" NO_DEFAULT_PATH)
else()
    findComponent(SSL openssl/aes.h ssl "/opt/openssl" "/opt/openssl" "openssl/include" "openssl/lib" "" "")
endif()

if(SSL_INC AND SSL_LIB_DIR AND USE_SSH)		
	option(USE_SSH "Required for running jobs on remote machines" ON)
    mark_as_advanced(SSL_INC SSL_LIB_DIR)
else()
	option(USE_SSH "Required for running jobs on remote machines" OFF)
    mark_as_advanced(CLEAR SSL_INC SSL_LIB_DIR)
endif()

# QuaZip
if(APPLE)
    set(QUAZIP_incPaths /usr/include/* /opt/quazip)
    set(QUAZIP_incSuffixes "/quazip" "include/quazip" "quazip/include/quazip" "build/quazip" "build/include/quazip")
    findComponent(QUAZIP quazip.h quazip ${QUAZIP_incPaths} "/opt/quazip" ${QUAZIP_incSuffixes} "quazip/lib" "" "")
else()
    set(QUAZIP_incSuffixes "/quazip5" "include/quazip5" "quazip/include/quazip5" "build/quazip5" "build/include/quazip5")
    findComponent(QUAZIP quazip.h quazip5 "/opt/quazip" "/opt/quazip" ${QUAZIP_incSuffixes} "quazip/lib" "" "")
endif()

setComponent(QUAZIP "Required for the Model Repository and exporting projects")

# SQLite
findComponent(SQLITE sqlite3.h sqlite3 "/opt/sqlite" "/opt/sqlite" "sqlite/include" "sqlite/lib" "" "")

if(SQLITE_INC AND SQLITE_LIB_DIR AND USE_QUAZIP)		
	option(MODEL_REPO "Build code to connect to the Model Repository" ON)
    mark_as_advanced(SQLITE_INC SQLITE_LIB_DIR)
else()
    SET(MODEL_REPO OFF CACHE BOOL "Build code to connect to the Model Repository" FORCE)
    mark_as_advanced(CLEAR SQLITE_INC SQLITE_LIB_DIR)
endif()

# FFMPEG
set(FFMPEG_incPaths /usr/include/ /opt/ffmpeg)
set(FFMPEG_libPaths /usr/local/opt/ /opt/ffmpeg)

findComponent(FFMPEG libavformat/avformat.h avformat ${FFMPEG_incPaths} ${FFMPEG_libPaths} "ffmpeg/include" "ffmpeg/lib" "" "")
setComponent(FFMPEG "Required to export mpeg videos")

# OpenGL
find_package(OpenGL REQUIRED)

# ZLIB
find_package(ZLIB REQUIRED)
if(ZLIB_INCLUDE_DIR AND ZLIB_LIBRARY_RELEASE)
    mark_as_advanced(ZLIB_INCLUDE_DIR ZLIB_LIBRARY_RELEASE)
endif()
