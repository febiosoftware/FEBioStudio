#include <QDir>
#include <QStringList>
void recurseAddDir(QDir d, QStringList & list);
bool archive(const QString & filePath, const QDir & dir, const QString & comment = QString(""));
QStringList extractAllFiles(QString fileName, QString destDir);
