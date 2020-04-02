#pragma once
#include <QScrollArea>
#include <QtCore/QList>

class QPushButton;

class CToolItem : public QWidget
{
public:
	CToolItem(const QString& name, QWidget* tool, QWidget* parent = 0);

	void setTitle(const QString& t);

private:
	QPushButton* pb;
};

class CToolBox : public QScrollArea
{
	Q_OBJECT

public:
	CToolBox(QWidget* parent = 0);

	void addTool(const QString& name, QWidget* tool);

	CToolItem* getToolItem(int n);

private:
	QList<CToolItem*>	m_items;
};
