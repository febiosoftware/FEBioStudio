#include <vector>
#include <QString>
#include <QStringList>

class CRepoProject
{
public:
	CRepoProject(int columns, char** data);
	~CRepoProject(){}

	int id;
	int version;
	QString name;
	QString description;
	QString owner;
	QStringList files;
	std::vector<int> fileIDs;
	int fileID;
};
