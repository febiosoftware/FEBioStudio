#include "stdafx.h"
#include "FEDomainComponent.h"
#include <MeshTools/GGroup.h>

FEDomainComponent::FEDomainComponent(int ntype, FSModel* ps, int nstep)
{
	m_ps = ps;
	m_pItem = 0;
	m_nstepID = nstep;
	m_ntype = ntype;
	m_sztype = "(not defined)";

	m_itemType = FE_NODE_FLAG;
}

FEDomainComponent::FEDomainComponent(int ntype, FSModel* ps, FEItemListBuilder* pi, int nstep)
{
	m_ps = ps;
	m_ntype = ntype;
	m_nstepID = nstep;
	m_pItem = pi;
	m_sztype = "(not defined)";

	m_itemType = FE_NODE_FLAG;
}

FEDomainComponent::~FEDomainComponent(void)
{
	if (m_pItem) delete m_pItem;
}

unsigned int FEDomainComponent::GetMeshItemType() const
{
	return m_itemType;
}

void FEDomainComponent::SetMeshItemType(unsigned int itemType)
{
	m_itemType = itemType;
}

void FEDomainComponent::Save(OArchive& ar)
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

void FEDomainComponent::Load(IArchive& ar)
{
	TRACE("FEDomainComponent::Load");

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
				case GO_NODE: m_pItem = new GNodeList(m_ps); break;
				case GO_EDGE: m_pItem = new GEdgeList(m_ps); break;
				case GO_FACE: m_pItem = new GFaceList(m_ps); break;
				case GO_PART: m_pItem = new GPartList(m_ps); break;
				case FE_NODESET: m_pItem = new FENodeSet((GObject*)0); break;
				case FE_EDGESET: m_pItem = new FEEdgeSet((GObject*)0); break;
				case FE_SURFACE: m_pItem = new FESurface((GObject*)0); break;
				case FE_PART: m_pItem = new FEPart((GObject*)0); break;
				default:
					assert(false);
					throw ReadError("Unknown FEItemListBuilder type in FEBoundaryCondition::Load");
				}
				m_pItem->Load(ar);

				// set the parent mesh for FEGroup's
				FEGroup* pg = dynamic_cast<FEGroup*>(m_pItem);
				if (pg)
				{
					if (m_ps->FindGroupParent(pg) == false)
					{
						ar.log("Invalid mesh ID in FEDomainComponent::Load");
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
