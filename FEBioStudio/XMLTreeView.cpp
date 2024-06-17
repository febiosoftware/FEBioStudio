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

#include <QComboBox>
#include <QCompleter>
#include <QTextEdit>
#include <QTextCursor>
#include <QSize>
#include <QContextMenuEvent>
#include <QKeyEvent>
#include <QScrollBar>
#include <QHeaderView>
#include <QLineEdit>
#include <QCoreApplication>
#include "XMLTreeView.h"
#include "XMLTreeModel.h"
#include "MainWindow.h"
#include "ui_mainwindow.h"
#include "XMLDocument.h"
#include "XMLCommands.h"
#include "FindWidget.h"
#include <QApplication>
#include <QMenu>
#include <QAction>
#include <FECore/fecore_enum.h>
#include <FEBioLink/FEBioClass.h>
#include <stack>

#include <iostream>

#ifdef WIN32
#undef min
#undef max
#endif


class VariablePopupComboBox : public QComboBox
{
public:
    VariablePopupComboBox(QWidget* parent) : QComboBox(parent) {}

    void showPopup() override
    {
        view()->setMinimumWidth(view()->sizeHintForColumn(0));

        QComboBox::showPopup();
    }

    void setStringList(QStringList items)
    {
        addItems(items);

        QCompleter* completer = new QCompleter(items);
        completer->setCaseSensitivity(Qt::CaseInsensitive);
        completer->setFilterMode(Qt::MatchContains);

        setCompleter(completer);
    }

protected:
    void keyPressEvent(QKeyEvent* e)
    {
        QComboBox::keyPressEvent(e);

        QRect cr = contentsRect();
        cr.setWidth(completer()->popup()->sizeHintForColumn(0)
        + completer()->popup()->verticalScrollBar()->sizeHint().width());
        completer()->complete(cr);
    }
};

XMLItemDelegate::XMLItemDelegate(QObject* parent)
    : QStyledItemDelegate(parent), m_editor(new QWidget*(nullptr)), m_lastSelection(new pair<int,int>(-1, 0)), m_superID(new int(FEINVALID_ID))
{

}

XMLItemDelegate::~XMLItemDelegate()
{
    delete m_editor;
    delete m_lastSelection;
    delete m_superID;
}

void XMLItemDelegate::paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
    if(index.column() != NUM_COLUMNS - 1)
    {
        painter->save();
        painter->setPen(qApp->palette().color(QPalette::WindowText));
        // painter->drawRect(option.rect);
        painter->drawLine(option.rect.topRight(), option.rect.bottomRight());
        painter->restore();
    }

    QStyledItemDelegate::paint(painter, option, index);

}

