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
#include <QLabel>
#include <QProgressBar>
#include <QBoxLayout>
#include <QDialogButtonBox>
#include <QCloseEvent>
#include "ImageThread.h"
#include <ImageLib/ImageModel.h>
#include <ImageLib/ImageFilter.h>
#include <ImageLib/ImageSource.h>

//--------------------------------------------------------------------
CImageReadThread::CImageReadThread(CImageModel* imgModel) : m_imgModel(imgModel)
{

}
    
void CImageReadThread::run()
{
	emit taskChanged("Reading Image Data...");

	bool success = true;
	try
	{
		CImageSource* src = m_imgModel->GetImageSource();
		if (src)
		{
			success = src->Load();
			if (success == false) SetErrorString(QString::fromStdString(src->GetErrorString()));
		}
		else 
		{
			success = false;
			SetErrorString("Failed importing image data.");
		}
	}
	catch(std::runtime_error& e)
	{
		success = false;
		SetErrorString(e.what());
	}

	emit resultReady(success);
}

bool CImageReadThread::hasProgress()
{
	if (m_imgModel == nullptr) return false;
	CImageSource* src = m_imgModel->GetImageSource();
	if (src) return (src->GetProgress().valid);
	else return false;
}

double CImageReadThread::progress()
{
	if (m_imgModel == nullptr) return 0;
	CImageSource* src = m_imgModel->GetImageSource();
	if (src) return src->GetProgress().percent;
	return 0.0;
}

const char* CImageReadThread::currentTask()
{
	if (m_imgModel == nullptr) return "";
	CImageSource* src = m_imgModel->GetImageSource();
	if (src) return src->GetProgress().task;
	return "";
}

void CImageReadThread::stop()
{
	if (m_imgModel == nullptr) return;
	CImageSource* src = m_imgModel->GetImageSource();
	if (src) src->Terminate();
}

//--------------------------------------------------------------------

CImageFilterThread::CImageFilterThread(CImageModel* imgModel) : m_imgModel(imgModel)
{
	m_canceled = false;
}
    
void CImageFilterThread::run()
{
    bool success = true;

    try
    {
        for(int index = 0; index < m_imgModel->ImageFilters(); index++)
        {
            emit taskChanged(QString("Applying %1...").arg(m_imgModel->GetImageFilter(index)->GetName().c_str()));

            if(m_canceled)
            {
                break;
            }

            m_imgModel->GetImageFilter(index)->ApplyFilter();
        }

        if(m_canceled)
        {
            success = false;
        }
    }
    catch(std::exception& e)
    {
        success = false;
        SetErrorString(e.what());
    }

	emit resultReady(success);
}

void CImageFilterThread::stop()
{
    m_canceled = true;
}
