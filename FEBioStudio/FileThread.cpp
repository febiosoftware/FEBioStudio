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