QWidget* XMLItemDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    *m_superID = FEINVALID_ID;

    if(index.isValid())
    {
        XMLTreeItem* item = static_cast<XMLTreeItem*>(index.internalPointer());

        if(index.column() == TYPE)
        {
            XMLTreeItem* ancestor = item->ancestorItem(1);

            if(ancestor->data(TAG) == "Material")
            {
                *m_superID = FEMATERIAL_ID;
            }
            else if(ancestor->data(TAG) == "Boundary")
            {
                *m_superID = FEBC_ID;
            }
            else if(ancestor->data(TAG) == "Contact")
            {
                *m_superID = FESURFACEINTERFACE_ID;
            }
            else if(ancestor->data(TAG) == "Load")
            {
                *m_superID = FELOAD_ID;
            }
            else if(ancestor->data(TAG) == "Initial")
            {
                *m_superID = FEIC_ID;
            }
            else if(ancestor->data(TAG) == "Discrete")
            {
                *m_superID = FEDISCRETEMATERIAL_ID;
            }

            if(*m_superID != FEINVALID_ID)
            {
                QStringList classNames;
                for(auto item : FEBio::FindAllClasses(-1, *m_superID))
                {
                    classNames.append(item.sztype);
                }
                classNames.sort(Qt::CaseInsensitive);

                VariablePopupComboBox* pw = new VariablePopupComboBox(parent);
                pw->setEditable(true);
                pw->setStringList(classNames);

                *m_editor = pw;
            }
        }
        else if(index.column() == VALUE)
        {
            if(item->data(TAG) == "node_set")
            {
                XMLTreeItem* root = item->ancestorItem(0);
                XMLTreeItem* mesh = root->findChlid(TAG, "Mesh");

                if(mesh)
                {
                    vector<XMLTreeItem*> nodeSets = mesh->findAllChlidren(TAG, "NodeSet");
                    
                    if(nodeSets.size() > 0)
                    {
                        QStringList nodeSetNames;

                        for(auto set : nodeSets)
                        {
                            nodeSetNames.append(set->data(NAME));
                        }

                        VariablePopupComboBox* pw = new VariablePopupComboBox(parent);
                        pw->setEditable(true);
                        pw->setStringList(nodeSetNames);
                        
                        *m_editor = pw;
                    }
                }
            }
            else if(item->data(TAG) == "rb")
            {
                XMLTreeItem* root = item->ancestorItem(0);
                XMLTreeItem* material = root->findChlid(TAG, "Material");

                if(material)
                {
                    vector<XMLTreeItem*> rigidBodies = material->findAllChlidren(TYPE, "rigid body");
                    
                    if(rigidBodies.size() > 0)
                    {
                        QStringList rbNames;

                        for(auto set : rigidBodies)
                        {
                            rbNames.append(set->data(ID));
                        }

                        VariablePopupComboBox* pw = new VariablePopupComboBox(parent);
                        pw->setEditable(true);
                        pw->setStringList(rbNames);
                        
                        *m_editor = pw;
                    }
                }
            }
            else if(item->data(TAG) == "lc")
            {
                XMLTreeItem* root = item->ancestorItem(0);
                XMLTreeItem* loadData = root->findChlid(TAG, "LoadData");

                if(loadData)
                {
                    vector<XMLTreeItem*> lcs = loadData->findAllChlidren(TAG, "load_controller");
                    
                    if(lcs.size() > 0)
                    {
                        QStringList lcNames;

                        for(auto set : lcs)
                        {
                            lcNames.append(set->data(ID));
                        }

                        VariablePopupComboBox* pw = new VariablePopupComboBox(parent);
                        pw->setEditable(true);
                        pw->setStringList(lcNames);

                        *m_editor = pw;
                    }
                }
            }
        }
        else if(index.column() == COMMENT)
        { 
            QTextEdit* pw = new QTextEdit(parent);
            pw->setMinimumHeight(option.rect.height()*5);
            pw->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);
            
            *m_editor = pw;
        }
    }

    if(!*m_editor)
    {
        *m_editor = QStyledItemDelegate::createEditor(parent, option, index);
    }

    // null the editor pointer when the object is destroyed
    connect(*m_editor, &QObject::destroyed, [=]() { *m_editor = nullptr; });

    return *m_editor;
}

void XMLItemDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const 
{
    if(!index.isValid()) return;

    QStyledItemDelegate::setEditorData(editor, index);

}

void XMLItemDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    if (!index.isValid()) return;

    XMLTreeModel* xmlModel = static_cast<XMLTreeModel*>(model);
    CXMLDocument* doc = xmlModel->GetDocument();
    XMLTreeItem* item = static_cast<XMLTreeItem*>(index.internalPointer());
    
    QString oldString = item->data(index.column());
    QString newString;

    SetLastSelection();

    if(dynamic_cast<QComboBox*>(editor))
    {
        QComboBox* comboBox = dynamic_cast<QComboBox*>(editor);

        newString = comboBox->currentText();
    }
    else if(dynamic_cast<QTextEdit*>(editor))
    {
        QTextEdit* textEdit = dynamic_cast<QTextEdit*>(editor);
        newString = textEdit->toPlainText();
    }
    else if(dynamic_cast<QLineEdit*>(editor))
    {
        QLineEdit* lineEdit = dynamic_cast<QLineEdit*>(editor);
        newString = lineEdit->text();
    }
    else
    {
        return;
    }

    if(newString != oldString)
    {
    //     if(FEBio::GetClassId(*m_superID, newString.toStdString()) != -1)
    //     {
    //         FSModelComponent* comp = FEBio::CreateClass(*m_superID, newString.toStdString(), nullptr);

    //         for(int param = 0; param < comp->Parameters(); param++)
    //         {
    //             std::cout << comp->GetParam(param).GetShortName() << std::endl;
    //         }

    //         for(int prop = 0; prop < comp->Properties(); prop++)
    //         {
    //             std::cout << comp->GetProperty(prop).GetName() << std::endl;
    //         }
    //     }




        CCmdEditCell* cmd = new CCmdEditCell(QPersistentModelIndex(index), newString, oldString, xmlModel);

        doc->DoCommand(cmd);
    }
}

void XMLItemDelegate::SetSelection(int start, int length)
{
    QTextEdit* textEdit =dynamic_cast<QTextEdit*>(*m_editor);
    if(textEdit)
    {
        QTextCursor cursor = textEdit->textCursor();
        cursor.setPosition(start);
        cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, length);
        textEdit->setTextCursor(cursor);
    }

    QLineEdit* lineEdit =dynamic_cast<QLineEdit*>(*m_editor);
    if(lineEdit)
    {
        lineEdit->setSelection(start, length);
    }

    VariablePopupComboBox* comboBox = dynamic_cast<VariablePopupComboBox*>(*m_editor);
    if(comboBox)
    {
        comboBox->lineEdit()->setSelection(start, length);
    }
}

pair<int, int> XMLItemDelegate::GetSelection()
{
    SetLastSelection();

    return *m_lastSelection;    
}

void XMLItemDelegate::OnEditorSignal()
{
	QWidget* sender = dynamic_cast<QWidget*>(QObject::sender());
	emit commitData(sender);
}

void XMLItemDelegate::SetLastSelection() const
{
    if(dynamic_cast<QComboBox*>(*m_editor))
    {
        QComboBox* comboBox = dynamic_cast<QComboBox*>(*m_editor);

        int start = comboBox->lineEdit()->selectionStart();
        if(start == -1)
        {
            start = comboBox->lineEdit()->cursorPosition();
        }

        *m_lastSelection = pair<int,int>(start, comboBox->lineEdit()->selectionLength());
    }
    else if(dynamic_cast<QTextEdit*>(*m_editor))
    {
        QTextEdit* textEdit = dynamic_cast<QTextEdit*>(*m_editor);

        QTextCursor cursor = textEdit->textCursor();

        int start = std::min(cursor.position(), cursor.anchor());
        int stop = std::max(cursor.position(), cursor.anchor());

        *m_lastSelection = pair<int,int>(start, stop - start);
    }
    else if(dynamic_cast<QLineEdit*>(*m_editor))
    {
        QLineEdit* lineEdit = dynamic_cast<QLineEdit*>(*m_editor);

        int start = lineEdit->selectionStart();
        if(start == -1)
        {
            start = lineEdit->cursorPosition();
        }

        *m_lastSelection = pair<int,int>(start, lineEdit->selectionLength());
    }
}

/////////////////////////////////////////////////////////////////////////////////////

class Ui::XMLTreeView
{
public:
    QAction* addAttribute;
    QAction* addElement;
    QAction* removeSelectedRow;

    QAction* expandAll;
    QAction* expandChildren;
    QAction* collapseAll;
    QAction* collapseChildren;

    QMenu* columnsMenu;
    QAction* showIDColumn;
    QAction* showTypeColumn;
    QAction* showNameColumn;
    QAction* showCommentColumn;

