/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2021 University of Utah, The Trustees of Columbia University in
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
#include "FSModel.h"
#include <GeomLib/GGroup.h>
#include <GeomLib/GModel.h>
#include <set>
#include <memory>

using namespace std;

//=============================================================================
// FSInterface
//-----------------------------------------------------------------------------

FSInterface::FSInterface(int ntype, FSModel* ps, int nstep) : FSStepComponent(ps)
{
	m_ntype = ntype;
	m_nstepID = nstep;
	m_superClassID = FESURFACEINTERFACE_ID;
	m_bActive = true;
}

//-----------------------------------------------------------------------------
FSInterface::~FSInterface()
{

}

//-----------------------------------------------------------------------------
void FSInterface::SaveList(FSItemListBuilder* pitem, OArchive& ar)
{
	if (pitem)
	{
		ar.BeginChunk((int) pitem->Type());
		pitem->Save(ar);
		ar.EndChunk();
	}
}

//-----------------------------------------------------------------------------
FSItemListBuilder* FSInterface::LoadList(IArchive& ar)
{
	FSItemListBuilder* pitem = 0;

	FSModel* fem = GetFSModel();
	GModel* gm = &fem->GetModel();

	if (ar.OpenChunk() != IArchive::IO_OK) throw ReadError("error in FSInterface::LoadList");
	unsigned int ntype = ar.GetChunkID();
	switch (ntype)
	{
	case GO_NODE: pitem = new GNodeList(gm); break;
	case GO_EDGE: pitem = new GEdgeList(gm); break;
	case GO_FACE: pitem = new GFaceList(gm); break;
	case GO_PART: pitem = new GPartList(gm); break;
	case FE_NODESET: pitem = new FSNodeSet(nullptr); break;
	case FE_EDGESET: pitem = new FSEdgeSet(nullptr); break;
	case FE_SURFACE: pitem = new FSSurface(nullptr); break;
	case FE_ELEMSET: pitem = new FSElemSet(nullptr); break;
	default:
		assert(false);
	}
	if (pitem == 0) throw ReadError("unknown item list type (FSInterface::LoadList)");

	pitem->Load(ar);
	ar.CloseChunk();

	int nret = ar.OpenChunk();
	if (nret != IArchive::IO_END) throw ReadError("error in FSInterface::LoadList");

	// set the parent mesh for FSGroup's
	FSGroup* pg = dynamic_cast<FSGroup*>(pitem);
	if (pg)
	{
		if (fem->FindGroupParent(pg) == false)
		{
			ar.log("Invalid object ID in FSInterface::LoadList");
			delete pitem;
			pitem = nullptr;
		}
	}

	return pitem;
}

//=============================================================================
// FSPairedInterface
//-----------------------------------------------------------------------------

FSPairedInterface::FSPairedInterface(int ntype, FSModel* ps, int nstep) : FSInterface(ntype, ps, nstep)
{
	SetMeshItemType(FE_FACE_FLAG);
}

//-----------------------------------------------------------------------------
FSPairedInterface::~FSPairedInterface()
{
}

//-----------------------------------------------------------------------------
void FSPairedInterface::SetPrimarySurface(FSItemListBuilder* pg)
{ 
	SetItemList(pg, 0);
}

//-----------------------------------------------------------------------------
void FSPairedInterface::SetSecondarySurface(FSItemListBuilder* pg)
{ 
	SetItemList(pg, 1);
}

//-----------------------------------------------------------------------------
FSItemListBuilder* FSPairedInterface::GetPrimarySurface() { return GetItemList(0); }

//-----------------------------------------------------------------------------
FSItemListBuilder* FSPairedInterface::GetSecondarySurface() { return GetItemList(1); }

//-----------------------------------------------------------------------------
void FSPairedInterface::SwapPrimarySecondary()
{
	FSItemListBuilder* tmp = GetItemList(0);
	SetItemList(GetItemList(1), 0);
	SetItemList(tmp, 1);
}

