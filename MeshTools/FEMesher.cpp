#include "stdafx.h"
#include "FEMesher.h"
#include <GeomLib/GObject.h>
#include <GeomLib/GMeshObject.h>
#include <stack>
using namespace std;

//-----------------------------------------------------------------------------
// constructor
FEMesher::FEMesher()
{

}

//-----------------------------------------------------------------------------
// destructor
FEMesher::~FEMesher()
{

}

//-----------------------------------------------------------------------------
void FEMesher::Save(OArchive &ar)
{
	ar.BeginChunk(PARAMS);
	{
		ParamContainer::Save(ar);
	}
	ar.EndChunk();
}

//-----------------------------------------------------------------------------

void FEMesher::Load(IArchive& ar)
{
	TRACE("FEMesher::Load");

	while (IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();
		if (nid == PARAMS) ParamContainer::Load(ar);
		ar.CloseChunk();
	}
}

MeshingProgress FEMesher::Progress()
{
	MeshingProgress mp;
	return mp;
}

void FEMesher::Terminate()
{

}
