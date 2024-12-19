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
#include <GeomLib/GItem.h>
#include <GeomLib/FSGroup.h>
#include <QtCore/QObject>

//-------------------------------------------------------------------
// forward declaration
class CGLView;

//-------------------------------------------------------------------
// This class assist in hightlighting items in the CGLView. Highlighting allows
// users to pick items without changing the current selection.
// The highlighter keeps a list of all highlighted items. Only one of those
// items is the "active" item. The active item is the item that is currently
// under the mouse cursor. "Picking" the active item adds it to the list.
class GLHighlighter : public QObject
{
	Q_OBJECT

public:
	struct Item {
		FSObject* item = nullptr; // this should only be a GItem or FSGroup
		int	color;
	};

public:
	// return an instance of this highlighter (this class is a singleton)
	static GLHighlighter* Instance() { return &This; }

	// Attach the highlighter to a CGLView
	// This must be done prior to any highlighting.
	static void AttachToView(CGLView* view);

	// set the active item
	static void SetActiveItem(GItem* item);

	// "pick" the active item
	// This adds the active item to the list of highlighted items .
	// This also clears the active item
	static void PickActiveItem();

	// "pick" an item
	static void PickItem(GItem* item, int colorMode = 0);
	static void PickItem(FSGroup* item, int colorMode = 0);

	// return the currently acite item
	static GItem* GetActiveItem();

	// return the name of the highlighted item
	static QString GetActiveItemName();

	// clear all hightlights
	static void ClearHighlights();

	// set the type of items to highlight (NOT WORKING YET!!)
	static void setHighlightType(int type);

	// if true, the highlighted item will be tracked by the mouse
	static void setTracking(bool b);

	// see if tracking is true or not.
	static bool IsTracking();

	// set the color for active item
	static void SetActiveColor(GLColor c);

	// set the color for the picked items
	static void SetPickColor(GLColor c, int colorMode = 0);

	static BOX GetBoundingBox();

	static std::vector<Item> GetItems();

private:
	// constructor
	GLHighlighter();

signals:
	// emitted when the active item changes
	void activeItemChanged();

	// emitted when the active item is "picked"
	void itemPicked(GItem* item);

private:
	CGLView*		m_view;				// pointer to GL view
	GItem*			m_activeItem;		// pointer to the active item (or zero)
	std::vector<Item>	m_item;				// list of hightlighted items (except active item)
	bool			m_btrack;			// set active item via mouse tracking

	// the one-and-only highlighter
	static GLHighlighter This;
};