//-----------------------------------------------------------------------------
void FSPairedInterface::Save(OArchive& ar)
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

	FSItemListBuilder* surf1 = GetItemList(0);
	FSItemListBuilder* surf2 = GetItemList(1);
	if (surf1) ar.WriteChunk(CID_INTERFACE_SURFACE1_ID, surf1->GetID());
	if (surf2) ar.WriteChunk(CID_INTERFACE_SURFACE2_ID, surf2->GetID());
}

//-----------------------------------------------------------------------------
void FSPairedInterface::Load(IArchive &ar)
{
	TRACE("FSPairedInterface::Load");

	GModel& mdl = GetFSModel()->GetModel();

	while (IArchive::IO_OK == ar.OpenChunk())
	{
		switch (ar.GetChunkID())
		{
		case CID_INTERFACE_NAME: { string name; ar.read(name); SetName(name); } break;
		case CID_FEOBJ_INFO: { string info; ar.read(info); SetInfo(info); } break;
		case CID_INTERFACE_ACTIVE: ar.read(m_bActive); break;
		case CID_INTERFACE_STEP: ar.read(m_nstepID); break;
		case CID_INTERFACE_PARAMS: ParamContainer::Load(ar); break;
		case CID_INTERFACE_SURFACE1_ID: 
		{ 
			int nid = -1; 
			ar.read(nid); 
			FSItemListBuilder* surf1 = mdl.FindNamedSelection(nid); 
			SetItemList(surf1, 0);
		}
		break;
		case CID_INTERFACE_SURFACE2_ID: 
		{ 
			int nid = -1; 
			ar.read(nid); 
			FSItemListBuilder* surf2 = mdl.FindNamedSelection(nid); 
			SetItemList(surf2, 1);
		} 
		break;
		case CID_INTERFACE_SURFACE1: // obsolete in 2.1
		{
			FSItemListBuilder* surf1 = FSInterface::LoadList(ar);
			if (surf1)
			{
				if (surf1->GetName().empty())
				{
					string s = GetName();
					s += "Primary";
					surf1->SetName(s);
				}
				mdl.AddNamedSelection(surf1);
				SetPrimarySurface(surf1);
			}
		}
		break;
		case CID_INTERFACE_SURFACE2: // obsolete in 2.1
		{
			FSItemListBuilder* surf2 = FSInterface::LoadList(ar);
			if (surf2)
			{
				if (surf2->GetName().empty())
				{
					string s = GetName();
					s += "Secondary";
					surf2->SetName(s);
				}
				mdl.AddNamedSelection(surf2);
				SetSecondarySurface(surf2);
			}
		}
		break;
		case CID_SI_MASTER: // obsolete in 1.8
		{
			// The old master surface is now the secondary surface
			int nid; ar.read(nid);
			FSItemListBuilder* surf2 = mdl.FindNamedSelection(nid); assert(surf2);
			SetSecondarySurface(surf2);
		}
		break;
		case CID_SI_SLAVE: // obsolete in 1.8
		{
			// The old slave surface is now the primary surface
			int nid; ar.read(nid);
			FSItemListBuilder* surf1 = mdl.FindNamedSelection(nid); assert(surf1);
			SetPrimarySurface(surf1);
		}
		break;
		default:
			throw ReadError("unknown CID in FSPairedInterface::Load");
		}

		ar.CloseChunk();
	}
}


//=============================================================================
// FSSoloInterface
//-----------------------------------------------------------------------------

FSSoloInterface::FSSoloInterface(int ntype, FSModel* ps, int nstep) : FSInterface(ntype, ps, nstep)
{
	SetMeshItemType(FE_FACE_FLAG);
}

//-----------------------------------------------------------------------------
FSSoloInterface::~FSSoloInterface()
{

}

