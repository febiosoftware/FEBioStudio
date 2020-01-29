#include "LocalDatabaseHandler.h"

#ifdef MODEL_REPO
#include <sqlite3.h>
#include <vector>
#include <string>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include "RepoProject.h"
#include "DatabasePanel.h"

#include <iostream>

//static int callback(void *NotUsed, int argc, char **argv, char **azColName){
//int i;
//for(i=0; i<argc; i++){
//printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
//}
//printf("\n");
//return 0;
//}

static int addProjectCallback(void *dbPanel, int argc, char **argv, char **azColName)
{
	((CDatabasePanel*) dbPanel)->AddProject(argv);

	return 0;
}

static int addProjectFilesCallback(void *dbPanel, int argc, char **argv, char **azColName)
{
	((CDatabasePanel*) dbPanel)->AddProjectFile(argv);

	return 0;
}

static int setProjectDataCallback(void *dbPanel, int argc, char **argv, char **azColName)
{
	((CDatabasePanel*) dbPanel)->SetProjectData(argv);

	return 0;
}

static int setFileDataCallback(void *dbPanel, int argc, char **argv, char **azColName)
{
	((CDatabasePanel*) dbPanel)->SetFileData(argv);

	return 0;
}

static int addCurrentTagCallback(void *dbPanel, int argc, char **argv, char **azColName)
{
	((CDatabasePanel*) dbPanel)->AddCurrentTag(argv);

	return 0;
}

class CLocalDatabaseHandler::Imp
{
public:
	Imp(std::string& dbPath, CDatabasePanel* dbPanel) : dbPanel(dbPanel)
	{
		openDatabase(dbPath);
	}

	void openDatabase(std::string& dbPath)
	{
		char *zErrMsg = 0;

		int rc = sqlite3_open(dbPath.c_str(), &db);

		if( rc )
		{
			fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
			sqlite3_close(db);
			return;
		}

		rc = sqlite3_exec(
				db,
				"CREATE TABLE IF NOT EXISTS 'users' ("
				"	'ID'	INTEGER NOT NULL PRIMARY KEY UNIQUE,"
				"	'username'	TEXT NOT NULL UNIQUE);"
				"CREATE TABLE IF NOT EXISTS 'authors' ("
				"	'ID'	INTEGER NOT NULL PRIMARY KEY UNIQUE,"
				"	'firstName'	TEXT NOT NULL,"
				"	'lastName'	TEXT NOT NULL);"
				"CREATE TABLE IF NOT EXISTS 'projects' ("
				"	'ID'	INTEGER NOT NULL PRIMARY KEY UNIQUE,"
				"	'owner'	INTEGER NOT NULL,"
				"	'name'	TEXT NOT NULL,"
				"	'description'	TEXT NOT NULL,"
				"	'version'	INTEGER NOT NULL,"
				"   'fileID'      INTEGER NOT NULL UNIQUE);"
				"CREATE TABLE IF NOT EXISTS 'projectFilenames' ("
				"	'project'	INTEGER NOT NULL,"
				"	'filename'	INTEGER NOT NULL UNIQUE);"
				"CREATE TABLE IF NOT EXISTS 'tags' ("
				"	'ID'	INTEGER NOT NULL PRIMARY KEY UNIQUE,"
				"	'tag'	TEXT NOT NULL);"
				"CREATE TABLE IF NOT EXISTS 'projectTags' ("
				"	'project'	INTEGER NOT NULL,"
				"	'tag'	INTEGER NOT NULL);"
				"CREATE TABLE IF NOT EXISTS 'publications' ("
				"	'ID'	INTEGER NOT NULL PRIMARY KEY UNIQUE,"
				"	'title'	TEXT NOT NULL,"
				"	'date'	TEXT NOT NULL,"
				"	'journal'	TEXT NOT NULL,"
				"	'edition'	TEXT NOT NULL,"
				"   'DOI'	TEXT);"
				"CREATE TABLE IF NOT EXISTS 'publicationAuthors' ("
				"	'author'	INTEGER NOT NULL,"
				"	'order'	INTEGER NOT NULL,"
				"	'publication'	INTEGER NOT NULL);"
				"CREATE TABLE IF NOT EXISTS 'filenames' ("
				"	'ID'	INTEGER NOT NULL PRIMARY KEY UNIQUE,"
				"	'filename'	TEXT NOT NULL,"
				"   'description'   TEXT,"
				"	'localCopy'	INTEGER DEFAULT 0);"
				"CREATE TABLE IF NOT EXISTS 'fileTags' ("
				"	'file'	INTEGER NOT NULL,"
				"	'tag'	INTEGER NOT NULL);"
				"CREATE TABLE IF NOT EXISTS 'projectPubs' ("
				"	'project'	INTEGER NOT NULL,"
				"	'publication'	INTEGER NOT NULL);",
				NULL,
				NULL,
				&zErrMsg
				);

		if( rc!=SQLITE_OK )
		{
			fprintf(stderr, "SQL error: %s\n", zErrMsg);
			sqlite3_free(zErrMsg);
		}

//		std::string temp = ("SELECT sqlite_version();");
//
//		execute(temp, callback);
	}

