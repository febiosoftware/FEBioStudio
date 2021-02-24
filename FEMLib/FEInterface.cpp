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

#include "FEInterface.h"
#include <MeshTools/FEModel.h>
#include <MeshTools/GGroup.h>
#include <MeshTools/GModel.h>
#include <set>
#include <memory>
using namespace std;

//=============================================================================
// FEInterface
//-----------------------------------------------------------------------------

FEInterface::FEInterface(int ntype, FEModel* ps, int nstep)
{
	m_ntype = ntype;
	m_nstepID = nstep;
	m_bActive = true;
	m_ps = ps;
	m_sztype = "(not defined)";
}

//-----------------------------------------------------------------------------
FEInterface::~FEInterface()
{

}

//-----------------------------------------------------------------------------
void FEInterface::SaveList(FEItemListBuilder* pitem, OArchive& ar)
{
	if (pitem)
	{
		ar.BeginChunk((int) pitem->Type());
		pitem->Save(ar);
		ar.EndChunk();
	}
}

//-----------------------------------------------------------------------------
FEItemListBuilder* FEInterface::LoadList(IArchive& ar)
{
	FEItemListBuilder* pitem = 0;

	if (ar.OpenChunk() != IArchive::IO_OK) throw ReadError("error in FEInterface::LoadList");
	unsigned int ntype = ar.GetChunkID();
	switch (ntype)
	{
	case GO_NODE: pitem = new GNodeList(m_ps); break;
	case GO_EDGE: pitem = new GEdgeList(m_ps); break;
	case GO_FACE: pitem = new GFaceList(m_ps); break;
	case GO_PART: pitem = new GPartList(m_ps); break;
	case FE_NODESET: pitem = new FENodeSet((GObject*)0); break;
	case FE_EDGESET: pitem = new FEEdgeSet((GObject*)0); break;
	case FE_SURFACE: pitem = new FESurface((GObject*)0); break;
	case FE_PART   : pitem = new FEPart   ((GObject*)0); break;
	default:
		assert(false);
	}
	if (pitem == 0) throw ReadError("unknown item list type (FEInterface::LoadList)");

	pitem->Load(ar);
	ar.CloseChunk();

	int nret = ar.OpenChunk();
	if (nret != IArchive::IO_END) throw ReadError("error in FEInterface::LoadList");

	// set the parent mesh for FEGroup's
	FEGroup* pg = dynamic_cast<FEGroup*>(pitem);
	if (pg)
	{
		if (m_ps->FindGroupParent(pg) == false)
		{
			ar.log("Invalid object ID in FEInterface::LoadList");
			delete pitem;
			pitem = nullptr;
		}
	}

	return pitem;
}

//=============================================================================
// FEPairedInterface
//-----------------------------------------------------------------------------

FEPairedInterface::FEPairedInterface(int ntype, FEModel* ps, int nstep) : FEInterface(ntype, ps, nstep)
{
	m_surf1 = 0;
	m_surf2 = 0;
}

//-----------------------------------------------------------------------------
FEPairedInterface::~FEPairedInterface()
{
	delete m_surf1;
	delete m_surf2;
}

//-----------------------------------------------------------------------------
void FEPairedInterface::SwapPrimarySecondary()
{
	FEItemListBuilder* tmp = m_surf1;
	m_surf1 = m_surf2;
	m_surf2 = tmp;
}

//-----------------------------------------------------------------------------
void FEPairedInterface::Save(OArchive& ar)
{
	ar.WriteChunk(CID_INTERFACE_NAME, GetName());
	ar.WriteChunk(CID_FEOBJ_INFO, GetInfo());
	ar.WriteChunk(CID_INTERFACE_ACTIVE, m_bActive);
	ar.WriteChunk(CID_INTERFACE_STEP, m_nstepID);
	ar.BeginChunk(CID_INTERFACE_PARAMS);
	{
		ParamContainer::Save(ar);
	}
	ar.EndChunk();
	if (m_surf1)
	{
		ar.BeginChunk(CID_INTERFACE_SURFACE1);
		FEInterface::SaveList(m_surf1, ar);
		ar.EndChunk();
	}
	if (m_surf2)
	{
		ar.BeginChunk(CID_INTERFACE_SURFACE2);
		FEInterface::SaveList(m_surf2, ar);
		ar.EndChunk();
	}
}