//-----------------------------------------------------------------------------
void FSSoloInterface::Save(OArchive& ar)
{
	ar.WriteChunk(CID_INTERFACE_NAME  , GetName());
	ar.WriteChunk(CID_FEOBJ_INFO      , GetInfo());
	ar.WriteChunk(CID_INTERFACE_ACTIVE, m_bActive);
	ar.WriteChunk(CID_INTERFACE_STEP  , m_nstepID);
	FSItemListBuilder* pl = GetItemList();
	if (pl)
	{
		ar.WriteChunk(CID_INTERFACE_SURFACE1_ID, pl->GetID());
	}
	ar.BeginChunk(CID_INTERFACE_PARAMS);
	{
		ParamContainer::Save(ar);
	}
	ar.EndChunk();
}

//-----------------------------------------------------------------------------
void FSSoloInterface::Load(IArchive &ar)
{
	TRACE("FSSoloInterface::Load");

	FSModel* fem = GetFSModel();
	GModel* pgm = &fem->GetModel();

	while (IArchive::IO_OK == ar.OpenChunk())
	{
		switch (ar.GetChunkID())
		{
		case CID_INTERFACE_NAME  : { string s; ar.read(s); SetName(s); } break;
		case CID_FEOBJ_INFO      : { string s; ar.read(s); SetInfo(s); } break;
		case CID_INTERFACE_ACTIVE: ar.read(m_bActive); break;
		case CID_INTERFACE_STEP  : ar.read(m_nstepID); break;
		case CID_INTERFACE_PARAMS: ParamContainer::Load(ar); break;
		case CID_INTERFACE_SURFACE1_ID : 
		{
			int nid = 0;
			ar.read(nid);
			FSItemListBuilder* pItem = pgm->FindNamedSelection(nid);
			SetItemList(pItem);
		}
		break;
		case CID_INTERFACE_SURFACE1 :
		{
			// NOTE: We should only get here for older files (< FBS2.1)
			//       since model components no longer manage their own lists.
			//       If we get here, we'll read the list and then add it to the model
			FSItemListBuilder* pItem = FSInterface::LoadList(ar);
			if (pItem)
			{
				// this selection probably doesn't have a name yet.
				// We'll give it the same name as the domain component
				if (pItem->GetName().empty())
				{
					pItem->SetName(GetName());
				}

				pgm->AddNamedSelection(pItem);
				SetItemList(pItem);
			}
		}
		break;
		default:
			throw ReadError("unknown CID in FSSoloInterface::Load");
		}
		ar.CloseChunk();
	}
}


//=============================================================================
// FSRigidInterface
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
FSRigidInterface::FSRigidInterface(FSModel* ps, int nstep) : FSSoloInterface(FE_RIGID_INTERFACE, ps, nstep)
{ 
	SetTypeString("Rigid");
	m_pmat = 0; 
}

//-----------------------------------------------------------------------------
FSRigidInterface::FSRigidInterface(FSModel* ps, GMaterial* pm, FSItemListBuilder* pi, int nstep) : FSSoloInterface(FE_RIGID_INTERFACE, ps, nstep)
{
	SetTypeString("Rigid");
	m_pmat = pm; 
	SetItemList(pi); 
}

//-----------------------------------------------------------------------------
void FSRigidInterface::Save(OArchive &ar)
{
	int mid = (m_pmat? m_pmat->GetID() : -1);
	ar.WriteChunk(CID_INTERFACE_NAME, GetName());
	ar.WriteChunk(CID_INTERFACE_ACTIVE, m_bActive);
	ar.WriteChunk(CID_INTERFACE_STEP, m_nstepID);
	ar.WriteChunk(CID_RI_RIGIDBODY, mid);

	FSItemListBuilder* pl = GetItemList();
	if (pl) 
	{ 
		ar.WriteChunk(CID_INTERFACE_SURFACE1_ID, pl->GetID());
	}
}

