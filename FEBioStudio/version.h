#pragma once

//-----------------------------------------------------------------------------
// VERSION INFORMATION
#define VERSION			1
#define SUBVERSION		0
#define SUBSUBVERSION	0

//-----------------------------------------------------------------------------
// FSPRJ FILE VERSION
// 3.0: first version of .fsprj (same as last prv version)
// 3.1: Added support for mesh layers
// 3.2: Added support for checkable parameters
#define SAVE_VERSION	0x00030002

// lowest supported version number
#define MIN_PRV_VERSION	0x0001000D

//-----------------------------------------------------------------------------
// SVN VERSION
#ifdef SVN
#include "svnrev.h"
#else
#define SVNREVISION 0
#endif
