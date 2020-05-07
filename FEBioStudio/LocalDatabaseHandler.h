#include <string>
#include <string.h>
#include <vector>
#include <unordered_set>

class QJsonDocument;
class QString;
class QStringList;
class CRepoProject;
class CDatabasePanel;

class CLocalDatabaseHandler
{
	class Imp;

public:
	CLocalDatabaseHandler(std::string dbPath, CDatabasePanel* dbPanel);
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


	std::unordered_set<int> FullTextSearch(QString term);

	QString FilePathFromID(int ID, int type);
	QString FileNameFromID(int ID, int type);
	QString FullFileNameFromID(int ID, int type);
	int ProjectIDFromFileID(int ID);


private:
	Imp* imp;


};
