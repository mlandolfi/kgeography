/***************************************************************************
 *   Copyright (C) 2004 by Albert Astals Cid                               *
 *   tsdgeos@terra.es                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef MAPASKER_H
#define MAPASKER_H

#include "askwidget.h"
#include "popupmanager.h"

class QLabel;
class QScrollBar;

class flagDivisionAsker;
class infoWidget;
class mapWidget;

class mapAsker : public askWidget
{
Q_OBJECT
	public:
		mapAsker(QWidget *parent, map *m, QWidget *w, bool asker, uint count = 0);
	
	protected:
		void clean();
		void nextQuestionHook(QString division);
		void mousePressEvent(QMouseEvent *e);

	public slots:
		void setMovement(bool b);
		void setZoom(bool b);
	
	private slots:
		void handleMapClick(QRgb c, const QPoint &p);
		void showScrollBars(bool b);
		void setScrollBarsPosition(int x, int y);
		void setScrollBarsVisibleSize(int w, int h);
		void setScrollBarsMaximumSize(int w, int h);
	
	private:
		QLabel *p_next;
		QScrollBar *p_hsb, *p_vsb;
		int p_hsbms, p_vsbms; //H and V maximum size for scrollbars
		
		infoWidget *p_infoWidget;
		mapWidget *p_mapWidget;
		
		popupManager p_popupManager;
		bool p_shouldClearPopup, p_asker;
};

#endif
