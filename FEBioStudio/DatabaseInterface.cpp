/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2025 University of Utah, The Trustees of Columbia University in
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

#include "DatabaseInterface.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

#ifdef MODEL_REPO
#include <sqlite3.h>

CDatabaseInterface::CDatabaseInterface()
    : db(nullptr)
{

}

void CDatabaseInterface::setDBPath(string path)
{
    dbPath = path;
}

bool CDatabaseInterface::openDatabase()
{
    int rc = sqlite3_open(dbPath.c_str(), &db);

    if( rc )
    {
        fprintf(stderr, "Can't open database: %s\nError: %s\n", dbPath.c_str(), sqlite3_errmsg(db));
        sqlite3_close(db);
        return false;
    }

    return true;
}

void CDatabaseInterface::closeDatabase()
{
    sqlite3_close(db);
}

void CDatabaseInterface::initDatabase(string schema)
{
    char *zErrMsg = 0;

    if(!openDatabase()) return;

    // Completely empty the database
    sqlite3_db_config(db, SQLITE_DBCONFIG_RESET_DATABASE, 1, 0);
    sqlite3_exec(db, "VACUUM", 0, 0, 0);
    sqlite3_db_config(db, SQLITE_DBCONFIG_RESET_DATABASE, 0, 0);

    int rc = sqlite3_exec(db,schema.c_str(), NULL, NULL, &zErrMsg);

    if( rc!=SQLITE_OK )
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }

    closeDatabase();
}

void CDatabaseInterface::execute(string& query, int (*callback)(void*,int,char**,char**), void* arg)
{
    if(!openDatabase()) return;

    char *zErrMsg = 0;

    int rc = sqlite3_exec(db, query.c_str(), callback, arg, &zErrMsg);

    if( rc!=SQLITE_OK )
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }

    closeDatabase();
}

void CDatabaseInterface::getTable(string& query, char ***table, int* rows, int* cols)
{
    if(!openDatabase()) return;

    char *zErrMsg = 0;

    int rc = sqlite3_get_table(db, query.c_str(), table, rows, cols, &zErrMsg);

    if( rc!=SQLITE_OK )
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }

    closeDatabase();
}

void CDatabaseInterface::insert(string& tableName, vector<string>& columns, string& values, string conflict)
{
    string query("INSERT INTO ");
    query += tableName;
    query += "(" + columns[0];
    for(int index = 1; index < columns.size(); index++)
    {
        query += ", " + columns[index];
    }
    query += ") ";
    query += "VALUES " + values;

    execute(query);
}

void CDatabaseInterface::upsert(string& tableName, vector<string>& columns, string& values, string conflict)
{
    string query("INSERT INTO ");
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

    execute(query);
}

void CDatabaseInterface::freeTable(char** table)
{
    sqlite3_free_table(table);
}

void CDatabaseInterface::update(QJsonDocument& jsonDoc)
{
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

		insert(name, columnNames, values, conflict);
	}
}

std::string CDatabaseInterface::getSingleValue(string& query)
{
    char **table;
    int rows, cols;

    getTable(query, &table, &rows, &cols);

    std::string value = "";

    if(rows > 1)
    {
        value = table[1];
    }

    freeTable(table);

    return value;
}

#else
CDatabaseInterface::CDatabaseInterface() {}
void CDatabaseInterface::setDBPath(string path) {}
bool CDatabaseInterface::openDatabase() {}
void CDatabaseInterface::closeDatabase() {}
void CDatabaseInterface::initDatabase(string schema) {}
void CDatabaseInterface::execute(string& query, int (*callback)(void*,int,char**,char**), void* arg) {}
void CDatabaseInterface::getTable(string& query, char ***table, int* rows, int* cols) {}
void CDatabaseInterface::insert(string& tableName, vector<string>& columns, string& values, string conflict = "") {}
void CDatabaseInterface::upsert(string& tableName, vector<string>& columns, string& values, string conflict = "") {}
void CDatabaseInterface::freeTable(char** table) {}
#endif