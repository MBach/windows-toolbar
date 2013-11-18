#include "windowstoolbar.h"

#include <QFile>
#include <QWindow>

#include <QtDebug>

WindowsToolbar::WindowsToolbar()
	: _mainWindow(NULL), _skipBackward(NULL), _playPause(NULL), _stop(NULL), _skipForward(NULL),
	  _taskbarButton(NULL), _taskbarProgress(NULL), _thumbbar(NULL)
{
	/// Should I have to extract "Settings.cpp" and move it into a library?
	_settings = new QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName(), this);
	/*connect(_settings, &Settings::themeHasChanged, [=]() {
		// stuff
	});*/

	// First time (ever) the plugin is loaded
	if (_settings->value("hasProgressBarInTaskbar").isNull()) {
		_settings->setValue("hasProgressBarInTaskbar", true);
	}
	if (_settings->value("hasMediaPlayerButtonsInThumbnail").isNull()) {
		_settings->setValue("hasMediaPlayerButtonsInThumbnail", true);
	}
}

void WindowsToolbar::setMainWindow(QMainWindow *mainWindow)
{
	_mainWindow = mainWindow;
}

void WindowsToolbar::setMediaPlayer(QMediaPlayer *mediaPlayer)
{
	_mediaPlayer = mediaPlayer;
	this->init();
	connect(_mediaPlayer, &QMediaPlayer::positionChanged, [=] (qint64 pos) {
		if (_mediaPlayer->duration() > 0) {
			_taskbarProgress->setValue(100 * pos / _mediaPlayer->duration());
		}
	});
	connect(_mediaPlayer, &QMediaPlayer::stateChanged, this, &WindowsToolbar::updateOverlayIcon);
	connect(_mediaPlayer, &QMediaPlayer::stateChanged, this, &WindowsToolbar::updateThumbnailToolBar);
	connect(_mediaPlayer, &QMediaPlayer::stateChanged, this, &WindowsToolbar::updateProgressbarTaskbar);
}

QWidget* WindowsToolbar::configPage()
{
	QWidget *widget = new QWidget();
	_ui.setupUi(widget);
	// Init the UI with correct values
	_ui.progressBarTaskbar->setChecked(_settings->value("hasProgressBarInTaskbar").toBool());
	_ui.overlayIconTaskbar->setChecked(_settings->value("hasOverlayIcon").toBool());
	_ui.progressBarThumbnail->setChecked(_settings->value("hasProgressBarInThumbnail").toBool());
	_ui.mediaPlayerButtonsThumbnail->setChecked(_settings->value("hasMediaPlayerButtonsInThumbnail").toBool());

	// Connect the UI with the settings
	connect(_ui.progressBarTaskbar, &QCheckBox::stateChanged, [=](int s) {
		_settings->setValue("hasProgressBarInTaskbar", (s == Qt::Checked));
		this->updateProgressbarTaskbar();
	});
	connect(_ui.overlayIconTaskbar, &QCheckBox::stateChanged, [=](int s) {
		_settings->setValue("hasOverlayIcon", (s == Qt::Checked));
		this->updateOverlayIcon();
	});
	connect(_ui.progressBarThumbnail, &QCheckBox::stateChanged, [=](int s) {
		_settings->setValue("hasProgressBarInThumbnail", (s == Qt::Checked));
	});
	connect(_ui.mediaPlayerButtonsThumbnail, &QCheckBox::stateChanged, [=](int s) {
		_settings->setValue("hasMediaPlayerButtonsInThumbnail", (s == Qt::Checked));
		this->showThumbnailButtons(s == Qt::Checked);
	});
	return widget;
}

void WindowsToolbar::init()
{
	// Progress bar in the task bar
	_taskbarButton = new QWinTaskbarButton(this);
	_taskbarButton->setWindow(_mainWindow->windowHandle());
	_taskbarProgress = _taskbarButton->progress();

	// Init visibility of progressBar in the taskBar
	_taskbarProgress->setVisible(_settings->value("hasProgressBarInTaskbar").toBool());

	// Init visibility of overlay icon
	this->updateOverlayIcon();

	// Init visibility of progressBar in the thumbnail
	//this->show

	// Init visibility of media buttons
	this->showThumbnailButtons(_settings->value("hasMediaPlayerButtonsInThumbnail").toBool());
}