    ::CFindWidget* findWidget;

    QAction* find;
    QAction* replace;

public:
    void setupUi(::XMLTreeView* parent)
    {
        parent->setItemDelegate(new XMLItemDelegate);
        parent->setAlternatingRowColors(true);

        findWidget = new ::CFindWidget(parent);
        findWidget->setObjectName("findWidget");
        findWidget->hide();

        find = new QAction("Find", parent);
        find->setShortcut(QKeySequence::Find);
        parent->addAction(find);
        QObject::connect(find, &QAction::triggered, findWidget, &::CFindWidget::ShowFind);
        
        replace = new QAction("Replace", parent);
        replace->setShortcut(QKeySequence::Replace);
        parent->addAction(replace);
        QObject::connect(replace, &QAction::triggered, findWidget, &::CFindWidget::ShowReplace);

        addAttribute = new QAction("Add Attribute", parent);
        addAttribute->setObjectName("addAttribute");

        addElement = new QAction("Add Element", parent);
        addElement->setObjectName("addElement");

        removeSelectedRow = new QAction("Delete Row", parent);
        removeSelectedRow->setObjectName("removeSelectedRow");

        expandAll = new QAction("Expand All", parent);
        expandAll->setObjectName("expandAll");

        expandChildren = new QAction("Expand Children", parent);
        expandChildren->setObjectName("expandChildren");

        collapseAll = new QAction("Collapse All", parent);
        collapseAll->setObjectName("collapseAll");

        collapseChildren = new QAction("Collapse Children", parent);
        collapseChildren->setObjectName("collapseChildren");

        columnsMenu = new QMenu("Columns");

        showIDColumn = new QAction("ID", parent);
        showIDColumn->setObjectName("showIDColumn");
        showIDColumn->setCheckable(true);
        showIDColumn->setChecked(true);
        
        showTypeColumn = new QAction("Type", parent);
        showTypeColumn->setObjectName("showTypeColumn");
        showTypeColumn->setCheckable(true);
        showTypeColumn->setChecked(true);

        showNameColumn= new QAction("Name", parent);
        showNameColumn->setObjectName("showNameColumn");
        showNameColumn->setCheckable(true);
        showNameColumn->setChecked(true);

        showCommentColumn = new QAction("Comment", parent);
        showCommentColumn->setObjectName("showCommentColumn");
        showCommentColumn->setCheckable(true);
        showCommentColumn->setChecked(true);

        columnsMenu->addAction(showIDColumn);
        columnsMenu->addAction(showTypeColumn);
        columnsMenu->addAction(showNameColumn);
        columnsMenu->addAction(showCommentColumn);

        parent->header()->setContextMenuPolicy(Qt::CustomContextMenu);
        QObject::connect(parent->header(), &QHeaderView::customContextMenuRequested, parent, &::XMLTreeView::on_headerMenu_requested);
    }

public:
    QString searchTerm; 
    vector<XMLSearchResult> searchList;
    bool searchCaseSensative;


};

    

XMLTreeView::XMLTreeView(CMainWindow* wnd) : QTreeView(wnd), m_wnd(wnd), ui(new Ui::XMLTreeView)
{
    ui->setupUi(this);

    QMetaObject::connectSlotsByName(this);
}

XMLTreeView::~XMLTreeView()
{
    delete ui;
}

