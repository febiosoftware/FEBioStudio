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

#include "stdafx.h"
#include "ZipFiles.h"
#ifdef HAS_LIBZIP

#include <zip.h>
#include <fstream>


void recurseAddDir(QDir d, QStringList & list) 
{
	QStringList qsl = d.entryList(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files);

	for (QString file : qsl) {

		if(file.startsWith(".")) continue;

		QFileInfo finfo(QString("%1/%2").arg(d.path()).arg(file));

		if (finfo.isDir()) {

			QString dirname = finfo.fileName();
			QDir sd(finfo.filePath());

			recurseAddDir(sd, list);
		}
		else
			list << QDir::toNativeSeparators(finfo.filePath());
	}
}

bool archive(const QString & filePath, const QDir & dir) 
{
	QStringList sl;
	recurseAddDir(dir, sl);

    int errorp;
    zip_t* zipper = zip_open(filePath.toStdString().c_str(), ZIP_CREATE | ZIP_TRUNCATE, &errorp);
    if (!zipper) {
        return false;
    }

	if (!dir.exists()) {
		return false;
	}

	QFileInfoList files;
	foreach(QString fn, sl)	files << QFileInfo(fn);

	foreach(QFileInfo fileInfo, files) {

		QString fileNameWithRelativePath = fileInfo.filePath().remove(0, dir.absolutePath().length() + 1);

		QString realPath;

		if (fileInfo.isSymLink())
		{
			realPath = fileInfo.symLinkTarget();
		}
		else
		{
			realPath = fileInfo.filePath();
		}

        zip_source_t* source = zip_source_file(zipper, realPath.toStdString().c_str(), 0, 0);
        if(!source) return false;

        if(zip_file_add(zipper, fileNameWithRelativePath.toStdString().c_str(), source, ZIP_FL_ENC_UTF_8) < 0)
        {
            zip_source_free(source);
            return false;
        }
	}

    zip_close(zipper);

	return true;
}

QStringList extractAllFiles(const QString& archiveName, const QString& dir)
{
    QStringList fileList;

    zip_t* archive = zip_open(archiveName.toStdString().c_str(), 0, nullptr);
    if (archive == nullptr) {
        return fileList;
    }

    zip_int64_t count = zip_get_num_entries(archive, 0);
    zip_stat_t stat;
    for (zip_int64_t index = 0; index < count; ++index) {
        
        zip_stat_index(archive, index, 0, &stat);
        zip_file_t* file = zip_fopen_index(archive, index, 0);
        
        if (file != nullptr) {
            std::string filePath = dir.toStdString() + "/" + stat.name;

            fileList.append(filePath.c_str());

            // Create path to write to
            QDir parent = QFileInfo(filePath.c_str()).dir();
            parent.mkpath(parent.path());

            std::ofstream output(filePath, std::ios::binary);
            char buffer[4096];
            zip_int64_t bytesRead;

            while ((bytesRead = zip_fread(file, buffer, sizeof(buffer))) > 0) {
                output.write(buffer, static_cast<std::streamsize>(bytesRead));
            }

            zip_fclose(file);
            output.close();
        }
    }

    zip_close(archive);

    return fileList;
}

ZipThread::ZipThread(const QString & zipFile, const QStringList & filePaths, const QStringList & zippedFilePaths)
	: zipFile(zipFile), filePaths(filePaths), zippedFilePaths(zippedFilePaths), aborted(false)
{

}

void ZipThread::run()
{
    int errorp;
    zip_t* zipper = zip_open(zipFile.toStdString().c_str(), ZIP_CREATE | ZIP_TRUNCATE, &errorp);
    if (!zipper) {
        failed();
        return;
    }

    QFileInfoList files;
	foreach(QString fn, filePaths)	files << QFileInfo(fn);
	for(int index = 0; index < filePaths.size(); index++)
	{
        QFileInfo fileInfo(filePaths.at(index));

        QString fileNameWithRelativePath = zippedFilePaths.at(index);

		QString realPath;

		if (fileInfo.isSymLink())
		{
			realPath = fileInfo.symLinkTarget();
		}
		else
		{
			realPath = fileInfo.filePath();
		}

        zip_source_t* source = zip_source_file(zipper, realPath.toStdString().c_str(), 0, 0);
        if(!source)
        {
            zip_close(zipper);
            failed();
			return;
        }

        if(zip_file_add(zipper, fileNameWithRelativePath.toStdString().c_str(), source, ZIP_FL_ENC_UTF_8) < 0)
        {
            zip_source_free(source);
            failed();
			return;
        }
	}

    // Gets called periodically to set progress
    auto progressCallback = [](zip_t* zipper, double val, void* zipThread) { ((ZipThread*)zipThread)->progress(val*100, 100); };
    zip_register_progress_callback_with_state(zipper, 0.01, progressCallback, nullptr, this);

    // Cancels the run when callback returns non-zero value
    auto cancelCallback = [](zip_t* zipper, void* zipThread) -> int { return ((ZipThread*)zipThread)->aborted; };
    zip_register_cancel_callback_with_state(zipper, cancelCallback, nullptr, this);

    zip_close(zipper);

    if(aborted)
    {
        failed();
        return;
    }

	emit resultReady(true, "");
}

void ZipThread::abort()
{
	aborted = true;
}

void ZipThread::failed()
{
	QFile zip(zipFile);

	if(zip.exists())
	{
		zip.remove();
	}

    // If we manually aborted, don't show an error
    if(aborted)
    {
        emit resultReady(false, "");
    }
    else
    {
        emit resultReady(false, "Failed to zip project files.");
    }
}



#else
void ZipThread::run() {}
void ZipThread::abort() {}
bool archive(const QString & filePath, const QDir & dir) { return false; }
QStringList extractAllFiles(QString fileName, QString destDir) { return QStringList(); }
#endif
