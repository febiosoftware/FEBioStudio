#pragma once
#include <QtCore/QThread>

class CMainWindow;
class FileReader;
class CFEBioJob;
class xpltFileReader;

class CFileThread : public QThread
{
	Q_OBJECT

	void run() Q_DECL_OVERRIDE;

public:
	CFileThread(CMainWindow* wnd, FileReader* file, bool bclear, const QString& fileName);

	float getFileProgress() const;

signals:
	void resultReady(bool, const QString&);

private:
	CMainWindow*	m_wnd;
	FileReader*		m_fileReader;
	QString			m_fileName;
	bool			m_bclear;
};

class CPostFileThread : public QThread
{
	Q_OBJECT

	void run() Q_DECL_OVERRIDE;

public:
	CPostFileThread(CMainWindow* wnd, CFEBioJob* doc, xpltFileReader* file);

	float getFileProgress() const;

	CFEBioJob* GetFEBioJob() { return m_job; }

	xpltFileReader* GetFileReader() { return m_fileReader; }

signals:
	void resultReady(bool, const QString&);

private:
	CMainWindow*	m_wnd;
	CFEBioJob*		m_job;
	xpltFileReader*	m_fileReader;
};
