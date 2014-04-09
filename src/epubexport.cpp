/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/***************************************************************************
    begin                : Thu Mar 29 15:16:41 CEST 2012
    copyright            : (C) 2012 by Ale Rimoldi
    email                : a.l.e@ideale.ch

 ***************************************************************************/

#include <QDebug>

#include <QFile>
#include <QFileInfo>
#include <QDataStream>
#include <QByteArray>
#include <QBuffer> // for writing a QImage to a string (and then add it to the zip file)
#include <QImage> // for the cover

#include <QDomDocumentType>
#include <QDomImplementation>

#include <QList>
#include <QStringList>
#include <QVector>
#include <QVectorIterator>

#include <QRegExp>

#include <QUuid> // for generated the uuid if no isbn&co has been defined

#include <QProgressBar>

#include "epubexport.h"
#include "scribusdoc.h"
#include "scribuscore.h" // for reading the gui language

#include "ui/multiprogressdialog.h"

#include "filezip.h"

#include "styles/styleset.h"
#include "styles/paragraphstyle.h"
#include "styles/charstyle.h"

#include "sclayer.h"
#include "scpage.h"
#include "scribus.h" // for ScribusMainWindow
#include "util_formats.h" // for checking file extension
#include "documentinformation.h" // for filling the metadata

#include "util.h" // for parsing the list of pages (remove it when the function is moved to ScribusDoc)
#include "scribusstructs.h" // for getPageRect() remove it, it's moved to ScPage
#include "util_text.h" // contains desaxeString() to get the note's content
#include "sctextstruct.h" // for for getting  the char properties (notes...)
#include "marks.h" // for the footnotes (and in the future other marks...)

#include "cmsettings.h" // for cropping the image to the frame

// includes after the refactoring

#include "epubexport.h"

#include "module/epubexportEpub.h"
#include "module/epubexportStructure.h"
#include "module/epubexportCssParser.h"
#include "module/epubexportXhtml.h"


EpubExport::EpubExport(ScribusDoc* doc)
{
    progressDialog = 0;
    itemNumber = 0;
    this->doc = new EpubExportScribusDoc();
    this->doc->add(doc);

    qDebug() << "marksList" << this->doc->get()->marksList();
    qDebug() << "notesList" << this->doc->get()->notesList();
}

EpubExport::~EpubExport()
{
}

bool EpubExport::isDocItemTopLeftLessThan(const PageItem *docItem1, const PageItem *docItem2)
{
    return (docItem1->gXpos < docItem2->gXpos) ||
           ((docItem1->gXpos == docItem2->gXpos) && (docItem1->gYpos < docItem2->gYpos));
}

void EpubExport::doExport()
{
    qDebug() << "options" << options;
    qDebug() << "pageRange" << options.pageRange;

	options.targetFilename = "/tmp/"+options.targetFilename;
	qDebug() << "forcing the output of the .epub file to /tmp";

    epub = new EpubExportEpub();
    epub->setFilename(options.targetFilename);
    epub->create();

    /*
    cssParser = new EpubExportCssParser();
    cssParser->setFilepath("/tmp/style.css");
    cssParser->start();
    */

    EpubExportStructureContent content;
    while (doc->next())
    {
        structure = new EpubExportStructure();
        structure->setFilename(options.targetFilename);
        structure->readMetadata(doc->getMetadata());

        doc->readItems();
        if (progressDialog)
            progressDialog->setOverallTotalSteps(itemNumber);

        xhtml = new EpubExportXhtml();
        xhtml->setMetadata(structure->getMetadata());
        xhtml->setTitle("title of the chapter");
        xhtml->initialize(); // TODO: we have to initialize for each chapter

        epub->add("Text/" + xhtml->getFilename(), xhtml->get());
        structure->addContent(xhtml->getId(), "Text/" + xhtml->getFilename(), "application/xhtml+xml");

        if (!structure->hasCover())
        {
            structure->setCover(doc->getCover());
            structure->addContent("cover.png", "Images/cover.png", "image/png");
        }

        exportXhtml();
    }

	epub->add("META-INF/container.xml", structure->getContainer());

	epub->add("OEBPS/toc.ncx", structure->getNCX());

	epub->add("OEBPS/content.opf", structure->getOPF());

	epub->addUncompressed("OEBPS/Images/cover.png", structure->getCover());


    exportCSS();

	epub->close();
}


/**
 * replace the unicode characters to entities if needed.
 * and add some new line characters
 * TODO: we should find a way to correctly insert them in addText() itself, without having to "fix"
 * them afterwards (ale/20120606)
 */