//-----------------------------------------------------------------------------
void FEPairedInterface::Load(IArchive &ar)
{
	TRACE("FEPairedInterface::Load");

	GModel& mdl = m_ps->GetModel();

	while (IArchive::IO_OK == ar.OpenChunk())
	{
		switch (ar.GetChunkID())
		{
		case CID_INTERFACE_NAME: { string name; ar.read(name); SetName(name); };
		case CID_FEOBJ_INFO: { string info; ar.read(info); SetInfo(info); } break;
		case CID_INTERFACE_ACTIVE: ar.read(m_bActive); break;
		case CID_INTERFACE_STEP: ar.read(m_nstepID); break;
		case CID_INTERFACE_PARAMS: ParamContainer::Load(ar); break;
		case CID_INTERFACE_SURFACE1: m_surf1 = FEInterface::LoadList(ar); break;
		case CID_INTERFACE_SURFACE2: m_surf2 = FEInterface::LoadList(ar); break;
		case CID_SI_MASTER: // obsolete in 1.8
		{
			// The old master surface is now the secondary surface
			int nid; ar.read(nid);
			m_surf2 = mdl.FindNamedSelection(nid);
			assert(m_surf2);
		}
		break;
		case CID_SI_SLAVE: // obsolete in 1.8
		{
			// The old slave surface is now the primary surface
			int nid; ar.read(nid);
			m_surf1 = mdl.FindNamedSelection(nid);
			assert(m_surf1);
		}
		break;
		default:
			throw ReadError("unknown CID in FEPairedInterface::Load");
		}

		ar.CloseChunk();
	}
}


//=============================================================================
// FESoloInterface
//-----------------------------------------------------------------------------

FESoloInterface::FESoloInterface(int ntype, FEModel* ps, int nstep) : FEInterface(ntype, ps, nstep)
{
	m_pItem = 0;
}

//-----------------------------------------------------------------------------
FESoloInterface::~FESoloInterface()
{
	delete m_pItem;
}

//-----------------------------------------------------------------------------
void FESoloInterface::Save(OArchive& ar)
{
	ar.WriteChunk(CID_INTERFACE_NAME  , GetName());
	ar.WriteChunk(CID_FEOBJ_INFO      , GetInfo());
	ar.WriteChunk(CID_INTERFACE_ACTIVE, m_bActive);
	ar.WriteChunk(CID_INTERFACE_STEP  , m_nstepID);
	ar.BeginChunk(CID_INTERFACE_PARAMS);
	{
		ParamContainer::Save(ar);
	}
	ar.EndChunk();
	if (m_pItem)
	{
		ar.BeginChunk(CID_INTERFACE_SURFACE1);
		FEInterface::SaveList(m_pItem, ar);
		ar.EndChunk();
	}
}

//-----------------------------------------------------------------------------
void FESoloInterface::Load(IArchive &ar)
{
	TRACE("FESoloInterface::Load");

	while (IArchive::IO_OK == ar.OpenChunk())
	{
		switch (ar.GetChunkID())
		{
		case CID_INTERFACE_NAME  : { string s; ar.read(s); SetName(s); } break;
		case CID_FEOBJ_INFO      : { string s; ar.read(s); SetInfo(s); } break;
		case CID_INTERFACE_ACTIVE: ar.read(m_bActive); break;
		case CID_INTERFACE_STEP  : ar.read(m_nstepID); break;
		case CID_INTERFACE_PARAMS: ParamContainer::Load(ar); break;
		case CID_INTERFACE_SURFACE1 : m_pItem = FEInterface::LoadList(ar); break;
		default:
			throw ReadError("unknown CID in FESoloInterface::Load");
		}
		ar.CloseChunk();
	}
}


//=============================================================================
// FERigidInterface
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
FERigidInterface::FERigidInterface(FEModel* ps, int nstep) : FESoloInterface(FE_RIGID_INTERFACE, ps, nstep)
{ 
	SetTypeString("Rigid");
	m_pmat = 0; 
}

//-----------------------------------------------------------------------------
FERigidInterface::FERigidInterface(FEModel* ps, GMaterial* pm, FEItemListBuilder* pi, int nstep) : FESoloInterface(FE_RIGID_INTERFACE, ps, nstep)
{
	SetTypeString("Rigid");
	m_pmat = pm; 
	m_pItem = pi; 
}

//-----------------------------------------------------------------------------
void FERigidInterface::Save(OArchive &ar)
{
	int mid = (m_pmat? m_pmat->GetID() : -1);
	ar.WriteChunk(CID_INTERFACE_NAME, GetName());
	ar.WriteChunk(CID_INTERFACE_ACTIVE, m_bActive);
	ar.WriteChunk(CID_INTERFACE_STEP, m_nstepID);
	ar.WriteChunk(CID_RI_RIGIDBODY, mid);
	if (m_pItem) 
	{ 
		ar.BeginChunk(CID_INTERFACE_SURFACE2);
		FEInterface::SaveList(m_pItem, ar);
		ar.EndChunk();
	}
}

