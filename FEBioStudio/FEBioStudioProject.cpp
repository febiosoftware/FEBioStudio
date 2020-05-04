#include "stdafx.h"
#include "FEBioStudioProject.h"
#include <XML/XMLWriter.h>
#include <XML/XMLReader.h>
#include <QDir>

FEBioStudioProject::FEBioStudioProject()
{

}

QString FEBioStudioProject::GetProjectFileName() const
{
	return m_projectFile;
}

int FEBioStudioProject::Files() const
{
	return m_fileList.size();
}

QString FEBioStudioProject::GetFileName(int n) const
{
	return m_fileList[n];
}

bool FEBioStudioProject::Contains(const QString& fileName) const
{
	for each (QString s in m_fileList) { if (s == fileName) return true; }
	return false;
}

void FEBioStudioProject::AddFile(const QString& fileName)
{
	if (Contains(fileName)) return;
	m_fileList.push_back(fileName);
	Save();
}

void FEBioStudioProject::Clear()
{
	m_fileList.clear();
	Save();
}

void FEBioStudioProject::Close()
{
	m_projectFile.clear();
	m_fileList.clear();
}

void FEBioStudioProject::Remove(const QString& file)
{
	QStringList::iterator it = m_fileList.begin();
	int n = m_fileList.size();
	for (int i = 0; i < m_fileList.size(); ++i, ++it)
	{
		if (file == *it) 
		{
			m_fileList.erase(it);
			Save();
			break;
		}
	}
}

bool FEBioStudioProject::Save()
{
	if (m_projectFile.isEmpty() == false) return Save(m_projectFile);
	else return false;
}

bool FEBioStudioProject::Save(const QString& file)
{
	std::string fileName = file.toStdString();

	QDir dir(file);

	XMLWriter xml;
	if (xml.open(fileName.c_str()) == false) return false;

	XMLElement root("FEBioStudioProject");
	root.add_attribute("version", "1.0");
	xml.add_branch(root);
	for (int i = 0; i < Files(); ++i)
	{
		QString fileName = m_fileList[i];

		fileName = dir.relativeFilePath(fileName);

		string sfile = fileName.toStdString();

		XMLElement file("file");
		file.add_attribute("path", sfile);
		xml.add_empty(file);
	}
	xml.close_branch();
	xml.close();

	return true;
}

bool FEBioStudioProject::Open(const QString& file)
{
	std::string fileName = file.toStdString();

	XMLReader xml;
	if (xml.Open(fileName.c_str()) == false) return false;
	XMLTag tag;
	if (xml.FindTag("FEBioStudioProject", tag) == false) return false;

	QDir dir(file);
	QStringList newFiles;

	try {
		++tag;
		do
		{
			if (tag == "file")
			{
				const char* relPath = tag.AttributeValue("path", true);
				if (relPath == nullptr) return false;

				QString absPath = dir.absoluteFilePath(QString::fromStdString(relPath));
				absPath = QDir::toNativeSeparators(dir.cleanPath(absPath));
				newFiles.push_back(absPath);
			}
			else return false;
			++tag;
		} while (!tag.isend());
	}
	catch (XMLReader::EndOfFile e)
	{
		// no worries, all good
	}

	xml.Close();

	m_fileList = newFiles;
	m_projectFile = file;

	return true;
}