QString EpubExport::getFixedXhtml(QString xhtml) {

    xhtml = xhtml.replace(SpecialChars::NBSPACE, "&nbsp;");

    QRegExp pattern("<(html|head|meta|/title|link|/head|body|/body|/html|/p)([^>]*)>");
    xhtml = xhtml.replace(pattern, "<\\1\\2>\n");

    return xhtml;
}

/**
 * add the content of xhtmlDocument to the current epub file
 */
void EpubExport::addXhtml()
{
	EPUBExportXhtmlFile file;
	file.section = section;
	file.filename = QString("Section%1.xhtml").arg(section + 1, 4, 10, QChar('0'));
	// qDebug() << "file.filename" << file.filename;
	file.title = QString("Section %1").arg(section + 1, 4, 10, QChar('0')); // TODO: as soon as we have a TOC, take the title from the text
	// passing somethign else then -1 to toString() adds indenting line breaks. we prefer to manually add
    // some breaks with getFixedXhtml()
	epub->get()->add("OEBPS/Text/" + file.filename, getFixedXhtml(xhtmlDocument.toString(-1)), true);
	xhtmlFile.append(file);

	struct EPUBExportContentItem contentItem;
	contentItem.id = file.filename;
	contentItem.href = "Text/" + file.filename;
	contentItem.mediaType = "application/xhtml+xml";
	// contentItem.title = QString("Section %1").arg(section + 1, 4, 10, QChar('0')); // TODO: as soon as we have a TOC, take the title from the text
	contentItems.append(contentItem);
}


QString EpubExport::getStylenameSanitized(QString stylename)
{
    return stylename.remove(QRegExp("[^a-zA-Z\\d_-]"));
}


/**
  * add OEBPS/Styles/style.css to the current epub file
  */ 
