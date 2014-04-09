/**
 * this parser is based on http://www.qtcentre.org/threads/30707-Parse-Text-File-with-Qt
 * if this custom css parser fails to be useful for the css files we get,
 * please, have a look at http://freecode.com/projects/libcroco and replace it
 * the supported CSS BNF syntax is at the end of the .h file
 *
 * for now it supports what the epub exporter creates + the h1..h9 styles
 */
#include "module/epubexportCssParser.h"

#include <QFile>
#include <QTextStream>
#include <QDebug>

EpubExportCssParser::EpubExportCssParser()
{
    filepath = "";
}

EpubExportCssParser::~EpubExportCssParser()
{
}

//states tell the parser which element it is parsing.
const int EpubExportCssParser::STATE_TAG = 1;
const int EpubExportCssParser::STATE_STYLENAME = 2;
const int EpubExportCssParser::STATE_FORMAT = 3;
const int EpubExportCssParser::STATE_PROPERTY_NAME = 4;
const int EpubExportCssParser::STATE_PROPERTY_VALUE = 5;
const int EpubExportCssParser::STATE_COMMENT_START = 6;
const int EpubExportCssParser::STATE_COMMENT = 7;
const int EpubExportCssParser::STATE_COMMENT_STOP = 8;

void EpubExportCssParser::start()
{
    if (filepath == "") 
        return;

    QFile file(filepath);
    if (!file.open(QFile::ReadOnly))
        return;

    QTextStream ts(&file);

    QChar ch;
    int currentState = 1;
    int previousState = 1;

    EpubExportCssParserStyle currentStyle = EpubExportCssParserStyle();
    QList<EpubExportCssParserStyle> currentStyles;
    EpubExportCssParserFormatting currentFormat = EpubExportCssParserFormatting();
    bool valid = true;
    QString comment = "";

    while (valid && !ts.atEnd()) {
        ts >> ch;
        // qDebug() << "ch:" << ch;
        if (ch == '/')
        {
            if (currentState == EpubExportCssParser::STATE_COMMENT_STOP)
            {
                currentState = previousState;
                previousState = 0;
            }
            else
            {
                comment = "";
                if (currentState != STATE_COMMENT)
                {
                    previousState = currentState;
                    currentState = EpubExportCssParser::STATE_COMMENT_START;
                }
                continue;
            }
        }

        if (currentState == EpubExportCssParser::STATE_TAG)
        {
            if (ch.isSpace())
            {
                if ((currentStyle.tag != "") || (currentStyle.name != ""))
                {
                    qDebug() << "add current tag" << currentStyle.tag;
                    currentStyles << currentStyle;
                    currentStyle = EpubExportCssParserStyle();
                }
            }
            else if (ch == '.')
                currentState = EpubExportCssParser::STATE_STYLENAME;
            else if (ch == '#')
            {
                currentState = EpubExportCssParser::STATE_STYLENAME;
                currentStyle.type = 'i';
            }
            else if (ch == '{')
            {
                if ((currentStyle.tag != "") || (currentStyle.name != ""))
                {
                    qDebug() << "add current tag" << currentStyle.tag;
                    currentStyles << currentStyle;
                    currentStyle = EpubExportCssParserStyle();
                }
                currentState = EpubExportCssParser::STATE_FORMAT;
            }
            else
                currentStyle.tag += ch;
            qDebug() << "current tag" << currentStyle.tag;
        }
        else if (currentState == EpubExportCssParser::STATE_STYLENAME)
        {
            if (ch.isSpace())
            {
                qDebug() << "add currentStyle :" << currentStyle.tag+"."+currentStyle.name;
                currentStyles << currentStyle;
                currentStyle = EpubExportCssParserStyle();
                currentState = EpubExportCssParser::STATE_TAG;
            }
            else if (ch == '{')
            {
                qDebug() << "add currentStyle {:" << currentStyle.tag+"."+currentStyle.name;
                currentStyles << currentStyle;
                currentStyle = EpubExportCssParserStyle();
                currentState = EpubExportCssParser::STATE_FORMAT;
            }
            else
                currentStyle.name += ch;
        }
        else if (currentState == STATE_FORMAT)
        {
            if (ch == '}')
            {
                // TODO: for each current style
                // qDebug() << "first style name:" <<  currentStyles.first().name;
                if (!currentStyles.isEmpty())
                {
                    QString styleName = currentStyles.first().name;
                    if (styleName == "")
                        styleName = (currentStyles.first().type == 'c' ? "Default Character Style" : "Default Paragraph Style"); 

                    // qDebug() << "first style tag name:" <<  currentStyles.first().tag;
                    // qDebug() << "current format:" << currentFormat.format;
                        style->insert(styleName, currentFormat);
                    currentStyles.clear();
                }
                currentFormat = EpubExportCssParserFormatting();
                currentState = STATE_TAG;
            }
            else
            {
                currentFormat.format += ch;
            }
        }
        else if (currentState == EpubExportCssParser::STATE_COMMENT_START)
        {
            if (ch == '*')
                currentState = EpubExportCssParser::STATE_COMMENT;
            else
                valid = false;
        }
        else if (currentState == EpubExportCssParser::STATE_COMMENT)
        {
            // qDebug() << "/* ch */:" << ch;
            if (ch == '*')
                currentState = EpubExportCssParser::STATE_COMMENT_STOP;
            comment += ch;
        }
        else if (currentState == EpubExportCssParser::STATE_COMMENT_STOP)
        {
            if (ch != '*')
                currentState = EpubExportCssParser::STATE_COMMENT;
            comment += ch;
        }

    }

    qDebug() << "style:" << style;

    file.close();
}

QDebug operator<<(QDebug dbg, const EpubExportCssParserStyle &style)
{
    dbg.nospace() << "("
                  << "tag:" << style.tag
                  << "type:" << style.type
                  << "name:" << style.name
                  << "headingLevel:" << style.headingLevel
                  << ")";
    return dbg.space();
}

QDebug operator<<(QDebug dbg, const EpubExportCssParserFormatting &formatting)
{
    dbg.nospace() << "("
                  << "format:" << formatting.format
                  << ")";
    return dbg.space();
}

    QMap<QString, EpubExportCssParserFormatting> *pStyle;
QDebug operator<<(QDebug dbg, const QMap<QString, EpubExportCssParserFormatting> &style)
{

    QMapIterator<QString, EpubExportCssParserFormatting> i(style);
    while (i.hasNext()) {
        i.next();
        dbg.nospace() << "[" << i.key() << ": " << i.value() << "]";
    }

    return dbg.space();
}
