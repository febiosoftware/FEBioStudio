#include "stdafx.h"
#include "FEBioStudioProject.h"
#include <XML/XMLWriter.h>
#include <XML/XMLReader.h>
#include <QDir>
#include <sstream>

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

QString FEBioStudioProject::GetProjectFileName() const
{
	return m_projectFile;
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

void FEBioStudioProject::AddGroup(const QString& groupName, int parentId)
{
	FEBioStudioProject::ProjectItem* grp = FindGroup(parentId);
	assert(grp);
	if (grp) grp->AddGroup(groupName);
	Save();
}

bool FEBioStudioProject::AddFile(const QString& file, int parent)
{
	if (ContainsFile(file)) return false;
	if (parent == -1) m_rootItem->AddFile(file);
	else
	{
		FEBioStudioProject::ProjectItem* grp = FindGroup(parent); assert(grp);
		grp->AddFile(file);
	}
	Save();
	return true;
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
}

bool FEBioStudioProject::ContainsFile(const QString& fileName) const
{
	return m_rootItem->ContainsFile(fileName);
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
	for (int i = 0; i < parent.Items(); ++i)
	{
		FEBioStudioProject::ProjectItem& item = parent.Item(i);
		if (item.IsType(FEBioStudioProject::PROJECT_FILE))
		{
			QString relPath = prj.ToRelativePath(item.Name());
			string sfile = relPath.toStdString();

			XMLElement xmlfile("file");
			xmlfile.add_attribute("path", sfile);
			xml.add_empty(xmlfile);
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
		else if (tag == "file")
		{
			const char* relPath = tag.AttributeValue("path", true);
			if (relPath != nullptr)
			{
				QString absPath = prj.ToAbsolutePath(QString::fromStdString(relPath));
				parent.AddFile(absPath);
			}
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

	try {
		ParseTags(tag, *this, *m_rootItem);
	}
	catch (XMLReader::EndOfFile e)
	{
		// no worries, all good
	}

	xml.Close();

	return true;
}

QString FEBioStudioProject::ToAbsolutePath(const QString& relativePath)
{
	QFileInfo fi(m_projectFile);
	QDir dir = fi.absoluteDir();
	return QDir::toNativeSeparators(dir.absoluteFilePath(relativePath));
}

QString FEBioStudioProject::ToRelativePath(const QString& absolutePath)
{
	QFileInfo fi(m_projectFile);
	QDir dir = fi.absoluteDir();
	return QDir::toNativeSeparators(dir.relativeFilePath(absolutePath));
}
