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

#pragma once
#include <QString>
#include <QStringList>
#include <vector>

class FEBioStudioProject
{
public:
	enum ProjectItemType
	{
		PROJECT_FILE,
		PROJECT_GROUP,
		PROJECT_PLUGIN
	};

	class ProjectItem
	{
	public:
		ProjectItem(ProjectItemType type, const QString& name, ProjectItem* parent = nullptr) : m_type(type), m_name(name) { m_id = newId(); m_parent = parent; }
		~ProjectItem();

		QString Name() const { return m_name; }

		ProjectItemType Type() const { return m_type; }

		bool IsType(ProjectItemType type) const { return (type == m_type); }
		bool IsFile() const { return (m_type == PROJECT_FILE); }
		bool IsGroup() const { return (m_type == PROJECT_GROUP); }
		bool IsPlugin() const { return (m_type == PROJECT_PLUGIN); }

		int Items() const { return m_items.size(); }

		int Id() const { return m_id; }

		ProjectItem* FindItem(int id);
		ProjectItem* FindItem(const QString& name);

		void GetFilePaths(QStringList& filePaths);

		ProjectItem* Parent() { return m_parent; }
		const ProjectItem* Parent() const { return m_parent; }

		ProjectItem& Item(int i) { return *m_items[i]; }
		const ProjectItem& Item(int i) const { return *m_items[i]; }

		ProjectItem& AddFile(const QString& filePath);
		ProjectItem& AddGroup(const QString& name);
		ProjectItem& AddPlugin(const QString& name);

		bool ContainsFile(const QString& fileName) const;

		std::vector<int> AllGroups() const;

		QString Info() const { return m_info; }
		void SetInfo(const QString& infoString) { m_info = infoString; }

	private:
		int newId();
		ProjectItem(const ProjectItem& pi) { }
		void operator = (const ProjectItem& pi) { }

		void AddItem(ProjectItem* item);
		void SetParent(ProjectItem* item);
		void Remove(ProjectItem* item);
		void RemoveSelf();

		void SetName(const QString& name) { m_name = name; }

	private:
		int					m_id;
		ProjectItemType		m_type;
		QString				m_name;
		QString				m_info;		//!< user provided description of item
		QList<ProjectItem*>	m_items;
		ProjectItem*		m_parent;

		static int m_count;

		friend class FEBioStudioProject;
	};

public:
	FEBioStudioProject();
	~FEBioStudioProject();

	QString GetProjectFileName() const;
	QString GetProjectPath() const;

	bool Save(const QString& file);
	bool Save();

	bool Open(const QString& file);

	void Clear();

	void Close();

	bool ContainsFile(const QString& fileName) const;

	QString ToAbsolutePath(const QString& relativePath) const;
	QString ToRelativePath(const QString& absolutePath) const;

	const ProjectItem& RootItem() const;

	const ProjectItem* FindGroup(int groupId) const;
	ProjectItem* FindGroup(int groupId);

	ProjectItem* FindFile(int fileId);
	const ProjectItem* FindFile(int fileId) const;

	ProjectItem* FindItem(int fileId);
	const ProjectItem* FindItem(int fileId) const;

	const ProjectItem* FindFile(const QString& fileName) const;
	ProjectItem* FindFile(const QString& fileName);

	QStringList GetFilePaths();

	ProjectItem* AddGroup(const QString& groupName, int parentId = -1);
	ProjectItem* AddPlugin(const QString& groupName, int parentId = -1);

	void MoveToGroup(int itemId, int groupId);

	void RemoveGroup(int groupId);

	void RenameGroup(int groupId, const QString& newName);

	void RemoveFile(int fileId);

	ProjectItem* AddFile(const QString& file, int parent = -1);

	bool IsEmpty() const;

private:
	QString				m_projectFile;
	ProjectItem*		m_rootItem;
};
