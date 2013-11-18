#ifndef WINDOWSTOOLBAR_H
#define WINDOWSTOOLBAR_H

#include "mediaplayerplugininterface.h"

#include "ui_config.h"

#include <QtWinExtras>
#include <QMainWindow>

class WindowsToolbar : public MediaPlayerPluginInterface
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID MediaPlayerPluginInterface_iid)
	Q_INTERFACES(MediaPlayerPluginInterface)
private:
	Ui::ConfigForm *_ui;

	QMainWindow *_mainWindow;

	QString _theme;
	QMediaPlayer *_mediaPlayer;

	QWinThumbnailToolButton *_skipBackward;
	QWinThumbnailToolButton *_playPause;
	QWinThumbnailToolButton *_stop;
	QWinThumbnailToolButton *_skipForward;

	QWinTaskbarButton* _taskbarButton;
	QWinTaskbarProgress* _taskbarProgress;

	QSettings *_settings;

public:
	WindowsToolbar();

	virtual ~WindowsToolbar();

	virtual QString name() const { return "WindowsToolBar"; }

	virtual QString version() const { return "0.1"; }

	QWidget *configPage();

	void setMainWindow(QMainWindow *mainWindow);

	void setMediaPlayer(QMediaPlayer *mediaPlayer);

private:
	void init();

private slots:
	void updateTaskbar();
	void updateThumbnailToolBar();
};

#endif // WINDOWSTOOLBAR_H
