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
	m_folderList.clear();
	Save();
}

int FEBioStudioProject::Folders() const
{
	return m_folderList.size();
}

QString FEBioStudioProject::GetFolder(int n) const
{
	return m_folderList.at(n);
}

int FEBioStudioProject::AddFolder(const QString& folderName)
{
	for (int i = 0; i < Folders(); ++i) {
		if (m_folderList[i] == folderName) return i;
	}
	m_folderList.push_back(folderName);
	Save();
	return Folders() - 1;
}

void FEBioStudioProject::Close()
{
	m_projectFile.clear();
	m_fileList.clear();
	m_folderList.clear();
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

void FEBioStudioProject::MoveToFolder(const QString& file, int folderIndex)
{
	assert((folderIndex >= -1) && (folderIndex < Folders()));
	QList<File>::iterator it = m_fileList.begin();
	int n = m_fileList.size();
	for (int i = 0; i < m_fileList.size(); ++i, ++it)
	{
		if (file == it->m_fileName)
		{
			it->m_folder = folderIndex;
			Save();
			break;
		}
	}
}

bool FEBioStudioProject::RemoveFolder(const QString& folderName)
{
	int nfolder = -1;
	for (int i = 0; i < m_folderList.size(); ++i)
	{
		if (m_folderList[i] == folderName)
		{
			nfolder = i;
			break;
		}
	}

	if (nfolder == -1) return false;

	m_folderList.removeAt(nfolder);

	for (int i = 0; i < m_fileList.size(); ++i)
	{
		File& file = m_fileList[i];
		if (file.m_folder == nfolder)
		{
			file.m_folder = -1;
		}
		else if (file.m_folder > nfolder)
		{
			file.m_folder--;
		}
	}

	Save();

	return true;
}

bool FEBioStudioProject::RenameFolder(const QString& folderName, const QString& newName)
{
	if (newName.isEmpty()) return false;

	int nfolder = -1;
	for (int i = 0; i < m_folderList.size(); ++i)
	{
		if (m_folderList[i] == folderName)
		{
			nfolder = i;
			break;
		}
	}

	if (nfolder == -1) return false;

	m_folderList[nfolder] = newName;

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
	for (int n = 0; n <= Folders(); ++n)
	{
		if (n < Folders())
		{
			string folderName = GetFolder(n).toStdString();
			XMLElement folder("folder");
			folder.add_attribute("name", folderName);
			xml.add_branch(folder);
		}

		int folder = (n < Folders() ? n : -1);

		for (int i = 0; i < Files(); ++i)
		{
			FEBioStudioProject::File file = GetFile(i);

			if (file.m_folder == folder)
			{
				QString relPath = dir.relativeFilePath(file.m_fileName);

				string sfile = relPath.toStdString();

				XMLElement xmlfile("file");
				xmlfile.add_attribute("path", sfile);
				xml.add_empty(xmlfile);
			}
		}

		if (n < Folders())
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
	m_folderList.clear();

	try {
		++tag;
		do
		{
			if (tag == "folder")
			{
				string folderName;
				const char* sz = tag.AttributeValue("name", true);
				if (sz) folderName = sz;
				else
				{
					int n = Folders();
					stringstream ss;
					ss << "Folder" << n + 1;
					folderName = ss.str();
				}

				int folder = AddFolder(QString::fromStdString(folderName));

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

							AddFile(absPath, folder);
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
