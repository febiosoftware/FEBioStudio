#include "stdafx.h"
#include "FEDomainComponent.h"
#include <MeshTools/GGroup.h>

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
	if (m_pItem) delete m_pItem;
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

	// write the parameters
	ar.BeginChunk(PARAMS);
	{
		ParamContainer::Save(ar);
	}
	ar.EndChunk();

	// write the list
	if (m_pItem)
	{
		ar.BeginChunk(LIST);
		{
			ar.BeginChunk(m_pItem->Type());
			{
				m_pItem->Save(ar);
			}
			ar.EndChunk();
		}
		ar.EndChunk();
	}
}

//-----------------------------------------------------------------------------

void FSDomainComponent::Load(IArchive& ar)
{
	TRACE("FSDomainComponent::Load");

	FSModel* fem = GetFSModel();

	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();
		switch (nid)
		{
		case NAME: { string name; ar.read(name); SetName(name); } break;
		case CID_FEOBJ_INFO: { string info; ar.read(info); SetInfo(info); } break;
		case STEP: ar.read(m_nstepID); break;
		case PARAMS:
			ParamContainer::Load(ar);
			break;
		case LIST:
		{
			ar.OpenChunk();
			{
				int ntype = ar.GetChunkID();
				m_pItem = 0;
				switch (ntype)
				{
				case GO_NODE: m_pItem = new GNodeList(fem); break;
				case GO_EDGE: m_pItem = new GEdgeList(fem); break;
				case GO_FACE: m_pItem = new GFaceList(fem); break;
				case GO_PART: m_pItem = new GPartList(fem); break;
				case FE_NODESET: m_pItem = new FSNodeSet((GObject*)0); break;
				case FE_EDGESET: m_pItem = new FSEdgeSet((GObject*)0); break;
				case FE_SURFACE: m_pItem = new FSSurface((GObject*)0); break;
				case FE_PART: m_pItem = new FSPart((GObject*)0); break;
				default:
					assert(false);
					throw ReadError("Unknown FEItemListBuilder type in FSBoundaryCondition::Load");
				}
				m_pItem->Load(ar);

				// set the parent mesh for FSGroup's
				FSGroup* pg = dynamic_cast<FSGroup*>(m_pItem);
				if (pg)
				{
					if (fem->FindGroupParent(pg) == false)
					{
						ar.log("Invalid mesh ID in FSDomainComponent::Load");
						delete m_pItem;
						m_pItem = nullptr;
					}
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
