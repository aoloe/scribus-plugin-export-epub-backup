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

#include <QDomDocument>
#include <QDomProcessingInstruction>
#include <QDomElement>

#include "module/epubexportXhtml.h"

#include "module/epubexportStructure.h"

EpubExportXhtml::EpubExportXhtml()
{
}

EpubExportXhtml::~EpubExportXhtml()
{
}

void EpubExportXhtml::initialize()
{
	QDomText text;
	QDomElement element;

    const QDomDocumentType doctype = (new QDomImplementation())->createDocumentType("html", "-//W3C//DTD XHTML 1.1//EN", "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd");
    document = QDomDocument(doctype);

	QDomProcessingInstruction xmlDeclaration = document.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"utf-8\"");
	document.appendChild(xmlDeclaration);


	QDomElement elementRoot = document.createElement("html");
	elementRoot.setAttribute("xmlns", "http://www.w3.org/1999/xhtml");
	elementRoot.setAttribute("xml:lang", metadata.language);
	document.appendChild(elementRoot);

    QDomElement head = document.createElement("head");
    elementRoot.appendChild(head);

	element = document.createElement("meta");
	element.setAttribute("http-equiv", "Content-Type");
	element.setAttribute("content", "application/xhtml+xml; charset=utf-8");
	head.appendChild(element);

	element = document.createElement("title");
	head.appendChild(element);
	text = document.createTextNode(title);
	element.appendChild(text);

	element = document.createElement("link");
	element.setAttribute("rel", "stylesheet");
	element.setAttribute("href", "../Styles/style.css");
	element.setAttribute("type", "text/css");
	head.appendChild(element);

    elementBody = document.createElement("body");
    elementRoot.appendChild(elementBody);
}

/**
 * @return the string representation of the xhtml file
 */
QString get()
{
    return "";
}

QDebug operator<<(QDebug dbg, const EpubExportXhtml &xhtml)
{
    dbg.nospace() << "(" << "Debug not implemented" << ")";
    return dbg.space();
}