	void execute(std::string& query, int (*callback)(void*,int,char**,char**)=NULL, void* arg = NULL)
	{
		int rc;
		char *zErrMsg = 0;

		rc = sqlite3_exec(db, query.c_str(), callback, arg, &zErrMsg);

		if( rc!=SQLITE_OK )
		{
			fprintf(stderr, "SQL error: %s\n", zErrMsg);
			sqlite3_free(zErrMsg);
		}
	}

	void getTable(std::string& query, char ***table, int* rows, int* cols)
	{
		int rc;
		char *zErrMsg = 0;

		rc = sqlite3_get_table(db, query.c_str(), table, rows, cols, &zErrMsg);

		if( rc!=SQLITE_OK )
		{
			fprintf(stderr, "SQL error: %s\n", zErrMsg);
			sqlite3_free(zErrMsg);
		}
	}

	void upsert(std::string& tableName, std::vector<std::string>& columns, std::string& values, std::string conflict = "")
	{
		std::string query("INSERT INTO ");
		query += tableName;
		query += "(" + columns[0];
		for(int index = 1; index < columns.size(); index++)
		{
			query += ", " + columns[index];
		}
		query += ") ";
		query += "VALUES " + values;

		if(conflict.compare("") !=0)
		{
			query += " ON CONFLICT(ID) DO UPDATE SET ";
			query += columns[0] + "=excluded." + columns[0];
			for(int index = 1; index < columns.size(); index++)
			{
				query += ", " + columns[index] + "=excluded." + columns[index];
			}
		}
		query += ";";

		std::cout << query << std::endl;

		execute(query);
	}

	void checkLocalCopies()
	{
		char **table;
		int rows, cols;

		std::string query = "SELECT ID FROM filenames";
		getTable(query, &table, &rows, &cols);

		std::string hasCopy = "UPDATE filenames set localCopy = 1 WHERE ID IN (";
		std::string noCopy = "UPDATE filenames set localCopy = 0 WHERE ID IN (";

		bool updateHasCopy = false;
		bool updateNoCopy = false;

		for(int row = 1; row < rows + 1; row++)
		{
			QString filename = GetFullFilename(std::stoi(table[row]), 1);


			QFile file(filename);
			if(file.exists())
			{
				hasCopy += table[row];
				hasCopy += ", ";
				updateHasCopy = true;
			}
			else
			{
				noCopy += table[row];
				noCopy += ", ";
				updateNoCopy = true;
			}

		}

		// Remove the last, unnecessary comma and space
		hasCopy.pop_back();
		hasCopy.pop_back();
		noCopy.pop_back();
		noCopy.pop_back();


		hasCopy += ");";
		noCopy += ");";

		cout << hasCopy << endl;
		cout << noCopy << endl;

		sqlite3_free_table(table);

		if(updateHasCopy) execute(hasCopy);
		if(updateNoCopy) execute(noCopy);
	}

