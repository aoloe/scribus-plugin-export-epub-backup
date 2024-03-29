/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef EPUBEXPORTEPUB_H
#define EPUBEXPORTEPUB_H

#include <QObject>
#include <QDebug>

#include "scribusapi.h" // for SCRIBUS_API

// class Zip;
// class EpubExportZip;
#include "module/epubexportZip.h"

class EpubExportEpub : public QObject
{
    Q_OBJECT

public:
	EpubExportEpub();
	~EpubExportEpub();

    void setFilename(QString filename) {this->filename = filename;}

    void create();
    // void close();

    void add(QString filename, QString content);
    void add(QString filename, QByteArray content);
    void addUncompressed(QString filename, QString content);
    void addUncompressed(QString filename, QByteArray content);
private:
	EpubExportZip *file;
    QString filename;
    inline EpubExportZip::CompressionLevel getDefaultCompressionLevel(bool compressing)
    {
        return compressing ? EpubExportZip::Deflate9 : EpubExportZip::Store;
    }
};

QDebug operator<<(QDebug dbg, const EpubExportEpub &epub);

#endif // EPUBEXPORTEPUB_H

