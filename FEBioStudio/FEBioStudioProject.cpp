/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2021 University of Utah, The Trustees of Columbia University in
the City of New York, and others.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/

#include "stdafx.h"
#include "FEBioStudioProject.h"
#include <XML/XMLWriter.h>
#include <XML/XMLReader.h>
#include <QDir>
#include <sstream>

using std::string;
using std::stringstream;

int FEBioStudioProject::ProjectItem::m_count = 0;

int FEBioStudioProject::ProjectItem::newId() { return m_count++; }

bool FEBioStudioProject::ProjectItem::ContainsFile(const QString& fileName) const
{
	QList<ProjectItem*>::const_iterator it;
	for (it = m_items.begin(); it != m_items.end(); ++it)
	{
		ProjectItem& item = *(*it);
		if (item.IsFile())
		{
			if (item.Name() == fileName) return true;
		}
		else if (item.IsGroup())
		{
			bool b = item.ContainsFile(fileName);
			if (b) return true;
		}
	}
	return false;
}

FEBioStudioProject::ProjectItem* FEBioStudioProject::ProjectItem::FindItem(int id)
{
	if (id == m_id) return this;
	if (m_items.empty() == false)
	{
		QList<ProjectItem*>::iterator it;
		for (it = m_items.begin(); it != m_items.end(); ++it)
		{
			ProjectItem* pg = (*it)->FindItem(id);
			if (pg) return pg;
		}
	}
	return nullptr;
}

FEBioStudioProject::ProjectItem* FEBioStudioProject::ProjectItem::FindItem(const QString& name)
{
	if (name == m_name) return this;
	if (m_items.empty() == false)
	{
		QList<ProjectItem*>::iterator it;
		for (it = m_items.begin(); it != m_items.end(); ++it)
		{
			ProjectItem* pg = (*it)->FindItem(name);
			if (pg) return pg;
		}
	}
	return nullptr;
}


void FEBioStudioProject::ProjectItem::GetFilePaths(QStringList& filePaths)
{
	for(auto item : m_items)
	{
		if(item->IsFile()) filePaths.append(item->Name());

		item->GetFilePaths(filePaths);
	}
}

FEBioStudioProject::ProjectItem::~ProjectItem()
{
	QList<ProjectItem*>::iterator it;
	for (it = m_items.begin(); it != m_items.end(); ++it)
	{
		delete *it;
	}
	m_items.clear();
}

void FEBioStudioProject::ProjectItem::Remove(ProjectItem* item)
{
	assert(item);
	assert(item->m_parent == this);
	QList<ProjectItem*>::iterator it;
	for (it = m_items.begin(); it != m_items.end(); ++it)
	{
		if ((*it) == item)
		{
			m_items.erase(it);
			item->m_parent = nullptr;
			return;
		}
	}
	assert(false);
}

void FEBioStudioProject::ProjectItem::SetParent(ProjectItem* parent)
{
	assert(m_parent);
	m_parent->Remove(this);
	m_parent = parent;
}

void FEBioStudioProject::ProjectItem::AddItem(ProjectItem* item)
{
	// sanity checks
	if (item == nullptr) { assert(false); return; }
	if (item == this) { assert(false); return; }
	if (item->Parent() == nullptr) { assert(false); return; }

	// if item is already a child, don't do anything
	if (item->Parent() == this) return;

	// add it to the list
	item->SetParent(this);
	m_items.push_back(item);
}

FEBioStudioProject::ProjectItem& FEBioStudioProject::ProjectItem::AddFile(const QString& filePath) 
{ 
	// see if this file already exists
	QList<ProjectItem*>::iterator it;
	for (it = m_items.begin(); it != m_items.end(); ++it)
	{
		ProjectItem& item = *(*it);
		if (item.IsFile() && (item.Name() == filePath))
		{
			return item;
		}
	}
	m_items.push_back(new ProjectItem(PROJECT_FILE, filePath, this)); return *m_items.last(); 
}