	QString GetFilePath(int ID, int type)
	{
		char **table;
		int rows, cols;

		std::string query("");
		QString path;

		// If it's an entire project file
		if(type == FULL)
		{
			query += "SELECT users.username FROM projects JOIN users ON projects.owner = users.ID WHERE projects.fileID = ";
			query += std::to_string(ID);

			getTable(query, &table, &rows, &cols);

			if(rows == 1)
			{
				path += table[1];
			}
		}
		// If it's a single file from a project
		else
		{
			query += "SELECT users.username, projects.name FROM projects JOIN users ON projects.owner = users.ID JOIN projectFilenames ON projects.ID = projectFilenames.project WHERE projectFilenames.filename = ";
			query += std::to_string(ID);

			getTable(query, &table, &rows, &cols);

			if(rows == 1)
			{
				path += table[2];
				path += "/";
				path += table[3];
			}
		}

		sqlite3_free_table(table);

		return path;
	}

	QString GetFileName(int ID, int type)
	{
		char **table;
		int rows, cols;

		std::string query;
		QString filename;

		// If it's an entire project file
		if(type == FULL)
		{
			query += "SELECT name FROM projects WHERE fileID = ";
			query += std::to_string(ID);
		}
		// If it's a single file from a project
		else
		{
			query += "SELECT filename FROM filenames WHERE ID = ";
			query += std::to_string(ID);

		}

		getTable(query, &table, &rows, &cols);

		if(rows == 1)
		{
			filename += table[1];

			if(type == FULL) filename += ".prj";
		}

		sqlite3_free_table(table);

		return filename;
	}

	QString GetFullFilename(int ID, int type)
	{
		QString filename = dbPanel->RepositoryFolder();
		filename += "/";
		filename += GetFilePath(ID, type);
		filename += "/";
		filename += GetFileName(ID, type);

		return filename;
	}

	QString GetFullPath(int ID, int type)
	{
		QString filename = dbPanel->RepositoryFolder();
		filename += "/";
		filename += GetFilePath(ID, type);
		filename += "/";
		filename += GetFileName(ID, type);

		return filename;
	}

	int ProjectIDFromFileID(int ID, int type)
	{
		char **table;
		int rows, cols;

		std::string query;
		int projID;

		// If it's an entire project file
		if(type == FULL)
		{
			query += "SELECT ID FROM projects WHERE fileID = ";
			query += std::to_string(ID);
		}
		// If it's a single file from a project
		else
		{
			query += "SELECT project FROM projectFilenames WHERE filename = ";
			query += std::to_string(ID);

		}

		getTable(query, &table, &rows, &cols);

		if(rows == 1)
		{
			projID += stoi(table[1]);
		}

		sqlite3_free_table(table);

		return projID;
	}

public:
	sqlite3* db;
	CDatabasePanel* dbPanel;

};

CLocalDatabaseHandler::CLocalDatabaseHandler(std::string dbPath, CDatabasePanel* dbPanel)
{
	imp = new Imp(dbPath, dbPanel);
}

CLocalDatabaseHandler::~CLocalDatabaseHandler(){}

void CLocalDatabaseHandler::update(QJsonDocument& jsonDoc)
{
	// Empty tables for which upsert would not work
	std::string query("DELETE FROM projectFilenames");
	imp->execute(query);
	query = "DELETE FROM projectTags";
	imp->execute(query);
	query = "DELETE FROM projectPubs";
	imp->execute(query);
	query = "DELETE FROM fileTags";
	imp->execute(query);
	query = "DELETE FROM publicationAuthors";
	imp->execute(query);

	QJsonArray jsonProjects = jsonDoc.array();
	for(QJsonValueRef jsonProject : jsonProjects)
	{
		QJsonObject projectObj = jsonProject.toObject();
		std::string name = projectObj.value("name").toString().toStdString();

		std::string conflict("");
		std::vector<std::string> columnNames;
		QJsonArray columns = projectObj.value("columns").toArray();
		for(QJsonValueRef column : columns)
		{
			std::string columnName = column.toString().toStdString();
			if(columnName.compare("ID") == 0) conflict = "ID";
			columnNames.push_back(column.toString().toStdString());
		}

		if(columnNames.size() == 0) continue;

		std::string values = projectObj.value("values").toString().toStdString();

		imp->upsert(name, columnNames, values, conflict);
	}

	imp->checkLocalCopies();
}

