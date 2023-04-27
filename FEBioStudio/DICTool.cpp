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

#include "DICTool.h"
#include "MainWindow.h"
#include "ModelDocument.h"
#include "DlgStartThread.h"
#include "WzdDIC.h"

#include <ImageLib/DICAnalysis.h>

class ImageAnalysisThread;

class ImageAnalysisLogger : public TaskLogger
{
public:
	ImageAnalysisLogger(ImageAnalysisThread* thread) : m_thread(thread) {}

	void Log(const std::string& msg) override;

private:
	ImageAnalysisThread* m_thread;
};

class ImageAnalysisThread : public CustomThread
{

public:
	ImageAnalysisThread(CImageAnalysis* analysis) : m_analysis(analysis), m_logger(this) {}

	void run() Q_DECL_OVERRIDE
	{
		if (m_analysis)
		{
			m_analysis->SetTaskLogger(&m_logger);
			m_analysis->run();
		}
		emit resultReady(true);
	}

	void WriteLog(QString msg)
	{
		emit writeLog(msg);
	}

public:
	bool hasProgress() override { return (m_analysis != nullptr); }

	double progress() override { return (m_analysis ? m_analysis->GetProgress().percent : 0.0); }

	const char* currentTask() override { return (m_analysis ? m_analysis->GetProgress().task : ""); }

	void stop() override { if (m_analysis) m_analysis->Terminate(); }

private:
	CImageAnalysis* m_analysis = nullptr;
	ImageAnalysisLogger m_logger;
};

CDICTool::CDICTool(CMainWindow* wnd) : CBasicTool(wnd, "DIC", HAS_APPLY_BUTTON), m_wnd(wnd)
{
    SetApplyButtonText("Run");
}


bool CDICTool::OnApply()
{
    CWzdDIC wzd(m_wnd);

    if(wzd.exec())
    {
        CDICAnalysis analysis(wzd.GetRefImage(), wzd.GetDefImage(), wzd.GetSubSize(), wzd.GetSubSpacing(), wzd.GetFilename());

        CDlgStartThread dlg(m_wnd, new ImageAnalysisThread(&analysis));

        if(!dlg.exec())
        {
            return false;
        }
    }

    return true;

}