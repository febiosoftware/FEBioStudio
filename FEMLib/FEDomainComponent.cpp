#include "stdafx.h"
#include "FEDomainComponent.h"
#include <FEMLib/FSModel.h>
#include <GeomLib/GGroup.h>
#include "FEMKernel.h"
#include <sstream>

FSDomainComponent::FSDomainComponent(int ntype, FSModel* ps, int nstep) : FSStepComponent(ps)
{
	m_nstepID = nstep;
	m_ntype = ntype;
}

FSDomainComponent::FSDomainComponent(int ntype, FSModel* ps, FEItemListBuilder* pi, int nstep) : FSStepComponent(ps)
{
	m_ntype = ntype;
	m_nstepID = nstep;
	SetItemList(pi);
	SetMeshItemType(0);
}

FSDomainComponent::~FSDomainComponent(void)
{
}

void FSDomainComponent::Save(OArchive& ar)
{
	// write the name
	ar.WriteChunk(NAME, GetName());
	ar.WriteChunk(CID_FEOBJ_INFO, GetInfo());

	// write the step
	ar.WriteChunk(STEP, m_nstepID);

	ar.WriteChunk(STATUS, m_bActive);

	// write the selection type
	ar.WriteChunk(SELECTION_TYPE, GetMeshItemType());

	// write list ID
	FEItemListBuilder* pl = GetItemList();
	if (pl) ar.WriteChunk(LIST_ID, pl->GetID());

	// write the parameters
	ar.BeginChunk(PARAMS);
	{
		ParamContainer::Save(ar);
	}
	ar.EndChunk();

	if (Properties() > 0)
	{
		ar.BeginChunk(CID_PROPERTY_LIST);
		{
			SaveProperties(ar);
		}
		ar.EndChunk();
	}
}

//-----------------------------------------------------------------------------

void FSDomainComponent::Load(IArchive& ar)
{
	TRACE("FSDomainComponent::Load");

	FSModel* fem = GetFSModel();
	GModel* pgm = &fem->GetModel();

	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();
		switch (nid)
		{
		case NAME: { string name; ar.read(name); SetName(name); } break;
		case CID_FEOBJ_INFO: { string info; ar.read(info); SetInfo(info); } break;
		case STEP: ar.read(m_nstepID); break;
		case STATUS: ar.read(m_bActive); break;
		case SELECTION_TYPE: { unsigned int itemType = 0; ar.read(itemType); SetMeshItemType(itemType); } break;
		case PARAMS:
			ParamContainer::Load(ar);
			break;
		case CID_PROPERTY_LIST:
			LoadProperties(ar);
			break;
		case LIST_ID:
		{
			int nid = 0;
			ar.read(nid);
			FEItemListBuilder* pItem = pgm->FindNamedSelection(nid);
			SetItemList(pItem);
		}
		break;
		case LIST:
		{
			// NOTE: We should only get here for older files (< FBS2.1)
			//       since domain components no longer manage their own lists.
			//       If we get here, we'll read the list and then add it to the model
			ar.OpenChunk();
			{
				int ntype = ar.GetChunkID();
				FEItemListBuilder* pItem = nullptr;
				switch (ntype)
				{
				case GO_NODE: pItem = new GNodeList(pgm); break;
				case GO_EDGE: pItem = new GEdgeList(pgm); break;
				case GO_FACE: pItem = new GFaceList(pgm); break;
				case GO_PART: pItem = new GPartList(pgm); break;
				case FE_NODESET: pItem = new FSNodeSet(nullptr); break;
				case FE_EDGESET: pItem = new FSEdgeSet(nullptr); break;
				case FE_SURFACE: pItem = new FSSurface(nullptr); break;
				case FE_ELEMSET: pItem = new FSElemSet(nullptr); break;
				default:
					assert(false);
					throw ReadError("Unknown FEItemListBuilder type in FSBoundaryCondition::Load");
				}
				pItem->Load(ar);

				// set the parent mesh for FSGroup's
				FSGroup* pg = dynamic_cast<FSGroup*>(pItem);
				if (pg)
				{
					if (fem->FindGroupParent(pg) == false)
					{
						ar.log("Invalid mesh ID in FSDomainComponent::Load");
						delete pItem;
						pItem = nullptr;
					}
				}

				// add the item to the model
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
			ar.CloseChunk();
		}
		break;
		}
		ar.CloseChunk();
	}
}

//-----------------------------------------------------------------------------
FSMeshSelection::FSMeshSelection(FSModel* fem) : FSModelComponent(fem)
{
	
}


void FSMeshSelection::Save(OArchive& ar)
{
	// write list ID
	FEItemListBuilder* pl = GetItemList();
	if (pl) ar.WriteChunk(LIST_ID, pl->GetID());
}

//-----------------------------------------------------------------------------
void FSMeshSelection::Load(IArchive& ar)
{
	TRACE("FSMeshSelection::Load");

	FSModel* fem = GetFSModel();
	GModel* pgm = &fem->GetModel();

	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();
		switch (nid)
		{
		case LIST_ID:
		{
			int nid = 0;
			ar.read(nid);
			FEItemListBuilder* pItem = pgm->FindNamedSelection(nid);
			SetItemList(pItem);
		}
		break;
		}
		ar.CloseChunk();
	}
}
