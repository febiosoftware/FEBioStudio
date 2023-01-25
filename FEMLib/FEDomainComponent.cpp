#include "stdafx.h"
#include "FEDomainComponent.h"
#include <FEMLib/FSModel.h>
#include <GeomLib/GGroup.h>
#include "FEMKernel.h"
#include <sstream>

FSDomainComponent::FSDomainComponent(int ntype, FSModel* ps, int nstep) : FSStepComponent(ps)
{
	m_pItem = 0;
	m_nstepID = nstep;
	m_ntype = ntype;

	m_itemType = 0;
}

FSDomainComponent::FSDomainComponent(int ntype, FSModel* ps, FEItemListBuilder* pi, int nstep) : FSStepComponent(ps)
{
	m_ntype = ntype;
	m_nstepID = nstep;
	m_pItem = pi;

	m_itemType = 0;
}

FSDomainComponent::~FSDomainComponent(void)
{
	m_pItem = nullptr;
}

FEItemListBuilder* FSDomainComponent::GetItemList() { return m_pItem; }
void FSDomainComponent::SetItemList(FEItemListBuilder* pi) { m_pItem = pi; }

unsigned int FSDomainComponent::GetMeshItemType() const
{
	return m_itemType;
}

void FSDomainComponent::SetMeshItemType(unsigned int itemType)
{
	m_itemType = itemType;
}

void FSDomainComponent::Save(OArchive& ar)
{
	// write the name
	ar.WriteChunk(NAME, GetName());
	ar.WriteChunk(CID_FEOBJ_INFO, GetInfo());

	// write the step
	ar.WriteChunk(STEP, m_nstepID);

	// write the selection type
	ar.WriteChunk(SELECTION_TYPE, m_itemType);

	// write list ID
	if (m_pItem) ar.WriteChunk(LIST_ID, m_pItem->GetID());

	// write the parameters
	ar.BeginChunk(PARAMS);
	{
		ParamContainer::Save(ar);
	}
	ar.EndChunk();
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
		case SELECTION_TYPE: ar.read(m_itemType); break;
		case PARAMS:
			ParamContainer::Load(ar);
			break;
		case LIST_ID:
		{
			int nid = 0;
			ar.read(nid);
			m_pItem = pgm->FindNamedSelection(nid);
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
				case FE_NODESET: pItem = new FSNodeSet((GObject*)0); break;
				case FE_EDGESET: pItem = new FSEdgeSet((GObject*)0); break;
				case FE_SURFACE: pItem = new FSSurface((GObject*)0); break;
				case FE_PART: pItem = new FSPart((GObject*)0); break;
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
					m_pItem = pItem;
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
FSMeshSelection::FSMeshSelection(FSModel* fem) : FSModelComponent(fem), m_pItem(nullptr)
{
	m_itemType = 0;
}

FEItemListBuilder* FSMeshSelection::GetItemList() { return m_pItem; }
void FSMeshSelection::SetItemList(FEItemListBuilder* pi) { m_pItem = pi; }

unsigned int FSMeshSelection::GetMeshItemType() const
{
	return m_itemType;
}

void FSMeshSelection::SetMeshItemType(unsigned int meshItem)
{
	m_itemType = meshItem;
}