void CLocalDatabaseHandler::GetProjects()
{
	std::string query("SELECT projects.ID, name FROM projects");

	imp->execute(query, addProjectCallback, imp->dbPanel);
}

void CLocalDatabaseHandler::GetProjectFiles(int ID)
{
	std::string query("SELECT filenames.ID, filenames.filename, filenames.localCopy from filenames join projectFilenames on filenames.ID = projectFilenames.filename where projectFilenames.project = ");
	query += std::to_string(ID);

	imp->execute(query, addProjectFilesCallback, imp->dbPanel);
}

void CLocalDatabaseHandler::GetProjectData(int ID)
{
	std::string query("SELECT name, description, username, version FROM projects JOIN users on users.id = projects.owner WHERE projects.id = ");
	query += std::to_string(ID);

	imp->execute(query, setProjectDataCallback, imp->dbPanel);
}

void CLocalDatabaseHandler::GetFileData(int ID)
{
	std::string query("SELECT filename, description FROM filenames WHERE id = ");
	query += std::to_string(ID);

	imp->execute(query, setFileDataCallback, imp->dbPanel);
}

void CLocalDatabaseHandler::GetProjectTags(int ID)
{
	std::string query("SELECT tags.tag FROM tags JOIN projectTags on tags.id = projectTags.tag WHERE projectTags.project = ");
	query += std::to_string(ID);

	imp->execute(query, addCurrentTagCallback, imp->dbPanel);
}

std::unordered_set<int> CLocalDatabaseHandler::FullTextSearch(QString term)
{
	char **table;
	int rows, cols;

	std::unordered_set<int> projects;

	// Matches owners, names, and descriptions of projects
	QString query = QString("SELECT ID FROM projects WHERE owner LIKE '%%1%' OR name LIKE '%%1%' OR description LIKE '%%1%'").arg(term);
	std::string queryStd = query.toStdString();

	cout << queryStd <<endl;

	imp->getTable(queryStd, &table, &rows, &cols);

	for(int row = 1; row <= rows; row++)
	{
		projects.insert(stoi(table[row]));
	}

	sqlite3_free_table(table);

	// Matches filenames and descriptions
	query = QString("SELECT projectFilenames.project FROM filenames JOIN projectFilenames ON filenames.ID = projectFilenames.filename WHERE filenames.filename LIKE '%%1%' OR filenames.description LIKE '%%1%'").arg(term);
	queryStd = query.toStdString();

	cout << queryStd <<endl;

	imp->getTable(queryStd, &table, &rows, &cols);

	for(int row = 1; row <= rows; row++)
	{
		projects.insert(stoi(table[row]));
	}

	sqlite3_free_table(table);

	// Matches project tags
	query = QString("SELECT projectTags.project FROM tags JOIN projectTags ON tags.ID = projectTags.tag WHERE tags.tag LIKE '%%1%'").arg(term);
	queryStd = query.toStdString();

	cout << queryStd <<endl;

	imp->getTable(queryStd, &table, &rows, &cols);

	for(int row = 1; row <= rows; row++)
	{
		projects.insert(stoi(table[row]));
	}

	sqlite3_free_table(table);

	return projects;
}



QString CLocalDatabaseHandler::FilePathFromID(int ID, int type)
{
	return imp->GetFilePath(ID, type);
}

QString CLocalDatabaseHandler::FileNameFromID(int ID, int type)
{
	return imp->GetFileName(ID, type);
}

QString CLocalDatabaseHandler::FullFileNameFromID(int ID, int type)
{
	return imp->GetFullFilename(ID, type);
}

int CLocalDatabaseHandler::ProjectIDFromFileID(int ID, int type)
{
	return imp->ProjectIDFromFileID(ID, type);
}



#else

CLocalDatabaseHandler::CLocalDatabaseHandler(std::string dbPath, CDatabasePanel* dbPanel){}
CLocalDatabaseHandler::~CLocalDatabaseHandler(){}
void CLocalDatabaseHandler::update(QJsonDocument& jsonDoc){}

#endif