void EpubExport::exportCSS()
{
    int n = 0;
	QString wr = QString();

    const StyleSet<ParagraphStyle>* paragraphStyles = & doc->get()->paragraphStyles();
    n = paragraphStyles->count();
    // qDebug() << "n pstyle: " << n;
    for (int i = 0; i < n; ++i )
    {
        // for a list of properties:  grep -r "paragraphStyle()" . | vim -
        // for a list of properties:  grep -r "charStyle()" . | vim -
        // see fileloader/scribus150format/scribus150format_save.cpp
        ParagraphStyle paragraphStyle = (*paragraphStyles)[i];
        CharStyle charStyle = paragraphStyle.charStyle();
        wr += "p." + getStylenameSanitized(paragraphStyle.name()) + "{\n";
        // qDebug() << "style name: " << paragraphStyle.name();
        // qDebug() << "style alignment: " << paragraphStyle.alignment();
        // qDebug() << "style font: " << charStyle.font().scName();
        // line height: fixed (<- baseline) or auto
        // alignment
        // evt. tabs for lists
        // left right and first indents
        wr += "    padding-top:" + QString::number(paragraphStyle.gapBefore()) + "pt;\n";
        wr += "    padding-bottom:" + QString::number(paragraphStyle.gapAfter()) + "pt;\n";

        wr += "    font-size:" + QString::number(charStyle.fontSize() / 10) + "pt;\n";
        QString fontname = charStyle.font().scName();
		// as long as bold and italic are not in the features list, get the property
		// by guessing from the font name (ale/20120916)
		// rule.pattern = QRegExp("[\\\\|\\<|\\>|\\=|\\!|\\+|\\-|\\*|\\/|\\%]+");
		QRegExp regexpItalic("(\\bitalic\\b)");
		regexpItalic.setCaseSensitivity(Qt::CaseInsensitive);
		if (regexpItalic.indexIn(fontname) >= 0)
		{
			wr += "    font-variant:italic;\n";
		}
		QRegExp regexpBold("(\\bbold\\b)"); // TODO: add other keywords for bold
		regexpBold.setCaseSensitivity(Qt::CaseInsensitive);
		if (regexpBold.indexIn(fontname) >= 0)
		{
			wr += "    font-weight:bold;\n"; // TODO: add other keywords for italic
		}

		QStringList featureList = charStyle.features();
		QStringList::ConstIterator it;
		for (it = featureList.begin(); it != featureList.end(); ++it)
		{
			QString feature = it->trimmed();
			// qDebug() << "feature" << feature;
			if ((feature == CharStyle::UNDERLINE) || (feature == CharStyle::UNDERLINEWORDS))
				wr += "    text-decoration:underline;\n";
			else if (feature == CharStyle::STRIKETHROUGH)
				wr += "    text-decoration:line-through;\n";
		}

        // paragraphStyle.hasDropCap()
		// paragraphStyle.dcCharStyleName();
		// paragraphStyle.dropCapOffset());

        // font family + style!
        // tracking and width space
        // underline
        // stroke
        // horizontal / vertical scaling
        // color

        // charStyle.name();
        // charStyle.parent();
        // charStyle.font().scName()
        // charStyle.features().join(" ")
        // charStyle.fillColor();
        // charStyle.fillShade();
        // charStyle.strokeColor();
        // charStyle.strokeShade();
        // charStyle.shadowXOffset() / 10.0
        // charStyle.shadowYOffset() / 10.0
        // charStyle.outlineWidth() / 10.0
        // charStyle.underlineOffset() / 10.0
        // charStyle.underlineWidth() / 10.0
        // charStyle.strikethruOffset() / 10.0
        // charStyle.strikethruWidth() / 10.0
        // charStyle.scaleH() / 10.0
        // charStyle.scaleV() / 10.0
        // charStyle.baselineOffset() / 10.0
        // charStyle.language();
        // qDebug() << "charStyle font size: " << charStyle.fontSize();
        wr += "}\n";
    }

    const StyleSet<CharStyle>* charStyles = & doc->get()->charStyles();
    n = charStyles->count();
    // qDebug() << "n cstyle: " << n;
    for (int i = 0; i < n; ++i )
    {
        CharStyle charStyle = (*charStyles)[i];
        wr += "span." + getStylenameSanitized(charStyle.name()) + "{\n";
        // qDebug() << "style name: " << charStyle.name();
        // qDebug() << "style font: " << charStyle.font().scName();
        wr += "    font-size:" + QString::number(charStyle.fontSize() / 10) + "pt;\n";
        // qDebug() << "style font size: " << charStyle.fontSize();
        wr += "}\n";
    }
    //
    // add epub exporter related styles (must be included in the imported css if one is imported
    // (or check if the exist?)
    wr += "div.picture{\n";
    // qDebug() << "style name: " << paragraphStyle.name();
    // qDebug() << "style alignment: " << paragraphStyle.alignment();
    // qDebug() << "style font: " << charStyle.font().scName();
    // line height: fixed (<- baseline) or auto
    // alignment
    // evt. tabs for lists
    // left right and first indents
    wr += "    text-align:left;\n";
    wr += "    page-break-before:always;\n";
    wr += "    page-break-inside:avoid;\n";
    wr += "}\n";

    // write the stylesheet
	epub->get()->add("OEBPS/Styles/style.css", wr, true);

	struct EPUBExportContentItem contentItem;
	contentItem.id = "stylesheet";
	contentItem.href = "Styles/style.css";
	contentItem.mediaType = "text/css";
	contentItems.append(contentItem);

    /*
	docu.writeAttribute("VHOCH"  , doc->typographicPrefs().valueSuperScript);
	docu.writeAttribute("VHOCHSC", doc->typographicPrefs().scalingSuperScript);
	docu.writeAttribute("VTIEF"  , doc->typographicPrefs().valueSubScript);
	docu.writeAttribute("VTIEFSC", doc->typographicPrefs().scalingSubScript);
	docu.writeAttribute("VKAPIT" , doc->typographicPrefs().valueSmallCaps);
	docu.writeAttribute("LANGUAGE", doc->hyphLanguage());
     */
}

/**
  * create the xhtml document structure and give access to xhtmlBody where addText and addImage
  * will add the content
  *
  * <?xml version="1.0" encoding="UTF-8" ?>
  * <!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
  * <html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en">
  *   <head>
  *     <meta http-equiv="Content-Type" content="application/xhtml+xml; charset=utf-8" />
  *     <title>Pride and Prejudice</title>
  *     <link rel="stylesheet" href="css/main.css" type="text/css" />
  *   </head>
  *   <body>
  *   </body>
  * </html>
  */
void EpubExport::initializeXhtml()
{
}

