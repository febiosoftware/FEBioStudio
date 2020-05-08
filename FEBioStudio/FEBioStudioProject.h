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
		File(const QString& fileName, int group = -1)
		{
			m_fileName = fileName;
			m_group = group;
		}

		File(const File& f)
		{
			m_fileName = f.m_fileName;
			m_group = f.m_group;
		}

		void operator = (const File& f)
		{
			m_fileName = f.m_fileName;
			m_group = f.m_group;
		}

	public:
		QString	m_fileName;
		int		m_group;
	};

public:
	FEBioStudioProject();

	QString GetProjectFileName() const;

	int Files() const;

	QString GetFileName(int n) const;
	File GetFile(int n) const;

	void AddFile(const QString& fileName, int folder = -1);

	int Groups() const;
	QString GetGroupName(int n) const;
	int AddGroup(const QString& groupName);

	void MoveToGroup(const QString& file, int groupIndex);

	bool RemoveGroup(const QString& groupName);

	bool RenameGroup(const QString& groupName, const QString& newName);

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
	QStringList		m_groupList;
};