//-----------------------------------------------------------------------------
void FSRigidInterface::Load(IArchive &ar)
{
	TRACE("FSRigidInterface::Load");

	FSModel* fem = GetFSModel();
	GModel* pgm = &fem->GetModel();

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
				m_pmat = GetFSModel()->GetMaterialFromID(mid);
			}
			break;
		case CID_INTERFACE_SURFACE1_ID:
		{
			int nid = 0;
			ar.read(nid);
			FSItemListBuilder* pItem = pgm->FindNamedSelection(nid);
			SetItemList(pItem);
		}
		break;
		case CID_INTERFACE_SURFACE2: 
		{
			// NOTE: We should only get here for older files (< FBS2.1)
			//       since model components no longer manage their own lists.
			//       If we get here, we'll read the list and then add it to the model
			FSItemListBuilder* pItem = FSInterface::LoadList(ar);
			if (pItem)
			{
				// this selection probably doesn't have a name yet.
				// We'll give it the same name as the domain component
				if (pItem->GetName().empty())
				{
					pItem->SetName(GetName());
				}

				pgm->AddNamedSelection(pItem);
				SetItemList(pItem);
			}
		}
		break;
		case CID_RI_LIST:	// obsolete in 1.8
			{
				int nid; ar.read(nid); 
				FSItemListBuilder* pItem = pgm->FindNamedSelection(nid);
				assert(pItem);
				SetItemList(pItem);
			}
			break;
		default:
			throw ReadError("unknown CID in FSRigidInterface::Load");
		}

		ar.CloseChunk();
	}
}

//=============================================================================
// FSRigidWallInterface
//-----------------------------------------------------------------------------

FSRigidWallInterface::FSRigidWallInterface(FSModel* ps, int nstep) : FSSoloInterface(FE_RIGID_WALL, ps, nstep)
{
	SetTypeString("Rigid Wall");

	AddBoolParam  (false, "laugon"   , "augmented lagrangian"  );
	AddDoubleParam(0.2  , "tolerance", "augmentation tolerance");
	AddDoubleParam(1    , "penalty"  , "penalty factor"        );
	AddDoubleParam(0    , "a"        , "Normal-x"              );
	AddDoubleParam(0    , "b"        , "Normal-y"              );
	AddDoubleParam(0    , "c"        , "Normal-z"              );
	AddDoubleParam(0    , "d"        , "distance to origin"    );
	AddDoubleParam(0.0  , "offset"   , "plane displacment"     );
}

//-----------------------------------------------------------------------------
void FSRigidWallInterface::GetPlaneEquation(double a[4])
{
	a[0] = GetParam(PA).GetFloatValue();
	a[1] = GetParam(PB).GetFloatValue();
	a[2] = GetParam(PC).GetFloatValue();
	a[3] = GetParam(PD).GetFloatValue();
}

//=============================================================================
// FSRigidSphereInterface
//-----------------------------------------------------------------------------

FSRigidSphereInterface::FSRigidSphereInterface(FSModel* ps, int nstep) : FSSoloInterface(FE_RIGID_SPHERE_CONTACT, ps, nstep)
{
	SetTypeString("Rigid Sphere");

	AddBoolParam  (false, "laugon", "augmented lagrangian");
	AddDoubleParam(0.2  , "tolerance", "augmentation tolerance");
	AddDoubleParam(1    , "penalty", "penalty factor");
	AddDoubleParam(0    , "radius", "radius");
	AddVecParam   (vec3d(0,0,0), "center", "center");

	// the default load curve defines a linear ramp
	// we don't want that here. By default, the plane should not move
	LoadCurve LC;
	LC.Clear();
	LC.Add(0, 0);
	LC.Add(0, 0);
	AddDoubleParam(0.0, "ux", "x displacment");
	AddDoubleParam(0.0, "uy", "y displacment");
	AddDoubleParam(0.0, "uz", "z displacment");
}

//-----------------------------------------------------------------------------
double FSRigidSphereInterface::Radius()
{
	return GetFloatValue(RADIUS);
}

//-----------------------------------------------------------------------------
vec3d FSRigidSphereInterface::Center()
{
	return GetVecValue(CENTER);
}

