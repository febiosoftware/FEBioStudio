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

#pragma once

//-----------------------------------------------------------------------------
// FE State Flags
#define FE_HIDDEN		0x01		// was the item hidden by the user
#define FE_SELECTED		0x02		// is the item currently selected ?
#define FE_DISABLED		0x04		// should the item be evaluated ?
#define FE_ACTIVE		0x08		// does the item contain data?
#define FE_INVISIBLE	0x10		// is the item invisible because the parent material was hidden? 
#define FE_ERODED		0x20		// the item is "eroded" and should be treated as no longer present
#define FE_EXTERIOR		0x40		// the item is "exterior"
// Even when not hidden, the item may not be shown since e.g. the material is hidden

//-----------------------------------------------------------------------------
// base class for mesh item classes.
//
class FEItem
{
public:
	FEItem() { m_state = 0; m_nid = -1; m_gid = 0; m_ntag = 0; }
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

	void SetExterior(bool b) { if (b) m_state = m_state | FE_EXTERIOR; else m_state = m_state & ~FE_EXTERIOR; }
	bool IsExterior() const { return ((m_state & FE_EXTERIOR) != 0); }

	bool IsEnabled() const { return (IsDisabled() == false); }

	void Show(bool bshow = true)
	{
		if (bshow) m_state = m_state & ~FE_INVISIBLE;
		else m_state = (m_state | FE_INVISIBLE) & ~FE_SELECTED;
	}

	bool IsVisible() const { return (IsInvisible() == false) && (IsHidden() == false) && (IsEroded() == false); }

	unsigned int GetFEState() const { return m_state; }
	void SetFEState(unsigned int state) { m_state = state; }

	int GetID() const { return m_nid; }
	void SetID(int nid) { m_nid = nid; }

public:
	int	m_ntag;
	int	m_gid;
	int	m_nid;

protected:
	unsigned int m_state;	// the state flag of the mesh(-item)
};
