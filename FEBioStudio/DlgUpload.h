#pragma once
#include <QDialog>
#include <vector>
#include "LaunchConfig.h"

namespace Ui {
	class CDlgUpload;
}

class QStringList;
class CPublicationWidget;

//class ClickableLabel : public QLabel {
//    Q_OBJECT
//
//public:
//    explicit ClickableLabel(QWidget* parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());
//    ~ClickableLabel();
//
//signals:
//    void clicked();
//
//protected:
//    void mousePressEvent(QMouseEvent* event);
//
//    void enterEvent(QEvent *ev) override
//    {
////    	setCursor(Qt::PointingHandCursor);
//    	setStyleSheet("QLabel { color : red; }");
//    }
//
//    void leaveEvent(QEvent *ev) override
//    {
////    	setCursor(Qt::ArrowCursor);
//    	setStyleSheet("QLabel { color : black; }");
//    }
//
//};
//
//class TagLabel : public QFrame
//{
//	Q_OBJECT
//
//public:
//	QLabel* label;
//	ClickableLabel* remove;
//
//	TagLabel(QString text, QWidget* parent = NULL);
//public slots:
//	void deleteThis();
//
//
//};

class CDlgUpload : public QDialog
{
	Q_OBJECT

public:
	CDlgUpload(QWidget* parent);

	void setName(QString name);
	void setDescription(QString desc);
	void setOwner(QString owner);
	void setVersion(QString version);
	void setTags(QStringList& tags);
	void setPublications(const std::vector<CPublicationWidget*>& pub);

	void setTagList(QStringList& tags);


	QString getName();
	QString getDescription();
	QString getOwner();
	QString getVersion();
	QStringList getTags();
	QList<QVariant> getPublicationInfo();


public slots:
	void on_addTagBtn_clicked();
	void on_delTagBtn_clicked();

private:
	Ui::CDlgUpload*	ui;
	void UpdateLaunchConfigBox(int index);
};