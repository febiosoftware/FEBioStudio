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

#include <QWidget>
#include <CUILib/GLSceneView.h>
#include <GLLib/GLCamera.h>
#include <GLLib/GLScene.h>

class CMainWindow;
class matrix;
class GObject;
class GLTriad;
class CFiberODFAnalysis;
class FSMaterial;
struct CODF;

using std::string;

class CFiberGLWidget;

class CODFScene : public GLScene
{
public:
	CODFScene(CFiberGLWidget* w) : m_w(w) {}
	void Render(GLRenderEngine& engine, GLContext& rc) override;

	void RenderCanvas(QPainter& painter, GLContext& rc) override;

	BOX GetBoundingBox() override;

	BOX GetSelectionBox() override;

private:
	CFiberGLWidget* m_w;
};

class CFiberGLWidget : public CGLManagedSceneView
{
public:
    CFiberGLWidget();

    void setAnalysis(CFiberODFAnalysis* analysis);
    void setODF(CODF* odf);

	void RenderBackground() override;

private:
    CFiberODFAnalysis* m_analysis;
    CODF* m_ODF;

    GLTriad* m_ptriad;

	friend class CODFScene;
};

namespace Ui
{
    class CFiberODFWidget;
}

class CFiberODFWidget : public QWidget
{
    Q_OBJECT

public:
    CFiberODFWidget(CMainWindow* wnd);
    void setAnalysis(CFiberODFAnalysis* analysis);

private slots:
    void on_runAll_triggered(bool b);
	void on_runSel_triggered(bool b);
    void on_genButton_pressed();
    void on_nextButton_pressed();
    void on_backButton_pressed();
    void on_odfSelector_currentIndexChanged(int index);
    void on_odfCheck_stateChanged(int state);
    void on_copyODF_triggered();
    void on_copyEFD_triggered();
    void on_saveSphHarm_triggered();
    void on_saveODFs_triggered();
    void on_saveStats_triggered();

private:
    FSMaterial* getMaterial(std::string type);
    void findMaterials(FSMaterial* mat, std::string type, std::string name, std::vector<std::pair<std::string,FSMaterial*>>& materials);

private:
    CMainWindow* m_wnd;
    Ui::CFiberODFWidget* ui;
    CFiberODFAnalysis* m_analysis;

};