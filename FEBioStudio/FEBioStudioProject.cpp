#include "stdafx.h"
#include "FEBioStudioProject.h"
#include <XML/XMLWriter.h>
#include <XML/XMLReader.h>
#include <QDir>
#include <sstream>

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
	return m_fileList[n].m_fileName;
}

FEBioStudioProject::File FEBioStudioProject::GetFile(int n) const
{
	return m_fileList[n];
}

bool FEBioStudioProject::Contains(const QString& fileName) const
{
	for (QList<File>::const_iterator it = m_fileList.begin(); it != m_fileList.end(); ++it)
	{
		if (it->m_fileName == fileName) return true; 
	}
	return false;
}

void FEBioStudioProject::AddFile(const QString& fileName, int folder)
{
	if (Contains(fileName)) return;

	File newFile(fileName, folder);
	m_fileList.push_back(newFile);
	Save();
}

void FEBioStudioProject::Clear()
{
	m_fileList.clear();
	m_groupList.clear();
	Save();
}

int FEBioStudioProject::Groups() const
{
	return m_groupList.size();
}

QString FEBioStudioProject::GetGroupName(int n) const
{
	return m_groupList.at(n);
}

int FEBioStudioProject::AddGroup(const QString& groupName)
{
	for (int i = 0; i < Groups(); ++i) {
		if (m_groupList[i] == groupName) return i;
	}
	m_groupList.push_back(groupName);
	Save();
	return Groups() - 1;
}

void FEBioStudioProject::Close()
{
	m_projectFile.clear();
	m_fileList.clear();
	m_groupList.clear();
}

void FEBioStudioProject::Remove(const QString& file)
{
	QList<File>::iterator it = m_fileList.begin();
	int n = m_fileList.size();
	for (int i = 0; i < m_fileList.size(); ++i, ++it)
	{
		if (file == it->m_fileName) 
		{
			m_fileList.erase(it);
			Save();
			break;
		}
	}
}

void FEBioStudioProject::MoveToGroup(const QString& file, int groupIndex)
{
	assert((groupIndex >= -1) && (groupIndex < Groups()));
	QList<File>::iterator it = m_fileList.begin();
	int n = m_fileList.size();
	for (int i = 0; i < m_fileList.size(); ++i, ++it)
	{
		if (file == it->m_fileName)
		{
			it->m_group = groupIndex;
			Save();
			break;
		}
	}
}

bool FEBioStudioProject::RemoveGroup(const QString& groupName)
{
	int ngroup = -1;
	for (int i = 0; i < m_groupList.size(); ++i)
	{
		if (m_groupList[i] == groupName)
		{
			ngroup = i;
			break;
		}
	}

	if (ngroup == -1) return false;

	m_groupList.removeAt(ngroup);

	for (int i = 0; i < m_fileList.size(); ++i)
	{
		File& file = m_fileList[i];
		if (file.m_group == ngroup)
		{
			file.m_group = -1;
		}
		else if (file.m_group > ngroup)
		{
			file.m_group--;
		}
	}

	Save();

	return true;
}

bool FEBioStudioProject::RenameGroup(const QString& groupName, const QString& newName)
{
	if (newName.isEmpty()) return false;

	int ngroup = -1;
	for (int i = 0; i < m_groupList.size(); ++i)
	{
		if (m_groupList[i] == groupName)
		{
			ngroup = i;
			break;
		}
	}

	if (ngroup == -1) return false;

	m_groupList[ngroup] = newName;

	Save();
	return true;
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
	for (int n = 0; n <= Groups(); ++n)
	{
		if (n < Groups())
		{
			string groupName = GetGroupName(n).toStdString();
			XMLElement folder("group");
			folder.add_attribute("name", groupName);
			xml.add_branch(folder);
		}

		int group = (n < Groups() ? n : -1);

		for (int i = 0; i < Files(); ++i)
		{
			FEBioStudioProject::File file = GetFile(i);

			if (file.m_group == group)
			{
				QString relPath = dir.relativeFilePath(file.m_fileName);

				string sfile = relPath.toStdString();

				XMLElement xmlfile("file");
				xmlfile.add_attribute("path", sfile);
				xml.add_empty(xmlfile);
			}
		}

		if (n < Groups())
		{
			xml.close_branch();
		}
	}
	xml.close_branch();
	xml.close();

	m_projectFile = file;

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
	m_fileList.clear();
	m_groupList.clear();

	try {
		++tag;
		do
		{
			if (tag == "group")
			{
				string groupName;
				const char* sz = tag.AttributeValue("name", true);
				if (sz) groupName = sz;
				else
				{
					int n = Groups();
					stringstream ss;
					ss << "group" << n + 1;
					groupName = ss.str();
				}

				int group = AddGroup(QString::fromStdString(groupName));

				if ((tag.isempty() == false) && (tag.isleaf() == false))
				{
					++tag;
					do
					{
						if (tag == "file")
						{
							const char* relPath = tag.AttributeValue("path", true);
							if (relPath == nullptr) return false;

							QString absPath = dir.absoluteFilePath(QString::fromStdString(relPath));
							absPath = QDir::toNativeSeparators(dir.cleanPath(absPath));

							AddFile(absPath, group);
						}
						else return false;
						++tag;
					} 
					while (!tag.isend());
				}
			}
			else if (tag == "file")
			{
				const char* relPath = tag.AttributeValue("path", true);
				if (relPath == nullptr) return false;

				QString absPath = dir.absoluteFilePath(QString::fromStdString(relPath));
				absPath = QDir::toNativeSeparators(dir.cleanPath(absPath));

				AddFile(absPath, -1);
			}
			else return false;
			++tag;
		} 
		while (!tag.isend());
	}
	catch (XMLReader::EndOfFile e)
	{
		// no worries, all good
	}

	xml.Close();

	m_projectFile = file;

	return true;
}