void XMLTreeView::setModel(QAbstractItemModel* newModel)
{
    XMLTreeModel* current = dynamic_cast<XMLTreeModel*>(model());

    bool firstModel = false;
    if(current)
    {
        disconnect(this, &XMLTreeView::expanded, current, &XMLTreeModel::ItemExpanded);
        disconnect(this, &XMLTreeView::collapsed, current, &XMLTreeModel::ItemCollapsed);
        disconnect(current, &XMLTreeModel::dataChanged, this, &XMLTreeView::ResetSearch);
    }
    else
    {
        firstModel = true;
    }

    QTreeView::setModel(newModel);

    // Set the column sizes the first time we set a model
    if(firstModel)
    {
        int width = m_wnd->GetEditorSize().width();
        setColumnWidth(TAG, width*0.25);
        setColumnWidth(ID, width*0.05);
        setColumnWidth(TYPE, width*0.15);
        setColumnWidth(NAME, width*0.15);
        setColumnWidth(VALUE, width*0.10);
    }

    current = static_cast<XMLTreeModel*>(newModel);

    expandToMatch(current->root());

    connect(this, &XMLTreeView::expanded, current, &XMLTreeModel::ItemExpanded);
    connect(this, &XMLTreeView::collapsed, current, &XMLTreeModel::ItemCollapsed);
    connect(current, &XMLTreeModel::dataChanged, this, &XMLTreeView::ResetSearch);

    ResetSearch();
}

CXMLDocument* XMLTreeView::GetDocument()
{
    return dynamic_cast<CXMLDocument*>(m_wnd->GetDocument());
}

void XMLTreeView::on_removeSelectedRow_triggered()
{
    QModelIndex index = currentIndex();
    if(!index.isValid()) return;

    XMLTreeItem* item = static_cast<XMLTreeItem*>(index.internalPointer());

    // Don't delete the root item
    if(item->parentItem() == static_cast<XMLTreeModel*>(model())->GetRoot())
    {
        return;
    }

    CCmdRemoveRow* cmd = new CCmdRemoveRow(QPersistentModelIndex(index.parent()), index.row(), item, static_cast<XMLTreeModel*>(model()));

    GetDocument()->DoCommand(cmd);
}

void XMLTreeView::on_addAttribute_triggered()
{
    QModelIndex index = currentIndex();
    if(!index.isValid()) return;

    if(static_cast<XMLTreeItem*>(index.internalPointer())->GetItemType() == XMLTreeItem::ATTRIBUTE)
    {
        return;
    }

    CCmdAddAttribute* cmd = new CCmdAddAttribute(QPersistentModelIndex(index), static_cast<XMLTreeModel*>(model()));

    GetDocument()->DoCommand(cmd);

    // Select new item and begin edititing it
    expand(index);
    XMLTreeItem* item = static_cast<XMLTreeItem*>(index.internalPointer());
    setCurrentIndex(model()->index(item->FirstElement() - 1, 0, index));

    edit(currentIndex().siblingAtColumn(TAG));
}

void XMLTreeView::on_addElement_triggered()
{
    QModelIndex index = currentIndex();
    if(!index.isValid()) return;

    if(static_cast<XMLTreeItem*>(index.internalPointer())->GetItemType() == XMLTreeItem::ATTRIBUTE)
    {
        return;
    }

    CCmdAddElement* cmd = new CCmdAddElement(QPersistentModelIndex(index), static_cast<XMLTreeModel*>(model()));

    GetDocument()->DoCommand(cmd);

    // Select new item and begin edititing it
    expand(index);
    XMLTreeItem* item = static_cast<XMLTreeItem*>(index.internalPointer());
    setCurrentIndex(model()->index(item->childCount() - 1, 0, index));

    edit(currentIndex().siblingAtColumn(TAG));
}

void XMLTreeView::ResetSearch()
{
    ui->searchTerm = "";
}