//=============================================================================
// FSSlidingInterface
//-----------------------------------------------------------------------------

FSSlidingInterface::FSSlidingInterface(FSModel* ps, int nstep) : FSPairedInterface(FE_SLIDING_INTERFACE, ps, nstep)
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
// FSSlidingWithGapsInterface
//-----------------------------------------------------------------------------

FSSlidingWithGapsInterface::FSSlidingWithGapsInterface(FSModel* ps, int nstep) : FSPairedInterface(FE_SLIDING_WITH_GAPS, ps, nstep)
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
// FSFacetOnFacetInterface
//-----------------------------------------------------------------------------

FSFacetOnFacetInterface::FSFacetOnFacetInterface(FSModel* ps, int nstep) : FSPairedInterface(FE_FACET_ON_FACET_SLIDING, ps, nstep)
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
// FSTiedInterface
//-----------------------------------------------------------------------------

FSTiedInterface::FSTiedInterface(FSModel* ps, int nstep) : FSPairedInterface(FE_TIED_INTERFACE, ps, nstep)
{
	SetTypeString("Tied contact");

	AddIntParam   (0    , "laugon"   , "Contact enforcement"   )->SetEnumNames("Penalty method\0Augmented Lagrangian\0Lagrange Multipliers\0");
	AddDoubleParam(0.2  , "tolerance", "augmentation tolerance");
	AddDoubleParam(1.0  , "penalty"  , "penalty factor"        );
	AddDoubleParam(0    , "minaug"   , "min augmentations"     );
	AddDoubleParam(10   , "maxaug"   , "max augmentations"     );
}

//=============================================================================
// FSF2FTiedInterface
//-----------------------------------------------------------------------------

FSF2FTiedInterface::FSF2FTiedInterface(FSModel* ps, int nstep) : FSPairedInterface(FE_FACET_ON_FACET_TIED, ps, nstep)
{
	SetTypeString("Tied facet-on-facet");

	AddBoolParam(false, "laugon", "augmented Lagrangian");
	AddDoubleParam(0.2, "tolerance", "augmentation tolerance");
	AddDoubleParam(1.0, "penalty", "penalty factor");
	AddDoubleParam(0, "minaug", "min augmentations");
	AddDoubleParam(10, "maxaug", "max augmentations");
	AddBoolParam(false, "gap_offset", "gap offset");
}

//=============================================================================
// FSStickyInterface
//-----------------------------------------------------------------------------

FSStickyInterface::FSStickyInterface(FSModel* ps, int nstep) : FSPairedInterface(FE_STICKY_INTERFACE, ps, nstep)
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
// FSPeriodicBoundary
//-----------------------------------------------------------------------------

FSPeriodicBoundary::FSPeriodicBoundary(FSModel* ps, int nstep) : FSPairedInterface(FE_PERIODIC_BOUNDARY, ps, nstep)
{
	SetTypeString("periodic boundary");

	AddBoolParam  (false, "laugon"   , "augmented Lagrangian"  );
	AddDoubleParam(0.2  , "tolerance", "augmentation tolerance");
	AddDoubleParam(1.0  , "penalty"  , "penalty factor"        );
	AddBoolParam  (false, "two_pass" , "two-pass"              );
}

//=============================================================================
// FSPoroContact
//-----------------------------------------------------------------------------

