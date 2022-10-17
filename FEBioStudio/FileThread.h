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
#include <QtCore/QThread>

class CMainWindow;
class FileReader;
class CDocument;

class QueuedFile
{
public:
	enum Flags {
		NO_THREAD = 0x01,
		NEW_DOCUMENT = 0x02,
		RELOAD_DOCUMENT = 0x04,
		AUTO_SAVE_RECOVERY = 0x08
	};

public:
	QueuedFile(CDocument* doc, const QString& fileName, FileReader* fileReader, int flags)
	{
		m_doc = doc;
		m_fileName = fileName;
		m_fileReader = fileReader;
		m_flags = flags;
	}

	QueuedFile(const QueuedFile& qf)
	{
		m_doc = qf.m_doc;
		m_fileName = qf.m_fileName;
		m_fileReader = qf.m_fileReader;
		m_flags = qf.m_flags;
	}

	void operator = (const QueuedFile& qf)
	{
		m_doc = qf.m_doc;
		m_fileName = qf.m_fileName;
		m_fileReader = qf.m_fileReader;
		m_flags = qf.m_flags;
	}

public:
	CDocument*		m_doc;
	QString			m_fileName;
	FileReader*		m_fileReader;
	int				m_flags;
};

class CFileThread : public QThread
{
	Q_OBJECT

	void run() Q_DECL_OVERRIDE;

public:
	CFileThread(CMainWindow* wnd, const QueuedFile& file);

	float getFileProgress() const;

	QueuedFile GetFile();

signals:
	void resultReady(bool, const QString&);

private:
	CMainWindow*	m_wnd;
	QueuedFile		m_file;
	bool			m_success;
};
