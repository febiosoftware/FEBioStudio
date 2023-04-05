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

#include <ImageLib/ImageSITK.h>
#include <ImageLib/DICImage.h>
#include <ImageLib/DICMatching.h>
#include <ImageLib/DICQ4.h>

CDICTool::CDICTool(CMainWindow* wnd) : CBasicTool(wnd, "DIC", HAS_APPLY_BUTTON), m_wnd(wnd)
{
    m_img1 = 0;
    m_img2 = 1;
    addIntProperty(&m_img1, "Reference Image")->setFlags(CProperty::Visible | CProperty::Editable);
    addIntProperty(&m_img2, "Deformed Image")->setFlags(CProperty::Visible | CProperty::Editable);

    SetApplyButtonText("Run");
}

bool CDICTool::OnApply()
{
    CDICImage refImg(dynamic_cast<CImageSITK*>(m_wnd->GetModelDocument()->GetImageModel(m_img1)->Get3DImage()));
    CDICImage defImg(dynamic_cast<CImageSITK*>(m_wnd->GetModelDocument()->GetImageModel(m_img2)->Get3DImage()));

    CDICMatching match(refImg, defImg, 1);

    CDICQ4 interp(match);

    //for(int i = 0; i < 5; i++)
    //{
    //    interp.WriteVTK(i, 1);
    //}

    return true;
}