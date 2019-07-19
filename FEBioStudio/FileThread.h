#pragma once
#include <QtCore/QThread>

class CMainWindow;
class FileReader;

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
