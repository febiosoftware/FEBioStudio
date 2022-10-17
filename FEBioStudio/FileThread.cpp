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
#include "FileThread.h"
#include "MainWindow.h"
#include <MeshIO/FileReader.h>

CFileThread::CFileThread(CMainWindow* wnd, const QueuedFile& file) : m_wnd(wnd), m_file(file)
{
	m_success = false;
	QObject::connect(this, SIGNAL(resultReady(bool, const QString&)), wnd, SLOT(on_finishedReadingFile(bool, const QString&)));
	QObject::connect(this, SIGNAL(finished()), this, SLOT(deleteLater()));
}

void CFileThread::run()
{
	std::string sfile = m_file.m_fileName.toStdString();
	const char* szfile = sfile.c_str();
	if (m_file.m_fileReader == 0)
	{
		emit resultReady(false, QString("Don't know how to read this file!"));
	}
	else
	{
		m_success = m_file.m_fileReader->Load(szfile);
		std::string err = m_file.m_fileReader->GetErrorMessage();
		emit resultReady(m_success, QString(err.c_str()));
	}
}

float CFileThread::getFileProgress() const
{
	if (m_file.m_fileReader) return m_file.m_fileReader->GetFileProgress();
	return (m_success ? 1.f : 0.f);
}

QueuedFile CFileThread::GetFile()
{
	return m_file;
}
