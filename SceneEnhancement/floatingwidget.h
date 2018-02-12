#pragma once
#include <QWidget>
#include <QTextEdit>
#include <QtWidgets/QRadioButton>

class FloatingWidget : public QWidget {
	Q_OBJECT

public:
	FloatingWidget(QWidget * parent = Q_NULLPTR);
	~FloatingWidget();
	QTextEdit *TextCatA;
	QTextEdit *TextCatB;
	QTextEdit *TextMediumPref;
	QTextEdit *TextDepthPref;
	QTextEdit *TextHeightPref;

	// preference type
	QRadioButton *RadioHeight;
	QRadioButton *RadioMedium;
	QRadioButton *RadioDepth;

	// relation type
	QRadioButton *RadioGreater;
	QRadioButton *RadioEqual;
	QRadioButton *RadioLess;

private slots :
	void insertPref();
	
};