void EpubExport::exportXhtml()
{
	initializeXhtml();

    int n = doc->get()->DocPages.count();
    int m = doc->get()->DocItems.count();
	if (m == 0)
	{
		addXhtml();
		return;
	}

    doc->get()->scMW()->setStatusBarInfoText(tr("Exporting to EPUB"));
    doc->get()->scMW()->mainWindowProgressBar->setMaximum(m);
    int mm = 0;
    int jj = 0;
	QString content = QString();
    PageItem* docItem;
	section = 0;
    for (int i = 0; i < n; i++)
    {
		// TODO: if the page is on a new section, create a new file
		if (itemList[i].count() == 0)
			continue;
		int sectionId = doc->get()->getSectionKeyForPageIndex(itemList[i][0]->OwnPage); // TODO: use the real page that we are handling
		// TODO: create the file as Section0001 ... check the name from sigil
        // qDebug() << "sectionId" << sectionId;
        // qDebug() << "section" << section;
		if (section < sectionId)
		{
			addXhtml();
			section = sectionId;
			initializeXhtml();
		}
        qSort(itemList[i].begin(), itemList[i].end(), EpubExport::isDocItemTopLeftLessThan);

        mm = itemList[i].count();
        jj = jj + mm;

        doc->get()->scMW()->mainWindowProgressBar->setValue(jj);
        for (int j = 0; j < mm; j++) {
            docItem = itemList[i].at(j);
            if (docItem->asTextFrame())
            {
                if (!docItem->isNoteFrame()) {
                    // what to do with footnotes?
                    // TODO: store the footnote text and add it to the end of the section or the document
                }
                else
                addText(docItem);
            }
            else if (docItem->asImageFrame())
            {
                addImage(docItem);
            }
            if (progressDialog)
                progressDialog->setOverallProgress(progressDialog->overallProgress() + 1);
        }
    }
    doc->get()->scMW()->mainWindowProgressBar->setValue(mm);
    doc->get()->scMW()->setStatusBarInfoText("");

	addXhtml();
}


/**
 * example of charStyle() use in svgexplugin.cpp 
 * cf. short-words/parse.cpp
 * cf. Scribus150Format::writeITEXTs() for getting the local formatting
 * in plugins/fileloader/scribus150format/scribus150format_save.cpp
 *
 * - get the list of runs (chunks different formatting) in the text frame
 * - foreach run:
 *   - if it's a p
 *     - create a p element with it's style name
 *     - set this element as the latest inserted paragraph
 *     - set the the dom element as the current one
 *   - get the character style
 *   - get the local formatting of the first char in the run
 *     - font name
 *       - if the the font name suggests an italic or bold add italic or bold to the list of the formatting
 *   - foreach formatting (feature)
 *     - if it's bold, italic, superscript, subscript create an element with the specfic tag
 *     - if it's underline, strikethrough add the formatting to the span's style attribute
 *   - if there is a character style of a local formatting:
 *     - if not specific tag has been createdi create a span element
 *     - add the class and style attributes to the element
 *     - add the span to the paragraph element and set it as the current element
 *   - insert the text line by line (with a br at the end of each but the last line)
 *   - add the p to the dom
 *   - set the current element as the latest element paragraph created
 */
