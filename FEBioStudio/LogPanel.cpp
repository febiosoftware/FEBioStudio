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

#include "stdafx.h"
#include "ui_logpanel.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QRegularExpression>

void parseEscapeSequence(int attribute, QListIterator< QString > & i, QTextCharFormat & textCharFormat, QTextCharFormat const & defaultTextCharFormat);

CLogPanel::CLogPanel(QWidget* parent) : QWidget(parent), ui(new Ui::CLogPanel)
{
	ui->setupUi(this);
}

void CLogPanel::Clear(LogTarget trg)
{
	switch (trg)
	{
	case LogTarget::FBS_LOG   : ui->clearLog(0); break;
	case LogTarget::FEBIO_LOG : ui->clearLog(1); break;
	case LogTarget::BUILD_LOG : ui->clearLog(2); break;
	}
}

void CLogPanel::on_logSave_clicked(bool b)
{
	QString txt = ui->currentTxt()->toPlainText();

	QString fileName = QFileDialog::getSaveFileName(this, "Save", "", "Text Files (*.txt)");
	if (fileName.isEmpty() == false)
	{
		// convert to const char*
		std::string sfile = fileName.toStdString();
		const char* szfile = sfile.c_str();

		// open the file
		FILE* fp = fopen(szfile, "wb");
		if (fp == 0)
		{
			QMessageBox::critical(this, "FEBio Studio", "Failed saving log");
			return;
		}

		// convert data to string
		std::string s = txt.toStdString();
		size_t len = s.length();
		size_t nwritten = fwrite(s.c_str(), sizeof(char), len, fp);

		// close the file
		fclose(fp);
	}
}

void CLogPanel::on_logClear_clicked(bool b)
{
	ui->currentTxt()->clear();
}

void CLogPanel::on_combo_currentIndexChanged(int i)
{
	ui->setOutput(i);
}

void CLogPanel::ShowLog(LogTarget trg)
{
	ui->showTxt((int)trg);
}

void CLogPanel::AddText(const QString& txt, CLogPanel::LogTarget trg)
{
	int n = (int)trg;

	QTextDocument * document = ui->txt[n]->document();
	QRegularExpression const escapeSequenceExpression(R"(\x1B\[([\d;]+)m)");
	QTextCursor cursor(document);
	cursor.movePosition(QTextCursor::End);
	QTextCharFormat textCharFormat = cursor.charFormat();
	QRegularExpressionMatch match = escapeSequenceExpression.match(txt);
	int offset = match.capturedStart();
	cursor.insertText(txt.mid(0, offset), textCharFormat);
	while (!(offset < 0)) {
		int previousOffset = offset + match.capturedLength();
		QStringList capturedTexts = match.capturedTexts().back().split(';');
		QListIterator< QString > i(capturedTexts);
		while (i.hasNext()) {
			bool ok = false;
			int attribute = i.next().toInt(&ok);
			Q_ASSERT(ok);
			parseEscapeSequence(attribute, i, textCharFormat, ui->defaultTextCharFormat);
		}
		match = escapeSequenceExpression.match(txt, previousOffset);
		offset = match.capturedStart();
		if (offset < 0) {
			cursor.insertText(txt.mid(previousOffset), textCharFormat);
		} else {
			cursor.insertText(txt.mid(previousOffset, offset - previousOffset), textCharFormat);
		}
	}
	cursor.setCharFormat(ui->defaultTextCharFormat);
	cursor.movePosition(QTextCursor::End);

	// NOTE: Calling this sometimes causes a crash. Not sure why. 
	ui->txt[n]->ensureCursorVisible();
}

