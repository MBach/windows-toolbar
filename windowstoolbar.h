#ifndef WINDOWSTOOLBAR_H
#define WINDOWSTOOLBAR_H

#include <QtWinExtras>

#include "miamcore_global.h"
#include "interfaces/mediaplayerplugin.h"
#include "model/trackdao.h"
#include "mediaplayer.h"

#include "ui_config.h"

/**
 * \brief       Plugin to enable Windows 7 Features
 * \details     The WindowsToolbar class is a plugin for Windows only which enables Windows 7 features.
 *      It shows a Progress Bar when reading files, Taskbar Buttons to control the media player, and the current cover in preview
 * \author      Matthieu Bachelier
 * \version     1.0
 * \copyright   GNU General Public License v3
 */
class WindowsToolbar : public MediaPlayerPlugin
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID MediaPlayerPlugin_iid)
	Q_INTERFACES(MediaPlayerPlugin)

private:
	Ui::ConfigForm _ui;

	MusicSearchEngine *_musicSearchEngine;
	AbstractMediaPlayerControl *_mediaPlayerControl;

	QWinThumbnailToolButton *_skipBackward;
	QWinThumbnailToolButton *_playPause;
	QWinThumbnailToolButton *_stop;
	QWinThumbnailToolButton *_skipForward;
	QWinThumbnailToolButton *_toggleShuffle;

	QWinTaskbarButton* _taskbarButton;
	QWinTaskbarProgress* _taskbarProgress;

	QWinThumbnailToolBar *_thumbbar;

public:
	explicit WindowsToolbar(QObject *parent = nullptr);

	virtual ~WindowsToolbar();

	virtual QWidget *configPage() override;

	inline virtual QStringList extensions() const override { return QStringList(); }

	inline virtual bool hasView() const override { return false; }

	virtual void init() override;

	inline virtual bool isConfigurable() const override { return true; }

	inline virtual QString name() const override { return "WindowsToolBar"; }

	inline virtual void setMusicSearchEngine(MusicSearchEngine *musicSearchEngine) { _musicSearchEngine = musicSearchEngine; }

	virtual void setMediaPlayerControl(AbstractMediaPlayerControl *mediaPlayerControl) override;

	inline virtual QString version() const override { return "1.1"; }

private:
	void showThumbnailButtons(bool visible);

private slots:
	/** Update the cover when the current media in the player has changed. */
	void updateCover(const QString &uri = QString());

	void updateOverlayIcon();

	void updateProgressbarTaskbar();

	/** Update icons for buttons. */
	void updateThumbnailToolBar();
};

#endif // WINDOWSTOOLBAR_H
