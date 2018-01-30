#pragma once
#include <QtCore/QVector>

class PercentColor;
class QColor;
enum ColorDistanceType
{
	HSV,
	LAB,
	YIQ,
	RGB,
	MIX,
};
enum SampleType
{	
	Pos,
	Neg
};
class ColorPalette
{
public:
	static double GetColorPaletteDistance(ColorPalette *cp1, ColorPalette *cp2, bool useorder = false);
	static double GetColorDistance(QColor &c1, QColor &c2, ColorDistanceType type = LAB);
	static double GetPercentColorDistance(PercentColor &c1, PercentColor &c2, ColorDistanceType type = LAB);
	ColorPalette();
	ColorPalette(QVector<QColor> colors);
	ColorPalette(QVector<QColor> colors, QString filepath);
	QVector<QColor> Colors;
	int ClusterIndex;
	SampleType SampleType;
	QString FilePath;

private:


	
};
