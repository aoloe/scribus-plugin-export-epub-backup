/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef EPUBEXPORTSTYLE_H
#define EPUBEXPORTSTYLE_H

#include <QObject>
#include <QDebug>

#include "scribusapi.h" // for SCRIBUS_API

class EpubExportStyle : public QObject
{
    Q_OBJECT

public:
	EpubExportStyle();
	~EpubExportStyle();
};

QDebug operator<<(QDebug dbg, const EpubExportStyle &style);

#endif // EPUBEXPORTSTYLE_H
