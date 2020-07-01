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

#include "stdafx.h"
#include "ZipFiles.h"
#ifdef HAS_QUAZIP
#include <JlCompress.h>
#include <quazip.h>
#include <quazipfile.h>

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

bool archive(const QString & filePath, const QDir & dir, const QString & comment) 
{
	QStringList sl;
	recurseAddDir(dir, sl);

	QuaZip zip(filePath);
	zip.setFileNameCodec("IBM866");

	if (!zip.open(QuaZip::mdCreate)) {
		return false;
	}

	if (!dir.exists()) {
		return false;
	}

	QFile inFile;

	QFileInfoList files;
	foreach(QString fn, sl)	files << QFileInfo(fn);

	QuaZipFile outFile(&zip);

	char c;
	foreach(QFileInfo fileInfo, files) {

		if (!fileInfo.isFile())
			continue;

		QString fileNameWithRelativePath = fileInfo.filePath().remove(0, dir.absolutePath().length() + 1);

		inFile.setFileName(fileInfo.filePath());

		if (!inFile.open(QIODevice::ReadOnly)) {
			return false;
		}

		QString realPath;

		if (fileInfo.isSymLink())
		{
			realPath = fileInfo.symLinkTarget();

			//			if (!outFile.open(QIODevice::WriteOnly, QuaZipNewInfo(fileNameWithRelativePath, fileInfo.symLinkTarget())), NULL, 0, 0, 0, true) {
			//				return false;
			//			}
		}
		else
		{
			realPath = fileInfo.filePath();

			//			if (!outFile.open(QIODevice::WriteOnly, QuaZipNewInfo(fileNameWithRelativePath, fileInfo.filePath())), NULL, 0, 0, 0, true) {
			//				return false;
			//			}
		}

		QuaZipNewInfo zipFileInfo(fileNameWithRelativePath, realPath);

		// For writing uncompressed files
		//		zipFileInfo.uncompressedSize = fileInfo.size();

		//		if (!outFile.open(QIODevice::WriteOnly, zipFileInfo, NULL, 0, 0, 0, true)) {
		//			return false;
		//		}

		if (!outFile.open(QIODevice::WriteOnly, zipFileInfo)) {
			return false;
		}

		while (inFile.getChar(&c) && outFile.putChar(c));


		if (outFile.getZipError() != UNZ_OK) {
			return false;
		}

		outFile.close();

		if (outFile.getZipError() != UNZ_OK) {
			return false;
		}

		inFile.close();
	}

	zip.close();

	if (zip.getZipError() != 0) {
		return false;
	}

	return true;
}

QStringList extractAllFiles(QString fileName, QString destDir)
{
	return JlCompress::extractFiles(fileName, JlCompress::getFileList(fileName), destDir);
}


bool archive(const QString & filePath, const QStringList & filePaths, const QStringList & localFilePaths)
{
	QuaZip zip(filePath);
	zip.setFileNameCodec("IBM866");

	if (!zip.open(QuaZip::mdCreate)) {
		return false;
	}

	QFile inFile;

	QuaZipFile outFile(&zip);

	char c;
	for(int index = 0; index < filePaths.size(); index++)
	{
		QFileInfo fileInfo(filePaths.at(index));


		if (!fileInfo.isFile())
					continue;

		QString fileNameWithRelativePath = localFilePaths.at(index);

		inFile.setFileName(fileInfo.filePath());

		if (!inFile.open(QIODevice::ReadOnly)) {
			return false;
		}

		QuaZipNewInfo zipFileInfo(fileNameWithRelativePath, filePaths.at(index));

		if (!outFile.open(QIODevice::WriteOnly, zipFileInfo)) {
			return false;
		}

		while (inFile.getChar(&c) && outFile.putChar(c));


		if (outFile.getZipError() != UNZ_OK) {
			return false;
		}

		outFile.close();

		if (outFile.getZipError() != UNZ_OK) {
			return false;
		}

		inFile.close();
	}

	zip.close();

	if (zip.getZipError() != 0) {
		return false;
	}

	return true;
}

#else
void recurseAddDir(QDir d, QStringList & list) {}
bool archive(const QString & filePath, const QDir & dir, const QString & comment) { return false; }
QStringList extractAllFiles(QString fileName, QString destDir) { return QStringList(); }
#endif
