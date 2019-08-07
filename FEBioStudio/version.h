#pragma once

//-----------------------------------------------------------------------------
// VERSION INFORMATION
#define VERSION			1
#define SUBVERSION		0
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
