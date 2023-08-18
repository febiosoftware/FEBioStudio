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

#pragma once

#include <vector>
#include <QString>
#include <QColor>
#include <QPoint>

class CImageSlice;
class CDlgPixelInspector;

class CPixelInfoSource 
{

public:
    CPixelInfoSource();

    virtual void SetInspector(CDlgPixelInspector* inspector);

    QPoint GetStartIndices();
    std::vector<QString>& GetPixelVals();
    std::vector<QColor>& GetPixelColors();

    virtual void UpdatePixelInfo() = 0;

protected:
    CDlgPixelInspector* m_inspector;

    std::vector<QString> m_pixelVals;
    std::vector<QColor> m_pixelColors;

    QPoint m_startIndices;
    QPoint m_infoPoint;
};

class CSliceInfoSource : public CPixelInfoSource
{
public:
    CSliceInfoSource() {}

    void UpdatePixelInfo() override;

    virtual void SetInspector(CDlgPixelInspector* inspector) override;

protected:
    CImageSlice* m_infoSlice;
};