FSPoroContact::FSPoroContact(FSModel* ps, int nstep) : FSPairedInterface(FE_PORO_INTERFACE, ps, nstep)
{
	SetTypeString("Biphasic contact");

	AddBoolParam  (false, "laugon"                , "augmented Lagrangian"         );
	AddDoubleParam(0.2  , "tolerance"             , "augmentation tolerance"       );
    AddDoubleParam(0    , "gaptol"                , "gap tolerance"                );
    AddDoubleParam(0    , "ptol"                  , "pressure tolerance"           );
    AddDoubleParam(1.0  , "penalty"               , "penalty factor"               );
	AddBoolParam  (false, "two_pass"              , "two pass"                     );
	AddBoolParam  (false, "auto_penalty"          , "auto-penalty"                 );
	AddDoubleParam(0.0  , "pressure_penalty"      , "pressure penalty"             );
	AddBoolParam  (false, "symmetric_stiffness"   , "symmetric stiffness"          );
	AddDoubleParam(1.0  , "search_radius"         , "search radius"                );
	AddIntParam   (0    , "seg_up"                , "segment updates"              );
	AddDoubleParam(0    , "minaug"                , "min augmentations"            );
	AddDoubleParam(10   , "maxaug"                , "max augmentations"            );
	AddDoubleParam(0.01 , "search_tol"            , "search tolerance"             );
	AddBoolParam  (false, "update_penalty"        , "update-penalty"               );
	AddBoolParam  (false, "smooth_aug"            , "augmentation smoothing"       );
	AddDoubleParam(0    , "fric_coeff"            , "friction coefficient"         );
    AddDoubleParam(0    , "contact_frac"          , "solid-solid area fraction"    );
    AddBoolParam  (false, "smooth_fls"            , "smooth fluid load support"    );
    AddIntParam   (0    , "knmult"                , "higher-order stiffness"       );
    AddBoolParam  (false, "node_reloc"            , "relocated nodes"              );
    AddBoolParam  (false, "flip_primary"          , "flip normal on primary"       );
    AddBoolParam  (false, "flip_secondary"        , "flip normal on secondary"     );
    AddBoolParam  (false, "shell_bottom_primary"  , "use shell bottom on primary"  );
    AddBoolParam  (false, "shell_bottom_secondary", "use shell bottom on secondary");
}

//=============================================================================
// FSPoroSoluteContact
//-----------------------------------------------------------------------------

FSPoroSoluteContact::FSPoroSoluteContact(FSModel* ps, int nstep) : FSPairedInterface(FE_PORO_SOLUTE_INTERFACE, ps, nstep)
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
// FSMultiphasicContact
//-----------------------------------------------------------------------------