//-----------------------------------------------------------------------------
void FERigidInterface::Load(IArchive &ar)
{
	TRACE("FERigidInterface::Load");

	while (IArchive::IO_OK == ar.OpenChunk())
	{
		switch (ar.GetChunkID())
		{
		case CID_INTERFACE_NAME:
			{
				char sz[256] = {0};
				ar.read(sz); SetName(sz);
			}
			break;
		case CID_INTERFACE_ACTIVE: ar.read(m_bActive); break;
		case CID_INTERFACE_STEP: ar.read(m_nstepID); break;
		case CID_RI_RIGIDBODY: 
			{
				int mid;
				ar.read(mid);
				m_pmat = m_ps->GetMaterialFromID(mid);
			}
			break;
		case CID_INTERFACE_SURFACE1: m_pItem = FEInterface::LoadList(ar); break;
		case CID_RI_LIST:	// obsolete in 1.8
			{
				int nid; ar.read(nid); 
				GModel& mdl = m_ps->GetModel();
				m_pItem = mdl.FindNamedSelection(nid);
				assert(m_pItem);
			}
			break;
		default:
			throw ReadError("unknown CID in FERigidInterface::Load");
		}

		ar.CloseChunk();
	}
}

//=============================================================================
// FERigidWallInterface
//-----------------------------------------------------------------------------

FERigidWallInterface::FERigidWallInterface(FEModel* ps, int nstep) : FESoloInterface(FE_RIGID_WALL, ps, nstep)
{
	SetTypeString("Rigid Wall");

	AddBoolParam  (false, "laugon"   , "augmented lagrangian"  );
	AddDoubleParam(0.2  , "tolerance", "augmentation tolerance");
	AddDoubleParam(1    , "penalty"  , "penalty factor"        );
	AddDoubleParam(0    , "a"        , "a"                     );
	AddDoubleParam(0    , "b"        , "b"                     );
	AddDoubleParam(0    , "c"        , "c"                     );
	AddDoubleParam(0    , "d"        , "d"                     );
	AddDoubleParam(0.0  , "offset"   , "plane displacment"     );
}

//-----------------------------------------------------------------------------
void FERigidWallInterface::GetPlaneEquation(double a[4])
{
	a[0] = GetParam(PA).GetFloatValue();
	a[1] = GetParam(PB).GetFloatValue();
	a[2] = GetParam(PC).GetFloatValue();
	a[3] = GetParam(PD).GetFloatValue();
}

//=============================================================================
// FERigidSphereInterface
//-----------------------------------------------------------------------------

FERigidSphereInterface::FERigidSphereInterface(FEModel* ps, int nstep) : FESoloInterface(FE_RIGID_SPHERE_CONTACT, ps, nstep)
{
	SetTypeString("Rigid Sphere");

	AddBoolParam  (false, "laugon", "augmented lagrangian");
	AddDoubleParam(0.2  , "tolerance", "augmentation tolerance");
	AddDoubleParam(1    , "penalty", "penalty factor");
	AddDoubleParam(0    , "radius", "radius");
	AddVecParam   (vec3d(0,0,0), "center", "center");

	// the default load curve defines a linear ramp
	// we don't want that here. By default, the plane should not move
	FELoadCurve LC;
	LC.Clear();
	LC.Add(LOADPOINT(0, 0));
	LC.Add(LOADPOINT(0, 0));
	AddDoubleParam(0.0, "ux", "x displacment")->SetLoadCurve(LC);
	AddDoubleParam(0.0, "uy", "y displacment")->SetLoadCurve(LC);
	AddDoubleParam(0.0, "uz", "z displacment")->SetLoadCurve(LC);
}

//-----------------------------------------------------------------------------
FELoadCurve* FERigidSphereInterface::GetLoadCurve(int i)
{ 
	return GetParamLC(FERigidSphereInterface::UX + i);
}

//-----------------------------------------------------------------------------
double FERigidSphereInterface::Radius()
{
	return GetFloatValue(RADIUS);
}

//-----------------------------------------------------------------------------
vec3d FERigidSphereInterface::Center()
{
	return GetVecValue(CENTER);
}

//=============================================================================
// FESlidingInterface
//-----------------------------------------------------------------------------

FESlidingInterface::FESlidingInterface(FEModel* ps, int nstep) : FEPairedInterface(FE_SLIDING_INTERFACE, ps, nstep)
{
	SetTypeString("Sliding contact");

	AddBoolParam  (false, "laugon"      , "augmented Lagrangian"   );
	AddDoubleParam(0.2  , "tolerance"   , "augmentation tolerance" );
	AddDoubleParam(1.0  , "penalty"     , "penalty"                );
	AddBoolParam  (false, "two_pass"    , "two pass"               );
	AddBoolParam  (false, "auto_penalty", "auto-penalty"           );
	AddDoubleParam(0    , "fric_coeff"  , "friction coefficient"   );
	AddDoubleParam(0    , "fric_penalty", "friction penalty factor");
	AddDoubleParam(0.01 , "search_tol"  , "search tolerance"       );
	AddIntParam   (0    , "type"        , "contact type"           )->SetEnumNames("node-on-facet\0facet-on-facet\0")->SetState(Param_EDITABLE | Param_PERSISTENT);
	AddDoubleParam(0    , "minaug"      , "min augmentations"      );
	AddDoubleParam(10   , "maxaug"      , "max augmentations"      );
	AddDoubleParam(0.0  , "gaptol"      , "gap tolerance"          );
	AddIntParam   (0    , "seg_up"      , "max segment updates"    );
	AddBoolParam(false, "update_penalty", "update-penalty");
}

