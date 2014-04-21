#ifndef EPUBEXPORTZIP_H
#define EPUBEXPORTZIP_H

#include "third_party/zip/zip.h"

class EpubExportZip : public Zip
{
public:
    EpubExportZip();
    ~EpubExportZip();
public:
    void addString(const QString, QByteArray& content);
    void addString(const QString, QByteArray& content, CompressionLevel level);
    void addString(const QString, QString& content, CompressionLevel level);
};

#endif // EPUBEXPORTZIP_H
