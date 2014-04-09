/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef EPUBEXPORT_H
#define EPUBEXPORT_H

#include <QObject>
#include <QDomElement>
#include <QVector>
#include <QMap>
#include <QDebug>

#include "scribusapi.h" // for SCRIBUS_API
#include "documentinformation.h" // for SCRIBUS_API
#include "module/epubexportScribusDoc.h"
// #include "module/epubexportEpub.h"
class EpubExportEpub;
class EpubExportStructure;
class EpubExportCssParser;
class EpubExportXhtml;

class QString;
class ScribusDoc;
class DocumentInformation; // for the metadata
class ScPage;
class ScLayer;

class MultiProgressDialog;

class PageItem;

class MarginStruct; // for getPageRect() remove it, it's moved to ScPage

struct EpubExportOptions
{
    QString targetFilename;
    QList<int> pageRange;
    int imageMaxWidth;
    int imageMaxWidthThreshold;
    EpubExportOptions()
    {
        targetFilename = "";
        imageMaxWidth = 0;
        imageMaxWidthThreshold = 0;
    }
};

struct EPUBExportRuns
{
    int pos;
    int length;
    char type; // p=paragraph, f=formatting
    QVector< QVector<QString> > content;
};

struct EPUBExportXhtmlFile
{
    int section;
    QString title;
    QString filename;
};

struct EPUBExportContentItem
{
    QString id;
    QString href;
    QString mediaType;
};

class EpubExport : public QObject
/*
 * Inspired by svgexplugin
 */
{
    Q_OBJECT

public:
	EpubExport(ScribusDoc* doc);
	~EpubExport();

	EpubExportOptions options;
    void setOptions(EpubExportOptions options) {this->options = options;}

	/*!
	\author Ale Rimoldi
	\brief Create the SVG exporter
	\param doc QString file name
	 */
	void doExport();

    /*!
        \author Ale Rimoldi
        \brief Check if the item is higher and more on the left side
        \return boolean
    */
    static bool isDocItemTopLeftLessThan(const PageItem *docItem1, const PageItem *docItem2);

    void setProgressDialog(MultiProgressDialog* dialog) {progressDialog = dialog;}

public slots:
    void cancelRequested();

private:
    MultiProgressDialog* progressDialog;

    EpubExportScribusDoc* doc;
    EpubExportEpub* epub;
    EpubExportStructure* structure;
    EpubExportCssParser* cssParser;
    EpubExportXhtml* xhtml;

	DocumentInformation documentMetadata;
    QVector< QList<PageItem*> > itemList;
    int itemNumber;

    // the following moethods should move to ScPage/PageItem
    MarginStruct getPageBleeds(const ScPage* page);
    QRect getPageRect(const ScPage* page);
    QList<ScPage *> getPagesWithItem(PageItem* item);

	void readItems();
    QList<QString> imageFileNames; // list the file names of the images already inserted in the epub file

    // QMap<int, ScLayer*> layerList;
    QList<int> layerNotPrintableList;

    QVector<EPUBExportContentItem> contentItems;

    int section;
	QDomDocument xhtmlDocument;
	QDomElement xhtmlRoot;
	QDomElement xhtmlBody;
    QVector<EPUBExportXhtmlFile> xhtmlFile; // list of the xhtml files for the toc
    QString getStylenameSanitized(QString stylename);
    void exportCSS();
    void exportItems();
    void exportXhtml();
    void initializeXhtml();
    QString getFixedXhtml(QString);
    void addXhtml();
    void addText(PageItem* docItem);
    void addImage(PageItem* docItem);

    // @todo: use the text/storytext methods as soon as they are implemented
    /*
 	uint nrOfParagraphs(); --> use StoryText::nrOfParagraphs()!
	int startOfParagraph();
 	int startOfParagraph(uint index);
    */
	void initOfRuns(PageItem* docItem);
 	uint nrOfRuns();
 	int startOfRun(uint index);
 	int endOfRun(uint index);
    // QVector<uint> runs; // the indexes where the runs start
    // QVector<uint> paragraph; // the indexes where the paragraphs start
    // QMap<int, char> runs; // <postion,paragraph/formatting>
    QVector<EPUBExportRuns> runs; // the indexes where the runs start

};

QDebug operator<<(QDebug dbg, const EpubExportOptions options);

QDebug operator<<(QDebug dbg, const QList<ScPage*> &pages);
QDebug operator<<(QDebug dbg, const QVector<EPUBExportRuns> &runs);
QDebug operator<<(QDebug dbg, const EPUBExportRuns run);

#endif // EPUBEXPORT_H
