#pragma once
#include <QPlainTextEdit>

class CMainWindow;

class CTextEditor : public QPlainTextEdit
{
	Q_OBJECT

public:
	CTextEditor(CMainWindow* parent);

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
	LineNumberArea(CTextEditor* editor) : QWidget(editor){ m_txtEditor = editor; }

	QSize sizeHint() const 
	{
		return QSize(m_txtEditor->lineNumberAreaWidth(), 0);
	}

protected:
	void paintEvent(QPaintEvent* event)
	{
		m_txtEditor->lineNumberAreaPaintEvent(event);
	}

private:
	CTextEditor*	m_txtEditor;
};
