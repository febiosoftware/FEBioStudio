#pragma once

#include <QWidget>
#include <vector>

class QVBoxLayout;

class WrapLabel : public QWidget
{
public:
	WrapLabel(QString text = "", QWidget* parent = nullptr);

	void setText(QString text);

	QString text();

protected:
	void resizeEvent(QResizeEvent *event) override;

private:
	void reflow(int width);
	void addLabel(QString& text);

private:
	QVBoxLayout* layout;

	QString fullString;
	std::vector<QStringList> words;
	std::vector<std::vector<int>> lengths;
	std::vector<QLabel*> labels;
	int spaceSize;
	bool processEvent;
};