void XMLTreeView::contextMenuEvent(QContextMenuEvent* event)
{
    setCurrentIndex(indexAt(event->pos()));

    if(!currentIndex().isValid()) return;
    
    XMLTreeItem* item = static_cast<XMLTreeItem*>(currentIndex().internalPointer());

    QMenu menu;

    if(item->GetItemType() == XMLTreeItem::ELEMENT)
    {
        menu.addAction(ui->addAttribute);
        menu.addAction(ui->addElement);
    }
    
    if(item->Depth() != 0)
    {
        if(menu.actions().size() > 0)
        {
            menu.addSeparator();
        }
        menu.addAction(ui->removeSelectedRow);
    }

    menu.addSeparator();
    menu.addAction(ui->find);
    menu.addAction(ui->replace);

    menu.addSeparator();
    menu.addAction(ui->expandAll);
    if(item->childCount() > 0)
    {
        menu.addAction(ui->expandChildren);
    }
    menu.addAction(ui->collapseAll);
    if(item->childCount() > 0)
    {
        menu.addAction(ui->collapseChildren);
    }

    menu.addSeparator();
    menu.addMenu(ui->columnsMenu);

    menu.exec(viewport()->mapToGlobal(event->pos()));
}

void XMLTreeView::resizeEvent(QResizeEvent *event)
{
    QTreeView::resizeEvent(event);

    ui->findWidget->move(width() - ui->findWidget->width(), 0);
}

void XMLTreeView::on_headerMenu_requested(const QPoint& pos)
{
    ui->columnsMenu->exec(viewport()->mapToGlobal(pos));
}

void XMLTreeView::on_findWidget_next()
{
    rerunFind();

    findNext();
}

void XMLTreeView::on_findWidget_prev()
{
    rerunFind();

    findPrev();
}

void XMLTreeView::on_findWidget_replace()
{
    rerunFind();

    if(findNext(true))
    {
        XMLItemDelegate* delegate = dynamic_cast<XMLItemDelegate*>(itemDelegate());

        pair<int,int> selection = delegate->GetSelection();

        QString oldText = currentIndex().data().toString();
        QString newText = oldText;

        newText.remove(selection.first, selection.second).insert(selection.first, ui->findWidget->GetReplaceText());

        XMLTreeModel* xmlModel = static_cast<XMLTreeModel*>(model());

        CCmdEditCell* cmd = new CCmdEditCell(QPersistentModelIndex(currentIndex()), newText, oldText, xmlModel);

        xmlModel->GetDocument()->DoCommand(cmd);

        delegate->SetSelection(selection.first + selection.second, 0);

        executeDelayedItemsLayout();

    }
}

void XMLTreeView::on_findWidget_replaceAll()
{
    rerunFind();

    CCmdGroup* group = new CCmdGroup("Replace All");

    Qt::CaseSensitivity cs = ui->searchCaseSensative ? Qt::CaseSensitive : Qt::CaseInsensitive;

    XMLTreeModel* xmlModel = static_cast<XMLTreeModel*>(model());

    for(auto result : ui->searchList)
    {
        QString oldText = result.item->data(result.column);
        QString newText = oldText;
        newText.replace(ui->searchTerm, ui->findWidget->GetReplaceText(), cs);

        CCmdEditCell* cmd = new CCmdEditCell(QPersistentModelIndex(xmlModel->itemToIndex(result.item, result.column)), newText, oldText, xmlModel);

        group->AddCommand(cmd);
    }

    xmlModel->GetDocument()->DoCommand(group);
}

void XMLTreeView::on_expandAll_triggered()
{
    expandAll();
}

void XMLTreeView::on_expandChildren_triggered()
{
    QModelIndex index = currentIndex();
    if(!index.isValid()) return;

    expandRecursively(index);
}

void XMLTreeView::on_collapseAll_triggered()
{
    collapseAll();
}

void XMLTreeView::on_collapseChildren_triggered(QModelIndex index)
{
    if(!index.isValid())
    {
        index = currentIndex();
        if(!index.isValid()) return;
    }

    XMLTreeItem* item = static_cast<XMLTreeItem*>(index.internalPointer());

    for(int child = 0; child < item->childCount(); child++)
    {
        on_collapseChildren_triggered(model()->index(child, 0, index));
    }

    collapse(index);    
}

