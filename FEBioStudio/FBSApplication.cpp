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
#include "stdafx.h"
#include "FBSApplication.h"
#include "MainWindow.h"

FBSApplication* FBSApplication::m_This = nullptr;

FBSApplication* FBSApplication::Instance() { return m_This; }

FBSApplication::FBSApplication(int& argc, char** argv) : QApplication(argc, argv)
{
	m_pWnd = nullptr;
	m_This = this;
}

CMainWindow* FBSApplication::GetMainWindow()
{
	return m_pWnd;
}

void FBSApplication::SetMainWindow(CMainWindow* wnd) 
{ 
	m_pWnd = wnd; 
}

#ifdef __APPLE__
bool FBSApplication::event(QEvent* event)
{
	if (event->type() == QEvent::FileOpen) {
		QFileOpenEvent* openEvent = static_cast<QFileOpenEvent*>(event);

		QString fileName;
		// Handle custom URL scheme
		if (!openEvent->url().isEmpty())
		{
			fileName = openEvent->url().toString();
		}
		else
		{
			fileName = openEvent->file();
		}

		fileName.replace("file://", "");

		m_pWnd->OpenFile(fileName);
	}

	return QApplication::event(event);
}
#endif