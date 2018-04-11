#include "floatingwidget.h"
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QPushButton>


FloatingWidget::FloatingWidget(QWidget * parent) : QWidget(parent) 
{
	setWindowFlags(Qt::FramelessWindowHint);
	setWindowFlags(Qt::WindowStaysOnTopHint);
	setWindowFlags(Qt::Tool);
	//setAttribute(Qt::WA_TranslucentBackground, true);
	QPalette myPalette;
	QColor myColor(255,255,255);
	//myColor.setAlphaF(0.1);
	myPalette.setBrush(backgroundRole(), myColor);
	this->setPalette(myPalette);
	this->setAutoFillBackground(true);	
	
	auto main_layout = new QVBoxLayout;
	
	// type of preferences
	QGroupBox *type_groupBox = new QGroupBox(tr("Type of Preference"));
	RadioMedium = new QRadioButton(tr("Medium Preference"));
	RadioDepth = new QRadioButton(tr("Depth Preference"));
	RadioHeight = new QRadioButton(tr("Height Preference"));	
	RadioMedium->setChecked(true);
	QVBoxLayout *vbox = new QVBoxLayout;
	vbox->addWidget(RadioMedium);
	vbox->addWidget(RadioDepth);
	vbox->addWidget(RadioHeight);
	//vbox->addStretch(1);
	vbox->setSpacing(5);
	type_groupBox->setLayout(vbox);

	// category A
	QGroupBox *cata_gb = new QGroupBox(tr("Category A"));
	QHBoxLayout *cata_hbox = new QHBoxLayout;
	TextCatA = new QTextEdit();
	//TextCatA->setText("Hello, world!");
	cata_hbox->addWidget(TextCatA);	
	cata_gb->setLayout(cata_hbox);
		
	// category B
	QGroupBox *catb_gb = new QGroupBox(tr("Category B"));
	QHBoxLayout *catb_hbox = new QHBoxLayout;
	TextCatB = new QTextEdit();
	//TextCatB->setText("notebook");
	catb_hbox->addWidget(TextCatB);
	catb_gb->setLayout(catb_hbox);

	// the relationship
	QGroupBox *relation_groupBox = new QGroupBox(tr("Relation between A and B"));
	RadioGreater = new QRadioButton(tr(">"));
	RadioEqual = new QRadioButton(tr("="));
	RadioLess = new QRadioButton(tr("<"));
	RadioMedium->setChecked(true);
	QHBoxLayout *hbox = new QHBoxLayout;
	hbox->addWidget(RadioGreater);
	hbox->addWidget(RadioEqual);
	hbox->addWidget(RadioLess);
	hbox->addSpacing(5);
	relation_groupBox->setLayout(hbox);

	QPushButton *btn_insert = new QPushButton(tr("&Insert"));

	// Medium Preferences
	QGroupBox *m_gb = new QGroupBox(tr("Medium preferences"));
	QHBoxLayout *mp_hbox = new QHBoxLayout;
	TextMediumPref = new QTextEdit();
	//TextMediumPref->setText(tr("medium"));
	mp_hbox->addWidget(TextMediumPref);
	m_gb->setLayout(mp_hbox);

	// Depth Preferences
	QGroupBox *d_gb = new QGroupBox(tr("Depth preferences"));
	QHBoxLayout *dp_hbox = new QHBoxLayout;
	TextDepthPref = new QTextEdit();
	//TextDepthPref->setText(tr("depth"));
	dp_hbox->addWidget(TextDepthPref);
	d_gb->setLayout(dp_hbox);

	// Height Preferences
	QGroupBox *h_gb = new QGroupBox(tr("Height preferences"));
	QHBoxLayout *hp_hbox = new QHBoxLayout;
	TextHeightPref = new QTextEdit();
	//TextHeightPref->setText(tr("height"));
	hp_hbox->addWidget(TextHeightPref);
	h_gb->setLayout(hp_hbox);

	main_layout->addWidget(type_groupBox);
	main_layout->addWidget(cata_gb);	
	main_layout->addWidget(catb_gb);
	main_layout->addWidget(relation_groupBox);
	main_layout->addWidget(btn_insert);
	main_layout->addWidget(m_gb);
	main_layout->addWidget(d_gb);
	main_layout->addWidget(h_gb);
	
	setLayout(main_layout);

	// method
	connect(btn_insert, SIGNAL(clicked()), this, SLOT(insertPref()));
}

FloatingWidget::~FloatingWidget() {
	
}

void FloatingWidget::insertPref()
{
	QTextEdit *cur_pref_text;
	if (RadioHeight->isChecked())
		cur_pref_text = TextHeightPref;
	if (RadioDepth->isChecked())
		cur_pref_text = TextDepthPref;	
	if (RadioMedium->isChecked())
		cur_pref_text = TextMediumPref;
	auto org_txt = cur_pref_text->toPlainText();
	auto cat_a = TextCatA->toPlainText();
	auto cat_b = TextCatB->toPlainText();
	QString relation;
	if (RadioGreater->isChecked())
		relation = tr(" > ");
	if (RadioEqual->isChecked())
		relation = tr(" = ");
	if (RadioLess->isChecked())
		relation = tr(" < ");
	auto new_pref = cat_a + relation + cat_b;
	QString new_txt = org_txt + new_pref + tr("\n");
	cur_pref_text->setText(new_txt);
}
