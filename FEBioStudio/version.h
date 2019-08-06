#pragma once

//-----------------------------------------------------------------------------
// VERSION INFORMATION
#define VERSION			2
#define SUBVERSION		2
#define SUBSUBVERSION	0

//-----------------------------------------------------------------------------
// FILE VERSION
#define SAVE_VERSION	0x00020002

// lowest supported version number
#define MIN_PRV_VERSION	0x0001000D

//-----------------------------------------------------------------------------
// SVN VERSION
#ifdef SVN
#include "svnrev.h"
#else
#define SVNREVISION 0
#endif
