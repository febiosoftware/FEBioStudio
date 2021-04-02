/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2020 University of Utah, The Trustees of Columbia University in 
the City of New York, and others.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/

#pragma once

//-----------------------------------------------------------------------------
// VERSION INFORMATION
#define VERSION			1
#define SUBVERSION		4
#define SUBSUBVERSION	1

//-----------------------------------------------------------------------------
// FSM FILE VERSION
// 3.0: first version of .fsm (same as last prv/fsprj version)
// 3.1: Added support for mesh layers
// 3.2: Added support for checkable parameters
// 3.3: Modified how some discrete element sets are stored
// 3.4: Added load_type parameter to FERigidForce
// 3.5: Added density, k to uncoupled-prestrain material
// 3.6: Added search_radius parameter to f2f contact. 
// 3.7: Made value parameter of FEInitFluidPressure variable. 
// 3.8: Added filename to log data
// 3.9: Made fixed charge density a variable parameter for multi- and triphasic materials. 
// 3.10: Added density to prestrain elastic material.
// 3.11: changes to FEMeshData classes. 
// 3.12: Added shell nodal normal flag to GPart
// 3.13: Added "relative" flag to FERigidDisplacement. 
#define SAVE_VERSION	0x0003000D

// lowest supported version number
#define MIN_PRV_VERSION	0x0001000D

//-----------------------------------------------------------------------------
// SVN VERSION
#ifdef SVN
#include "svnrev.h"
#else
#define SVNREVISION 0
#endif