//=============================================================================
// FESlidingWithGapsInterface
//-----------------------------------------------------------------------------

FESlidingWithGapsInterface::FESlidingWithGapsInterface(FEModel* ps, int nstep) : FEPairedInterface(FE_SLIDING_WITH_GAPS, ps, nstep)
{
	SetTypeString("Sliding with gaps");

	AddBoolParam  (false, "laugon"      , "augmented Lagrangian");
	AddDoubleParam(  0.2, "tolerance"   , "augmentation tolerance");
	AddDoubleParam(  1.0, "penalty"     , "penalty");
	AddBoolParam  (false, "two_pass"    , "two pass");
	AddBoolParam  (false, "auto_penalty", "auto-penalty");
	AddDoubleParam(    0, "fric_coeff"  , "friction coefficient");
	AddDoubleParam(    0, "fric_penalty", "friction penalty factor");
	AddDoubleParam( 0.01, "search_tol"  , "search tolerance");
	AddDoubleParam(    0, "minaug"      , "min augmentations");
	AddDoubleParam(   10, "maxaug"      , "max augmentations");
	AddDoubleParam(  0.0, "gaptol"      , "gap tolerance");
	AddIntParam   (    0, "seg_up"      , "max segment updates");
    AddBoolParam  (false, "update_penalty", "update-penalty"       );
}

//=============================================================================
// FEFacetOnFacetInterface
//-----------------------------------------------------------------------------

FEFacetOnFacetInterface::FEFacetOnFacetInterface(FEModel* ps, int nstep) : FEPairedInterface(FE_FACET_ON_FACET_SLIDING, ps, nstep)
{
	SetTypeString("Facet-on-facet sliding");

	AddBoolParam  (false, "laugon"      , "augmented Lagrangian");
	AddDoubleParam(  0.2, "tolerance"   , "augmentation tolerance");
	AddDoubleParam(  1.0, "penalty"     , "penalty");
	AddBoolParam  (false, "two_pass"    , "two pass");
	AddBoolParam  (false, "auto_penalty", "auto-penalty");
	AddDoubleParam(    0, "fric_coeff"  , "friction coefficient");
	AddDoubleParam(    0, "fric_penalty", "friction penalty factor");
	AddDoubleParam( 0.01, "search_tol"  , "search tolerance");
	AddDoubleParam(    0, "minaug"      , "min augmentations");
	AddDoubleParam(   10, "maxaug"      , "max augmentations");
	AddDoubleParam(  0.0, "gaptol"      , "gap tolerance");
	AddIntParam   (    0, "seg_up"      , "max segment updates");
	AddBoolParam(false, "update_penalty", "update-penalty");
	AddDoubleParam(0  , "search_radius", "search radius");
}

//=============================================================================
// FETiedInterface
//-----------------------------------------------------------------------------

FETiedInterface::FETiedInterface(FEModel* ps, int nstep) : FEPairedInterface(FE_TIED_INTERFACE, ps, nstep)
{
	SetTypeString("Tied contact");

	AddIntParam   (0    , "laugon"   , "Contact enforcement"   )->SetEnumNames("Penalty method\0Augmented Lagrangian\0Lagrange Multipliers\0");
	AddDoubleParam(0.2  , "tolerance", "augmentation tolerance");
	AddDoubleParam(1.0  , "penalty"  , "penalty factor"        );
	AddDoubleParam(0    , "minaug"   , "min augmentations"     );
	AddDoubleParam(10   , "maxaug"   , "max augmentations"     );
}

//=============================================================================
// FEF2FTiedInterface
//-----------------------------------------------------------------------------

FEF2FTiedInterface::FEF2FTiedInterface(FEModel* ps, int nstep) : FEPairedInterface(FE_FACET_ON_FACET_TIED, ps, nstep)
{
	SetTypeString("Tied facet-on-facet");

	AddBoolParam(false, "laugon", "augmented Lagrangian");
	AddDoubleParam(0.2, "tolerance", "augmentation tolerance");
	AddDoubleParam(1.0, "penalty", "penalty factor");
	AddDoubleParam(0, "minaug", "min augmentations");
	AddDoubleParam(10, "maxaug", "max augmentations");
}

//=============================================================================
// FEStickyInterface
//-----------------------------------------------------------------------------

