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

#pragma once

#include <string>
#include <vector>

class sqlite3;

using std::string;
using std::vector;

class QJsonDocument;
class CDatabaseInterface
{
public:
    CDatabaseInterface();

	void setDBPath(string path);

    bool openDatabase();
    void closeDatabase();

	void initDatabase(string schema);

	void execute(string& query, int (*callback)(void*,int,char**,char**)=nullptr, void* arg = nullptr);
	void getTable(string& query, char ***table, int* rows, int* cols);
	void insert(string& tableName, vector<string>& columns, string& values, string conflict = "");
	void upsert(string& tableName, vector<string>& columns, string& values, string conflict = "");
    void freeTable(char** table);

    void update(QJsonDocument& jsonDoc);
    std::string getSingleValue(string& query);


public:
	sqlite3* db;
	string dbPath;
};
