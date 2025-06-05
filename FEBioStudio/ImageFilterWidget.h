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
#include <QListWidget>
#include <QDropEvent>
#include <QModelIndex>
#include <iostream>

class QListWidget;
class QPushButton;
class CPropertyListView;
class CPropertyList;
class CMainWindow;
class CImageModel;

class QModelIndex;

class FilterListWidget : public QListWidget
{
    Q_OBJECT

public:
    FilterListWidget() {}

signals:
    void internalMove(int fromIndex, int toIndex);

protected:
    void dropEvent(QDropEvent *event)
    {
        int fromIndex = currentRow();

        QListWidget::dropEvent(event);

        int toIndex = currentRow();

        if(fromIndex != toIndex)
        {
            emit internalMove(fromIndex, toIndex);
        }
    }
};

class CMainWindow;

class CImageFilterWidget : public QWidget
{
    Q_OBJECT

public:
    CImageFilterWidget(CMainWindow* wnd);
    ~CImageFilterWidget();

    void SetImageModel(CImageModel* img);

private:
    void Clear();
    void UpdateApplyButton();

public slots:
    void Update();

signals:
    void filterStatusChanged(bool unapplied);

private slots:
    void on_list_itemSelectionChanged();
    void on_list_internalMove(int fromIndex, int toIndex);
    void on_addFilterBtn_clicked();
    void on_delFilterBtn_clicked();
    void on_applyFilters_clicked();
    void on_filterProps_changed();

private:
    CImageModel* m_imgModel;
    
    QListWidget* m_list;
    QPushButton* m_applyFilters;
    CPropertyListView* m_filterProps;

    std::vector<CPropertyList*> m_props;

	CMainWindow* m_wnd;
};