#pragma once
#include "FEFileReader.h"

namespace Post {

class FEU3DImport : public FEFileReader
{
public:
	FEU3DImport();

	bool Load(FEPostModel& fem, const char* szfile);
};

}
