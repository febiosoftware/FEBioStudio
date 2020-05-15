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
