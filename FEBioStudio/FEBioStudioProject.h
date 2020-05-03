#pragma once
#include <QString>
#include <QStringList>
#include <vector>

class FEBioStudioProject
{
public:
	FEBioStudioProject();

	QString GetProjectFileName() const;

	int Files() const;

	QString GetFileName(int n) const;

	void AddFile(const QString& fileName);

	bool Save(const QString& file);
	bool Save();

	bool Open(const QString& file);

	void Clear();

private:
	QString			m_projectFile;
	QStringList		m_fileList;
};
