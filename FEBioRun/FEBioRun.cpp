/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2026 University of Utah, The Trustees of Columbia University in
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
#include "FEBioRun.h"
#include <FEBioLib/FEBioModel.h>
#include <FEBioLib/febio.h>
#include <FECore/FEMaterial.h>
#include "../FEBioStudio/FEBioJob.h"
#include "../FEMLib/FEMaterial.h"
#include "../FEBioLink/FEBioClass.h"

using namespace FEBio;

class FBSLogStream : public LogStream
{
public:
	FBSLogStream(FEBioOutputHandler* outputHandler) : m_outputHandler(outputHandler) {}

	void print(const char* sz) override
	{
		if (m_outputHandler) m_outputHandler->write(sz);
	}

private:
	FEBioOutputHandler* m_outputHandler;
};

static bool terminateRun = false;

static bool interrup_cb(FEModel* fem, unsigned int nwhen, void* pd)
{
	if (terminateRun)
	{
		terminateRun = false;
		throw std::exception();
	}
	return true;
}


static bool progress_cb(FEModel* pfem, unsigned int nwhen, void* pd)
{
	FEBioModel& fem = static_cast<FEBioModel&>(*pfem);

	FEBioProgressTracker* progressTracker = (FEBioProgressTracker*)pd;
	if (pd == nullptr) return true;

	// get the number of steps
	int nsteps = fem.Steps();

	// calculate progress
	double starttime = fem.GetStartTime();
	double endtime = fem.GetEndTime();
	double f = 0.0;
	if (nwhen != CB_INIT)
	{
		double ftime = fem.GetCurrentTime();
		if (endtime != starttime) f = (ftime - starttime) / (endtime - starttime);
		else
		{
			// this only happens (I think) when the model is solved
			f = 1.0;
		}
	}

	double pct = 0.0;
	if (nsteps > 1)
	{
		int N = nsteps;
		int n = fem.GetCurrentStepIndex();
		pct = 100.0 * ((double)n + f) / (double)N;
	}
	else
	{
		pct = 100.0 * f;
	}

	progressTracker->SetProgress(pct);

	return true;
}

int FEBio::runModel(const std::string& cmd, FEBioOutputHandler* outputHandler, FEBioProgressTracker* progressTracker, CFEBioJob* job)
{
	terminateRun = false;

	FEBioModel fem;
	fem.CreateReport(true);

	// attach the output handler
	if (outputHandler)
	{
		fem.GetLogFile().SetLogStream(new FBSLogStream(outputHandler));
	}

	// attach a callback to interrupt and measure progress
	fem.AddCallback(interrup_cb, CB_ALWAYS, nullptr);

	if (progressTracker)
		fem.AddCallback(progress_cb, CB_MAJOR_ITERS, progressTracker);

	try {
		febio::CMDOPTIONS ops;
		if (febio::ProcessOptionsString(cmd, ops) == false)
		{
			if (outputHandler)
			{
				outputHandler->write("Failed processing command line.");
			}
			return false;
		}

		int returnCode = febio::RunModel(fem, &ops);
		if (job)
		{
			job->m_jobReport = fem.GetReport();
			job->m_timingInfo = fem.GetTimingInfo();
			job->m_modelStats = fem.GetModelStats();
			job->m_stepStats = fem.GetStepStats();
			job->m_timestepStats = fem.GetTimeStepStats();
		}
		return returnCode;
	}
	catch (...)
	{

	}

	return 1;
}

void FEBio::TerminateRun()
{
	terminateRun = true;
}

bool FEBio::RunMaterialTest(MaterialTest test, std::vector<std::pair<double, double> >& out)
{
	out.clear();

	FEModel fem;

	// Create an FEBio material from the FSMaterial
	FEMaterial* febmat = dynamic_cast<FEMaterial*>(FEBio::CreateFECoreClassFromModelComponent(test.mat, &fem));

	// run the test
	bool b = febio::RunMaterialTest(febmat, test.time, test.steps, test.strain, test.testName.c_str(), out);

	delete febmat;

	return b;
}
