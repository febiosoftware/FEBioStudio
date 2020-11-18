/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2020 University of Utah, The Trustees of Columbia University in 
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

#include <string>
#include <string.h>
#include <vector>
#include <unordered_set>
#include <map>
#include <QList>
#include <QtGlobal>

class QJsonDocument;
class QString;
class QVariant;
class QStringList;
class CRepoProject;
class CRepositoryPanel;

class CLocalDatabaseHandler
{
	class Imp;

public:
	CLocalDatabaseHandler(CRepositoryPanel* dbPanel);
	~CLocalDatabaseHandler();

	void init(std::string schema);

	void update(QJsonDocument& jsonDoc);

	void GetCategories();
	void GetProjects();
	QStringList GetTags();
	void GetProjectFiles(int ID);

	void GetProjectData(int ID);
	void GetProjectTags(int ID);
	void GetProjectPubs(int ID);
	void GetFileData(int ID);
	void GetFileTags(int ID);

	void GetCategoryMap(std::map<int, std::string>& categoryMap);

	QList<QList<QVariant>> GetProjectFileInfo(int projID);


	std::unordered_set<int> FullTextSearch(QString term);
	std::unordered_set<int> FileSearch(QString term);

	QString ProjectNameFromID(int ID);
	QString FilePathFromID(int ID, int type);
	QString FileNameFromID(int ID, int type);
	QString FullFileNameFromID(int ID, int type);
	QString CategoryFromID(int ID);
	int ProjectIDFromFileID(int ID);
	int CategoryIDFromName(std::string name);

	bool isValidUpload(QString& projectName, QString& category);
	qint64 currentProjectsSize(QString username);
	qint64 projectsSize(int ID);


private:
	Imp* imp;
};
