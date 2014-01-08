/***************************************************************************
 *   Copyright (C) 2004-2006 by Albert Astals Cid                          *
 *   aacid@kde.org                                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "mapchooser.h"

#include <klocalizedstring.h>
#include <kglobal.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <klistwidgetsearchline.h>

#include <qimage.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlistwidget.h>
#include <qdialogbuttonbox.h>
#include <qpushbutton.h>

#include "settings.h"

bool myLessThan(const QString &s1, const QString &s2)
{
	return s1.localeAwareCompare(s2) < 0;
}

mapChooser::mapChooser(QWidget *parent) : QDialog(parent)
{
	setWindowTitle(i18n("Choose Map to Use"));

	QVBoxLayout *mainLayout = new QVBoxLayout();

	QHBoxLayout *horizontalLayout = new QHBoxLayout();

	QVBoxLayout *listFilterLayout = new QVBoxLayout();
	p_listBox = new QListWidget();
	KListWidgetSearchLine *searchLine = new KListWidgetSearchLine(this, p_listBox);
	searchLine->setPlaceholderText(i18n("Filter Maps"));
	listFilterLayout -> addWidget(searchLine);
	listFilterLayout -> addWidget(p_listBox);

	horizontalLayout->addLayout(listFilterLayout);

	p_imageContainer = new QLabel();
	p_imageContainer -> setFixedSize(300, 225);
	p_imageContainer -> setAlignment(Qt::AlignCenter);
	horizontalLayout -> addWidget(p_imageContainer);

	mainLayout->addLayout(horizontalLayout);

	QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
									 | QDialogButtonBox::Cancel);
	connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
	connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
	mainLayout->addWidget(buttonBox);

	// FIXME: KGlobal::dirs() is deprecated
	QStringList list = KGlobal::dirs() -> findAllResources("appdata", "*.kgm");
	QStringList::iterator it;
	QStringList texts;
	QSet<QString> alreadySeens;
	QStringList errorTexts;
	QString lastMapFile = kgeographySettings::self() -> lastMap();
	QString stringToSelect;
	KGmap *m;
	for(it = list.begin(); it != list.end(); ++it)
	{
		QString mapFilename = *it;
		m = p_reader.parseMap(mapFilename);
		if (!m)
		{
			errorTexts << i18n("Error parsing %1: %2", mapFilename, p_reader.getError());
		}
		else
		{
			QString text = i18nc(m -> getFileName().toUtf8(), m -> getName().toUtf8());
			// avoid multiple and should guarantee that first in KDEDIRS is chosen)
			if (!alreadySeens.contains(text)) {
				texts << text;
				alreadySeens.insert(text);
				p_maps.insert(text, m);
				if ( mapFilename == lastMapFile )
				{
					stringToSelect = text;
				}
			}
		}
	}

	if ( errorTexts.size() > 0 )
	{
		KMessageBox::errorList(this, i18n("Error parsing"), errorTexts);
	}

	connect(p_listBox, SIGNAL(currentTextChanged(const QString&)), this, SLOT(putImage(const QString&)));
	connect(p_listBox, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(accept()));
	
	qSort(texts.begin(), texts.end(), myLessThan);
	foreach(const QString &text, texts) p_listBox -> addItem(text);
	if (p_listBox -> count() == 0)
		buttonBox->button(QDialogButtonBox::Ok)->setDisabled(true);
	else {
		QList<QListWidgetItem *> itemsWhere = p_listBox->findItems(stringToSelect, Qt::MatchExactly);
		if ( itemsWhere.size() > 0 )
			p_listBox->setCurrentItem(itemsWhere[0]);
		else
			p_listBox -> setCurrentRow(0);
	}

	setLayout(mainLayout);

	p_listBox -> setFocus();
}

mapChooser::~mapChooser()
{
	qDeleteAll(p_maps.values());
}

KGmap *mapChooser::getMap()
{
	KGmap *m;
	m = p_maps[p_listBox -> currentItem() -> text()];
	p_maps.remove(p_listBox -> currentItem() -> text());
	return m;
}

void mapChooser::putImage(const QString &mapName)
{
	KGmap *m;
	m = p_maps[mapName];
	QImage image(m -> getMapFile());
	image = image.scaled(300, 225, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	p_imageContainer -> setPixmap( QPixmap::fromImage(image) );
}

#include "mapchooser.moc"