void CLogPanel::parseEscapeSequence(int attribute, QListIterator< QString > & i, QTextCharFormat & textCharFormat, QTextCharFormat const & defaultTextCharFormat)
{
	switch (attribute) {
	case 0 : { // Normal/Default (reset all attributes)
		textCharFormat = defaultTextCharFormat;
		break;
	}
	case 1 : { // Bold/Bright (bold or increased intensity)
		textCharFormat.setFontWeight(QFont::Bold);
		break;
	}
	case 2 : { // Dim/Faint (decreased intensity)
		textCharFormat.setFontWeight(QFont::Light);
		break;
	}
	case 3 : { // Italicized (italic on)
		textCharFormat.setFontItalic(true);
		break;
	}
	case 4 : { // Underscore (single underlined)
		textCharFormat.setUnderlineStyle(QTextCharFormat::SingleUnderline);
		textCharFormat.setFontUnderline(true);
		break;
	}
	case 5 : { // Blink (slow, appears as Bold)
		textCharFormat.setFontWeight(QFont::Bold);
		break;
	}
	case 6 : { // Blink (rapid, appears as very Bold)
		textCharFormat.setFontWeight(QFont::Black);
		break;
	}
	case 7 : { // Reverse/Inverse (swap foreground and background)
		QBrush foregroundBrush = textCharFormat.foreground();
		textCharFormat.setForeground(textCharFormat.background());
		textCharFormat.setBackground(foregroundBrush);
		break;
	}
	case 8 : { // Concealed/Hidden/Invisible (usefull for passwords)
		textCharFormat.setForeground(textCharFormat.background());
		break;
	}
	case 9 : { // Crossed-out characters
		textCharFormat.setFontStrikeOut(true);
		break;
	}
	case 10 : { // Primary (default) font
		textCharFormat.setFont(defaultTextCharFormat.font());
		break;
	}
	case 11:
	case 12:
	case 13:
	case 14:
	case 15:
	case 16:
	case 17:
	case 18:
	case 19 : {
		QFontDatabase fontDatabase;
		QString fontFamily = textCharFormat.fontFamily();
		QStringList fontStyles = fontDatabase.styles(fontFamily);
		int fontStyleIndex = attribute - 11;
		if (fontStyleIndex < fontStyles.length()) {
			textCharFormat.setFont(fontDatabase.font(fontFamily, fontStyles.at(fontStyleIndex), textCharFormat.font().pointSize()));
		}
		break;
	}
	case 20 : { // Fraktur (unsupported)
		break;
	}
	case 21 : { // Set Bold off
		textCharFormat.setFontWeight(QFont::Normal);
		break;
	}
	case 22 : { // Set Dim off
		textCharFormat.setFontWeight(QFont::Normal);
		break;
	}
	case 23 : { // Unset italic and unset fraktur
		textCharFormat.setFontItalic(false);
		break;
	}
	case 24 : { // Unset underlining
		textCharFormat.setUnderlineStyle(QTextCharFormat::NoUnderline);
		textCharFormat.setFontUnderline(false);
		break;
	}
	case 25 : { // Unset Blink/Bold
		textCharFormat.setFontWeight(QFont::Normal);
		break;
	}
	case 26 : { // Reserved
		break;
	}
	case 27 : { // Positive (non-inverted)
		QBrush backgroundBrush = textCharFormat.background();
		textCharFormat.setBackground(textCharFormat.foreground());
		textCharFormat.setForeground(backgroundBrush);
		break;
	}
	case 28 : {
		textCharFormat.setForeground(defaultTextCharFormat.foreground());
		textCharFormat.setBackground(defaultTextCharFormat.background());
		break;
	}
	case 29 : {
		textCharFormat.setUnderlineStyle(QTextCharFormat::NoUnderline);
		textCharFormat.setFontUnderline(false);
		break;
	}
	case 30:
	case 31:
	case 32:
	case 33:
	case 34:
	case 35:
	case 36:
	case 37 : {
		int colorIndex = attribute - 30;
		QColor color;
		if (QFont::Normal < textCharFormat.fontWeight()) {
			switch (colorIndex) {
			case 0 : {
				color = Qt::darkGray;
				break;
			}
			case 1 : {
				color = Qt::red;
				break;
			}
			case 2 : {
				color = Qt::green;
				break;
			}
			case 3 : {
				color = Qt::yellow;
				break;
			}
			case 4 : {
				color = Qt::blue;
				break;
			}
			case 5 : {
				color = Qt::magenta;
				break;
			}
			case 6 : {
				color = Qt::cyan;
				break;
			}
			case 7 : {
				color = Qt::white;
				break;
			}
			default : {
				Q_ASSERT(false);
			}
			}
		} else {
			switch (colorIndex) {
			case 0 : {
				color = Qt::black;
				break;
			}
			case 1 : {
				color = Qt::darkRed;
				break;
			}
			case 2 : {
				color = Qt::darkGreen;
				break;
			}
			case 3 : {
				color = Qt::darkYellow;
				break;
			}
			case 4 : {
				color = Qt::darkBlue;
				break;
			}
			case 5 : {
				color = Qt::darkMagenta;
				break;
			}
			case 6 : {
				color = Qt::darkCyan;
				break;
			}
			case 7 : {
				color = Qt::lightGray;
				break;
			}
			default : {
				Q_ASSERT(false);
			}
			}
		}
		textCharFormat.setForeground(color);
		break;
	}
	case 38 : {
		if (i.hasNext()) {
			bool ok = false;
			int selector = i.next().toInt(&ok);
			Q_ASSERT(ok);
			QColor color;
			switch (selector) {
			case 2 : {
				if (!i.hasNext()) {
					break;
				}
				int red = i.next().toInt(&ok);
				Q_ASSERT(ok);
				if (!i.hasNext()) {
					break;
				}
				int green = i.next().toInt(&ok);
				Q_ASSERT(ok);
				if (!i.hasNext()) {
					break;
				}
				int blue = i.next().toInt(&ok);
				Q_ASSERT(ok);
				color.setRgb(red, green, blue);
				break;
			}
			case 5 : {
				if (!i.hasNext()) {
					break;
				}
				int index = i.next().toInt(&ok);
				Q_ASSERT(ok);
				switch (index) {
				case 0x00:
				case 0x01:
				case 0x02:
				case 0x03:
				case 0x04:
				case 0x05:
				case 0x06:
				case 0x07 : { // 0x00-0x07:  standard colors (as in ESC [ 30..37 m)
				return parseEscapeSequence(index - 0x00 + 30, i, textCharFormat, defaultTextCharFormat);
				}
				case 0x08:
				case 0x09:
				case 0x0A:
				case 0x0B:
				case 0x0C:
				case 0x0D:
				case 0x0E:
				case 0x0F : { // 0x08-0x0F:  high intensity colors (as in ESC [ 90..97 m)
				return parseEscapeSequence(index - 0x08 + 90, i, textCharFormat, defaultTextCharFormat);
				}
				default:
				if ((index >= 0x10) && (index <= 0xE7))
				{ // 0x10-0xE7:  6*6*6=216 colors: 16 + 36*r + 6*g + b (0≤r,g,b≤5)
					index -= 0x10;
				int red = index % 6;
				index /= 6;
				int green = index % 6;
				index /= 6;
				int blue = index % 6;
				index /= 6;
				Q_ASSERT(index == 0);
				color.setRgb(red, green, blue);
				}
				else if ((index >= 0xE8)&&(index <= 0xFF))
				{ // 0xE8-0xFF:  grayscale from black to white in 24 steps
					qreal intensity = qreal(index - 0xE8) / (0xFF - 0xE8);
				color.setRgbF(intensity, intensity, intensity);
				}
				break;
				}
				textCharFormat.setForeground(color);
				break;
			}
			default : {
				break;
			}
			}
		}
		break;
	}
	case 39 : {
		textCharFormat.setForeground(defaultTextCharFormat.foreground());
		break;
	}
	case 40:
	case 41:
	case 42:
	case 43:
	case 44:
	case 45:
	case 46:
	case 47 : {
		int colorIndex = attribute - 40;
		QColor color;
		switch (colorIndex) {
		case 0 : {
			color = Qt::darkGray;
			break;
		}
		case 1 : {
			color = Qt::red;
			break;
		}
		case 2 : {
			color = Qt::green;
			break;
		}
		case 3 : {
			color = Qt::yellow;
			break;
		}
		case 4 : {
			color = Qt::blue;
			break;
		}
		case 5 : {
			color = Qt::magenta;
			break;
		}
		case 6 : {
			color = Qt::cyan;
			break;
		}
		case 7 : {
			color = Qt::white;
			break;
		}
		default : {
			Q_ASSERT(false);
		}
		}
		textCharFormat.setBackground(color);
		break;
	}
	case 48 : {
		if (i.hasNext()) {
			bool ok = false;
			int selector = i.next().toInt(&ok);
			Q_ASSERT(ok);
			QColor color;
			switch (selector) {
			case 2 : {
				if (!i.hasNext()) {
					break;
				}
				int red = i.next().toInt(&ok);
				Q_ASSERT(ok);
				if (!i.hasNext()) {
					break;
				}
				int green = i.next().toInt(&ok);
				Q_ASSERT(ok);
				if (!i.hasNext()) {
					break;
				}
				int blue = i.next().toInt(&ok);
				Q_ASSERT(ok);
				color.setRgb(red, green, blue);
				break;
			}
			case 5 : {
				if (!i.hasNext()) {
					break;
				}
				int index = i.next().toInt(&ok);
				Q_ASSERT(ok);
				switch (index) {
				case 0x00:
				case 0x01:
				case 0x02:
				case 0x03:
				case 0x04:
				case 0x05:
				case 0x06:
				case 0x07 : { // 0x00-0x07:  standard colors (as in ESC [ 40..47 m)
				return parseEscapeSequence(index - 0x00 + 40, i, textCharFormat, defaultTextCharFormat);
				}
				case 0x08:
				case 0x09:
				case 0x0A:
				case 0x0B:
				case 0x0C:
				case 0x0D:
				case 0x0E:
				case 0x0F : { // 0x08-0x0F:  high intensity colors (as in ESC [ 100..107 m)
				return parseEscapeSequence(index - 0x08 + 100, i, textCharFormat, defaultTextCharFormat);
				}
				default:
				if ((index >= 0x10)&&(index <= 0xE7))
				{ // 0x10-0xE7:  6*6*6=216 colors: 16 + 36*r + 6*g + b (0≤r,g,b≤5)
					index -= 0x10;
				int red = index % 6;
				index /= 6;
				int green = index % 6;
				index /= 6;
				int blue = index % 6;
				index /= 6;
				Q_ASSERT(index == 0);
				color.setRgb(red, green, blue);
				}
				else if ((index >= 0xE8)&&(index <= 0xFF))
				{ // 0xE8-0xFF:  grayscale from black to white in 24 steps
					qreal intensity = qreal(index - 0xE8) / (0xFF - 0xE8);
					color.setRgbF(intensity, intensity, intensity);
				}
				break;
				}
				textCharFormat.setBackground(color);
				break;
			}
			default : {
				break;
			}
			}
		}
		break;
	}
	case 49 : {
		textCharFormat.setBackground(defaultTextCharFormat.background());
		break;
	}
	case 90:
	case 91:
	case 92:
	case 93:
	case 94:
	case 95:
	case 96:
	case 97 : {
		int colorIndex = attribute - 90;
		QColor color;
		switch (colorIndex) {
		case 0 : {
			color = Qt::darkGray;
			break;
		}
		case 1 : {
			color = Qt::red;
			break;
		}
		case 2 : {
			color = Qt::green;
			break;
		}
		case 3 : {
			color = Qt::yellow;
			break;
		}
		case 4 : {
			color = Qt::blue;
			break;
		}
		case 5 : {
			color = Qt::magenta;
			break;
		}
		case 6 : {
			color = Qt::cyan;
			break;
		}
		case 7 : {
			color = Qt::white;
			break;
		}
		default : {
			Q_ASSERT(false);
		}
		}
		color.setRedF(color.redF() * 0.8);
		color.setGreenF(color.greenF() * 0.8);
		color.setBlueF(color.blueF() * 0.8);
		textCharFormat.setForeground(color);
		break;
	}
	case 100:
	case 101:
	case 102:
	case 103:
	case 104:
	case 105:
	case 106:
	case 107 : {
		int colorIndex = attribute - 100;
		QColor color;
		switch (colorIndex) {
		case 0 : {
			color = Qt::darkGray;
			break;
		}
		case 1 : {
			color = Qt::red;
			break;
		}
		case 2 : {
			color = Qt::green;
			break;
		}
		case 3 : {
			color = Qt::yellow;
			break;
		}
		case 4 : {
			color = Qt::blue;
			break;
		}
		case 5 : {
			color = Qt::magenta;
			break;
		}
		case 6 : {
			color = Qt::cyan;
			break;
		}
		case 7 : {
			color = Qt::white;
			break;
		}
		default : {
			Q_ASSERT(false);
		}
		}
		color.setRedF(color.redF() * 0.8);
		color.setGreenF(color.greenF() * 0.8);
		color.setBlueF(color.blueF() * 0.8);
		textCharFormat.setBackground(color);
		break;
	}
	default : {
		break;
	}
	}
}