FEStickyInterface::FEStickyInterface(FEModel* ps, int nstep) : FEPairedInterface(FE_STICKY_INTERFACE, ps, nstep)
{
	SetTypeString("Sticky contact");

	AddBoolParam  (false, "laugon"   , "augmented Lagrangian"  );
	AddDoubleParam(0.2  , "tolerance", "augmentation tolerance");
	AddDoubleParam(1.0  , "penalty"  , "penalty factor"        );
	AddDoubleParam(0    , "minaug"   , "min augmentations"     );
	AddDoubleParam(10   , "maxaug"   , "max augmentations"     );
	AddDoubleParam(0    , "snap_tol" , "snap tolerance");
	AddDoubleParam(0    , "max_traction", "max traction");
}

//=============================================================================
// FEPeriodicBoundary
//-----------------------------------------------------------------------------

FEPeriodicBoundary::FEPeriodicBoundary(FEModel* ps, int nstep) : FEPairedInterface(FE_PERIODIC_BOUNDARY, ps, nstep)
{
	SetTypeString("periodic boundary");

	AddBoolParam  (false, "laugon"   , "augmented Lagrangian"  );
	AddDoubleParam(0.2  , "tolerance", "augmentation tolerance");
	AddDoubleParam(1.0  , "penalty"  , "penalty factor"        );
	AddBoolParam  (false, "two_pass" , "two-pass"              );
}

//=============================================================================
// FEPoroContact
//-----------------------------------------------------------------------------

FEPoroContact::FEPoroContact(FEModel* ps, int nstep) : FEPairedInterface(FE_PORO_INTERFACE, ps, nstep)
{
	SetTypeString("Biphasic contact");

	AddBoolParam  (false, "laugon"             , "augmented Lagrangian"  );
	AddDoubleParam(0.2  , "tolerance"          , "augmentation tolerance");
    AddDoubleParam(0    , "gaptol"             , "gap tolerance"         );
    AddDoubleParam(0    , "ptol"               , "pressure tolerance"    );
	AddDoubleParam(1.0  , "penalty"            , "penalty factor"        );
	AddBoolParam  (false, "two_pass"           , "two pass"              );
	AddBoolParam  (false, "auto_penalty"       , "auto-penalty"          );
	AddDoubleParam(0.0  , "pressure_penalty"   , "pressure penalty"      );
	AddBoolParam  (false, "symmetric_stiffness", "symmetric stiffness"   );
	AddDoubleParam(1.0  , "search_radius"      , "search radius"         );
    AddIntParam   (0    , "seg_up"             , "segment updates"       );
    AddDoubleParam(0    , "minaug"             , "min augmentations"     );
    AddDoubleParam(10   , "maxaug"             , "max augmentations"     );
	AddDoubleParam(0.01 , "search_tol"         , "search tolerance"      );
	AddBoolParam(false  , "update_penalty"     , "update-penalty"        );
	AddBoolParam(false  , "smooth_aug"         , "Augmentation smoothing");
}

//=============================================================================
// FEPoroSoluteContact
//-----------------------------------------------------------------------------

FEPoroSoluteContact::FEPoroSoluteContact(FEModel* ps, int nstep) : FEPairedInterface(FE_PORO_SOLUTE_INTERFACE, ps, nstep)
{
	SetTypeString("Biphasic-solute contact");

	AddBoolParam  (false, "laugon"               , "augmented Lagrangian"   );
	AddDoubleParam(0.2  , "tolerance"            , "augmentation tolerance" );
    AddDoubleParam(0    , "gaptol"               , "gap tolerance"          );
    AddDoubleParam(0    , "ptol"                 , "pressure tolerance"     );
    AddDoubleParam(0    , "ctol"                 , "concentration tolerance");
	AddDoubleParam(1.0  , "penalty"              , "penalty factor"         );
	AddBoolParam  (false, "two_pass"             , "two pass"               );
	AddBoolParam  (false, "auto_penalty"         , "auto-penalty"           );
	AddDoubleParam(1.0  , "pressure_penalty"     , "pressure penalty"       );
	AddBoolParam  (false, "symmetric_stiffness"  , "symmetric stiffness"    );
	AddDoubleParam(1.0  , "concentration_penalty", "concentration penalty"  );
	AddDoubleParam(0.0  , "ambient_pressure"     , "ambient pressure"       );
	AddDoubleParam(0.0  , "ambient_concentration", "ambient concentration"  );
	AddDoubleParam(1.0  , "search_radius"        , "search radius"          );
    AddIntParam   (0    , "seg_up"               , "segment updates"        );
    AddDoubleParam(0    , "minaug"               , "min augmentations"      );
    AddDoubleParam(10   , "maxaug"               , "max augmentations"      );
	AddDoubleParam(0.01 , "search_tol"           , "search tolerance"       );
	AddBoolParam(false, "update_penalty", "update-penalty");
}

//=============================================================================
// FEMultiphasicContact
//-----------------------------------------------------------------------------

