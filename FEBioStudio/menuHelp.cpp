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
#include "MainWindow.h"
#include "ui_mainwindow.h"
#include "version.h"
#include <QDesktopServices>
#include <QMessageBox>

void CMainWindow::on_actionFEBioURL_triggered()
{
	QDesktopServices::openUrl(QUrl("https://febio.org/"));
}

void CMainWindow::on_actionFEBioForum_triggered()
{
	QDesktopServices::openUrl(QUrl("https://forums.febio.org/"));
}

void CMainWindow::on_actionFEBioResources_triggered()
{
	QDesktopServices::openUrl(QUrl("https://febio.org/knowledgebase/"));
}

void CMainWindow::on_actionFEBioPubs_triggered()
{
	QDesktopServices::openUrl(QUrl("https://febio.org/publications/"));
}

void CMainWindow::on_actionAbout_triggered()
{
	QString txt = QString(\
		"<h1>FEBio Studio</h1>\
		<p><b>Version %1.%2.%3</b></p>\
		<p>Weiss Lab, University of Utah</p>\
		<p>Ateshian Lab, Columbia University</p>\
		<p>Copyright (c) 2019 - 2020, All rights reserved</p>\
		<hr>\
		<p>When using FEBio or FEBioStudio in your publications, please cite:</p>\
		<p><b>Maas SA, Ellis BJ, Ateshian GA, Weiss JA: FEBio: Finite Elements for Biomechanics. Journal of Biomechanical Engineering, 134(1):011005, 2012</b></p>"\
	).arg(VERSION).arg(SUBVERSION).arg(SUBSUBVERSION);

	QMessageBox about(this);
	about.setWindowTitle("About FEBio Studio");
	about.setText(txt);
	about.setIconPixmap(QPixmap(":/icons/FEBioStudio_large.png"));
	about.exec();
}

void CMainWindow::on_actionWelcome_triggered()
{
	ShowWelcomePage();
}
