/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef EPUBEXPORTXHTML_H
#define EPUBEXPORTXHTML_H

#include <QObject>
#include <QDebug>

#include "scribusapi.h" // for SCRIBUS_API

#include "module/epubexportStructure.h"

#include <QDomDocument>
#include <QDomElement>

class EpubExportXhtml : public QObject
{
    Q_OBJECT

public:
	EpubExportXhtml();
	~EpubExportXhtml();

    void setMetadata(EpubExportStructureMetadata metadata) {this->metadata = metadata;}
    void setTitle(QString title) {this->title = title;}
    QString getId() {return id;}
    QString getFilename() {return filename;}


    void initialize();
    QString get();


private:
    EpubExportStructureMetadata metadata;
    QString title;
    QString id;
    QString filename;

    QDomDocument document;
    QDomElement elementRoot;
    QDomElement elementBody;
};

QDebug operator<<(QDebug dbg, const EpubExportXhtml &xhtml);

#endif // EPUBEXPORTXHTML_H
