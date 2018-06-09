#pragma once
#include <QWidget>
#include <QLabel>
#include <QtWidgets/QRadioButton>
#include <QTextEdit>

class FloatingWidget : public QWidget {
	Q_OBJECT

public:
	FloatingWidget(QWidget * parent = Q_NULLPTR);
	~FloatingWidget();
	QLabel *TextCatA;
	QLabel *TextCatB;
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