void WindowsToolbar::showThumbnailButtons(bool visible)
{
	if (visible) {
		_thumbbar = new QWinThumbnailToolBar(this);
		_thumbbar->setWindow(_mainWindow->windowHandle());

		// Four buttons are enough
		_skipBackward = new QWinThumbnailToolButton(_thumbbar);
		_playPause = new QWinThumbnailToolButton(_thumbbar);
		_stop = new QWinThumbnailToolButton(_thumbbar);
		_skipForward = new QWinThumbnailToolButton(_thumbbar);

		_thumbbar->addButton(_skipBackward);
		_thumbbar->addButton(_playPause);
		_thumbbar->addButton(_stop);
		_thumbbar->addButton(_skipForward);

		_skipBackward->setIcon(QIcon(":/player/" + theme() + "/skipBackward"));
		this->updateThumbnailToolBar();
		_stop->setIcon(QIcon(":/player/" + theme() + "/stop"));
		_skipForward->setIcon(QIcon(":/player/" + theme() + "/skipForward"));

		// Connect each buttons to the main program
		connect(_skipBackward, &QWinThumbnailToolButton::clicked, [=] () { emit skip(false); });
		connect(_skipForward, &QWinThumbnailToolButton::clicked, [=] () { emit skip(true); });
		connect(_playPause, &QWinThumbnailToolButton::clicked, [=]() {
			if (_mediaPlayer->state() == QMediaPlayer::PlayingState) {
				_mediaPlayer->pause();
			} else {
				_mediaPlayer->play();
			}
		});
		connect(_stop, &QWinThumbnailToolButton::clicked, _mediaPlayer, &QMediaPlayer::stop);
	} else if (_thumbbar) {
		///XXX the thumbnail window is not resizing properly when removing buttons?
		_thumbbar->clear();
		delete _thumbbar;
		_thumbbar = NULL;
	}
}

void WindowsToolbar::updateOverlayIcon()
{
	if (_settings->value("hasOverlayIcon").toBool()) {
		switch (_mediaPlayer->state()) {
		// Icons are inverted from updateThumbnailToolBar() method because it's reflecting the actual state of the player
		case QMediaPlayer::PlayingState:
			_taskbarButton->setOverlayIcon(QIcon(":/player/" + theme() + "/play"));
			break;
		case QMediaPlayer::PausedState:
			_taskbarButton->setOverlayIcon(QIcon(":/player/" + theme() + "/pause"));
			break;
		case QMediaPlayer::StoppedState:
			_taskbarButton->setOverlayIcon(_stop->icon());
			break;
		}
	} else {
		_taskbarButton->clearOverlayIcon();
	}
}

void WindowsToolbar::updateProgressbarTaskbar()
{
	switch (_mediaPlayer->state()) {
	case QMediaPlayer::PlayingState:
		_taskbarProgress->resume();
		_taskbarProgress->setVisible(_settings->value("hasProgressBarInTaskbar").toBool());
		break;
	case QMediaPlayer::PausedState:
		_taskbarProgress->pause();
		_taskbarProgress->setVisible(_settings->value("hasProgressBarInTaskbar").toBool());
		break;
	case QMediaPlayer::StoppedState:
		_taskbarProgress->hide();
		break;
	}
}

void WindowsToolbar::updateThumbnailToolBar()
{
	_skipBackward->setIcon(QIcon(":/player/" + theme() + "/skipBackward"));
	if (_mediaPlayer->state() == QMediaPlayer::PlayingState) {
		_playPause->setIcon(QIcon(":/player/" + theme() + "/pause"));
	} else {
		_playPause->setIcon(QIcon(":/player/" + theme() + "/play"));
	}
	_stop->setIcon(QIcon(":/player/" + theme() + "/stop"));
	_skipForward->setIcon(QIcon(":/player/" + theme() + "/skipForward"));
}
