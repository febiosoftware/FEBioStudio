#pragma once

#include <QWidget>
#include <QLabel>
#include <vector>

class QVBoxLayout;

class WrapLabel : public QWidget
{
public:
	WrapLabel(QString text = "", QWidget* parent = nullptr);

	virtual void setText(QString text);

	virtual QString text();

protected:
	void resizeEvent(QResizeEvent *event) override;
	void setLengths();

protected:
	QVBoxLayout* layout;

	QString fullString;
	std::vector<QStringList> words;
	std::vector<std::vector<int>> lengths;
	std::vector<QLabel*> labels;
	int spaceSize;
	bool processEvent;

private:
	virtual void reflow(int width);
	virtual void addLabel(QString& text);
};
