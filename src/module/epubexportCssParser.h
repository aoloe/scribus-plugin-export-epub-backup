#ifndef EPUBEXPORTCSSPARSE_H
#define EPUBEXPORTCSSPARSE_H

#include <QObject>
#include <QMap>

struct EpubExportCssParserStyle
{
    QString tag; // p, h1..9, em, strong, b, i, ...
    QChar type; // [p]aragraph, [c]haracter, [i]d, [ ]all
    QString name;
    int headingLevel;
};

struct EpubExportCssParserFormatting
{
    QString format; // TODO: replace it with a list of property => format
};

class EpubExportCssParser : public QObject
{
    Q_OBJECT
public:
    EpubExportCssParser();
    ~EpubExportCssParser();
    void setFilepath(QString filepath) {this->filepath = filepath;}
    void start();

private:
    static const int STATE_TAG;
    static const int STATE_STYLENAME;
    static const int STATE_FORMAT;
    static const int STATE_PROPERTY_NAME;
    static const int STATE_PROPERTY_VALUE;
    static const int STATE_COMMENT_START;
    static const int STATE_COMMENT;
    static const int STATE_COMMENT_STOP;

    QString filepath;

    QMap<QString, EpubExportCssParserFormatting> *style; // TODO: only for first testing replace with the ones below
    QMap<QString, EpubExportCssParserFormatting> *pStyle;
    QMap<QString, EpubExportCssParserFormatting> *spanStyle;
    QMap<QString, EpubExportCssParserFormatting> *hStyle;
    QMap<QString, EpubExportCssParserFormatting> *idStyle;
};

QDebug operator<<(QDebug dbg, const EpubExportCssParserStyle &style);
QDebug operator<<(QDebug dbg, const EpubExportCssParserFormatting &formatting);

/*
 this is a modified syntax build upon the example from
 http://echopoint.sourceforge.net/LinkedArticles/CascadingStyleSheets.html
 we will need to add supoort (and ignore) for
 quick overlook, in the BNF, there is no definition for :
  - comments
  - @import
  - @media
  - "sub styles" are not recognized (ul li { }) or only the last one is recognized

styleEntry    ::= <classNameSeq> "{"
        [ <styleAttributeSeq> ]
       "}"
classNameSeq  ::= <classNameSelector> { "," <classNameSelector> }
styleAttributeSeq  ::= attrName ":" attrValue ";"
classNameSelector ::= <javaClassName>   |
      <javaClassName>#<id>  |
      <javaClassName>!<group>
javaClassName  ::= {<javaPackageName> "." } <identifier>
javaPackageName  ::= <identifier>
id      ::= <identifier>
group    ::= <identifier>
attrName   ::= <identifier>
attrValue   ::= <string>   |
      <number>   |
      "null"   |
      <color>   |
      <font>   |
      <insets>  |
      <image>   |
      <corners>  |
      <rect>   |
      <intarray>
color    ::= "#" <hexnumber> |
      "color(" <number> "," <number> "," <number> ")" |
      "rgb("   <number> "," <number> "," <number> ")" |
      "black" | "blue" | "cyan" | "darkgray" | "green" |
      "lightgray" | "magenta" | "orange" | "pink" | "red" |
      "white" | "yellow"
insets    ::= "insets(" <number> "," <number> "," <number> "," <number> ")" |
      "insets(" <number> "," <number>  ")" |
      "insets(" <number> ")"
image    ::= "image(" <string> "," <number> "," <number> ")" |
      "image(" <string> ")"
corners    ::= "corners(" <string> "," <number> "," <number> "," <string> "," <number> "," <number> ")" |
      "corners(" <string> "," <number> "," <number> ")"
rect    ::= "rect(" <number> "," <number> "," <number> "," <number> ")"
intarray   ::= "int(" <number> [ "," <number> ] ")"
font    ::= <fontName> "," <fontStyle> "," <fontSize> |
      "font(" <fontName> "," <fontStyle> "," <fontSize> ")"
fontName   ::= "arial" | "helvetica" | "monospace" | "sans_serif" | "sans serif" |
      "serif" | "times" | "times_new_roman" | "times new roman" | "times_roman" |
      "times roman" | "verdana"
fontStyle   ::= "plain" | "bold" | "italic" | "underline" [ "|" <fontStyle> ]
fontSize   ::= <number>
identiifer    ::= <letter> { <letter> | <digit> | "_" }
string    ::= any <letter> | """ { any <letter> } """ | "'" { any <letter> } "'"
number    ::= <digit> { <digit> }
hexnumber   ::= <hexdigit> { <hexdigit> }
hexdigit   ::= <digit> | "A" | "B" | "C" | "D" | "E" | "F"
digit    ::= "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9"
*/


#endif // EPUBEXPORTCSSPARSE_H

