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

#pragma once

//! Base class for mesh item classes.
class FSMeshItem
{
private:
	//! Item flags
	//! Even when not hidden, the item may not be shown since e.g. the material is hidden
	enum {
		// State Flags
		ITEM_HIDDEN    = 0x0001,	//!< was the item hidden by the user
		ITEM_SELECTED  = 0x0002,	//!< is the item currently selected ?
		ITEM_DISABLED  = 0x0004,	//!< should the item be evaluated ?
		ITEM_ACTIVE    = 0x0008,	//!< does the item contain data?
		ITEM_INVISIBLE = 0x0010,	//!< is the item invisible because the parent material was hidden? 
		ITEM_ERODED    = 0x0020,	//!< the item is "eroded" and should be treated as no longer present
		ITEM_EXTERIOR  = 0x0040,	//!< the item is "exterior"
		ITEM_REQUIRED  = 0x0080,	//!< the item is required and should not be deleted during mesh operations
		ITEM_EXPORT    = 0x0100		//!< item should be exported (ony used by FEBioExport4)
};

public:
	//! Default constructor
	FSMeshItem() { m_state = 0; m_nid = -1; m_gid = 0; m_ntag = 0; }
	//! Virtual destructor
	virtual ~FSMeshItem() {}

	//! Assignment operator
	void operator = (const FSMeshItem& it)
	{
		m_gid = it.m_gid;
		m_nid = it.m_nid;
		m_ntag = it.m_ntag;
		m_state = it.m_state;
	}

	//! Check if the item is hidden
	bool IsHidden() const { return ((m_state & ITEM_HIDDEN) != 0); }
	//! Check if the item is selected
	bool IsSelected() const { return ((m_state & ITEM_SELECTED) != 0); }
	//! Check if the item is disabled
	bool IsDisabled() const { return ((m_state & ITEM_DISABLED) != 0); }
	//! Check if the item is active
	bool IsActive() const { return ((m_state & ITEM_ACTIVE) != 0); }
	//! Check if the item is invisible
	bool IsInvisible() const { return ((m_state & ITEM_INVISIBLE) != 0); }

	//! Select the item
	void Select() { m_state = m_state | ITEM_SELECTED; }
	//! Unselect the item
	void Unselect() { m_state = m_state & ~ITEM_SELECTED; }

	//! Hide the item and unselect it
	void Hide() { m_state = (m_state | ITEM_HIDDEN) & ~ITEM_SELECTED; }
	//! Unhide the item
	void Unhide() { m_state = m_state & ~ITEM_HIDDEN; }

	//! Enable the item
	void Enable() { m_state = m_state & ~ITEM_DISABLED; }
	//! Disable the item
	void Disable() { m_state = m_state | ITEM_DISABLED; }

	//! Activate the item
	void Activate() { m_state = m_state | ITEM_ACTIVE; }
	//! Deactivate the item
	void Deactivate() { m_state = m_state & ~ITEM_ACTIVE; }

	//! Set the eroded state of the item
	void SetEroded(bool b) { if (b) m_state = m_state | ITEM_ERODED; else m_state = m_state & ~ITEM_ERODED; }
	//! Check if the item is eroded
	bool IsEroded() const { return ((m_state & ITEM_ERODED) != 0); }

	//! Set the exterior state of the item
	void SetExterior(bool b) { if (b) m_state = m_state | ITEM_EXTERIOR; else m_state = m_state & ~ITEM_EXTERIOR; }
	//! Check if the item is exterior
	bool IsExterior() const { return ((m_state & ITEM_EXTERIOR) != 0); }

	//! Check if the item is required
	bool IsRequired() const { return ((m_state & ITEM_REQUIRED) != 0); }
	//! Set the required state of the item
	void SetRequired(bool b) { if (b) m_state = m_state | ITEM_REQUIRED; else m_state = m_state & ~ITEM_REQUIRED; }

	//! Check if the item is enabled (opposite of disabled)
	bool IsEnabled() const { return (IsDisabled() == false); }

	//! Check if the item can be exported
	bool CanExport() const { return ((m_state & ITEM_EXPORT) != 0); }
	//! Set the export state of the item
	void SetExport(bool b) { if (b) m_state = m_state | ITEM_EXPORT; else m_state = m_state & ~ITEM_EXPORT; }

	//! Show or hide the item (affects visibility)
	void Show(bool bshow = true)
	{
		if (bshow) m_state = m_state & ~ITEM_INVISIBLE;
		else m_state = (m_state | ITEM_INVISIBLE) & ~ITEM_SELECTED;
	}

	//! Check if the item is visible (not invisible, not hidden, and not eroded)
	bool IsVisible() const { return (IsInvisible() == false) && (IsHidden() == false) && (IsEroded() == false); }

	//! Get the ID of the item
	int GetID() const { return m_nid; }
	//! Set the ID of the item
	void SetID(int nid) { m_nid = nid; }

public:
	//! Tag member variable
	int	m_ntag;
	//! Group ID member variable
	int	m_gid;
	//! Node/Item ID member variable
	int	m_nid;

protected:
	//! Get the current state flags
	unsigned int GetState() const { return m_state; }
	//! Set the state flags
	void SetState(unsigned int state) { m_state = state; }

private:
	//! The state flag of the mesh item
	unsigned int m_state;
};