FEBioStudioProject::ProjectItem& FEBioStudioProject::ProjectItem::AddGroup(const QString& name) 
{ 
	// see if this item already exists
	QList<ProjectItem*>::iterator it;
	for (it = m_items.begin(); it != m_items.end(); ++it)
	{
		ProjectItem& item = *(*it);
		if (item.IsGroup() && (item.Name() == name))
		{
			return item;
		}
	}
	m_items.push_back(new ProjectItem(PROJECT_GROUP, name, this)); return *m_items.last(); 
}


FEBioStudioProject::ProjectItem& FEBioStudioProject::ProjectItem::AddPlugin(const QString& name)
{
	// see if this item already exists
	QList<ProjectItem*>::iterator it;
	for (it = m_items.begin(); it != m_items.end(); ++it)
	{
		ProjectItem& item = *(*it);
		if (item.IsGroup() && (item.Name() == name))
		{
			return item;
		}
	}
	m_items.push_back(new ProjectItem(PROJECT_PLUGIN, name, this)); return *m_items.last();
}

void FEBioStudioProject::ProjectItem::RemoveSelf()
{
	assert(m_parent);
	QList<ProjectItem*>::iterator it;
	for (it = m_items.begin(); it != m_items.end(); ++it)
	{
		ProjectItem* pi = *it;
		pi->m_parent = m_parent;
		m_parent->m_items.push_back(pi);
	}
	m_items.clear();
	m_parent->Remove(this);
}

std::vector<int> FEBioStudioProject::ProjectItem::AllGroups() const
{
	std::vector<int> groupList;
	QList<ProjectItem*>::const_iterator it;
	for (it = m_items.begin(); it != m_items.end(); ++it)
	{
		if ((*it)->IsGroup()) groupList.push_back((*it)->Id());
	}
	return groupList;
}

FEBioStudioProject::FEBioStudioProject()
{
	m_rootItem = new ProjectItem(PROJECT_GROUP, "root");
}

FEBioStudioProject::~FEBioStudioProject()
{
	Save();
}


QString FEBioStudioProject::GetProjectFileName() const
{
	return m_projectFile;
}

QString FEBioStudioProject::GetProjectPath() const
{
	if (m_projectFile.isEmpty() == false)
	{
		QFileInfo fi(m_projectFile);
		return fi.absolutePath();
	}
	else return QString("");
}

const FEBioStudioProject::ProjectItem& FEBioStudioProject::RootItem() const
{
	return *m_rootItem;
}

FEBioStudioProject::ProjectItem* FEBioStudioProject::FindGroup(int groupId)
{
	if (groupId == -1) return m_rootItem;
	ProjectItem* item = m_rootItem->FindItem(groupId);
	if (item->IsGroup()) return item;
	else return nullptr;
}

const FEBioStudioProject::ProjectItem* FEBioStudioProject::FindGroup(int groupId) const
{
	if (groupId == -1) return m_rootItem;
	ProjectItem* item = m_rootItem->FindItem(groupId);
	if (item->IsGroup()) return item;
	else return nullptr;
}

const FEBioStudioProject::ProjectItem* FEBioStudioProject::FindFile(int fileId) const
{
	ProjectItem* item = m_rootItem->FindItem(fileId);
	if (item->IsFile()) return item;
	else return nullptr;
}

FEBioStudioProject::ProjectItem* FEBioStudioProject::FindFile(int fileId)
{
	ProjectItem* item = m_rootItem->FindItem(fileId);
	if (item->IsFile()) return item;
	else return nullptr;
}

const FEBioStudioProject::ProjectItem* FEBioStudioProject::FindItem(int fileId) const
{
	return m_rootItem->FindItem(fileId);
}

FEBioStudioProject::ProjectItem* FEBioStudioProject::FindItem(int fileId)
{
	return m_rootItem->FindItem(fileId);
}

const FEBioStudioProject::ProjectItem* FEBioStudioProject::FindFile(const QString& fileName) const
{
	QString file = ToAbsolutePath(fileName);
	ProjectItem* item = m_rootItem->FindItem(file);
	if (item && item->IsFile()) return item; else return nullptr;
}

FEBioStudioProject::ProjectItem* FEBioStudioProject::FindFile(const QString& fileName)
{
	QString file = ToAbsolutePath(fileName);
	ProjectItem* item = m_rootItem->FindItem(file);
	if (item && item->IsFile()) return item; else return nullptr;
}


