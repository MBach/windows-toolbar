#include "windowstoolbar.h"

#include <QtUiTools/QUiLoader>
#include <QFile>

#include <QtDebug>

#include <QWindow>

WindowsToolbar::WindowsToolbar()
	: _mainWindow(NULL), _skipBackward(NULL), _playPause(NULL), _stop(NULL), _skipForward(NULL)
{
	_configPage = new Ui::ConfigForm();
	/// Hardcoded = wrong! How can I use my custom QSettings class?
	QSettings settings("MmeMiamMiam", "MmeMiamMiamMusicPlayer");
	_theme = settings.value("theme").toString();
	if (_theme.isEmpty()) {
		_theme = "oxygen";
	}
}

WindowsToolbar::~WindowsToolbar()
{
	delete _configPage;
}

void WindowsToolbar::setMainWindow(QMainWindow *mainWindow)
{
	_mainWindow = mainWindow;
}

void WindowsToolbar::setMediaPlayer(QMediaPlayer *mediaPlayer) {
	_mediaPlayer = mediaPlayer;
	connect(_mediaPlayer, &QMediaPlayer::positionChanged, this, &WindowsToolbar::updateThumbnailToolBar);
	connect(_mediaPlayer, &QMediaPlayer::durationChanged, this, &WindowsToolbar::updateThumbnailToolBar);
	connect(_mediaPlayer, &QMediaPlayer::stateChanged, this, &WindowsToolbar::updateThumbnailToolBar);
	this->init();
}

QWidget* WindowsToolbar::configPage()
{
	QFile file(":/windows-toolbar/config.ui");
	file.open(QFile::ReadOnly);
	QUiLoader loader;
	QWidget *formWidget = loader.load(&file);
	file.close();
	return formWidget;
}

void WindowsToolbar::init()
{
	QWinThumbnailToolBar *thumbbar = new QWinThumbnailToolBar(this);
	thumbbar->setWindow(_mainWindow->windowHandle());

	_skipBackward = new QWinThumbnailToolButton(thumbbar);
	_playPause = new QWinThumbnailToolButton(thumbbar);
	_stop = new QWinThumbnailToolButton(thumbbar);
	_skipForward = new QWinThumbnailToolButton(thumbbar);

	_skipBackward->setIcon(QIcon(":/player/" + _theme + "/skipBackward"));
	_playPause->setIcon(QIcon(":/player/" + _theme + "/play"));
	_stop->setIcon(QIcon(":/player/" + _theme + "/stop"));
	_skipForward->setIcon(QIcon(":/player/" + _theme + "/skipForward"));

	thumbbar->addButton(_skipBackward);
	thumbbar->addButton(_playPause);
	thumbbar->addButton(_stop);
	thumbbar->addButton(_skipForward);

	connect(_skipBackward, &QWinThumbnailToolButton::clicked, [=] () { emit skip(false); });
	connect(_skipForward, &QWinThumbnailToolButton::clicked, [=] () { emit skip(true); });
	connect(_playPause, &QWinThumbnailToolButton::clicked, [=]() {
		if (_mediaPlayer->state() == QMediaPlayer::PlayingState) {
			_mediaPlayer->pause();
			_playPause->setIcon(QIcon(":/player/" + _theme + "/play"));
		} else {
			_mediaPlayer->play();
			_playPause->setIcon(QIcon(":/player/" + _theme + "/pause"));
		}
	});
	connect(_stop, &QWinThumbnailToolButton::clicked, _mediaPlayer, &QMediaPlayer::stop);
}

void WindowsToolbar::updateThumbnailToolBar()
{
	if (_mediaPlayer->state() == QMediaPlayer::PlayingState) {
		_playPause->setIcon(QIcon(":/player/" + _theme + "/pause"));
	} else {
		_playPause->setIcon(QIcon(":/player/" + _theme + "/play"));
	}
}
