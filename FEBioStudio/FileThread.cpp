#include "stdafx.h"
#include "FileThread.h"
#include "MainWindow.h"
#include "Document.h"
#include "PostDoc.h"
#include <MeshTools/PRVArchive.h>
#include <GeomLib/GObject.h>
#include <XPLTLib/xpltFileReader.h>

CFileThread::CFileThread(CMainWindow* wnd, FileReader* file, bool bclear, const QString& fileName) : m_wnd(wnd), m_fileReader(file), m_fileName(fileName)
{
	m_bclear = bclear;
	QObject::connect(this, SIGNAL(resultReady(bool, const QString&)), wnd, SLOT(finishedReadingFile(bool, const QString&)));
	QObject::connect(this, SIGNAL(finished()), this, SLOT(deleteLater()));
}

void CFileThread::run()
{
	std::string sfile = m_fileName.toStdString();
	const char* szfile = sfile.c_str();
	CDocument& doc = *m_wnd->GetDocument();
	if (m_fileReader == 0)
	{
		PRVArchive ar;
		m_fileReader = &ar;
		if (!ar.Load(szfile))
		{
			m_fileReader = 0;
			emit resultReady(false, QString("Failed opening file %1").arg(szfile));
		}
		else
		{
			try
			{
				doc.SetDocFilePath(sfile);
				doc.Load(ar.GetArchive());
				m_fileReader = 0;
				emit resultReady(true, "");
			}
			catch (InvalidVersion)
			{
				m_fileReader = 0;
				emit resultReady(false, QString("The file %1 has an invalid version number.").arg(szfile));
			}
			catch (ReadError e)
			{
				m_fileReader = 0;
				char* sz = 0;
				int L = CCallStack::GetCallStackString(0);
				sz = new char[L + 1];
				CCallStack::GetCallStackString(sz);
				QString err = QString("An error occurred while reading the file\n%1\n\nERROR: %2\nCall stack:\n%3").arg(szfile).arg(e.m_szmsg ? e.m_szmsg : "(unknown error)").arg(sz);
				delete[] sz;
				emit resultReady(false, err);
			}
			catch (GObjectException e)
			{
				m_fileReader = 0;
				emit resultReady(false, QString("An error occurred processing model:\n%1").arg(e.ErrorMsg()));
			}
			catch (...)
			{
				m_fileReader = 0;
				emit resultReady(false, QString("Failed opening file %1.").arg(szfile));
			}
			m_fileReader = 0;
		}
	}
	else
	{
		FEFileImport* reader = dynamic_cast<FEFileImport*>(m_fileReader);
		bool ret = false;
		if (m_bclear)
		{
			ret = doc.ImportModel(reader, szfile);
		}
		else
		{
			ret = doc.ImportGeometry(reader, szfile);
		}

		std::string err = m_fileReader->GetErrorMessage();
		emit resultReady(ret, QString(err.c_str()));

		// clean up 
		if (m_fileReader)
		{
			// we clean it up this to avoid race conditions
			FileReader* reader = m_fileReader;
			m_fileReader = 0;
			delete reader;
		}
	}
}

float CFileThread::getFileProgress() const
{
	if (m_fileReader) return m_fileReader->GetFileProgress();
	return 0.f;
}

CPostFileThread::CPostFileThread(CMainWindow* wnd, CFEBioJob* job, xpltFileReader* file) : m_wnd(wnd), m_job(job), m_fileReader(file)
{
	QObject::connect(this, SIGNAL(resultReady(bool, const QString&)), wnd, SLOT(finishedReadingPostFile(bool, const QString&)));
	QObject::connect(this, SIGNAL(finished()), this, SLOT(deleteLater()));
}

void CPostFileThread::run()
{
	if (m_fileReader)
	{
		CFEBioJob& job = *m_job;
		bool ret = job.OpenPlotFile(m_fileReader);
		std::string err = m_fileReader->GetErrorMessage();
		emit resultReady(ret, QString(err.c_str()));
	}
	else emit resultReady(false, "No file reader");
}

float CPostFileThread::getFileProgress() const
{
	if (m_fileReader) return m_fileReader->GetFileProgress();
	return 0.f;
}
