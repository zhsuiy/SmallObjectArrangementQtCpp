#pragma once
#include <QtCore/QVector>
#include "ColorPalette.h"
#include <QtCore/qmap.h>

namespace VisualizationTool
{

	void DrawAllFurnitureClusters(QMap<QString, QMap<int, QVector<ColorPalette*>>> all_furnitures);
	void DrawAllFurnitureClustersInOrder(QMap<QString, QMap<int, QVector<ColorPalette*>>> all_furnitures);
	void DrawClusterColors(QString furnituretype, QMap<int,QVector<ColorPalette*>> colors);
	void DrawClusterColorsInOrder(QString furnituretype, QMap<int, QVector<ColorPalette*>> colors);
	
}
