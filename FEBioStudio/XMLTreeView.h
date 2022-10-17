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

#pragma once
#include <QTreeView>
#include <QStyledItemDelegate>

using std::pair;

class CMainWindow;
class CXMLDocument;

class XMLItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
	explicit XMLItemDelegate(QObject* parent = nullptr);
    ~XMLItemDelegate();

    void paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const override;

	QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

	void setEditorData(QWidget* editor, const QModelIndex& index) const override;

	void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;

    pair<int, int> GetSelection();
    void SetSelection(int start, int length);

private slots:
	void OnEditorSignal();

private:
    void SetLastSelection() const;

private:
    // Evil pointer hack to get around the const in createEditor
    QWidget** m_editor;
    pair<int,int>* m_lastSelection;
    int* m_superID;
};

namespace Ui
{
    class XMLTreeView;
}

class XMLTreeView : public QTreeView
{
    Q_OBJECT

public:
    XMLTreeView(CMainWindow* wnd);
    ~XMLTreeView();

    void setModel(QAbstractItemModel* newModel) override;

    CXMLDocument* GetDocument();

public slots:
    void on_removeSelectedRow_triggered();
    void on_addAttribute_triggered();
    void on_addElement_triggered();

    void ResetSearch();

protected:
    void contextMenuEvent(QContextMenuEvent* event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void on_headerMenu_requested(const QPoint& pos);

    void on_findWidget_next();
    void on_findWidget_prev();
    void on_findWidget_replace();
    void on_findWidget_replaceAll();
    
    void on_expandAll_triggered();
    void on_expandChildren_triggered();
    void on_collapseAll_triggered();
    void on_collapseChildren_triggered(QModelIndex index = QModelIndex());
    
    void on_showIDColumn_triggered(bool b);
    void on_showTypeColumn_triggered(bool b);
    void on_showNameColumn_triggered(bool b);
    void on_showCommentColumn_triggered(bool b);

private:
    void rerunFind();
    bool findNext(bool stopOnMatch = false);
    bool findPrev(bool stopOnMatch = false);

    void expandAndSelect(const QModelIndex& index, const int cursorStart, int cursorEnd);
    void expandToMatch(const QModelIndex& index);

private:
    friend class Ui::XMLTreeView;
    Ui::XMLTreeView* ui;

    CMainWindow* m_wnd;
};