FSMultiphasicContact::FSMultiphasicContact(FSModel* ps, int nstep) : FSPairedInterface(FE_MULTIPHASIC_INTERFACE, ps, nstep)
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
	int NS = GetFSModel()->Solutes();
    for (int i=0; i<NS; ++i) {
        char szvar1[256],szvar2[256];
		sprintf(szvar1, "ambient_concentration");
		sprintf(szvar2, "ambient concentration %d", i+1);
		SoluteData& sol = GetFSModel()->GetSoluteData(i);
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
// FSTensionCompressionInterface
//-----------------------------------------------------------------------------

FSTensionCompressionInterface::FSTensionCompressionInterface(FSModel* ps, int nstep) : FSPairedInterface(FE_TENSCOMP_INTERFACE, ps, nstep)
{
	SetTypeString("Sliding-elastic contact");

	AddBoolParam  (false, "laugon"                , "augmented Lagrangian"         );
	AddDoubleParam(0.2  , "tolerance"             , "augmentation tolerance"       );
	AddDoubleParam(0.0  , "gaptol"                , "gap tolerance"                );
	AddDoubleParam(1.0  , "penalty"               , "penalty factor"               );
	AddBoolParam  (false, "auto_penalty"          , "auto-penalty"                 );
	AddBoolParam  (false, "two_pass"              , "two pass"                     );
	AddDoubleParam(0.01 , "search_tol"            , "projection tolerance"         );
	AddBoolParam  (false, "symmetric_stiffness"   , "symmetric stiffness"          );
	AddDoubleParam(1.0  , "search_radius"         , "search radius"                );
	AddIntParam   (0    , "seg_up"                , "segment updates"              );
	AddBoolParam  (false, "tension"               , "Tension contact"              );
	AddDoubleParam(0    , "minaug"                , "min augmentations"            );
	AddDoubleParam(10   , "maxaug"                , "max augmentations"            );
	AddDoubleParam(0    , "fric_coeff"            , "friction coefficient"         );
	AddBoolParam  (false, "smooth_aug"            , "Augmentation smoothing"       );
	AddBoolParam  (false, "node_reloc"            , "Relocated nodes"              );
    AddBoolParam  (false, "flip_primary"          , "flip normal on primary"       );
    AddBoolParam  (false, "flip_secondary"        , "flip normal on secondary"     );
	AddIntParam   (0    , "knmult"                , "higher-order stiffness"       );
	AddBoolParam  (false, "update_penalty"        , "update-penalty"               );
    AddBoolParam  (false, "shell_bottom_primary"  , "use shell bottom on primary"  );
    AddBoolParam  (false, "shell_bottom_secondary", "use shell bottom on secondary");
}

//=============================================================================
// FSTiedBiphasicInterface
//-----------------------------------------------------------------------------

FSTiedBiphasicInterface::FSTiedBiphasicInterface(FSModel* ps, int nstep) : FSPairedInterface(FE_TIEDBIPHASIC_INTERFACE, ps, nstep)
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
// FSTiedMultiphasicInterface
//-----------------------------------------------------------------------------

FSTiedMultiphasicInterface::FSTiedMultiphasicInterface(FSModel* ps, int nstep) : FSPairedInterface(FE_TIEDMULTIPHASIC_INTERFACE, ps, nstep)
{
	SetTypeString("Tied multiphasic contact");

	AddBoolParam(false, "laugon", "augmented Lagrangian");
	AddDoubleParam(0.2, "tolerance", "augmentation tolerance");
	AddDoubleParam(0, "gaptol", "gap tolerance");
	AddDoubleParam(0, "ptol", "pressure tolerance");
    AddDoubleParam(0, "ctol", "concentration tolerance");
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
// FSTiedElasticInterface
//-----------------------------------------------------------------------------

FSTiedElasticInterface::FSTiedElasticInterface(FSModel* ps, int nstep) : FSPairedInterface(FE_TIED_ELASTIC_INTERFACE, ps, nstep)
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
FSContactPotentialInterface::FSContactPotentialInterface(FSModel* ps, int nstep) : FSPairedInterface(FE_CONTACTPOTENTIAL_CONTACT, ps, nstep)
{
	SetTypeString("contact potential");

	AddDoubleParam(0, "kc");
	AddDoubleParam(4, "p", "power");
	AddDoubleParam(1, "R_in");
	AddDoubleParam(2, "R_out");
	AddDoubleParam(0, "w_tol");
}

//=============================================================================
// FSGapHeatFluxInterface
//-----------------------------------------------------------------------------

FSGapHeatFluxInterface::FSGapHeatFluxInterface(FSModel* fem, int nstep) : FSPairedInterface(FE_GAPHEATFLUX_INTERFACE, fem, nstep)
{
	SetTypeString("Gap heat flux");

	AddDoubleParam(0.0, "hc", "heat transfer coefficient");
	AddDoubleParam(0.01, "search_tol", "search tolerance");
}

//=============================================================================
// FSRigidJoint
//-----------------------------------------------------------------------------

FSRigidJoint::FSRigidJoint(FSModel* ps, int nstep) : FSInterface(FE_RIGID_JOINT, ps, nstep)
{ 
	m_pbodyA = m_pbodyB = 0;

	SetTypeString("Rigid joint");

	AddDoubleParam(0.1         , "tolerance", "augmentation tolerance");
	AddDoubleParam(1           , "penalty"  , "penalty factor"        );
	AddVecParam   (vec3d(0,0,0), "joint"    , "joint position"        );
}

//-----------------------------------------------------------------------------
void FSRigidJoint::Save(OArchive& ar)
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
void FSRigidJoint::Load(IArchive& ar)
{
	TRACE("FSRigidJoint::Load");
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
				m_pbodyA = GetFSModel()->GetMaterialFromID(mid);
			}
			break;
		case CID_RJ_RIGIDBODY_B: 
			{
				int mid; ar.read(mid);
				m_pbodyB = GetFSModel()->GetMaterialFromID(mid);
			}
			break;
		default:
			throw ReadError("unknown CID in FSRigidJoint::Load");
		}
		ar.CloseChunk();
	}
}

