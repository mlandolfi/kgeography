/***************************************************************************
 *   Copyright (C) 2004-2005 by Albert Astals Cid                          *
 *   tsdgeos@terra.es                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef MYPOPUP_H
#define MYPOPUP_H

#include <qframe.h>

class myPopup : public QFrame
{
Q_OBJECT
	friend class popupManager;
	
	private:
		myPopup(const QString &text, const QString &text2, const QString &flagFile, QWidget *parent);
		myPopup(const QString &text1, const QString &text2, QWidget *parent);
		myPopup(const QString &text, QWidget *parent);
	
	signals:
		void deleteMe();

	protected:
		void mousePressEvent(QMouseEvent *);
};

#endif