QStringList FEBioStudioProject::GetFilePaths()
{
	QStringList filePaths;

	m_rootItem->GetFilePaths(filePaths);

	return filePaths;
}

FEBioStudioProject::ProjectItem* FEBioStudioProject::AddGroup(const QString& groupName, int parentId)
{
	FEBioStudioProject::ProjectItem* grp = FindGroup(parentId);
	assert(grp);
	if (grp)
	{
		FEBioStudioProject::ProjectItem* newGroup = &grp->AddGroup(groupName);
		Save();
		return newGroup;
	}
	return nullptr;
}

FEBioStudioProject::ProjectItem* FEBioStudioProject::AddFile(const QString& file, int parent)
{
	// convert file to cleaned up format
	QString fileName = ToAbsolutePath(file);

	ProjectItem* newItem = nullptr;
	if (!ContainsFile(fileName))
	{
		if (parent == -1) newItem = &m_rootItem->AddFile(fileName);
		else
		{
			FEBioStudioProject::ProjectItem* it = FindItem(parent); assert(it);
			newItem = &it->AddFile(fileName);
		}
	}
	Save();
	return newItem;
}

bool FEBioStudioProject::IsEmpty() const
{
	return (m_rootItem->Items() == 0);
}

void FEBioStudioProject::Clear()
{
	delete m_rootItem;
	FEBioStudioProject::ProjectItem::m_count = 0;
	m_rootItem = new ProjectItem(PROJECT_GROUP, "root");
	Save();
}

void FEBioStudioProject::Close()
{
	delete m_rootItem;
	FEBioStudioProject::ProjectItem::m_count = 0;
	m_rootItem = new ProjectItem(PROJECT_GROUP, "root");
	m_projectFile.clear();
}

bool FEBioStudioProject::ContainsFile(const QString& fileName) const
{
	QString file = ToAbsolutePath(fileName);
	return m_rootItem->ContainsFile(file);
}

void FEBioStudioProject::MoveToGroup(int itemId, int groupId)
{
	// find the group
	ProjectItem* group = FindGroup(groupId);
	if (group == nullptr) { assert(false); return; }

	// find the item
	ProjectItem* item = m_rootItem->FindItem(itemId); 
	if (item == nullptr) { assert(false); return; }

	// make sure they are not the same
	if (item == group) { assert(false); return; }

	// make sure item is not root
	if (item == m_rootItem) { assert(false); return; }

	// add it to group
	group->AddItem(item);

	Save();
}

void FEBioStudioProject::RemoveGroup(int groupId)
{
	ProjectItem* group = FindGroup(groupId);
	assert(group && (group != m_rootItem));
	group->RemoveSelf();
	delete group;
	Save();
}

void FEBioStudioProject::RemoveFile(int fileId)
{
	ProjectItem* file = FindFile(fileId);
	assert(file->Parent());
	file->Parent()->Remove(file);
	delete file;
	Save();
}

void FEBioStudioProject::RenameGroup(int groupId, const QString& newName)
{
	ProjectItem* group = FindGroup(groupId);
	assert(group && (group != m_rootItem));
	group->SetName(newName);
	Save();
}

bool FEBioStudioProject::Save()
{
	if (m_projectFile.isEmpty() == false) return Save(m_projectFile);
	else return false;
}

