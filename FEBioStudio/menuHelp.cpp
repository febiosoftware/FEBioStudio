#include "stdafx.h"
#include "MainWindow.h"
#include "ui_mainwindow.h"
#include "version.h"
#include <QDesktopServices>
#include <QMessageBox>

void CMainWindow::on_actionOnlineHelp_triggered()
{
	QDesktopServices::openUrl(QUrl("https://febio.org/support/"));
}

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
	QDesktopServices::openUrl(QUrl("https://febio.org/teaching-hub/"));
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
		<p>Musculoskeletal Research Laboratories, University of Utah</p>\
		<p>Musculoskeletal Biomechanics Laboratory, Columbia University</p>\
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
