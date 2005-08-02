/*
   Copyright (C) 2001 Carsten Duvenhorst <duvenhorst@m2.uni-hannover.de>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include <kconfig.h>
#include <klineedit.h>
#include <klocale.h>

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qgroupbox.h>
#include <qslider.h>
#include <qtabwidget.h>
#include <kaboutdata.h>
#include <knuminput.h>
#include <qregexp.h>
#include <qlabel.h>

#include <audiocdencoder.h>
#include "kcmaudiocd.moc"
#include <kconfigdialogmanager.h>

KAudiocdModule::KAudiocdModule(QWidget *parent, const char *name)
  : AudiocdConfig(parent, name), configChanged(false)
{
    QString foo = i18n("Report errors found on the cd.");
				
    setButtons(Default|Apply);

    config = new KConfig("kcmaudiocdrc");

    QPtrList<AudioCDEncoder> encoders;
    AudioCDEncoder::find_all_plugins(0, encoders);
    AudioCDEncoder *encoder;
    for ( encoder = encoders.first(); encoder; encoder = encoders.next() ){
      KConfigSkeleton *config = NULL;
      QWidget *widget = encoder->getConfigureWidget(&config);
      if(widget && config){
	 tabWidget->addTab(widget, i18n("%1 Encoder").arg(encoder->type()));
         KConfigDialogManager *configManager = new KConfigDialogManager(widget, config, QString(encoder->type()+" EncoderConfigManager").latin1());
         encoderSettings.append(configManager);
      }
    }

    load();
  
    KConfigDialogManager *widget;
    for ( widget = encoderSettings.first(); widget; widget = encoderSettings.next() ){
      connect(widget, SIGNAL(widgetModified()), this, SLOT(slotModuleChanged()));
    }
   
    //CDDA Options
    connect(cd_autosearch_check,SIGNAL(clicked()),this,SLOT(slotConfigChanged()));
    connect(ec_enable_check,SIGNAL(clicked()),this,SLOT(slotEcEnable()));
    connect(ec_skip_check,SIGNAL(clicked()),SLOT(slotConfigChanged()));
    connect(cd_device_string,SIGNAL(textChanged(const QString &)),SLOT(slotConfigChanged()));

    // File Name
    connect(fileNameLineEdit, SIGNAL(textChanged(const QString &)), this, SLOT(slotConfigChanged()));
    connect(albumNameLineEdit, SIGNAL(textChanged(const QString &)), this, SLOT(slotConfigChanged()));
    connect( kcfg_replaceInput, SIGNAL( textChanged(const QString&) ), this, SLOT( updateExample() ) );
    connect( kcfg_replaceOutput, SIGNAL( textChanged(const QString&) ), this, SLOT( updateExample() ) );
    connect( example, SIGNAL( textChanged(const QString&) ), this, SLOT( updateExample() ) );

    KAboutData *about =
    new KAboutData(I18N_NOOP("kcmaudiocd"), I18N_NOOP("KDE Audio CD IO Slave"),
                   0, 0, KAboutData::License_GPL,
                   I18N_NOOP("(c) 2000 - 2005 Audio CD developers"));

    about->addAuthor("Benjamin C. Meyer", I18N_NOOP("Current Maintainer"), "ben@meyerhome.net");
    about->addAuthor("Carsten Duvenhorst", 0, "duvenhorst@duvnet.de");
    setAboutData(about);
}

KAudiocdModule::~KAudiocdModule()
{
    delete config;
}

void KAudiocdModule::updateExample()
{
  QString text = example->text();
  text.replace( QRegExp(kcfg_replaceInput->text()), kcfg_replaceOutput->text() );
  exampleOutput->setText(text);
}

void KAudiocdModule::defaults() {
	cd_autosearch_check->setChecked(true);
	cd_device_string->setText("/dev/cdrom");

	ec_enable_check->setChecked(true);
	ec_skip_check->setChecked(false);
	niceLevel->setValue(0);
	
	kcfg_replaceInput->setText("");
	kcfg_replaceOutput->setText("");
	example->setText(i18n("Cool artist - example audio file.wav"));
	KConfigDialogManager *widget;
	for ( widget = encoderSettings.first(); widget; widget = encoderSettings.next() ){
		widget->updateWidgetsDefault();
	}

	fileNameLineEdit->setText("%{albumartist} - %{number} - %{title}");
	albumNameLineEdit->setText("%{albumartist} - %{albumtitle}");
}

void KAudiocdModule::save() {
  if (!configChanged ) return;

  {
    KConfigGroupSaver saver(config, "CDDA");

    config->writeEntry("autosearch",cd_autosearch_check->isChecked());
    config->writeEntry("device",cd_device_string->text());
    config->writeEntry("disable_paranoia",!(ec_enable_check->isChecked()));
    config->writeEntry("never_skip",!(ec_skip_check->isChecked()));
    config->writeEntry("niceLevel", niceLevel->value());
  }
  
  {
    KConfigGroupSaver saver(config, "FileName");
    config->writeEntry("file_name_template", fileNameLineEdit->text());
    config->writeEntry("album_name_template", albumNameLineEdit->text());
    config->writeEntry("regexp_search",kcfg_replaceInput->text());
    config->writeEntry("regexp_replace", kcfg_replaceOutput->text());
    config->writeEntry("regexp_example", example->text());
  }

  KConfigDialogManager *widget;
  for ( widget = encoderSettings.first(); widget; widget = encoderSettings.next() ){
    widget->updateSettings();
  }
  
  config->sync();

  configChanged = false;

}

void KAudiocdModule::load() {

  {
    KConfigGroupSaver saver(config, "CDDA");

    cd_autosearch_check->setChecked(config->readBoolEntry("autosearch",true));
    cd_device_string->setText(config->readEntry("device","/dev/cdrom"));
    ec_enable_check->setChecked(!(config->readBoolEntry("disable_paranoia",false)));
    ec_skip_check->setChecked(!(config->readBoolEntry("never_skip",true)));
    niceLevel->setValue(config->readNumEntry("niceLevel", 0));
  }

  {
    KConfigGroupSaver saver(config, "FileName");
    fileNameLineEdit->setText(config->readEntry("file_name_template", "%{albumartist} - %{number} - %{title}"));
    albumNameLineEdit->setText(config->readEntry("album_name_template", "%{albumartist} - %{albumtitle}"));
    kcfg_replaceInput->setText(config->readEntry("regexp_search"));
    kcfg_replaceOutput->setText(config->readEntry("regexp_replace"));
    example->setText(config->readEntry("example", i18n("Cool artist - example audio file.wav")));
  }
  
  KConfigDialogManager *widget;
  for ( widget = encoderSettings.first(); widget; widget = encoderSettings.next() ){
    widget->updateWidgets();
  }
}

void KAudiocdModule::slotModuleChanged() {
	KConfigDialogManager *widget;
	for ( widget = encoderSettings.first(); widget; widget = encoderSettings.next() ){
		if(widget->hasChanged()){
			slotConfigChanged();
			break;
		}
	}
}

void KAudiocdModule::slotConfigChanged() {
	configChanged = true;
	emit changed(true);
}

/*
#    slot for the error correction settings
*/
void KAudiocdModule::slotEcEnable() {
  if (!(ec_skip_check->isChecked())) {
    ec_skip_check->setChecked(true);
  } else {
    if (ec_skip_check->isEnabled()) {
      ec_skip_check->setChecked(false);
    }
  }

  slotConfigChanged();
}

QString KAudiocdModule::quickHelp() const
{
  return i18n("<h1>Audio CDs</h1> The Audio CD IO-Slave enables you to easily"
              " create wav, MP3 or Ogg Vorbis files from your audio CD-ROMs or DVDs."
              " The slave is invoked by typing <i>\"audiocd:/\"</i> in Konqueror's location"
              " bar. In this module, you can configure"
              " encoding, and device settings. Note that MP3 and Ogg"
              " Vorbis encoding are only available if KDE was built with a recent"
              " version of the LAME or Ogg Vorbis libraries.");
}

extern "C"
{
    KCModule *create_audiocd(QWidget *parent, const char */*name*/)
    {
        return new KAudiocdModule(parent, "kcmaudiocd");
    }

}
