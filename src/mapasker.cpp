/***************************************************************************
 *   Copyright (C) 2004-2007 by Albert Astals Cid                          *
 *   aacid@kde.org                                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "mapasker.h"

#include <klocale.h>
#include <kmessagebox.h>

#include <qlabel.h>
#include <qlayout.h>
#include <qscrollbar.h>
#include <qstring.h>

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>

#include "map.h"
#include "mapwidget.h"

#include <kdebug.h>

static QString guessLocaleCode()
{
    const QString &lang = KGlobal::locale() -> language();
    QString code;

    if ( lang.isEmpty() || lang == "POSIX" || lang == "C" )
        code = "en";
    else {
        QString dummy;
        KLocale::splitLocale(lang, code, dummy, dummy, dummy);
    }
    return code;
}

mapAsker::mapAsker(QWidget *parent, KGmap *m, QWidget *w, bool asker, uint count)
	: askWidget(parent, m, w, count, asker), p_asker(asker), p_firstShow(true)
{
	QVBoxLayout *lay = new QVBoxLayout(this);
	lay -> setMargin(0);
	lay -> setSpacing(0);

	p_mapWidget = new mapWidget(this);
	p_popupManager.setWidget(p_mapWidget);
	lay -> addWidget(p_mapWidget);

	p_shouldClearPopup = false;

	connect(p_mapWidget, SIGNAL(clicked(QRgb, const QPoint&)), this, SLOT(handleMapClick(QRgb, const QPoint&)));
	connect(p_mapWidget, SIGNAL(setMoveActionChecked(bool)), this, SIGNAL(setMoveActionChecked(bool)));
	connect(p_mapWidget, SIGNAL(setZoomActionChecked(bool)), this, SIGNAL(setZoomActionChecked(bool)));
	connect(p_mapWidget, SIGNAL(setMoveActionEnabled(bool)), this, SIGNAL(setMoveActionEnabled(bool)));

	if (asker)
	{
		QVBoxLayout *vbl = static_cast<QVBoxLayout*>(w -> layout());
		p_next = new QLabel(w);
		p_next -> setAlignment(Qt::AlignTop | Qt::AlignHCenter);
		p_next -> setWordWrap(true);
		p_fill = new QWidget(w);
		p_fill -> show();
		vbl -> addWidget(p_next);
		vbl -> addWidget(p_fill, 1);
		nextQuestion();
	}
	else
	{
		p_next = 0;
		p_fill = 0;

		p_wikiAccessMgr = new QNetworkAccessManager(this);
		connect(p_wikiAccessMgr, SIGNAL(finished(QNetworkReply*)),
			this, SLOT(receiveWikipediaData(QNetworkReply*)));

	}
}

mapAsker::~mapAsker()
{
	delete p_next;
	delete p_fill;
}

bool mapAsker::isAsker() const
{
	return p_answers;
}

void mapAsker::receiveWikipediaData(QNetworkReply *reply)
{
	if ( reply->error() == QNetworkReply::NoError )
	{
		QByteArray bigXml = reply->readAll();
		int closingTextTagPos = bigXml.lastIndexOf("</text>");
		if ( closingTextTagPos > 0 )
		{
			int start = -1;
			QString localeCode = guessLocaleCode();
			QString toSearchStart("\n[[");
			toSearchStart.append(localeCode);
			toSearchStart.append(":");
			start = bigXml.lastIndexOf(toSearchStart, closingTextTagPos);

			int end = -1;
			if ( start > 0 )
			{
				QString toSearchEnd("]]");
				end = bigXml.indexOf(toSearchEnd, start);
				if ( end > 0 )
				{
					int chunkOffset = toSearchStart.length();
					int chunkStart = start + chunkOffset;
					int chunkEnd = end - chunkStart;
					QString translated = QString::fromUtf8(bigXml.mid(chunkStart, chunkEnd).data());
					QString wikiLink = QString( "http://%1.wikipedia.org/wiki/%2" ).arg(localeCode).arg(translated);
					p_popupManager.updateLink(wikiLink);
				}
			}
			else
			{
				toSearchStart = "#REDIRECT [[";
				if ( (start = bigXml.lastIndexOf(toSearchStart, closingTextTagPos)) > 0 )
				{
					end = bigXml.indexOf(" ", start);
					if ( end > 0 )
					{
						int chunkOffset = toSearchStart.length();
						int chunkStart = start + chunkOffset;
						int chunkEnd = end - chunkStart;
						QString redirected = QString::fromUtf8(bigXml.mid(chunkStart, chunkEnd).data());

						QUrl url(QString("http://en.wikipedia.org/wiki/Special:Export/") + redirected);
						QNetworkRequest request;
						request.setUrl(url);
						request.setRawHeader("User-Agent", "KGeography user-bot");
						p_wikiAccessMgr->get(request);
					}
				}
			}
		}
	}
}

void mapAsker::mousePressEvent(QMouseEvent*)
{
	p_popupManager.clear();
}

void mapAsker::setMovement(bool b)
{
	p_mapWidget -> setMapMove(b);
	p_popupManager.clear();
}

void mapAsker::setZoom(bool b)
{
	askWidget::setZoom(b);
	p_mapWidget -> setMapZoom(b);
	p_popupManager.clear();
}

void mapAsker::setOriginalZoom()
{
	p_mapWidget -> setOriginalImage();
	p_popupManager.clear();
}

void mapAsker::setAutomaticZoom(bool automaticZoom)
{
	p_mapWidget -> setAutomaticZoom(automaticZoom);
	p_popupManager.clear();
}

void mapAsker::handleMapClick(QRgb c, const QPoint &p)
{
	QString aux, cap;
	aux = p_map -> getWhatIs(c, !p_asker);
	if (aux == "nothing")
		KMessageBox::error(this, i18nc("@info", "You have found a bug in a map."
					       " Please contact the author and tell the %1 map has nothing associated to color %2,%3,%4.",
					       p_map -> getFile(), qRed(c), qGreen(c), qBlue(c)));
	else if (p_shouldClearPopup)
	{
		p_popupManager.clear();
		p_shouldClearPopup = false;
	}
	else if (!p_asker)
	{
		QString flagFile = p_map -> getDivisionFlagFile(aux);
		if (p_map -> getDivisionCanAsk(aux, division::eCapital)) cap = p_map -> getDivisionCapital(aux);
		if (!cap.isEmpty()) cap = i18nc("@item Capital name in map popup", "%1",
						i18nc(p_map -> getFileName().toUtf8(), cap.toUtf8()));

		QString wikiLink;
		if (p_map -> getDivisionCanAsk(aux, division::eClick))
		{
			QString localeCode = guessLocaleCode();
			wikiLink = QString( "http://%1.wikipedia.org/wiki/" ).arg(localeCode);
			wikiLink.append(i18nc(p_map -> getFileName().toUtf8(), aux.toUtf8()));

			if ( localeCode != "en" )
			{
				QUrl url(QString("http://en.wikipedia.org/wiki/Special:Export/") + aux.replace(' ', '_'));
				QNetworkRequest request;
				request.setUrl(url);
				request.setRawHeader("User-Agent", "KGeography user-bot");
				p_wikiAccessMgr->get(request);
			}
		}

		aux = i18nc("@item Region name in map popup", "%1", i18nc(p_map -> getFileName().toUtf8(), aux.toUtf8()));
		
		if (!flagFile.isEmpty()) p_popupManager.show(aux, wikiLink, cap, p, flagFile);
		else if (!cap.isEmpty()) p_popupManager.show(aux, wikiLink, cap , p);
		else p_popupManager.show(aux, wikiLink, p);
	}
	else if (!aux.isEmpty())
	{
		p_currentAnswer.setAnswer(QColor(c));
		questionAnswered(aux == lastDivisionAsked());
		nextQuestion();
	}
}

void mapAsker::nextQuestionHook(const QString &division)
{
	QString divisionName = i18nc(p_map -> getFileName().toUtf8(), division.toUtf8());
	p_next -> setText(i18nc("@info:status", "Please click on:<nl/>%1", divisionName));
	p_currentAnswer.setQuestion(i18nc("@item:intable column Question, %1 is region name", "%1", i18nc(p_map -> getFileName().toUtf8(), division.toUtf8())));
	p_next -> show();
	p_currentAnswer.setCorrectAnswer(p_map -> getColor(division));
}

QString mapAsker::getQuestionHook() const
{
        QString divisionType = i18nc(p_map -> getFileName().toUtf8(), p_map->getDivisionsString().toUtf8());
        return i18nc("@title", "%1 in Map", divisionType);
}

void mapAsker::showEvent(QShowEvent *)
{
	if (p_firstShow)
	{
		p_mapWidget -> init(p_map -> getMapFile());
		p_firstShow = false;
	}
}

QSize mapAsker::mapSize() const
{
	return p_mapWidget -> mapSize();
}

#include "mapasker.moc"
