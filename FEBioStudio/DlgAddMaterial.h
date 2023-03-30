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

#include "DlgAddPhysicsItem.h"
#include <XML/XMLReader.h>

namespace Ui
{
    class CDlgAddMaterial;
}

class CMainWindow;
class XMLTreeItem;

class CDlgAddMaterial : public CDlgAddPhysicsItem 
{
    Q_OBJECT

public:
    CDlgAddMaterial(QString windowName, int superID, int baseClassID, FSModel* fem, bool includeModuleDependencies, bool showStepList, CMainWindow* parent);


private:
    void ReadXML();

    XMLTreeItem* getChild(XMLTag& tag, int depth);

    void printInfo(XMLTreeItem* item, int depth);

public slots:
    void on_list_currentRowChanged(int index);

private:
    Ui::CDlgAddMaterial* ui;
    CMainWindow* m_wnd;
};