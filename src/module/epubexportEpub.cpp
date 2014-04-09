/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/***************************************************************************
    begin                : Fri Dec 29 18:30:00 CEST 2012
    copyright            : (C) 2012 by Ale Rimoldi
    email                : a.l.e@ideale.ch

 ***************************************************************************/

#include <QDebug>

#include "module/epubexportEpub.h"
#include "filezip.h"

EpubExportEpub::EpubExportEpub()
{
}

EpubExportEpub::~EpubExportEpub()
{
}

void EpubExportEpub::create()
{
    file = new FileZip(filename);
    file->create();
    
    // add mimetype to the current epub file
    // The mimetype file must be a text document in ASCII that contains the string application/epub+zip.
    // It must also be uncompressed, unencrypted, and the first file in the ZIP archive.
	file->add("mimetype", QString("application/epub+zip"), false);
}

void EpubExportEpub::close()
{
    file->close();
}

void EpubExportEpub::add(QString filename, QString content)
{
    file->add(filename, content, true);
}

void EpubExportEpub::addUncompressed(QString filename, QString content)
{
    file->add(filename, content, false);
}

void EpubExportEpub::add(QString filename, QByteArray content)
{
    file->add(filename, content, true);
}

void EpubExportEpub::addUncompressed(QString filename, QByteArray content)
{
    file->add(filename, content, false);
}

QDebug operator<<(QDebug dbg, const EpubExportEpub &epub)
{
    dbg.nospace() << "(" << "Debug not implemented" << ")";
    return dbg.space();
}
