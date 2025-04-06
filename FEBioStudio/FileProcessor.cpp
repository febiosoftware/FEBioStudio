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
#include "FileProcessor.h"
#include "MainWindow.h"
#include "MainStatusBar.h"
#include <FSCore/FileReader.h>
#include <QTimer>

CFileProcessor::CFileProcessor(CMainWindow* wnd) : m_wnd(wnd)
{

}

void CFileProcessor::ReadFile(QueuedFile file)
{
	m_fileName = file.m_fileName;

	// read the file, either threaded or directly
	if (file.m_flags & QueuedFile::NO_THREAD)
	{
		QString errorString;
		file.m_success = false;
		if (file.m_fileReader == nullptr)
		{
			errorString = "Don't know how to read file.";
		}
		else
		{
			std::string sfile = file.m_fileName.toStdString();
			file.m_success = file.m_fileReader->Load(sfile.c_str());
			std::string err = file.m_fileReader->GetErrorString();
			errorString = QString::fromStdString(err);
		}
		on_finishedReadingFile(file, errorString);
	}
	else
	{
		assert(m_fileThread == nullptr);
		m_fileThread = new CFileThread();
		QObject::connect(m_fileThread, &CFileThread::resultReady, this, &CFileProcessor::on_finishedReadingFile);
		m_fileThread->readFile(file);

		QTimer::singleShot(100, this, &CFileProcessor::checkFileProgress);
	}
}

void CFileProcessor::AddFile(QueuedFile file)
{
	m_fileQueue.push_back(file);
}

void CFileProcessor::ReadNextFileInQueue()
{
	// If a file is being processed, just wait
	if (m_fileThread) return;

	// make sure we have a file
	if (m_fileQueue.empty()) return;

	// get the next file name
	QueuedFile nextFile = m_fileQueue[0];

	// remove the last file that was read
	m_fileQueue.erase(m_fileQueue.begin());

	// start reading the file
	ReadFile(nextFile);
}

bool CFileProcessor::IsReadingFile() const
{
	return (m_fileThread != nullptr);
}

float CFileProcessor::GetFileProgress() const
{
	if (m_fileThread) return m_fileThread->getFileProgress();
	return 0.f;
}

void CFileProcessor::on_finishedReadingFile(QueuedFile file, QString errorString)
{
	m_fileThread = nullptr;
	m_fileName.clear();
	m_wnd->on_finishedReadingFile(file, errorString);
}

void CFileProcessor::checkFileProgress()
{
	if (IsReadingFile())
	{
		CMainStatusBar* statusBar = m_wnd->GetStatusBar();
		if (statusBar == nullptr) return;

		if (!statusBar->IsProgressVisible())
		{
			QString fileName = GetCurrentFileName();
			statusBar->showMessage(QString("Reading file %1 ...").arg(fileName));
			statusBar->showProgress();
			m_wnd->AddLogEntry(QString("Reading file %1 ... ").arg(fileName));
		}

		float f = GetFileProgress();
		int n = (int)(100.f * f);
		statusBar->setProgress(n);
		if (f < 1.0f) QTimer::singleShot(100, this, &CFileProcessor::checkFileProgress);
	}
}