void EpubExport::addText(PageItem* docItem)
{
    // TODO: it should be possible to export parts of a text frame. docItem should be
    // dynamically read from epubexportScribusDoc and have start / end bounds which are used while
    // pasing it!
    // initialize the local variables
    QDomElement elementCurrent;
    QDomElement elementParagraph;
    QString paragraphStyleName;
    QString characterStyleName;
    QString run_text;

    if ((docItem->prevInChain() == NULL) && (docItem->itemText.length() > 0))
    {
        // parse the text and define the runs list
        initOfRuns(docItem); // find a way to make it clear that this fills the runs' object variable
        // qDebug() << "runs:" << runs;

        foreach (EPUBExportRuns run, runs)
        {
            // qDebug() << "run:" << run;
            if (run.type == 'p')
            {
                elementParagraph = xhtmlDocument.createElement("p");

                paragraphStyleName = docItem->itemText.paragraphStyle(run.pos + 1).parent();
				paragraphStyleName = getStylenameSanitized(paragraphStyleName);
                // qDebug() << "paragraphStyle:" << paragraphStyleName;
				if (paragraphStyleName != "")
					elementParagraph.setAttribute("class", paragraphStyleName);

				elementCurrent = elementParagraph;
            }

			characterStyleName = docItem->itemText.charStyle(run.pos + 1).displayName();
			characterStyleName = getStylenameSanitized(characterStyleName);
			// qDebug() << "character style: " << characterStyleName;

            const CharStyle& style(docItem->itemText.charStyle(run.pos));

			QString fontname = style.font().scName();
			// qDebug() << "fontname:" << fontname;

			QStringList characterFeatures = style.features();
			// qDebug() << "characterFeatures" << characterFeatures;

			// as long as bold and italic are not in the features list, get the property
			// by guessing from the font name (ale/20120916)
			QRegExp regexpItalic("(\\bitalic\\b)"); // TODO: use other keywords for italics (also for style.css)
			regexpItalic.setCaseSensitivity(Qt::CaseInsensitive);
			if (regexpItalic.indexIn(fontname) >= 0)
				characterFeatures << CharStyle::ITALIC;

			QRegExp regexpBold("(\\bbold\\b)"); // TODO: use also other keywords for bold (also for style.css)
			regexpBold.setCaseSensitivity(Qt::CaseInsensitive);
			if (regexpBold.indexIn(fontname) >= 0)
				characterFeatures << CharStyle::BOLD;
			// qDebug() << "characterFeatures" << characterFeatures;

            QDomElement element;
			QStringList characterFormatting;

            foreach (QString feature, characterFeatures)
            {
				feature = feature.trimmed();
				// qDebug() << "feature" << feature;
				if (feature == CharStyle::BOLD)
					element = xhtmlDocument.createElement("b");
				else if (feature == CharStyle::ITALIC)
					element = xhtmlDocument.createElement("i");
				else if (feature == CharStyle::SUPERSCRIPT)
					element = xhtmlDocument.createElement("sup");
				else if (feature == CharStyle::SUBSCRIPT)
					element = xhtmlDocument.createElement("sub");
				else if ((feature == CharStyle::UNDERLINE) || (feature == CharStyle::UNDERLINEWORDS))
					characterFormatting << "text-decoration:underline";
				else if (feature == CharStyle::STRIKETHROUGH)
					characterFormatting << "text-decoration:line-through";
				else if (feature != CharStyle::INHERIT)
					qDebug() << "else feature" << feature;
					/*
					 * The following character formats are supported by Scribus
					 * but not (yet) exported to epub:
					 * OUTLINE: text-outline: 1px 1px #ccc;
					 * SHADOWED: text-shadow: 2px 2px #ff0000;
					 * ALLCAPS: text-transform:uppercase;
					 * SMALLCAPS: font-variant:small-caps;
					 * SHYPHEN: hyphenation is possible... how to use it?
					 */
            }

			// qDebug() << "tag name: " << element.tagName();
			if ((element.tagName() != "") || !characterFormatting.isEmpty() || (characterStyleName != ""))
            {
                if (element.tagName() == "")
                    element = xhtmlDocument.createElement("span");
                // qDebug() << "tag name: " << element.tagName();

                if (characterStyleName != "")
                    element.setAttribute("class", characterStyleName);

                if (!characterFormatting.isEmpty())
                    element.setAttribute("style", characterFormatting.join("; ") + ";");

				elementCurrent.appendChild(element);
				elementCurrent = element;
            }

            foreach (QVector<QString> content, run.content)
            {
                QVectorIterator<QString> list(content);
                while (list.hasNext()) {
                    QDomText elementText = xhtmlDocument.createTextNode(list.next());
                    elementCurrent.appendChild(elementText);
                    if (list.hasNext())
                        elementCurrent.appendChild(xhtmlDocument.createElement("br"));
                }
            }
            if (elementParagraph.text() != "")
                xhtmlBody.appendChild(elementParagraph);

			elementCurrent = elementParagraph;
        }
    }
}

/**
 * TODO: if the file is called cover.png renamed it, in order not to overwrite the generated cover
 */
