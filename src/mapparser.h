/***************************************************************************
 *   Copyright (C) 2004 by Albert Astals Cid                               *
 *   tsdgeos@terra.es                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef MAPPARSER_H
#define MAPPARSER_H

#include <tqxml.h>

class division;
class KGmap;

class mapReader : public QXmlSimpleReader
{
	public:
		mapReader();
		
		bool parseMap(const TQString &path);
		TQString getError();
		KGmap *getMap();
	
	private:
		TQString p_error;
		KGmap *p_map;
};

class mapParser : public QXmlDefaultHandler
{
	public:
		mapParser(KGmap *m, const TQString &path);
		bool startDocument();
		bool startElement(const TQString&, const TQString&, const TQString&, const TQXmlAttributes&);
		bool endElement(const TQString &, const TQString &name, const TQString &);
		bool characters(const TQString &ch);
		bool endDocument();
		TQString errorString();
	
	private:
		TQString getPreviousTag() const;
		void removeLastTag();
		
		division *p_division;
		int p_red, p_green, p_blue;
		KGmap *p_map;
		bool p_allowChars, p_mapNameSet, p_mapFileSet;
		bool p_divisionNameSet, p_colorSet, p_divisionIgnoreSet, p_flagFileSet, p_capitalSet;
		const TQString &p_path;
		TQString p_error, p_contents, p_previousTags;
};

#endif
