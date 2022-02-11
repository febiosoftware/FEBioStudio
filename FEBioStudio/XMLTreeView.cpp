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
#include <QSize>
#include <QContextMenuEvent>
#include <QKeyEvent>
#include <QScrollBar>
#include <QHeaderView>
#include "XMLTreeView.h"
#include "XMLTreeModel.h"
#include "MainWindow.h"
#include "ui_mainwindow.h"
#include "XMLDocument.h"
#include "XMLCommands.h"
#include <QApplication>
#include <QMenu>
#include <QAction>
#include <FECore/fecore_enum.h>
#include <FEBioLink/FEBioClass.h>

#include <iostream>


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
    : QStyledItemDelegate(parent), m_superID(new int(FEINVALID_ID))
{

}

XMLItemDelegate::~XMLItemDelegate()
{
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

    if(!index.isValid())
    {
        QWidget* pw = QStyledItemDelegate::createEditor(parent, option, index);
        return pw;
    }

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

        if(*m_superID == FEINVALID_ID)
        {
            QWidget* pw = QStyledItemDelegate::createEditor(parent, option, index);
            return pw;
        }

        QStringList classNames;
        for(auto item : FEBio::FindAllClasses(-1, *m_superID))
        {
            classNames.append(item.sztype);
        }
        classNames.sort(Qt::CaseInsensitive);

        VariablePopupComboBox* pw = new VariablePopupComboBox(parent);
        pw->setEditable(true);
        pw->setStringList(classNames);

        return pw;
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
                    return pw;

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
                    return pw;

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
                    return pw;

                }
            }
        }
    }
    else if(index.column() == COMMENT)
    { 
        QTextEdit* pw = new QTextEdit(parent);
        pw->setMinimumHeight(option.rect.height()*4);
        pw->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);
        return pw;
    }

    QWidget* pw = QStyledItemDelegate::createEditor(parent, option, index);
    return pw;
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

    if(dynamic_cast<QComboBox*>(editor))
    {
        QComboBox* comboBox = dynamic_cast<QComboBox*>(editor);

        newString = comboBox->currentText();
    }
    else if(dynamic_cast<QTextEdit*>(editor))
    {
        newString = dynamic_cast<QTextEdit*>(editor)->toPlainText();
    }
    else
    {
        newString = static_cast<QLineEdit*>(editor)->text();
    }

    if(newString != oldString)
    {
        // if(FEBio::GetClassId(*m_superID, newString.toStdString()) != -1)
        // {
        //     FSModelComponent* comp = FEBio::CreateClass(*m_superID, newString.toStdString(), nullptr);

        //     for(int param = 0; param < comp->Parameters(); param++)
        //     {
        //         std::cout << comp->GetParam(param).GetShortName() << std::endl;
        //     }
        // }




        CCmdEditCell* cmd = new CCmdEditCell(QPersistentModelIndex(index), newString, oldString, xmlModel);

        doc->DoCommand(cmd);
    }
}

void XMLItemDelegate::OnEditorSignal()
{
	QWidget* sender = dynamic_cast<QWidget*>(QObject::sender());
	emit commitData(sender);
}

/////////////////////////////////////////////////////////////////////////////////////

class Ui::XMLTreeView
{
public:
    QAction* addAttribute;
    QAction* addElement;
    QAction* removeSelectedRow;

public:
    void setupUi(::XMLTreeView* parent)
    {
        parent->setItemDelegate(new XMLItemDelegate);
        parent->setAlternatingRowColors(true);

        addAttribute = new QAction("Add Attribute", parent);
        addAttribute->setObjectName("addAttribute");

        addElement = new QAction("Add Element", parent);
        addElement->setObjectName("addElement");

        removeSelectedRow = new QAction("Delete Row", parent);
        removeSelectedRow->setObjectName("removeSelectedRow");

        parent->header()->setContextMenuPolicy(Qt::CustomContextMenu);
        QObject::connect(parent->header(), &QHeaderView::customContextMenuRequested, parent, &::XMLTreeView::on_headerMenu_requested);
    }

};

    

XMLTreeView::XMLTreeView(CMainWindow* wnd) : QTreeView(wnd), m_wnd(wnd), ui(new Ui::XMLTreeView)
{
    ui->setupUi(this);

    QMetaObject::connectSlotsByName(this);
}

void XMLTreeView::setModel(QAbstractItemModel* newModel)
{
    XMLTreeModel* current = dynamic_cast<XMLTreeModel*>(model());

    bool firstModel = false;
    if(current)
    {
        disconnect(this, &XMLTreeView::expanded, current, &XMLTreeModel::ItemExpanded);
        disconnect(this, &XMLTreeView::collapsed, current, &XMLTreeModel::ItemCollapsed);
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
}

CXMLDocument* XMLTreeView::GetDocument()
{
    return dynamic_cast<CXMLDocument*>(m_wnd->GetDocument());
}

void XMLTreeView::on_removeSelectedRow_triggered()
{
    QModelIndex index = currentIndex();
    XMLTreeItem* item = static_cast<XMLTreeItem*>(index.internalPointer());

    CCmdRemoveRow* cmd = new CCmdRemoveRow(QPersistentModelIndex(index.parent()), index.row(), item, static_cast<XMLTreeModel*>(model()));

    GetDocument()->DoCommand(cmd);
}

void XMLTreeView::on_addAttribute_triggered()
{
    QModelIndex index = currentIndex();

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
    CCmdAddElement* cmd = new CCmdAddElement(QPersistentModelIndex(index), static_cast<XMLTreeModel*>(model()));

    GetDocument()->DoCommand(cmd);

    // Select new item and begin edititing it
    expand(index);
    XMLTreeItem* item = static_cast<XMLTreeItem*>(index.internalPointer());
    setCurrentIndex(model()->index(item->childCount() - 1, 0, index));

    edit(currentIndex().siblingAtColumn(TAG));
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

    menu.exec(viewport()->mapToGlobal(event->pos()));
}

void XMLTreeView::on_headerMenu_requested(const QPoint& pos)
{

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