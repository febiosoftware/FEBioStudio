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
#include "MainWindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <PostLib/ImgAnimation.h>
#include <PostLib/AVIAnimation.h>
#include <PostLib/MPEGAnimation.h>
#include <PostLib/GIFAnimation.h>

void CMainWindow::on_actionRecordNew_triggered()
{
	QStringList filters;
#ifdef WIN32
	int noff = 1;
	filters << "Windows AVI files (*.avi)";
#else
	int noff = 0;
#endif

	filters << "MPG files (*.mpg)"
			<< "GIF files (*.gif)"
			<< "PNG files (*.png)"
			<< "Bitmap files (*.bmp)"
			<< "JPG files (*.jpg)";


	QFileDialog dlg(this, "Save");
	dlg.setNameFilters(filters);
	dlg.setFileMode(QFileDialog::AnyFile);
	dlg.setAcceptMode(QFileDialog::AcceptSave);
	if (dlg.exec())
	{
		QString fileName = dlg.selectedFiles().first();
		string sfile = fileName.toStdString();
		char szfilename[512] = { 0 };
		snprintf(szfilename, sizeof szfilename, "%s", sfile.c_str());
		int l = (int)sfile.length();
		char* ch = strrchr(szfilename, '.');

		int nfilter = filters.indexOf(dlg.selectedNameFilter());

		CGLView* glview = GetGLView();

		bool bret = false;
		CAnimation* panim = 0;
#ifdef WIN32
		if (nfilter == 0)
		{
			panim = new CAVIAnimation;
			if (ch == 0) sprintf(szfilename + l, ".avi");
			bret = glview->NewAnimation(szfilename, panim, GL_BGR_EXT);
		}
		else if (nfilter == noff)
#else
		if (nfilter == noff)
#endif
		{
#ifdef FFMPEG
			panim = new CMPEGAnimation;
			if (ch == 0) sprintf(szfilename + l, ".mpg");
			bret = glview->NewAnimation(szfilename, panim);
#else
			QMessageBox::critical(this, "FEBio Studio", "This video format is not supported in this version");
#endif
		}
		else if (nfilter == noff + 1)
		{
			panim = new CGIFAnimation;
			if (ch == 0) sprintf(szfilename + l, ".gif");
			bret = glview->NewAnimation(szfilename, panim);
		}
		else if (nfilter == noff + 2)
		{
			panim = new CPNGAnimation;
			if (ch == 0) sprintf(szfilename + l, ".png");
			bret = glview->NewAnimation(szfilename, panim);

		}
		else if (nfilter == noff + 3)
		{
			panim = new CBmpAnimation;
			if (ch == 0) sprintf(szfilename + l, ".bmp");
			bret = glview->NewAnimation(szfilename, panim);
		}
		else if (nfilter == noff + 4)
		{
			panim = new CJpgAnimation;
			if (ch == 0) sprintf(szfilename + l, ".jpg");
			bret = glview->NewAnimation(szfilename, panim);
		}

		if (bret)
		{
			UpdateTitle();
		}
		else bret = QMessageBox::critical(this, "FEBio Studio", "Failed creating animation stream.");

		RedrawGL();
	}
}

void CMainWindow::on_actionRecordStart_triggered()
{
	if (GetGLView()->HasRecording())
	{
		GetGLView()->StartAnimation();
		UpdateTitle();
	}
	else QMessageBox::information(this, "FEBio Studio", "You need to create a new video file before you can start recording");
}

void CMainWindow::on_actionRecordPause_triggered()
{
	if (GetGLView()->HasRecording())
	{
		if (GetGLView()->RecordingMode() == VIDEO_RECORDING)
		{
			GetGLView()->PauseAnimation();
			UpdateTitle();
		}
	}
	else QMessageBox::information(this, "FEBio Studio", "You need to create a new video file first.");
}

void CMainWindow::on_actionRecordStop_triggered()
{
	if (GetGLView()->HasRecording())
	{
		if (GetGLView()->RecordingMode() != VIDEO_STOPPED)
		{
			GetGLView()->StopAnimation();
			UpdateTitle();
		}
	}
	else QMessageBox::information(this, "FEBio Studio", "You need to create a new video file first.");
}
