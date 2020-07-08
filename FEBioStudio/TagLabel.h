#pragma once

#include "WrapLabel.h"

class TagLabel : public WrapLabel
{
	Q_OBJECT

public:
	TagLabel(QString text = "", QWidget* parent = nullptr);

	void setTagList(QStringList tags);

	QString text() override;

signals:
	void linkActivated(const QString& link);

private:
	void setText(QString text) override {};
	void reflow(int width) override;
	void addLabel(QString& text) override;

};
