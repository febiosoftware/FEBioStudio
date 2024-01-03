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
#include <QMessageBox>
#include "FEBioMonitorDoc.h"
#include "../FEBioStudio/MainWindow.h"
#include "../FEBioStudio/GLView.h"
#include "GLMonitorScene.h"
#include <FECore/FEModel.h>


// NOTE: make sure this is included last, 
// otherwise we may get compilation errors related to Qt
#include <FEBioLib/FEBioModel.h>
#include <FECore/log.h>

class FEBLogStream : public LogStream
{
public:
	FEBLogStream(FEBioMonitorDoc* doc) : m_doc(doc) {}

	void print(const char* sz) override
	{
		m_doc->appendLog(sz);
	}

private:
	FEBioMonitorDoc* m_doc;
};

FEBioMonitorThread::FEBioMonitorThread(FEBioMonitorDoc* doc) : m_doc(doc)
{
	connect(this, &QThread::finished, this, &QObject::deleteLater);
	connect(this, &FEBioMonitorThread::jobFinished, doc, &FEBioMonitorDoc::onJobFinished);
}

bool cb_processEvent(FEModel* fem, unsigned int nwhen, void* pd)
{
	FEBioMonitorDoc* doc = (FEBioMonitorDoc*)pd;
	return doc->processFEBioEvent(fem, nwhen);
}

void FEBioMonitorThread::run()
{
	std::string filename = m_doc->GetFEBioInputFile().toStdString();
	if (filename.empty()) {
		emit jobFinished(false);
		return;
	}

	FEBioModel fem;
	fem.GetLogFile().SetLogStream(new FEBLogStream(m_doc));

	fem.AddCallback(cb_processEvent, CB_ALWAYS, (void*)m_doc);

	if (fem.Input(filename.c_str()) == false)
	{
		emit jobFinished(false);
		return;
	}

	bool b = fem.Init();
	if (b == false)
	{
		emit jobFinished(false);
		return;
	}

	b = fem.Solve();
	emit jobFinished(b);
}

FEBioMonitorDoc::FEBioMonitorDoc(CMainWindow* wnd) : CGLDocument(wnd)
{
	SetDocTitle("[FEBio Monitor]");
	m_isOutputReady = false;
	m_scene = new CGLMonitorScene(this);

	m_bValid = false;
	m_time = 0.0;

	CGLView* view = wnd->GetGLView();

	connect(this, &FEBioMonitorDoc::outputReady, this, &FEBioMonitorDoc::readOutput);
	connect(this, &FEBioMonitorDoc::updateView, view, &CGLView::updateView);
}

FEBioMonitorDoc::~FEBioMonitorDoc()
{

}

void FEBioMonitorDoc::SetFEBioInputFile(QString febFile)
{
	assert(m_febFile.isEmpty());
	m_febFile = febFile;
}

QString FEBioMonitorDoc::GetFEBioInputFile() const
{
	return m_febFile;
}

void FEBioMonitorDoc::RunJob()
{
	m_isOutputReady = false;
	FEBioMonitorThread* thread = new FEBioMonitorThread(this);
	thread->start();
}

void FEBioMonitorDoc::onJobFinished(bool b)
{
	if (b)
		QMessageBox::information(m_wnd, "FEBio Studio", "NORMAL TERMINATION");
	else
		QMessageBox::critical(m_wnd, "FEBio Studio", "ERROR TERMINATION");
}

void FEBioMonitorDoc::appendLog(const char* sz)
{
	if ((sz == nullptr) || (sz[0] == 0)) return;

	bool doEmit = false;

	m_mutex.lock();
	if (m_outputBuffer.isEmpty()) m_outputBuffer = sz;
	else m_outputBuffer.append(sz);
	if (m_isOutputReady == false)
	{
		m_isOutputReady = true;
		doEmit = true;
	}
	m_mutex.unlock();

	if (doEmit)
	{
		emit outputReady();
	}
}

void FEBioMonitorDoc::readOutput()
{
	QString s;
	m_mutex.lock();
	s = m_outputBuffer;
	m_outputBuffer.clear();
	m_isOutputReady = false;
	m_mutex.unlock();
	
	m_wnd->AddOutputEntry(s);
}

bool FEBioMonitorDoc::processFEBioEvent(FEModel* fem, int nevent)
{
	CGLMonitorScene* scene = dynamic_cast<CGLMonitorScene*>(m_scene);
	switch (nevent)
	{
	case CB_INIT:
		scene->InitScene(fem);
		m_bValid = true;
		break;
	case CB_MAJOR_ITERS:
	case CB_MINOR_ITERS:
		m_time = fem->GetTime().currentTime;
		scene->UpdateScene();
		break;
	}
	emit updateView();
	return true;
}

double FEBioMonitorDoc::GetTimeValue() const
{
	return m_time;
}