FEMultiphasicContact::FEMultiphasicContact(FEModel* ps, int nstep) : FEPairedInterface(FE_MULTIPHASIC_INTERFACE, ps, nstep)
{
	SetTypeString("Multiphasic contact");

	AddBoolParam  (false, "laugon"               , "augmented Lagrangian"   );
	AddDoubleParam(0.2  , "tolerance"            , "augmentation tolerance" );
    AddDoubleParam(0    , "gaptol"               , "gap tolerance"          );
    AddDoubleParam(0    , "ptol"                 , "pressure tolerance"     );
    AddDoubleParam(0    , "ctol"                 , "concentration tolerance");
	AddDoubleParam(1.0  , "penalty"              , "penalty factor"         );
	AddBoolParam  (false, "two_pass"             , "two pass"               );
	AddBoolParam  (false, "auto_penalty"         , "auto-penalty"           );
	AddDoubleParam(1.0  , "pressure_penalty"     , "pressure penalty"       );
	AddBoolParam  (false, "symmetric_stiffness"  , "symmetric stiffness"    );
	AddDoubleParam(1.0  , "concentration_penalty", "concentration penalty"  );
	AddDoubleParam(0.0  , "ambient_pressure"     , "ambient pressure"       );
	int NS = m_ps->Solutes();
    for (int i=0; i<NS; ++i) {
        char szvar1[256],szvar2[256];
		sprintf(szvar1, "ambient_concentration");
		sprintf(szvar2, "ambient concentration %d", i+1);
		FESoluteData& sol = m_ps->GetSoluteData(i);
        const char* sz1 = strdup(szvar1);
        const char* sz2 = strdup(szvar2);
        AddIndxDoubleParam(0.0  , "sol", i+1, sz1, sz2);
    }
	AddDoubleParam(1.0  , "search_radius"        , "search radius"          );
    AddIntParam   (0    , "seg_up"               , "segment updates"        );
    AddDoubleParam(0    , "minaug"               , "min augmentations"      );
    AddDoubleParam(10   , "maxaug"               , "max augmentations"      );
	AddBoolParam(false, "update_penalty", "update-penalty");
}

//=============================================================================
// FETensionCompressionInterface
//-----------------------------------------------------------------------------

FETensionCompressionInterface::FETensionCompressionInterface(FEModel* ps, int nstep) : FEPairedInterface(FE_TENSCOMP_INTERFACE, ps, nstep)
{
	SetTypeString("Sliding-elastic contact");

	AddBoolParam  (false, "laugon"             , "augmented Lagrangian"  );
	AddDoubleParam(0.2  , "tolerance"          , "augmentation tolerance");
	AddDoubleParam(0.0  , "gaptol"             , "gap tolerance"         );
	AddDoubleParam(1.0  , "penalty"            , "penalty factor"        );
	AddBoolParam  (false, "auto_penalty"       , "auto-penalty"          );
	AddBoolParam  (false, "two_pass"           , "two pass"              );
	AddDoubleParam(0.01 , "search_tol"         , "projection tolerance"  );
	AddBoolParam  (false, "symmetric_stiffness", "symmetric stiffness"   );
	AddDoubleParam(1.0  , "search_radius"      , "search radius"         );
	AddIntParam   (0    , "seg_up"             , "segment updates"       );
	AddBoolParam  (false, "tension"            , "Tension contact"       );
	AddDoubleParam(0    , "minaug"             , "min augmentations"     );
	AddDoubleParam(10   , "maxaug"             , "max augmentations"     );
	AddDoubleParam(0    , "fric_coeff"         , "friction coefficient"  );
	AddBoolParam  (false, "smooth_aug"         , "Augmentation smoothing");
	AddBoolParam  (false, "node_reloc"         , "Relocated nodes"       );
    AddBoolParam  (false, "flip_primary"       , "flip normal on primary"  );
    AddBoolParam  (false, "flip_secondary"     , "flip normal on secondary");
	AddIntParam   (0    , "knmult"             , "higher-order stiffness");
	AddBoolParam  (false, "update_penalty"     , "update-penalty");
    AddBoolParam  (false, "shell_bottom_primary"  , "use shell bottom on primary"  );
    AddBoolParam  (false, "shell_bottom_secondary", "use shell bottom on secondary");
}

//=============================================================================
// FETiedBiphasicInterface
//-----------------------------------------------------------------------------

