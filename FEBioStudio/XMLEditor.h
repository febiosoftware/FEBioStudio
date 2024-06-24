#pragma once
#include <QPlainTextEdit>

class CMainWindow;

class XMLEditor : public QPlainTextEdit
{
	Q_OBJECT

public:
	XMLEditor(CMainWindow* parent);

	void lineNumberAreaPaintEvent(QPaintEvent* event);
	int lineNumberAreaWidth();

	void SetDocument(QTextDocument* doc, const QString& title = "");

	void toggleLineComment();
	void duplicateLine();
	void deleteLine();

protected:
	void resizeEvent(QResizeEvent* event) override;
	void paintEvent(QPaintEvent* event) override;
	void wheelEvent(QWheelEvent* ev) override;

private slots:
	void updateLineNumberAreaWidth(int newBlockCount);
	void highlightCurrentLine();
	void updateLinearNumberArea(const QRect&, int);

private:
	CMainWindow*		m_wnd;
	QWidget*			lineNumberArea;
	QPair<int, int>		m_countCache;
};

class LineNumberArea : public QWidget
{
public:
	LineNumberArea(XMLEditor* editor) : QWidget(editor){ codeEditor = editor; }

	QSize sizeHint() const 
	{
		return QSize(codeEditor->lineNumberAreaWidth(), 0);
	}

protected:
	void paintEvent(QPaintEvent* event)
	{
		codeEditor->lineNumberAreaPaintEvent(event);
	}

private:
	XMLEditor*	codeEditor;
};
