#pragma once
#include "FEFileReader.h"

namespace Post {

class FEU3DImport : public FEFileReader
{
public:
	FEU3DImport();

	bool Load(FEModel& fem, const char* szfile);
};

}
