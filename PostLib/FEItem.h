#pragma once

//-----------------------------------------------------------------------------
// FE State Flags
#define FE_HIDDEN		0x01		// was the item hidden by the user
#define FE_SELECTED		0x02		// is the item currently selected ?
#define FE_DISABLED		0x04		// should the item be evaluated ?
#define FE_ACTIVE		0x08		// does the item contain data?
#define FE_INVISIBLE	0x10		// is the item invisible because the parent material was hidden? 
#define FE_ERODED		0x20		// the item is "eroded" and should be treated as no longer present
// Even when not hidden, the item may not be shown since e.g. the material is hidden

namespace Post {
//-----------------------------------------------------------------------------
// base class for mesh item classes.
//
class FEItem
{
public:
	FEItem() { m_state = 0; m_nId = -1; m_ntag = 0; }
	virtual ~FEItem() {}

	bool IsHidden() const { return ((m_state & FE_HIDDEN) != 0); }
	bool IsSelected() const { return ((m_state & FE_SELECTED) != 0); }
	bool IsDisabled() const { return ((m_state & FE_DISABLED) != 0); }
	bool IsActive() const { return ((m_state & FE_ACTIVE) != 0); }
	bool IsInvisible() const { return ((m_state & FE_INVISIBLE) != 0); }

	void Select() { m_state = m_state | FE_SELECTED; }
	void Unselect() { m_state = m_state & ~FE_SELECTED; }

	void Hide() { m_state = (m_state | FE_HIDDEN) & ~FE_SELECTED; }
	void Unhide() { m_state = m_state & ~FE_HIDDEN; }

	void Enable() { m_state = m_state & ~FE_DISABLED; }
	void Disable() { m_state = m_state | FE_DISABLED; }

	void Activate() { m_state = m_state | FE_ACTIVE; }
	void Deactivate() { m_state = m_state & ~FE_ACTIVE; }

	void SetEroded(bool b) { if (b) m_state = m_state | FE_ERODED; else m_state = m_state & ~FE_ERODED; }
	bool IsEroded() const { return ((m_state & FE_ERODED) != 0); }

	bool IsEnabled() const { return (IsDisabled() == false); }

	void Show(bool bshow)
	{
		if (bshow) m_state = m_state & ~FE_INVISIBLE;
		else m_state = (m_state | FE_INVISIBLE) & ~FE_SELECTED;
	}

	bool IsVisible() const { return (IsInvisible() == false) && (IsHidden() == false) && (IsEroded() == false); }

	unsigned int GetFEState() const { return m_state; }
	void SetFEState(unsigned int state) { m_state = state; }

	int GetID() const { return m_nId; }
	void SetID(int nid) { m_nId = nid; }

public:
	int	m_ntag;
	int	m_nId;

	unsigned int m_state;	// the state flag of the mesh(-item)
};
}