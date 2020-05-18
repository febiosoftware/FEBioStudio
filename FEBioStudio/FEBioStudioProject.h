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
		PROJECT_GROUP
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

		int Items() const { return m_items.size(); }

		int Id() const { return m_id; }

		ProjectItem* FindItem(int id);

		ProjectItem* Parent() { return m_parent; }
		const ProjectItem* Parent() const { return m_parent; }

		ProjectItem& Item(int i) { return *m_items[i]; }
		const ProjectItem& Item(int i) const { return *m_items[i]; }

		ProjectItem& AddFile(const QString& filePath) { m_items.push_back(new ProjectItem(PROJECT_FILE, filePath, this)); return *m_items.last(); }
		ProjectItem& AddGroup(const QString& name) { m_items.push_back(new ProjectItem(PROJECT_GROUP, name, this)); return *m_items.last(); }

		bool ContainsFile(const QString& fileName) const;

		std::vector<int> AllGroups() const;

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
		QList<ProjectItem*>	m_items;
		ProjectItem*		m_parent;

		static int m_count;

		friend class FEBioStudioProject;
	};

public:
	FEBioStudioProject();

	QString GetProjectFileName() const;

	bool Save(const QString& file);
	bool Save();

	bool Open(const QString& file);

	void Clear();

	void Close();

	bool ContainsFile(const QString& fileName) const;

	QString ToAbsolutePath(const QString& relativePath);
	QString ToRelativePath(const QString& absolutePath);

	const ProjectItem& RootItem() const;

	const ProjectItem* FindGroup(int groupId) const;
	ProjectItem* FindGroup(int groupId);

	ProjectItem* FindFile(int fileId);
	const ProjectItem* FindFile(int fileId) const;

	void AddGroup(const QString& groupName, int parentId);

	void MoveToGroup(int itemId, int groupId);

	void RemoveGroup(int groupId);

	void RenameGroup(int groupId, const QString& newName);

	void RemoveFile(int fileId);

	bool AddFile(const QString& file, int parent = -1);

	bool IsEmpty() const;

private:
	QString				m_projectFile;
	ProjectItem*		m_rootItem;
};