void XMLTreeView::on_showIDColumn_triggered(bool b)
{
    setColumnHidden(ID, !b);
}

void XMLTreeView::on_showTypeColumn_triggered(bool b)
{
    setColumnHidden(TYPE, !b);
}

void XMLTreeView::on_showNameColumn_triggered(bool b)
{
    setColumnHidden(NAME, !b);
}

void XMLTreeView::on_showCommentColumn_triggered(bool b)
{
    setColumnHidden(COMMENT, !b);
}

void XMLTreeView::rerunFind()
{
    if(ui->searchTerm != ui->findWidget->GetFindText() || ui->searchCaseSensative != ui->findWidget->GetCaseSensative()) 
    {
        ui->searchTerm = ui->findWidget->GetFindText();
        ui->searchCaseSensative = ui->findWidget->GetCaseSensative();
        ui->searchList.clear();

        XMLTreeItem* root = static_cast<XMLTreeModel*>(model())->GetRoot();

        root->FindAll(ui->searchTerm, ui->findWidget->GetCaseSensative(), ui->searchList);

        ui->findWidget->SetTotal(ui->searchList.size());
    }
}

bool XMLTreeView::findNext(bool stopOnMatch)
{
    bool alreadyMatched = false;

    if(ui->searchList.size() == 0) return alreadyMatched; 

    if(!currentIndex().isValid()) return alreadyMatched;
    
    XMLTreeItem* selectedItem = static_cast<XMLTreeItem*>(currentIndex().internalPointer());

    int foundIndex = 0;
    XMLSearchResult searchResult;

    for(int index = 0; index < ui->searchList.size(); index++)
    {
        searchResult = ui->searchList[index];

        if(searchResult.item == selectedItem)
        {
            if(searchResult.column == currentIndex().column())
            {
                pair<int,int> currentSelection = dynamic_cast<XMLItemDelegate*>(itemDelegate())->GetSelection();

                if(stopOnMatch)
                {
                    if(currentSelection.first == searchResult.stringIndex && currentSelection.second == ui->searchTerm.size())
                    {
                        alreadyMatched = true;
                        foundIndex = index;
                        break;
                    }
                }

                if(currentSelection.first != -1)
                {
                    if(currentSelection.first == searchResult.stringIndex && currentSelection.second == 0)
                    {
                        foundIndex = index;
                        break;
                    }

                    if(searchResult.stringIndex > currentSelection.first)
                    {
                        foundIndex = index;
                        break;
                    }
                }
            }            

            if(searchResult.column > currentIndex().column())
            {
                foundIndex = index;
                break;
            }

            continue;
        }

        bool found = false;
        int limit = std::min(selectedItem->Depth(), searchResult.item->Depth());
        int searchRow, selectedRow;
        for(int depth = 0; depth <= limit; depth++)
        {
            searchRow = searchResult.item->ancestorItem(depth)->row();
            selectedRow = selectedItem->ancestorItem(depth)->row();

            if(searchRow > selectedRow)
            {
                foundIndex = index;
                found = true;
                break;
            }
            
            if(searchRow < selectedRow)
            {
                break;
            }
        }

        if(found)
        {
            break;
        }
        
        if(searchRow == selectedRow)
        {
            if(searchResult.item->Depth() > selectedItem->Depth())
            {
                foundIndex = index;
                break;
            }
        }
    }

    searchResult = ui->searchList[foundIndex];

    ui->findWidget->SetCurrent(foundIndex + 1);

    expandAndSelect(static_cast<XMLTreeModel*>(model())->itemToIndex(searchResult.item, searchResult.column), searchResult.stringIndex, ui->searchTerm.size()); 

    return alreadyMatched; 
}

