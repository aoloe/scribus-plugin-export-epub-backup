/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef EPUBEXPORTTEXT_H
#define EPUBEXPORTTEXT_H

#include <QObject>
#include <QDebug>

#include "scribusapi.h" // for SCRIBUS_API

class EpubExportText : public QObject
{
    Q_OBJECT

public:
	EpubExportText();
	~EpubExportText();
};

QDebug operator<<(QDebug dbg, const EpubExportText &text);

#endif // EPUBEXPORTTEXT_H
