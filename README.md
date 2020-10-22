<img src="Documentation/InstallerImages/Splash.png" href="https://gibboncode.org" alt="FEBio Studio" width="75%">
<!-- ![febio studio logo](Documentation/InstallerImages/Splash.png) -->

FEBio Studio is an integrated environment for FEBio, which allows users to generate, and solve FEBio models, as well as visualize and analyze the FEBio results. FEBio Studio also provides access to the online FEBio model repository, where users can find examples, tutorial models, and curated FEBio models that were used in peer-reviewed research studies.

Executables for FEBio Studio can be downloaded from https://febio.org/downloads/.  Please inform us of publications that use FEBio or FEBio Studio in research.  Information can be found on the Publications tab on febio.org.  

Support forums can be found at https://forums.febio.org/.

[![DOI](https://img.shields.io/badge/Citation-DOI:10.1115/1.4005694-green.svg)](https://dx.doi.org/10.1115%2F1.4005694)

[![License](https://img.shields.io/badge/License-MIT-orange.svg)](LICENSE)

### Table of contents
- [FEBio Studio Build Guide](#Build)  
- [Contributing](#Contributing)
- [Code of Conduct](#Conduct)

# FEBio Studio Build Guide <a name="Build"></a>

## Prerequisites

### CMake 
FEBio Studio's build process utilizes CMake, an open-source, cross-platform tool designed to streamline the configuration of the build environment. The CMake in this repository will help you to locate necessary third party libraries on your machine, set up include and library paths, and allow you to choose which of FEBio Studio’s features you would like to include in your build.

Please download the latest release of CMake from https://cmake.org/, and install it on your machine before proceeding. 

### Qt

Qt is a cross-platform framework primarily used to create graphical user interfaces. Qt dependence is built into the large majority of FEBio Studio's code, and so it is absolutely required. A free, open-source version of Qt can be downloaded from [Qt's download page](https://www.qt.io/download). The Qt installer will allow you to choose many different versions of Qt, and individually decide which components of the version that you would like to install. 

We recommend that you install version 5.14.2 of Qt. Some older versions of Qt do not support all of the features that we use, and some newer versions have caused bugs in FEBio Studio. 

If you wish, you may install all of Qt's components, but it's not necessary. Only the following components are used by FEBio Studio:

* The "core" Qt component. The name of this component differs depending on your platform. 
    * On Windows, when compiling with Microsoft Visual Studio 2017 (which is highly recommended), this component is named _MSVC 2017 64-bit_.
    * On macOS, this component is named _macOS_.
    * On Linux, this component is named _Desktop gcc 64-bit_.
* The _Qt Charts_ component is used by FEBio Studio to create plots during post-simulation analysis. This component is required.
* The _Qt WebEngine_ component is used by FEBio Studio to display contextual help from our online manuals within certain FEBio Studio dialogs. This component is not required, but the online help feature will be unavailable without it. 

### Required Third Party Packages

FEBio Studio relies on several third party packages. Some are required to compile FEBio Studio at all, while others are only required if you would like to include specific features in FEBio Studio. The following third party packages are required:

* OpenGL is a widely-used 2D and 3D graphics application programming interface. FEBio Studi uses OpenGL to render everything in the model view area. This library is generally pre-installed on all modern operating systems. Be sure to have your latest graphic driver installed. 

* zlib is an open-source, lossless data-compression library that is used by FEBio Studio to read plot files that have been compressed. This library is generally pre-installed on macOS, and most Linux distributions. For Windows, you will need to download the latest source from [zlib's website](https://zlib.net/), and compile the library yourself. 

### Optional Third Party Packages

FEBio Studio makes use of the following third party packages to add additional functionality. If you do not need the functionality provided by a given package, you can still compile FEBio Studio without it. The libraries below are organized according to the type of functionality they add. 

#### Meshing

* TetGen is used by FEBio Studio for generating and remeshing tetrahedral meshes. To use this library, you will need to download the source and compile it yourself. The source for this library can be downloaded from [TetGen's website](http://wias-berlin.de/software/index.jsp?id=TetGen&lang=1).

* MMG is also used by FEBio Studio for remeshing tetrahedral meshes. Unlike TetGen, MMG does not generate an initial tetrahedral mesh, it only remeshes an existing mesh. MMG's remeshing process does, however, tend to produce a higher quality mesh than TetGen's. The _Tet Remesh_ tool in FEBio Studio relies on MMG. To use this library, you will need to download the source and compile it yourself. The source for this library can be downloaded from [MMG's GitHub account](https://github.com/MmgTools/mmg).

#### CAD Support

* Open CASCADE Technology (OCCT) is a software package used by FEBio Studio to import and create CAD objects. OCCT does not mesh CAD objects, and so is of little use without NetGen (see below). Instructions for installing the pre-compiled binaries on Windows, and a link to the source code for this package can be found on [Open CASCADE's website](https://old.opencascade.com/content/latest-release).

* NetGen is an automatic 3D tetrahedral mesh generator that is used by FEBio Studio to mesh CAD objects. NetGen does not allow for creating or importing CAD objects, and so is of no use without OCCT (see above). Additionally, NetGen depends on OCCT in order to compile, and so OCCT must be installed or compiled prior to compiling NetGen. The source code for this package can be found on [NetGen's GitHub page](https://github.com/NGSolve/netgen).

#### FEBio Project Repository  

* QuaZip is a library used to create and read zip files that interfaces cleanly with Qt. FEBio Studio uses this library to export and import projects as a single file. This library is required to use the FEBio Project Repository. QuaZip depends on Qt, and so Qt must be installed before you compile it. The source can be found on [QuaZip's GitHub page](https://github.com/stachenov/quazip).

* SQLite is an open-source library used for reading and writing data from self-contained SQL database files. FEBio Studio uses this library to store and retrieve information about the projects in the online FEBio Project Repository. SQLite is therefore required to use the repository. Many Linux distributions offer a version of SQLite through their package managers, however, FEBio Studio uses some relatively new SQLite features implemented in version 3.24.0. It is therefore recommended that SQLite be compiled from its source code. Instructions on how to compile SQLite, and a link to its source code can be found on [SQLite's website](https://sqlite.org/src/doc/trunk/README.md).

#### Remote FEBio Jobs

* libssh is used by FEBio Studio to run FEBio jobs on remote systems, and to send and retrieve the associated job files. There is another library called "libssh2", which is not a latter version of libssh, but rather a completely unrelated library; don't confuse the two. Instructions on how to install or compile libssh can be found on [their website](https://www.libssh.org/get-it/).

* OpenSSL is used by FEBio Studio to encrypt SSH passwords (for use in running remote jobs) while they are stored in RAM. Both libssh and OpenSSL are required for running remote FEBio jobs. Information on how to buld this library can be found on [OpenSSL's website](https://www.openssl.org/source/). OpenSSL is also available on macOS through [Homebrew](https://brew.sh/).

#### Miscellaneous

* FFMPEG is used by FEBio Studio for creating MP4 recordings. FFMPEG is available through most Linux package managers, and through [Homebrew](https://brew.sh/) on macOS. Installation files, and the source code can also be found on [FFMPEG's website](https://ffmpeg.org/download.html).

# Contributing <a name="Contributing"></a>

Refer to [CONTRIBUTING](CONTRIBUTING.md)

# Code of conduct <a name="Conduct"></a>

Refer to [CODE_OF_CONDUCT](CODE_OF_CONDUCT.md)
