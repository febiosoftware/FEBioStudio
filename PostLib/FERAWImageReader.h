#pragma once
#include "FEFileReader.h"

namespace Post {

	//-----------------------------------------------------------------------------
// This class reads in a RAW image file and converts it to a mesh.
// The pixels of the image correspond to nodes
//
class FERAWImageReader : public FEFileReader
{
public:
	struct OPTIONS {
		int		nx;
		int		ny;
		int		nz;
		int		nformat;
		float	wx;
		float	wy;
		float	wz;
	};

public:
	// constructor
	FERAWImageReader(FEPostModel* fem);

	// load image data
	bool Load(const char* szfile) override;

	// set the options (must be done before loading!)
	void SetOptions(OPTIONS& o) { m_ops = o; }

protected:
	OPTIONS	m_ops;
};
}