//=============================================================================
// FSSpringTiedInterface
//-----------------------------------------------------------------------------

FSSpringTiedInterface::FSSpringTiedInterface(FSModel* ps, int nstep) : FSPairedInterface(FE_SPRINGTIED_INTERFACE,ps, nstep)
{
	SetTypeString("Spring-tied contact");

	AddDoubleParam(0.0, "E", "Spring constant");
}

double FSSpringTiedInterface::SpringConstant() const
{
	return GetFloatValue(ECONST);
}

void FSSpringTiedInterface::BuildSpringList(vector<pair<int, int> >& L)
{
	FSFaceList* pfl = GetPrimarySurface()->BuildFaceList();
	FSNodeList* pnl = GetSecondarySurface()->BuildNodeList();
	if ((pfl == 0) || (pnl == 0)) return;

	unique_ptr<FSFaceList> ps(pfl);
	unique_ptr<FSNodeList> pm(pnl);

	set<pair<int, int> > S;

	FSFaceList::Iterator its = ps->First();
	for (int i=0; i<ps->Size(); ++i, ++its)
	{
		FSFace& f = *(its->m_pi);
		FSMesh& mesh = dynamic_cast<FSMesh&>(*(its->m_pm));
		int nf = f.Nodes();
		for (int n=0; n<nf; ++n)
		{
			FSNode& ns = mesh.Node(f.n[n]);
			vec3d nn = to_vec3d(f.m_nn[n]);
			vec3d nr = ns.r;

			int i0 = ns.m_nid;
			int i1 = -1;
			double Dmin = 0;
			FSNodeList::Iterator itm = pm->First();
			for (int j=0; j<pm->Size(); ++j, ++itm)
			{
				FSNode& nj = *(itm->m_pi);
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
// FSLinearConstraintSet
//-----------------------------------------------------------------------------

FSLinearConstraintSet::FSLinearConstraintSet()
{
	m_atol = 0.1;
	m_penalty = 1.0;
	m_nmaxaug = 50;
}

FSLinearConstraintSet::FSLinearConstraintSet(const FSLinearConstraintSet& lcs)
{
	m_atol = lcs.m_atol;
	m_penalty = lcs.m_penalty;
	m_nmaxaug = lcs.m_nmaxaug;

	m_set = lcs.m_set;
}

FSLinearConstraintSet& FSLinearConstraintSet::operator = (const FSLinearConstraintSet& lcs)
{
	m_atol = lcs.m_atol;
	m_penalty = lcs.m_penalty;
	m_nmaxaug = lcs.m_nmaxaug;

	m_set = lcs.m_set;

	return (*this);
}

//=============================================================================
FEBioInterface::FEBioInterface(FSModel* ps, int nstep) : FSPairedInterface(FE_FEBIO_INTERFACE, ps, nstep)
{
	SetMeshItemType(FE_FACE_FLAG | FE_PART_FLAG);
}

void FEBioInterface::Save(OArchive& ar)
{
	ar.BeginChunk(CID_FEBIO_META_DATA);
	{
		SaveClassMetaData(this, ar);
	}
	ar.EndChunk();

	ar.BeginChunk(CID_FEBIO_BASE_DATA);
	{
		FSPairedInterface::Save(ar);
	}
	ar.EndChunk();
}

void FEBioInterface::Load(IArchive& ar)
{
	TRACE("FEBioInterface::Load");
	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();
		switch (nid)
		{
		case CID_FEBIO_META_DATA: LoadClassMetaData(this, ar); break;
		case CID_FEBIO_BASE_DATA: FSPairedInterface::Load(ar); break;
		default:
			assert(false);
		}
		ar.CloseChunk();
	}
}
