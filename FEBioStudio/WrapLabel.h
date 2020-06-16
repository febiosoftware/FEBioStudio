#pragma once

#include <QWidget>
#include <vector>

class QVBoxLayout;

class WrapLabel : public QWidget
{
public:
	WrapLabel(QString text, QWidget* parent = nullptr);

protected:
	void resizeEvent(QResizeEvent *event) override;

private:
	QVBoxLayout* layout;

	QStringList words;
	std::vector<int> lengths;
	std::vector<QLabel*> labels;
	int spaceSize;
	bool processEvent;

};