void EpubExport::addImage(PageItem* docItem)
{
	QString filename(docItem->Pfile);
	if (filename == "")
        return;

    QFileInfo fileinfo = QFileInfo(filename);
    QString ext = fileinfo.suffix().toLower();

    int mediaType = 0;
    if (ext == "png")
        mediaType = FormatsManager::PNG;
    else if (extensionIndicatesJPEG(ext))
        mediaType = FormatsManager::JPEG;
    qDebug() << "mediaType" << mediaType;

    if (mediaType == 0)
    {
        // TODO: convert the other "acceptable" image types to png or jpeg (how to choose?)
        qDebug() << "image format not yet supported: " << filename;
        return;
    }

    QPixmap image; // null if the image has not been cropped nor scaled
    bool usingLoadedImage = false;

	// qDebug() << "image file" << filename;
	// qDebug() << "imageXScale" << docItem->imageXScale();
	// qDebug() << "imageYScale" << docItem->imageYScale();
	// qDebug() << "imageXOffset" << docItem->imageXOffset();
	// qDebug() << "imageYOffset" << docItem->imageYOffset();
	// qDebug() << "item width" << docItem->width();
	// qDebug() << "item height" << docItem->height();
	// qDebug() << "image BBoxX" << docItem->pixm.imgInfo.BBoxX;
	// qDebug() << "image BBoxH" << docItem->pixm.imgInfo.BBoxH;
	// qDebug() << "image xres" << docItem->pixm.imgInfo.xres;
	// qDebug() << "image yres" << docItem->pixm.imgInfo.yres;
	// qDebug() << "image width" << docItem->pixm.width();
	// qDebug() << "image height" << docItem->pixm.height();

    double cropX = docItem->imageXOffset();
    double cropY =  docItem->imageYOffset();

    // calculate the frame's width and height in "image pixels"
    double frameW = docItem->width() / docItem->imageXScale();
    double frameH = docItem->height() / docItem->imageYScale();

    if (!image.load(filename)) // TODO: if the image's width and height are already stored, only load the image when it has to be cropped or scaled
        return;
        // usingLoadedImage = true;

    QRect frameRect = QRect(- cropX, -cropY, frameW, frameH);
    QRect imageRect = QRect(0, 0, image.width(), image.height());
    QRect cropRect;
	// qDebug() << "frameRect" << frameRect;
	// qDebug() << "imageRect" << imageRect;

    if (frameRect != imageRect)
    {
        usingLoadedImage = true;
        cropRect = frameRect.intersected(imageRect);
        // qDebug() << "cropRect" << cropRect;
        if (!cropRect.isEmpty())
        {
            QPixmap imageTmp = image.copy(cropRect);
            image = imageTmp;
        }
    }

	qDebug() << "frameW" << frameW;
	qDebug() << "imageMaxWidth" << options.imageMaxWidth;
    int scaling = 100;

    ScPage* page = doc->get()->DocPages.at(docItem->OwnPage); // TODO: use the real page that we are handling
    qDebug() << "item width" << docItem->width();
    double proportion = docItem->width() / (page->width() - page->rightMargin() - page->leftMargin());
    qDebug() << "proportion" << proportion;
    qDebug() << "imageMaxWidthThreshold" << options.imageMaxWidthThreshold;

    int scaledWidth;
    if (proportion > static_cast<double>(options.imageMaxWidthThreshold) / 100)
        scaledWidth = options.imageMaxWidth;
    else
        scaledWidth = static_cast<int>(proportion * options.imageMaxWidth);
    qDebug() << "scaledWidth" << scaledWidth;

    qDebug() << "image width" << image.width();
    if (scaledWidth < frameW)
        scaling = static_cast<int>(static_cast<double>(scaledWidth) / image.width() * 100);
    qDebug() << "scaling" << scaling;

    QString zippedFilename = fileinfo.fileName();
    if (usingLoadedImage)
    {
        zippedFilename = fileinfo.completeBaseName()+"_c-%1-%2-%3-%4-s-%5."+fileinfo.suffix();
        zippedFilename = zippedFilename.arg(cropRect.x()).arg(cropRect.y()).arg(cropRect.width()).arg(cropRect.height()).arg(scaling);
    }
    zippedFilename.remove(QRegExp("[^a-zA-Z\\d\\s_.-]"));
    qDebug() << "zippedFilename" << zippedFilename;

    QString filepath = "Images/" + zippedFilename;

    // add the image to the dom
    QDomElement div = xhtmlDocument.createElement("div");
    // TODO: only set class picture if the images is maximized (or use different names: the goal is not to only set the page break before, if the picture is maximized)
    div.setAttribute("class", "picture");
    xhtmlBody.appendChild(div);
    QDomElement element = xhtmlDocument.createElement("img");
    // <image height="800" width="600" xlink:href="../Images/cover.jpeg"></image>
    element.setAttribute("height", (int) docItem->height()); // TODO: use the real width of the visible part of the image (as a rectangle)
    element.setAttribute("width", (int) docItem->width());
    element.setAttribute("alt", ""); // TODO do we have a way to define the metadata? eventually from the exif? epubcheck says it's mandatory... and it's not nice to leave it empty...
    element.setAttribute("src", "../"+filepath); // TODO: make sure that the name is unique in the target! (if it already exists prefix the frame name?)
    // TODO: set the width and height? from the docItem?
    div.appendChild(element);

    // TODO: add also the path to the original picture before the zippedFilename in order to catch files with
    // the same name but from different paths... and then? how to set the name?
    if (!imageFileNames.contains(zippedFilename))
    {
        imageFileNames << zippedFilename;
        qDebug() << "zippedFilename" << zippedFilename;
        qDebug() << "imageFileNames" << imageFileNames;
        if (scaling > 0 && scaling < 100)
        {
            usingLoadedImage = true;
            QPixmap imageTmp = image.scaledToWidth(scaledWidth, Qt::SmoothTransformation);
            image = imageTmp;
        }
        /*
        // TODO: some leftovers if we want ever do a color managed conversion of the pictures
        ScImage img;
        ScImage docItem->pixm;
        ImageInfoRecord imgInfo;
            ImageTypeEnum type -> 0 = jpg, 1 = tiff, 2 = psd, 3 = eps/ps, 4 = pdf, 5 = jpg2000, 6 = other
            ColorSpaceEnum colorspace -> 0 = RGB  1 = CMYK  2 = Grayscale 3 = Duotone
        CMSettings cms(c->doc(), Profil, Intent);
        cms.setUseEmbeddedProfile(Embedded);
        usingLoadedImage = img.loadPicture(fn, c->pixm.imgInfo.actualPageNumber, cms, ScImage::RGBData, 72, &realCMYK);
        bool loadPicture(const QString & fn, int page, const CMSettings& cmSettings, RequestType requestType, int gsRes, bool *realCMYK = 0, bool showMsg = false);
         */

        // copy the image into the zip
        if (!usingLoadedImage)
        {
            qDebug() << "standard file add";
            QFile file(fileinfo.filePath()); // TODO: if we already have a scimage we may have to change this
            epub->get()->add("OEBPS/"+filepath, &file, true);
        }
        else
        {
            QByteArray imageBytes;
            QBuffer buffer(&imageBytes);
            buffer.open(QIODevice::WriteOnly);
            image.save(&buffer, mediaType == FormatsManager::JPEG ? "jpg" : "png");
            epub->get()->add("OEBPS/"+filepath, imageBytes, false);


        }

        struct EPUBExportContentItem contentItem;
        contentItem.id = zippedFilename;
        contentItem.href = filepath;
        contentItem.mediaType = FormatsManager::instance()->mimetypeOfFormat(mediaType).first();
        contentItems.append(contentItem);
    } // if (imageFileNames.contains(zippedFilename))
}

