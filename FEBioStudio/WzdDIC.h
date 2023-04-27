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

#include <QWizard>

class CMainWindow;
class CImageSITK;

namespace Ui
{
    class CWzdDIC;
}

class CWzdDIC : public QWizard
{
    Q_OBJECT

public:
    CWzdDIC(CMainWindow* wnd);
    ~CWzdDIC();

    CImageSITK* GetRefImage();
    CImageSITK* GetDefImage();

    int GetSubSize();
    int GetSubSpacing();

    std::string GetFilename();

protected:
    bool validateCurrentPage() override;
    void initializePage(int id) override;

public slots:
    void on_referenceSelection_changed(int i);
    void on_deformedSelection_changed(int i);

    void on_divisions_changed();
    
    void on_clearMask_clicked();
    void on_loadMask_clicked();
    void on_drawMask_clicked();

    void on_fileDlgButton_clicked();



private:
    Ui::CWzdDIC* ui;

};