FETiedBiphasicInterface::FETiedBiphasicInterface(FEModel* ps, int nstep) : FEPairedInterface(FE_TIEDBIPHASIC_INTERFACE, ps, nstep)
{
	SetTypeString("Tied biphasic contact");

	AddBoolParam  (false, "laugon"             , "augmented Lagrangian"   );
	AddDoubleParam(0.2  , "tolerance"          , "augmentation tolerance" );
	AddDoubleParam(0    , "gaptol"             , "gap tolerance"          );
	AddDoubleParam(0    , "ptol"               , "pressure tolerance"     );
	AddDoubleParam(1.0  , "penalty"            , "penalty factor"         );
	AddBoolParam  (false, "auto_penalty"       , "auto-penalty"           );
	AddBoolParam  (false, "two_pass"           , "two pass"               );
	AddDoubleParam(0.01 , "search_tol"         , "projection tolerance"   );
	AddDoubleParam(1.0  , "pressure_penalty"   , "pressure penalty factor");
	AddBoolParam  (false, "symmetric_stiffness", "symmetric stiffness"    );
	AddDoubleParam(1.0  , "search_radius"      , "search radius"          );
	AddDoubleParam(0    , "minaug"             , "min augmentations"      );
	AddDoubleParam(10   , "maxaug"             , "max augmentations"      );
	AddBoolParam(false, "update_penalty", "update-penalty");
}

//=============================================================================
// FETiedMultiphasicInterface
//-----------------------------------------------------------------------------

FETiedMultiphasicInterface::FETiedMultiphasicInterface(FEModel* ps, int nstep) : FEPairedInterface(FE_TIEDMULTIPHASIC_INTERFACE, ps, nstep)
{
	SetTypeString("Tied multiphasic contact");

	AddBoolParam(false, "laugon", "augmented Lagrangian");
	AddDoubleParam(0.2, "tolerance", "augmentation tolerance");
	AddDoubleParam(0, "gaptol", "gap tolerance");
	AddDoubleParam(0, "ptol", "pressure tolerance");
	AddDoubleParam(1.0, "penalty", "penalty factor");
	AddBoolParam(false, "auto_penalty", "auto-penalty");
	AddBoolParam(false, "two_pass", "two pass");
	AddDoubleParam(0.01, "search_tol", "projection tolerance");
	AddDoubleParam(1.0, "pressure_penalty", "pressure penalty factor");
	AddDoubleParam(1.0, "concentration_penalty", "concentration penalty factor");
	AddBoolParam(false, "symmetric_stiffness", "symmetric stiffness");
	AddDoubleParam(1.0, "search_radius", "search radius");
	AddDoubleParam(0, "minaug", "min augmentations");
	AddDoubleParam(10, "maxaug", "max augmentations");
	AddBoolParam(false, "update_penalty", "update-penalty");
}

//=============================================================================
// FETiedElasticInterface
//-----------------------------------------------------------------------------

FETiedElasticInterface::FETiedElasticInterface(FEModel* ps, int nstep) : FEPairedInterface(FE_TIED_ELASTIC_INTERFACE, ps, nstep)
{
	SetTypeString("Tied-elastic");

	AddBoolParam(false, "laugon", "augmented Lagrangian");
	AddDoubleParam(0.2, "tolerance", "augmentation tolerance");
	AddDoubleParam(0, "gaptol", "gap tolerance");
	AddDoubleParam(1.0, "penalty", "penalty factor");
	AddBoolParam(false, "auto_penalty", "auto-penalty");
	AddBoolParam(false, "two_pass", "two pass");
	AddIntParam(1, "knmult");
	AddDoubleParam(0.01, "search_tol", "projection tolerance");
	AddBoolParam(false, "symmetric_stiffness", "symmetric stiffness");
	AddDoubleParam(1.0, "search_radius", "search radius");
	AddDoubleParam(0, "minaug", "min augmentations");
	AddDoubleParam(10, "maxaug", "max augmentations");
	AddBoolParam(false, "update_penalty", "update-penalty");
}

//=============================================================================
// FEGapHeatFluxInterface
//-----------------------------------------------------------------------------

FEGapHeatFluxInterface::FEGapHeatFluxInterface(FEModel* fem, int nstep) : FEPairedInterface(FE_GAPHEATFLUX_INTERFACE, fem, nstep)
{
	SetTypeString("Gap heat flux");

	AddDoubleParam(0.0, "hc", "heat transfer coefficient");
	AddDoubleParam(0.01, "search_tol", "search tolerance");
}

//=============================================================================
// FERigidJoint
//-----------------------------------------------------------------------------

FERigidJoint::FERigidJoint(FEModel* ps, int nstep) : FEInterface(FE_RIGID_JOINT, ps, nstep)
{ 
	m_pbodyA = m_pbodyB = 0;

	SetTypeString("Rigid joint");

	AddDoubleParam(0.1         , "tol", "augmentation tolerance");
	AddDoubleParam(1           , "pen", "penalty factor"        );
	AddVecParam   (vec3d(0,0,0), "rj" , "joint position"        );
}

