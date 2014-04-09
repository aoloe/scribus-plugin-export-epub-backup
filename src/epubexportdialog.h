/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef EPUBEXPORTDIALOG_H
#define EPUBEXPORTDIALOG_H

#include "ui_epubexportdialog.h"

#include <QList>
#include "epubexport.h"

class ScribusDoc;
// class EpubExport;
// class EpubExportOptions;

class EpubExportDialog : public QDialog, Ui::EpubExportDialog
{
	Q_OBJECT
	public:
		EpubExportDialog( QWidget* parent = 0, ScribusDoc* doc = 0, const char* name = 0, bool modal = FALSE, Qt::WFlags fl = 0 );
		~EpubExportDialog();
        void setOptions(EpubExportOptions options) {this->options = options;}
        EpubExportOptions getOptions() {return options;}

	private:
		//! \brief a parent doc reference
		ScribusDoc* m_Doc;
        EpubExportOptions options;
	private slots:

		void cancelButton_clicked();
		void exportButton_clicked();
		void chooseFile();
        void enableIntervalPages(bool enabled);
        void enableImageMaxWidth(bool enabled);
        QList<int> getPagesListFromString(const QString pages);
        void readIntervalFromDialog();
		void fileOutputChanged();

};

#endif // EPUBEXPORTDIALOG_H
