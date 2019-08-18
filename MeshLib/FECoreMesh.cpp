#include "FECoreMesh.h"

//-----------------------------------------------------------------------------
//! constructor
FECoreMesh::FECoreMesh()
{
}

//-----------------------------------------------------------------------------
//! destructor
FECoreMesh::~FECoreMesh()
{
}

//-----------------------------------------------------------------------------
//! This function checks if all elements are of the type specified in the argument
bool FECoreMesh::IsType(int ntype) const
{
	int NE = Elements();
	for (int i=0; i<NE; ++i)
	{
		const FEElement_& el = ElementRef(i);
		if (el.GetType() != ntype) return false;
	}
	return true;
}
