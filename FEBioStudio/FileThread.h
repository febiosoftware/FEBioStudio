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
#include <queue>

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
	QueuedFile() {}
	QueuedFile(CDocument* doc, const QString& fileName, FileReader* fileReader, int flags)
	{
		m_doc = doc;
		m_fileName = fileName;
		m_fileReader = fileReader;
		m_flags = flags;
	}

public:
	CDocument*		m_doc = nullptr;
	QString			m_fileName;
	FileReader*		m_fileReader = nullptr;
	int				m_flags = 0;
	bool			m_success = false;
};

class CFileThread : public QThread
{
	Q_OBJECT

	void run() Q_DECL_OVERRIDE;

public:
	CFileThread();

	void readFile(const QueuedFile& file);

	float getFileProgress() const;

	QueuedFile GetFile();

signals:
	void resultReady(QueuedFile, QString);

private:
	QueuedFile m_file;
};
