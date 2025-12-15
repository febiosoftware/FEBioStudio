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
#include "TimelinePanel.h"
#include "MainWindow.h"
#include "PostDocument.h"
#include "TimelineWidget.h"
#include <PostLib/FEPostModel.h>
#include <QBoxLayout>
using namespace Post;

class Ui::CTimelinePanel
{
public:
	CTimelineWidget*	timeline;

public:
	void setupUi(QWidget* parent)
	{
		QHBoxLayout* mainLayout = new QHBoxLayout;
		mainLayout->setContentsMargins(0,0,0,0);
		mainLayout->setSpacing(0);
		mainLayout->addWidget(timeline = new CTimelineWidget); timeline->setObjectName("timeline");
		parent->setLayout(mainLayout);

		QMetaObject::connectSlotsByName(parent);
	}
};


CTimelinePanel::CTimelinePanel(CMainWindow* wnd, QWidget* parent) : QWidget(parent), m_wnd(wnd), ui(new Ui::CTimelinePanel)
{
	ui->setupUi(this);
}

void CTimelinePanel::Update(bool reset)
{
	CPostDocument* doc = m_wnd->GetPostDocument();
	if (doc && doc->IsValid())
	{
		FEPostModel* fem = doc->GetFSModel();
		if (fem)
		{
			if (reset)
			{
				ui->timeline->clearData();
				FEPostModel& fem = *doc->GetFSModel();
				std::vector< std::pair<double, int> > data(fem.GetStates());
				int nstates = fem.GetStates();
				for (int i = 0; i < nstates; ++i)
				{
					data[i].first = fem.GetState(i)->m_time;
					data[i].second = fem.GetState(i)->m_status;
				}

				ui->timeline->setTimePoints(data);

				TIMESETTINGS& time = doc->GetTimeSettings();
				ui->timeline->setRange(time.m_start, time.m_end);
			}

			int ntime = doc->GetActiveState();
			ui->timeline->setSelection(ntime);

			double ftime = doc->GetCurrentTimeValue();
			ui->timeline->setCurrentTime(ftime);
		}
		else ui->timeline->clearData();
	}
	else ui->timeline->clearData();
}

void CTimelinePanel::SetRange(int nmin, int nmax)
{
	ui->timeline->setRange(nmin, nmax);
}

void CTimelinePanel::on_timeline_pointClicked(int i)
{
	m_wnd->SetCurrentTime(i);
}

void CTimelinePanel::on_timeline_rangeChanged(int nmin, int nmax)
{
	CPostDocument* doc = m_wnd->GetPostDocument();
	if (doc->IsValid())
	{
		TIMESETTINGS& time = doc->GetTimeSettings();
		time.m_start = nmin;
		time.m_end = nmax;

		int ntime = doc->GetActiveState();

		if ((ntime < nmin) || (ntime > nmax))
		{
			if (ntime < nmin) ntime = nmin;
			if (ntime > nmax) ntime = nmax;
		}
		m_wnd->SetCurrentTime(ntime);
	}
}
