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
#include "AVIAnimation.h"
#include "MainWindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include "ImgAnimation.h"
#include "MPEGAnimation.h"
#include "GIFAnimation.h"
#include "PostDocument.h"

void CMainWindow::on_actionRecordNew_triggered()
{
	// Make sure a recording is not in progress
	CGLView* glview = GetGLView();
	GLScreenRecorder& recorder = glview->GetScreenRecorder();
	if (recorder.HasRecording())
	{
		QMessageBox::information(this, "FEBio Studio", "A recording is already in progres.");
		return;
	}

	QStringList filters;
	filters << "MPG files (*.mpg)"
			<< "GIF files (*.gif)"
			<< "PNG files (*.png)"
			<< "Bitmap files (*.bmp)"
			<< "JPG files (*.jpg)";
#ifdef WIN32
	filters << "Windows AVI files (*.avi)";
#endif

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
		char* fileExtenion = strrchr(szfilename, '.');

		int nfilter = filters.indexOf(dlg.selectedNameFilter());

		bool bret = false;
		CAnimation* panim = nullptr;
		switch (nfilter)
		{
		case 0:
		{
#ifdef FFMPEG
			panim = new CMPEGAnimation;
			if (fileExtenion == nullptr) sprintf(szfilename + l, ".mpg");
#else
			QMessageBox::critical(this, "FEBio Studio", "This video format is not supported in this version");
#endif
		}
		break;
		case 1:
		{
			panim = new CGIFAnimation;
			if (fileExtenion == nullptr) sprintf(szfilename + l, ".gif");
		}
		break;
		case 2:
		{
			panim = new CPNGAnimation;
			if (fileExtenion == nullptr) sprintf(szfilename + l, ".png");
		}
		break;
		case 3:
		{
			panim = new CBmpAnimation;
			if (fileExtenion == nullptr) sprintf(szfilename + l, ".bmp");
		}
		break;
		case 4:
		{
			panim = new CJpgAnimation;
			if (fileExtenion == nullptr) sprintf(szfilename + l, ".jpg");
		}
		break;
#ifdef WIN32
		case 5:
		{
			panim = new CAVIAnimation;
			if (fileExtenion == nullptr) sprintf(szfilename + l, ".avi");
		}
		break;
#endif	
		}

		// get the video dimensions
		QSize size = glview->GetSafeFrameSize();

		// get the frame rate
		float fps = 10.f;
		if (GetPostDocument()) fps = GetPostDocument()->GetTimeSettings().m_fps;
		if (fps == 0.f) fps = 10.f;

		// try to create the new animation
		if (panim && 
			panim->Create(szfilename, size.width(), size.height(), fps) &&
			recorder.SetVideoStream(panim))
		{
			UpdateTitle();
		}
		else
		{
			delete panim;
			QMessageBox::critical(this, "FEBio Studio", "Failed creating video stream.");
		}

		RedrawGL();
	}
}

void CMainWindow::on_actionRecordStart_triggered()
{
	if (!GetGLView()->StartRecording())
		QMessageBox::information(this, "FEBio Studio", "You need to create a new video file before you can start recording");
	UpdateTitle();
}

void CMainWindow::on_actionRecordPause_triggered()
{
	if (!GetGLView()->PauseRecording())
		QMessageBox::information(this, "FEBio Studio", "You need to create a new video file first.");
	UpdateTitle();
}

void CMainWindow::on_actionRecordStop_triggered()
{
	if (!GetGLView()->StopRecording())
		QMessageBox::information(this, "FEBio Studio", "You need to create a new video file first.");
	UpdateTitle();
}