// @todo: use the text/storytext methods as soon as they are implemented
/*
 * read the runs beginnings and the number of runs in the text item
*/
void EpubExport::initOfRuns(PageItem* docItem)
{
    runs.clear();
    // paragraph.clear();
	CharStyle lastStyle;
	int lastPos = 0;
    // inspired by Scribus150Format::writeITEXTs
    struct EPUBExportRuns runsItem;
    runsItem.pos = 0;
    runsItem.type = 'p';
    runsItem.content.clear();
    int n = docItem->itemText.length();

    QVector<QString> lines;
    QString line = "";

    if (n == 0) {
        runsItem.length = 0;
        runs.append(runsItem);
    } else {
        lines.clear();
        QString content = docItem->itemText.text(0, docItem->itemText.length());

        for(int i = 0; i < n; ++i)
        {
            bool output = true;

            const CharStyle& style1(docItem->itemText.charStyle(i));
            const QChar ch = docItem->itemText.text(i);
            ScText* chProperties = docItem->itemText.item(i);
            if (chProperties->mark) {
                Mark* footnoteCall = chProperties->mark;
                if (footnoteCall->getType() == MARKNoteMasterType) {
                    TextNote* footnote = chProperties->mark->getData().notePtr;
                    qDebug() << "calling mark:" << footnoteCall->getString();
                    if (!footnote->saxedText().isEmpty()) {
                        StoryText footnoteText = desaxeString(doc->get(), footnote->saxedText());
                        qDebug() << "note text:" << footnoteText.text(0, footnoteText.length());
                        // TODO: refactor to be able to recursively call the html formatting!
                    }
                    qDebug() << "note text:" << footnote->saxedText();
                    qDebug() << "note mark:" << footnote->num();
                    qDebug() << "note style:" << footnote->notesStyle()->name();

                    // apptend 
                } else if (footnoteCall->getType() == MARKNoteFrameType) {
                }
                continue; // don't insert the marks as such!
            }

            /*
            // store formatting struct for:
            // - current paragraph style
            // - current character style
            // - sum of current paragraph and char style
            // - currentformatting
            // store last insterted formatting string (<strong><em><span style="font-size:12px");
            // --> struct EPUBExportCurrentFormatting
            */
            
            // if there is a paragraph change or the formatting has changed
            if (
                ch == SpecialChars::PARSEP ||
                ch == SpecialChars::COLBREAK ||
                ch == SpecialChars::FRAMEBREAK
            )
            {
                // qDebug() << "ch" << ch.unicode();
                runsItem.length = i - runsItem.pos;
                if (line != "")
                {
                    lines.append(line);
                    line = "";
                }
                if (!lines.isEmpty())
                {
                    runsItem.content.append(lines);
                    lines.clear();
                }
                if (!runsItem.content.isEmpty())
                    runs.append(runsItem);
                runsItem.pos = i;
                runsItem.type = 'p';
                runsItem.content.clear();
                output = false;
            }
            if (ch == SpecialChars::LINEBREAK)
            {
                if (line != "")
                {
                    lines.append(line);
                    line = "";
                }
                output = false;
            }
            // accepted special chars:
            // ch == SpecialChars::NBSPACE

            if (
                style1 != lastStyle
            )
            {
                // something new, write pending chars
                if  (i - lastPos > 0)
                {
                    // docu.writeEmptyElement("ITEXT");
                    /*if (item->asPathText()) // seems to cause problems when loading pathtext elements
                        putCStylePT(docu, lastStyle);
                    else*/
                        // putCStyle(docu, lastStyle);
                    // docu.writeAttribute("CH", textWithSoftHyphens(item->itemText, lastPos, i));
                }
                // runs.append(i);
                if (line != "")
                {
                    lines.append(line);
                    line = "";
                }
                if (!lines.isEmpty())
                {
                    runsItem.content.append(lines);
                    lines.clear();
                }
                runsItem.length = i - runsItem.pos;
                if (runsItem.type == 'p' || !runsItem.content.isEmpty())
                    runs.append(runsItem);
                runsItem.pos = i;
                runsItem.type = 'f';
                runsItem.content.clear();
                lastStyle = style1;
                lastPos = i;
            }
            if (
                ch == SpecialChars::NBHYPHEN ||
                ch == SpecialChars::NBSPACE ||
                ch == SpecialChars::ZWNBSPACE ||
                ch == SpecialChars::ZWSPACE
            )
            {
            }
            // @todo: ignore the following chars
            if (
                ch == SpecialChars::OBJECT ||
                ch == SpecialChars::TAB ||
                ch == SpecialChars::PAGENUMBER ||
                ch == SpecialChars::PAGECOUNT ||
                ch.unicode() < 32 || 
                (0xd800 <= ch.unicode() && ch.unicode() < 0xe000) ||
                ch.unicode() == 0xfffe || ch.unicode() == 0xffff
            )
            {
            }

            if (output)
                line += ch;
        }
        runsItem.length = n - runsItem.pos;
        if (line != "")
            lines.append(line);

        if (!lines.isEmpty())
            runsItem.content.append(lines);

        if (!runsItem.content.isEmpty())
            runs.append(runsItem);
    }
    // qDebug() << "runs: " << runs;
    // qDebug() << "paragraph: " << paragraph;
}