bool XMLTreeView::findPrev(bool stopOnMatch)
{
    bool alreadyMatched = false;

    if(ui->searchList.size() == 0) return alreadyMatched;

    if(!currentIndex().isValid()) return alreadyMatched;
    
    XMLTreeItem* selectedItem = static_cast<XMLTreeItem*>(currentIndex().internalPointer());

    int foundIndex = ui->searchList.size() - 1;
    XMLSearchResult searchResult;

    for(int index = ui->searchList.size() - 1; index >= 0; index--)
    {
        searchResult = ui->searchList[index];

        if(searchResult.item == selectedItem)
        {
            if(searchResult.column == currentIndex().column())
            {
                pair<int,int> currentSelection = dynamic_cast<XMLItemDelegate*>(itemDelegate())->GetSelection();

                if(stopOnMatch)
                {
                    if(currentSelection.first == searchResult.stringIndex && currentSelection.second == ui->searchTerm.size())
                    {
                        alreadyMatched = true;
                        foundIndex = index;
                        break;
                    }
                }

                if(currentSelection.first != -1)
                {
                    if(searchResult.stringIndex + ui->searchTerm.size() == currentSelection.first && currentSelection.second == 0)
                    {
                        foundIndex = index;
                        break;
                    }

                    if(searchResult.stringIndex + ui->searchTerm.size() < currentSelection.first)
                    {
                        foundIndex = index;
                        break;
                    }
                }
            }

            if(searchResult.column < currentIndex().column())
            {
                foundIndex = index;
                break;
            }

            continue;
        }

        bool found = false;
        int limit = std::min(selectedItem->Depth(), searchResult.item->Depth());
        int searchRow, selectedRow;
        for(int depth = 0; depth <= limit; depth++)
        {
            searchRow = searchResult.item->ancestorItem(depth)->row();
            selectedRow = selectedItem->ancestorItem(depth)->row();

            if(searchRow < selectedRow)
            {
                foundIndex = index;
                found = true;
                break;
            }
            
            if(searchRow > selectedRow)
            {
                break;
            }
        }

        if(found)
        {
            break;
        }
        
        if(searchRow == selectedRow)
        {
            if(searchResult.item->Depth() < selectedItem->Depth())
            {
                foundIndex = index;
                break;
            }
        }
    }

    searchResult = ui->searchList[foundIndex];

    ui->findWidget->SetCurrent(foundIndex + 1);

    expandAndSelect(static_cast<XMLTreeModel*>(model())->itemToIndex(searchResult.item, searchResult.column), searchResult.stringIndex, ui->searchTerm.size());

    return alreadyMatched;
}

void XMLTreeView::expandAndSelect(const QModelIndex& index, const int cursorStart, int cursorEnd)
{
    // QModelIndex parent = index.siblingAtColumn(0).parent();
    // while(parent.isValid())
    // {
    //     expand(parent);
    //     parent = parent.parent();
    // }

    // This is an ugly workaround, but I can't get anything else to work
    XMLTreeItem* item = static_cast<XMLTreeItem*>(index.internalPointer());
    XMLTreeItem* parent = item->parentItem();
    while(parent->Depth() >=0)
    {
        parent->SetExpanded(true);
        parent = parent->parentItem();
    }

    expandToMatch(static_cast<XMLTreeModel*>(model())->root());

    setCurrentIndex(index);

    edit(currentIndex());

    XMLItemDelegate* delegate = static_cast<XMLItemDelegate*>(itemDelegate());

    delegate->SetSelection(cursorStart, cursorEnd);
}

void XMLTreeView::expandToMatch(const QModelIndex& index)
{
    if(!index.isValid()) return;

    XMLTreeItem* item = static_cast<XMLTreeItem*>(index.internalPointer());

    if(item->Expanded())
    {
        expand(index);
    }
    else
    {
        return;
    }

    if(item->childCount() == 0)
    {
        return;
    }

    XMLTreeModel* xmlModel = static_cast<XMLTreeModel*>(model());

    for(int child = 0; child < item->childCount(); child++)
    {
        expandToMatch(xmlModel->index(child, 0, index));
    }
}