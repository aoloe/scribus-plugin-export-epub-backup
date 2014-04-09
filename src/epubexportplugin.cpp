/***************************************************************************
    begin                : Wed Mar 28 08:00:00 CEST 2012
    copyright            : (C) 2012 by Ale Rimoldi
    email                : a.l.e@ideale.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QMessageBox>

#include "epubexport.h"
#include "epubexportplugin.h"
#include "epubexportdialog.h"

#include "scribuscore.h"
#include "scribusdoc.h"

#include "ui/multiprogressdialog.h"

int epubexportplugin_getPluginAPIVersion()
{
	return PLUGIN_API_VERSION;
}

ScPlugin* epubexportplugin_getPlugin()
{
	EpubExportPlugin* plug = new EpubExportPlugin();
	Q_CHECK_PTR(plug);
	return plug;
}

void epubexportplugin_freePlugin(ScPlugin* plugin)
{
	EpubExportPlugin* plug = dynamic_cast<EpubExportPlugin*>(plugin);
	Q_ASSERT(plug);
	delete plug;
}

EpubExportPlugin::EpubExportPlugin() : ScActionPlugin()
{
	// Set action info in languageChange, so we only have to do
	// it in one place.
	languageChange();
}

EpubExportPlugin::~EpubExportPlugin()
{
	// unregisterAll();
};

void EpubExportPlugin::languageChange()
{
	// Note that we leave the unused members unset. They'll be initialised
	// with their default ctors during construction.
	// Action name
	m_actionInfo.name = "ExportAsEPUB";
	// Action text for menu, including accel
	m_actionInfo.text = tr("Save as &EPUB...");
	// Menu
	m_actionInfo.menu = "FileExport";
	m_actionInfo.enabledOnStartup = false;
	m_actionInfo.needsNumObjects = -1;
}

const QString EpubExportPlugin::fullTrName() const
{
	return QObject::tr("EPUB Export");
}

const ScActionPlugin::AboutData* EpubExportPlugin::getAboutData() const
{
	AboutData* about = new AboutData;
	about->authors = "Ale Rimoldi <a.l.e@ideale.ch>";
	about->shortDescription = tr("Export EPUB files");
	about->description = tr("Exports the current document to EUPB files,\nretaining as much as possible of the formatting.");
	about->license = "GPL";
	Q_CHECK_PTR(about);
	return about;
}

void EpubExportPlugin::deleteAboutData(const AboutData* about) const
{
	Q_ASSERT(about);
	delete about;
}

bool EpubExportPlugin::run(ScribusDoc* doc, QString target)
{
	Q_ASSERT(target.isNull());
	ScribusDoc* currDoc=doc;
	if (currDoc == 0)
		currDoc = ScCore->primaryMainWindow()->doc;
	if (currDoc == 0)
		return false;

    EpubExportOptions options;

	EpubExportDialog *dialog = new EpubExportDialog(currDoc->scMW(), currDoc, "dlg", true, 0);
    dialog->setOptions(options);
	if (dialog)
	{
        if (dialog->exec() == QDialog::Accepted)
        {

            options = dialog->getOptions();
            // qDebug() << "options:" << options;
            EpubExport *model = new EpubExport(currDoc);

            // TODO: find a way to avoid to pass the progress dialog to the model... eventually, it should
            // be the model that communicates back its progresses to the plugin...
            MultiProgressDialog* progressDialog = new MultiProgressDialog(tr("Exporting: %1").arg(options.targetFilename), CommonStrings::tr_Cancel, currDoc->scMW());
            progressDialog->setOverallTotalSteps(0);
            progressDialog->setOverallProgress(0);
            progressDialog->show();
            connect(progressDialog, SIGNAL(canceled()), model, SLOT(cancelRequested()));
            qApp->processEvents();

            model->setProgressDialog(progressDialog);

            model->setOptions(options);
            model->doExport();

            progressDialog->hide();
            delete progressDialog;
            delete model;
        }
		delete dialog;
		return true;
	}
	else
		return false;

}
