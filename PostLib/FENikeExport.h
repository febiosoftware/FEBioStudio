#pragma once
#include "FEFileExport.h"

namespace Post {
//-----------------------------------------------------------------------------
// Export model to a nike3d file
class FENikeExport : public FEFileExport
{
public:
	FENikeExport(void);
	~FENikeExport(void);

	bool Save(FEPostModel& fem, const char* szfile);
};

}
