#pragma once
#include <QString>
#include <QStringList>
#include <vector>

class FEBioStudioProject
{
public:
	class File
	{
	public:
		File(const QString& fileName, int folder = -1)
		{
			m_fileName = fileName;
			m_folder = folder;
		}

		File(const File& f)
		{
			m_fileName = f.m_fileName;
			m_folder = f.m_folder;
		}

		void operator = (const File& f)
		{
			m_fileName = f.m_fileName;
			m_folder = f.m_folder;
		}

	public:
		QString	m_fileName;
		int		m_folder;
	};

public:
	FEBioStudioProject();

	QString GetProjectFileName() const;

	int Files() const;

	QString GetFileName(int n) const;
	File GetFile(int n) const;

	void AddFile(const QString& fileName, int folder = -1);

	int Folders() const;
	QString GetFolder(int n) const;
	int AddFolder(const QString& folderName);

	void MoveToFolder(const QString& file, int folderIndex);

	bool RemoveFolder(const QString& folderName);

	bool RenameFolder(const QString& folderName, const QString& newName);

	bool Save(const QString& file);
	bool Save();

	bool Open(const QString& file);

	void Clear();

	void Close();

	void Remove(const QString& file);

	bool Contains(const QString& file) const;

private:
	QString			m_projectFile;
	QList<File>		m_fileList;
	QStringList		m_folderList;
};