void WriteProjectGroup(XMLWriter& xml, FEBioStudioProject& prj, FEBioStudioProject::ProjectItem& parent)
{
	std::string info = parent.Info().toStdString();
	if (info.empty() == false)
	{
		xml.add_leaf("info", info);
	}

	for (int i = 0; i < parent.Items(); ++i)
	{
		FEBioStudioProject::ProjectItem& item = parent.Item(i);
		if (item.IsType(FEBioStudioProject::PROJECT_FILE))
		{
			QString relPath = prj.ToRelativePath(item.Name());
			string sfile = relPath.toStdString();

			XMLElement xmlfile("file");
			xmlfile.add_attribute("path", sfile);
			std::string info = item.Info().toStdString();
			if ((info.empty() == false) || (item.Items() > 0))
			{
				xml.add_branch(xmlfile);
				{
					WriteProjectGroup(xml, prj, item);
				}
				xml.close_branch();
			}
			else xml.add_empty(xmlfile);
		}
		else if (item.IsType(FEBioStudioProject::PROJECT_GROUP))
		{
			XMLElement group("group");
			group.add_attribute("name", item.Name().toStdString());
			xml.add_branch(group);
			{
				WriteProjectGroup(xml, prj, item);
			}
			xml.close_branch();
		}
		else if (item.IsType(FEBioStudioProject::PROJECT_PLUGIN))
		{
			XMLElement group("febio_plugin");
			group.add_attribute("name", item.Name().toStdString());
			xml.add_branch(group);
			{
				WriteProjectGroup(xml, prj, item);
			}
			xml.close_branch();
		}
	}
}

bool FEBioStudioProject::Save(const QString& file)
{
	QFileInfo fi(file);

	QDir dir = fi.absoluteDir();

	XMLWriter xml;
	std::string fileName = file.toStdString();
	if (xml.open(fileName.c_str()) == false) return false;

	m_projectFile = file;

	XMLElement root("FEBioStudioProject");
	root.add_attribute("version", "1.0");
	xml.add_branch(root);
	{
		WriteProjectGroup(xml, *this, *m_rootItem);
	}
	xml.close_branch();

	return true;
}

void ParseTags(XMLTag& tag, FEBioStudioProject& prj, FEBioStudioProject::ProjectItem& parent)
{
	int n = 1;
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
				stringstream ss;
				ss << "group" << n++;
				groupName = ss.str();
			}

			FEBioStudioProject::ProjectItem& group = parent.AddGroup(QString::fromStdString(groupName));

			if ((tag.isempty() == false) && (tag.isleaf() == false)) 
			{
				ParseTags(tag, prj, group);
			}
		}
		else if (tag == "febio_plugin")
		{
			string pluginName = tag.AttributeValue("name");

			FEBioStudioProject::ProjectItem& plugin = parent.AddPlugin(QString::fromStdString(pluginName));

			if ((tag.isempty() == false) && (tag.isleaf() == false))
			{
				ParseTags(tag, prj, plugin);
			}
		}
		else if (tag == "file")
		{
			FEBioStudioProject::ProjectItem* item = nullptr;
			const char* relPath = tag.AttributeValue("path", true);
			if (relPath != nullptr)
			{
				QString absPath = prj.ToAbsolutePath(QString::fromStdString(relPath));
				item = &parent.AddFile(absPath);
			}

			if ((tag.isempty() == false) && (tag.isleaf() == false))
			{
				ParseTags(tag, prj, *item);
			}
		}
		else if (tag == "info")
		{ 
			QString info(tag.szvalue());
			parent.SetInfo(info);
		}
		else throw XMLReader::InvalidTag(tag);

		++tag;
	}
	while (!tag.isend());
}

bool FEBioStudioProject::Open(const QString& file)
{
	std::string fileName = file.toStdString();
	m_projectFile = file;

	XMLReader xml;
	if (xml.Open(fileName.c_str()) == false) return false;
	XMLTag tag;
	if (xml.FindTag("FEBioStudioProject", tag) == false) return false;

	QFileInfo fi(file);
	QDir dir = fi.absoluteDir();

	// add root item
	delete m_rootItem;
	FEBioStudioProject::ProjectItem::m_count = 0;
	m_rootItem = new ProjectItem(PROJECT_GROUP, "root");
	ParseTags(tag, *this, *m_rootItem);
	xml.Close();

	return true;
}

QString FEBioStudioProject::ToAbsolutePath(const QString& relativePath) const
{
	QFileInfo fi(m_projectFile);
	QDir dir = fi.absoluteDir();
	
	return QDir::toNativeSeparators(QDir::cleanPath(dir.absoluteFilePath(relativePath)));
}

QString FEBioStudioProject::ToRelativePath(const QString& absolutePath) const
{
	QFileInfo fi(m_projectFile);
	QDir dir = fi.absoluteDir();
	return QDir::toNativeSeparators(dir.relativeFilePath(absolutePath));
}