//-----------------------------------------------------------------------------
void FERigidJoint::Save(OArchive& ar)
{
	ar.WriteChunk(CID_INTERFACE_NAME, GetName());
	ar.WriteChunk(CID_INTERFACE_ACTIVE, m_bActive);
	ar.WriteChunk(CID_INTERFACE_STEP, m_nstepID);
	ar.BeginChunk(CID_INTERFACE_PARAMS);
	{
		ParamContainer::Save(ar);
	}
	ar.EndChunk();

	int ma = (m_pbodyA? m_pbodyA->GetID() : -1);
	int mb = (m_pbodyB? m_pbodyB->GetID() : -1);
	ar.WriteChunk(CID_RJ_RIGIDBODY_A, ma);
	ar.WriteChunk(CID_RJ_RIGIDBODY_B, mb);
}

//-----------------------------------------------------------------------------
void FERigidJoint::Load(IArchive& ar)
{
	TRACE("FERigidJoint::Load");
	while (IArchive::IO_OK == ar.OpenChunk())
	{
		switch (ar.GetChunkID())
		{
		case CID_INTERFACE_NAME:
			{
				char sz[256] = {0};
				ar.read(sz); SetName(sz);
			}
			break;
		case CID_INTERFACE_ACTIVE: ar.read(m_bActive); break;
		case CID_INTERFACE_STEP: ar.read(m_nstepID); break;
		case CID_INTERFACE_PARAMS: ParamContainer::Load(ar); break;
		case CID_RJ_RIGIDBODY_A: 
			{
				int mid; ar.read(mid);
				m_pbodyA = m_ps->GetMaterialFromID(mid);
			}
			break;
		case CID_RJ_RIGIDBODY_B: 
			{
				int mid; ar.read(mid);
				m_pbodyB = m_ps->GetMaterialFromID(mid);
			}
			break;
		default:
			throw ReadError("unknown CID in FERigidJoint::Load");
		}
		ar.CloseChunk();
	}
}

//=============================================================================
// FESpringTiedInterface
//-----------------------------------------------------------------------------

FESpringTiedInterface::FESpringTiedInterface(FEModel* ps, int nstep) : FEPairedInterface(FE_SPRINGTIED_INTERFACE,ps, nstep)
{
	SetTypeString("Spring-tied contact");

	AddDoubleParam(0.0, "E", "Spring constant");
}

double FESpringTiedInterface::SpringConstant() const
{
	return GetFloatValue(ECONST);
}

void FESpringTiedInterface::BuildSpringList(vector<pair<int, int> >& L)
{
	FEFaceList* pfl = m_surf1->BuildFaceList();
	FENodeList* pnl = m_surf2->BuildNodeList();
	if ((pfl == 0) || (pnl == 0)) return;

	auto_ptr<FEFaceList> ps(pfl);
	auto_ptr<FENodeList> pm(pnl);

	set<pair<int, int> > S;

	FEFaceList::Iterator its = ps->First();
	for (int i=0; i<ps->Size(); ++i, ++its)
	{
		FEFace& f = *(its->m_pi);
		FEMesh& mesh = dynamic_cast<FEMesh&>(*(its->m_pm));
		int nf = f.Nodes();
		for (int n=0; n<nf; ++n)
		{
			FENode& ns = mesh.Node(f.n[n]);
			vec3d nn = f.m_nn[n];
			vec3d nr = ns.r;

			int i0 = ns.m_nid;
			int i1 = -1;
			double Dmin = 0;
			FENodeList::Iterator itm = pm->First();
			for (int j=0; j<pm->Size(); ++j, ++itm)
			{
				FENode& nj = *(itm->m_pi);
				vec3d q = (itm->m_pi)->r;

				vec3d s = nr + nn*((q - nr)*nn);
				double D = (q - s).Length();
				if ((j == 0) || (D < Dmin))
				{
					i1 = nj.m_nid;
					Dmin = D;
				}
			}
			assert(i1 > -1);
			if (i1 > -1)
			{
				S.insert(pair<int,int>(i0,i1));
			}
		}
	}

	// convert the set to a vector
	if (S.empty() == false)
	{
		L.reserve(S.size());
		set<pair<int, int> >::iterator it;
		for (it = S.begin(); it != S.end(); ++it) L.push_back(*it);
	}
}

//=============================================================================
// FELinearConstraintSet
//-----------------------------------------------------------------------------

FELinearConstraintSet::FELinearConstraintSet()
{
	m_atol = 0.1;
	m_penalty = 1.0;
	m_nmaxaug = 50;
}

FELinearConstraintSet::FELinearConstraintSet(const FELinearConstraintSet& lcs)
{
	m_atol = lcs.m_atol;
	m_penalty = lcs.m_penalty;
	m_nmaxaug = lcs.m_nmaxaug;

	m_set = lcs.m_set;
}

FELinearConstraintSet& FELinearConstraintSet::operator = (const FELinearConstraintSet& lcs)
{
	m_atol = lcs.m_atol;
	m_penalty = lcs.m_penalty;
	m_nmaxaug = lcs.m_nmaxaug;

	m_set = lcs.m_set;

	return (*this);
}
