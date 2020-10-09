#include <QTextBrowser>

class MultiLineLabel : public QTextBrowser
{
public:
	MultiLineLabel(QWidget *parent = nullptr);

	MultiLineLabel(const QString &text, QWidget *parent = nullptr);

	void setText(const QString &text);

	void resizeEvent(QResizeEvent *event) override;

private:
	void init();
};
