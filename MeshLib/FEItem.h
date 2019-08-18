#pragma once

//-----------------------------------------------------------------------------
// FE State Flags
#define FE_VISIBLE	0x01
#define FE_SELECTED	0x02

//-----------------------------------------------------------------------------
// The FEItem class is the base class for all FEMesh items (nodes, edges, faces, elements)
// It stores the common attributes such as the state flags and the group ID.
//
class FEItem
{
public:
	FEItem() { m_state = FE_VISIBLE; m_gid = -1; }
	FEItem(const FEItem& it);

	void operator = (const FEItem& n);

	bool IsVisible() const { return ((m_state & FE_VISIBLE) != 0); }
	bool IsSelected() const { return ((m_state & FE_SELECTED) != 0); }

	void Select()   { m_state = m_state | FE_SELECTED; }
	void UnSelect() { m_state = m_state & ~FE_SELECTED; }
	void Show() { m_state = m_state | FE_VISIBLE; }
	void Hide() { m_state = 0; }

	unsigned int GetFEState() const { return m_state; }
	void SetFEState(unsigned int state) { m_state = state; }

public:
	int	m_ntag;	// tag of item
	int	m_gid;	// group id
	int	m_nid;	// item id (used e.g. in export).

protected:
	unsigned int m_state;	// the state flag of the mesh(-item)
};

//-----------------------------------------------------------------------------
// Copy constructor
// Note that the nid is not copied. (Not sure why).
inline FEItem::FEItem(const FEItem& it)
{
	m_ntag  = it.m_ntag;
	m_gid   = it.m_gid;
	m_state = it.m_state;
}

//-----------------------------------------------------------------------------
//! assignment operator
inline void FEItem::operator = (const FEItem& it)
{
	m_ntag = it.m_ntag;
	m_gid = it.m_gid;
	m_state = it.m_state;
}
