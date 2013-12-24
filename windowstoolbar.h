#ifndef WINDOWSTOOLBAR_H
#define WINDOWSTOOLBAR_H

#include "mediaplayerplugininterface.h"

#include "ui_config.h"

#include <QtWinExtras>
#include <QMainWindow>

#include <settings.h>
#include <mediaplayer.h>

#include "miamcore_global.h"

class WindowsToolbar : public QObject, public MediaPlayerPluginInterface
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID MediaPlayerPluginInterface_iid)
	Q_INTERFACES(MediaPlayerPluginInterface)

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
	WindowsToolbar();

    inline virtual QString name() const { return "WindowsToolBar"; }

    inline virtual QString version() const { return "1.0"; }

    inline virtual bool providesView() const { return false; }

    virtual QWidget *configPage();

    virtual void setMediaPlayer(QWeakPointer<MediaPlayer>);

private:
	void init();
	void showThumbnailButtons(bool visible);

private slots:
	/** Update the cover when the current media in the player has changed. */
	void updateCover();

	void updateOverlayIcon();

	void updateProgressbarTaskbar();

	/** Update icons for buttons. */
	void updateThumbnailToolBar();
};

#endif // WINDOWSTOOLBAR_H
