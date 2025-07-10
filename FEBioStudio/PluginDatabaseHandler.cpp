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

#include "PluginDatabaseHandler.h"
#include "DatabaseInterface.h"
#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>
#include <QString>
#include "DatabaseInterface.h"
#include "PluginManager.h"
#include <FECore/version.h>

#ifdef WIN32
    #define OS_ID "1"
#elif defined(__APPLE__)
    #define OS_ID "2"
#else
    #define OS_ID "3"
#endif

static int addPluginCallback(void *manager, int argc, char **argv, char **azColName)
{
	((CPluginManager*) manager)->AddRepoPlugin(argv);

	return 0;
}

CPluginDatabaseHandler::CPluginDatabaseHandler(CPluginManager* manager)
    : manager(manager)
{

}

CPluginDatabaseHandler::~CPluginDatabaseHandler() {}

void CPluginDatabaseHandler::init(std::string schema)
{
    // Grab a system path for the database
    QString dbPath = (QStandardPaths::writableLocation(
        QStandardPaths::AppLocalDataLocation) + "/plugins/");

    // Make the directory if it doesn't exist
    QDir dir;
    dir.mkpath(dbPath);

    interface.setDBPath((dbPath + "plugins.db").toStdString());

	interface.initDatabase(schema);
}

void CPluginDatabaseHandler::update(QJsonDocument& jsonDoc)
{
    interface.update(jsonDoc);
}

void CPluginDatabaseHandler::GetPlugins()
{
    std::string query("SELECT plugins.ID, plugins.name, users.username, plugins.description, plugins.source, plugins.image, "
        "COALESCE(SUM(downloads.downloads), 0) AS total_downloads FROM plugins JOIN users ON plugins.owner "
        "= users.ID LEFT JOIN downloads ON plugins.ID = downloads.plugin GROUP BY plugins.ID, plugins.name, "
        "users.username, plugins.description, plugins.image");

	interface.execute(query, addPluginCallback, manager);
}

void CPluginDatabaseHandler::GetPluginPubs(int ID)
{
	char **table;
	int rows, cols;

	QVariantMap data;

	// Get all publications for this project
	QString query = QString("SELECT publications.ID, title, year, journal, volume, issue, pages, DOI FROM publications JOIN pluginPubs ON publications.ID = pluginPubs.publication WHERE pluginPubs.plugin = %1").arg(ID);
	std::string queryStd = query.toStdString();

	interface.getTable(queryStd, &table, &rows, &cols);

	// Extract information about each project
	for(int row = 1; row <= rows; row++)
	{
		int rowStart = row*cols;

		data["title"] = QString(table[rowStart + 1]);
		data["year"] = QString(table[rowStart + 2]);
		data["journal"] = QString(table[rowStart + 3]);
		data["volume"] = QString(table[rowStart + 4]);
		data["issue"] = QString(table[rowStart + 5]);
		data["pages"] = QString(table[rowStart + 6]);
		data["DOI"] = QString(table[rowStart + 7]);

		// Get Authors for this publication
		char **table2;
		int rows2, cols2;

		QString query2 = QString("SELECT firstName, lastName FROM authors JOIN publicationAuthors ON publicationAuthors.author = authors.ID WHERE publicationAuthors.publication = %1 ORDER BY publicationAuthors.ordering").arg(table[rowStart]);
		std::string queryStd2 = query2.toStdString();

		interface.getTable(queryStd2, &table2, &rows2, &cols2);

		QStringList authorGiven;
		QStringList authorFamily;

		for(int row2 = 1; row2 <= rows2; row2++)
		{
			int rowStart2 = row2*cols2;

			authorGiven.push_back(table2[rowStart2]);
			authorFamily.push_back(table2[rowStart2 + 1]);

		}

		interface.freeTable(table2);

		data["authorGiven"] = authorGiven;
		data["authorFamily"] = authorFamily;

		manager->AddPublication(ID, data);
	}


	interface.freeTable(table);
}

void CPluginDatabaseHandler::GetPluginTags(int ID)
{
    char **table;
    int rows, cols;

    std::string query = "SELECT tags.tag FROM tags JOIN pluginTags ON tags.ID = pluginTags.tag JOIN plugins ON pluginTags.plugin = plugins.ID WHERE plugins.ID = " + std::to_string(ID);

    interface.getTable(query, &table, &rows, &cols);

    for(int row = 1; row <= rows; row++)
    {
        manager->AddTag(ID, table[row]);
    }

    interface.freeTable(table);
}


std::vector<std::string> CPluginDatabaseHandler::GetPluginVersions(int ID)
{
    std::vector<std::string> versions;

    std::string currentFEBioVersion = std::to_string(FE_SDK_MAJOR_VERSION) + "." 
        + std::to_string(FE_SDK_SUB_VERSION) + "." + std::to_string(FE_SDK_SUBSUB_VERSION);
    
    std::string query = "SELECT v1.version FROM pluginVersions "
        "JOIN plugins ON pluginVersions.plugin = plugins.ID "
        "JOIN versions v1 ON pluginVersions.version = v1.ID "
        "JOIN febioVersions v2 ON pluginVersions.febioVersion = v2.ID "
        "WHERE plugins.ID = " + std::to_string(ID) +
        " AND v2.version = '" + currentFEBioVersion + "'";

    char **table;
    int rows, cols;

    interface.getTable(query, &table, &rows, &cols);

    if(rows > 0)
    {
        for(int row = 1; row < rows + 1; row++)
        {
            versions.emplace_back(table[row]);
        }
    }

    interface.freeTable(table);

    return versions;
}

std::unordered_set<int> CPluginDatabaseHandler::GetPluginSearchResults(const QString& searchTerm)
{
	if(searchTerm.isEmpty()) return std::unordered_set<int>();

    QString query = QString("SELECT plugins.ID FROM plugins JOIN users ON plugins.owner=users.ID WHERE username LIKE '%%1%' "
            "OR name LIKE '%%1%' OR description LIKE '%%1%'").arg(searchTerm);

	char **table;
	int rows, cols;

	std::unordered_set<int> plugins;

    std::string queryStd = query.toStdString();

	interface.getTable(queryStd, &table, &rows, &cols);

	for(int row = 1; row <= rows; row++)
	{
		plugins.insert(std::stoi(table[row]));
	}

	interface.freeTable(table);

    // Matches plugin tags
    query = QString("SELECT pluginTags.plugin FROM tags JOIN pluginTags ON tags.ID = pluginTags.tag WHERE tags.tag LIKE '%%1%'").arg(searchTerm);
    queryStd = query.toStdString();

    interface.getTable(queryStd, &table, &rows, &cols);

    for(int row = 1; row <= rows; row++)
    {
        plugins.insert(std::stoi(table[row]));
    }

    interface.freeTable(table);

	return plugins;
}