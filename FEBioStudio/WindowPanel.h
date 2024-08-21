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
#include <QWidget>

// forward declarations
class CMainWindow;
class CGLDocument;
class FESelection;

//! Base class for window panels.
class CWindowPanel : public QWidget
{
public:
	//! constructor
	CWindowPanel(CMainWindow* wnd, QWidget* parent = 0);

	//! get the main window
	CMainWindow* GetMainWindow() { return m_wnd; }

	//! Get the main document
	CGLDocument*	GetDocument();

	//! Update the command panel, since the model has changed
	virtual void Update(bool breset = true);

	//! Process Esc key event (return true if processed)
	virtual bool OnEscapeEvent() { return false; }

	//! Process Del key event (return true if processed)
	virtual bool OnDeleteEvent() { return false; }

	//! event when user picks something from UI
	virtual bool OnPickEvent(const FESelection& sel) { return false; }

	//! undo event
	virtual bool OnUndo() { return false; }

	//! Mechanism for programmatically apply a command tool
	virtual void Apply() {}

private:
	CMainWindow*	m_wnd;	//!< pointer to main window
};
