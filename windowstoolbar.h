#ifndef WINDOWSTOOLBAR_H
#define WINDOWSTOOLBAR_H

#include <QtWinExtras>

#include "miamcore_global.h"
#include "interfaces/mediaplayerplugin.h"
#include "model/trackdao.h"
#include "mediaplayer.h"
#include "settings.h"

#include "ui_config.h"

/**
 * \brief       Plugin to enable Windows 7 Features
 * \details     The WindowsToolbar class is a plugin for Windows only which enables Windows 7 features.
 *      It shows a Progress Bar when reading files, Taskbar Buttons to control the media player, and the current cover in preview
 * \author      Matthieu Bachelier
 * \version     1.0
 * \copyright   GNU General Public License v3
 */
class WindowsToolbar : public QObject, public MediaPlayerPlugin
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID MediaPlayerPlugin_iid)
	Q_INTERFACES(MediaPlayerPlugin)

private:
	Ui::ConfigForm _ui;

	QWeakPointer<MediaPlayer> _mediaPlayer;

	QWinThumbnailToolButton *_skipBackward;
	QWinThumbnailToolButton *_playPause;
	QWinThumbnailToolButton *_stop;
	QWinThumbnailToolButton *_skipForward;

	QWinTaskbarButton* _taskbarButton;
	QWinTaskbarProgress* _taskbarProgress;

	QWinThumbnailToolBar *_thumbbar;

	Settings *_settings;

public:
	explicit WindowsToolbar();

	virtual ~WindowsToolbar();

	virtual QWidget *configPage();

	inline virtual bool isConfigurable() const { return true; }

	inline virtual QString name() const { return "WindowsToolBar"; }

	inline virtual QWidget* providesView() { return NULL; }

	virtual void setMediaPlayer(QWeakPointer<MediaPlayer>);

	inline virtual QString version() const { return "1.0"; }

private:
	void init();
	void showThumbnailButtons(bool visible);

private slots:
	/** Update the cover when the current media in the player has changed. */
	void updateCover(const QString &uri);

	void updateOverlayIcon();

	void updateProgressbarTaskbar();

	/** Update icons for buttons. */
	void updateThumbnailToolBar();
};

#endif // WINDOWSTOOLBAR_H
