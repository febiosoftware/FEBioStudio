/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2023 University of Utah, The Trustees of Columbia University in
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

#include "PixelInfoSource.h"
#include "DlgPixelInspector.h"

#include "ImageSlice.h"
#include <ImageLib/Image.h>

CPixelInfoSource::CPixelInfoSource() : m_inspector(nullptr)
{

}

void CPixelInfoSource::SetInspector(CDlgPixelInspector* inspector)
{
    m_inspector = inspector;

    if(!m_inspector)
    {
        m_startIndices = QPoint(0,0);
        m_pixelColors.clear();
        m_pixelVals.clear();
    }
    else
    {
        int size = m_inspector->GetRadius()*2+1;
        size *= size;
        
        m_pixelColors.resize(size);
        m_pixelVals.resize(size);
    }
}

QPoint CPixelInfoSource::GetStartIndices()
{
    return m_startIndices;
}

std::vector<QString>& CPixelInfoSource::GetPixelVals()
{
    return m_pixelVals;
}

std::vector<QColor>& CPixelInfoSource::GetPixelColors()
{
    return m_pixelColors;
}

///////////////////////////

void CSliceInfoSource::UpdatePixelInfo()
{
    CImage* original = m_infoSlice->GetOriginalSlice();
    CImage* display = m_infoSlice->GetDisplaySlice();

    int maxX = original->Width();
    int maxY = original->Height();

    int radius = m_inspector->GetRadius();

    int startX = m_infoPoint.x() - radius;
    int startY = (maxY - m_infoPoint.y()) + radius;
    m_startIndices = QPoint(startX, startY);

    int currentX = startX;
    int currentY = startY;
    for(int i = 0; i < m_pixelVals.size(); i++)
    {
        if(currentX < 0 || currentY < 0 || currentX >= maxX || currentY >= maxY)
        {
            m_pixelVals[i] = "";
            m_pixelColors[i] = QColorConstants::White;
        }
        else
        {
            m_pixelVals[i] = QString::number(original->Value(currentX, currentY));
            
            int colorVal = display->Value(currentX, currentY);
            m_pixelColors[i] = QColor(colorVal, colorVal, colorVal);
        }

        currentX++;
        if(currentX == startX + radius*2+1)
        {
            currentX = startX;
            currentY--;
        }
    }
}

void CSliceInfoSource::SetInspector(CDlgPixelInspector* inspector)
{
    if(!inspector)
    {
        m_infoSlice->DrawRect(false);
    }
    else
    {
        m_infoSlice->DrawRect(true);
    }

    CPixelInfoSource::SetInspector(inspector);
}