uint EpubExport::nrOfRuns()
{
	return runs.count();
}

int EpubExport::startOfRun(uint index)
{
	return index;
}

int EpubExport::endOfRun(uint index)
{
	return index + 1;
}

QDebug operator<<(QDebug dbg, const EpubExportOptions options)
{
    dbg.nospace() << "(targetFilename:" << options.targetFilename;
    QStringList output;
    foreach (int pageRange, options.pageRange) {
        output << QString::number(pageRange);
    }
    dbg.nospace() << ", (" << output.join(", ") << ")" << ", imageMaxWidth:" << options.imageMaxWidth  << ", imageMaxWidthThreshold:" << options.imageMaxWidthThreshold << ")";
    return dbg.space();
}

// not tested yet
QDebug operator<<(QDebug dbg, const EPUBExportRuns run)
{
    dbg.nospace() << "(pos:" << run.pos << ", lenght:" << run.length  << ", type:" << run.type  << ", content:" << run.content << ")";
    return dbg.space();
}

QDebug operator<<(QDebug dbg, const QVector<EPUBExportRuns> &runs)
{
    dbg.nospace() << "[" ;
    foreach (EPUBExportRuns run, runs)
    {
        dbg.nospace() << run;
    }
    dbg.nospace() << "]" ;
    return dbg.space();
}

// you have to declared it before using it... so put the signature in the .h file, too
QDebug operator<<(QDebug dbg, const QList<ScPage*> &pages)
{
    QStringList output;
    for (int i = 0; i < pages.length(); i++) {
        output << QString::number(pages.at(i)->pageNr());
    }
    dbg.nospace() << "(" << output.join(", ") << ")";
    return dbg.space();
}
