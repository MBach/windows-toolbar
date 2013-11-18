#include "windowstoolbar.h"

#include <QFile>
#include <QWindow>

#include <QtDebug>

WindowsToolbar::WindowsToolbar()
	: _mainWindow(NULL), _skipBackward(NULL), _playPause(NULL), _stop(NULL), _skipForward(NULL),
	  _taskbarButton(NULL), _taskbarProgress(NULL)
{
	_ui = new Ui::ConfigForm();
	/// Should I have to extract "Settings.cpp" and move it into a library?
	_settings = new QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
	_theme = _settings->value("theme").toString();
	if (_theme.isEmpty()) {
		_theme = "oxygen";
	}
}

WindowsToolbar::~WindowsToolbar()
{
	delete _ui;
}

void WindowsToolbar::setMainWindow(QMainWindow *mainWindow)
{
	_mainWindow = mainWindow;
}

void WindowsToolbar::setMediaPlayer(QMediaPlayer *mediaPlayer)
{
	_mediaPlayer = mediaPlayer;
	connect(_mediaPlayer, &QMediaPlayer::positionChanged, this, &WindowsToolbar::updateTaskbar);
	//connect(_mediaPlayer, &QMediaPlayer::durationChanged, this, &WindowsToolbar::updateThumbnailToolBar);
	connect(_mediaPlayer, &QMediaPlayer::stateChanged, this, &WindowsToolbar::updateThumbnailToolBar);
	connect(_mediaPlayer, &QMediaPlayer::stateChanged, this, &WindowsToolbar::updateTaskbar);
	this->init();
}

QWidget* WindowsToolbar::configPage()
{
	QWidget *widget = new QWidget();
	_ui->setupUi(widget);
	// Init the UI with correct values
	_ui->progressBarTaskbar->setChecked(_settings->value("hasProgressBarInTaskbar").toBool());
	_ui->overlayIconTaskbar->setChecked(_settings->value("hasOverlayIcon").toBool());
	_ui->progressBarThumbnail->setChecked(_settings->value("hasProgressBarInThumbnail").toBool());
	_ui->mediaPlayerButtonsThumbnail->setChecked(_settings->value("hasMediaPlayerButtonsInThumbnail").toBool());

	// Connect the UI with the settings
	connect(_ui->progressBarTaskbar, &QCheckBox::stateChanged, [=](int s) {
		_settings->setValue("hasProgressBarInTaskbar", (s == Qt::Checked));
	});
	connect(_ui->overlayIconTaskbar, &QCheckBox::stateChanged, [=](int s) {
		_settings->setValue("hasOverlayIcon", (s == Qt::Checked));
	});
	connect(_ui->progressBarThumbnail, &QCheckBox::stateChanged, [=](int s) {
		_settings->setValue("hasProgressBarInThumbnail", (s == Qt::Checked));
	});
	connect(_ui->mediaPlayerButtonsThumbnail, &QCheckBox::stateChanged, [=](int s) {
		_settings->setValue("hasMediaPlayerButtonsInThumbnail", (s == Qt::Checked));
	});
	return widget;
}

void WindowsToolbar::init()
{
	// Thumbnail buttons in the tool bar
	QWinThumbnailToolBar *thumbbar = new QWinThumbnailToolBar(this);
	thumbbar->setWindow(_mainWindow->windowHandle());

	// Four buttons are enough
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

	// Connect each buttons to the main program
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

	// Progress bar in the task bar
	_taskbarButton = new QWinTaskbarButton(this);
	_taskbarButton->setWindow(_mainWindow->windowHandle());
	_taskbarProgress = _taskbarButton->progress();
}

void WindowsToolbar::updateTaskbar()
{
	QMediaPlayer::State state = _mediaPlayer->state();
	if (_settings->value("hasProgressBarInTaskbar").toBool()) {
		if (_mediaPlayer->duration() > 0) {
			_taskbarProgress->setValue(100 * _mediaPlayer->position() / _mediaPlayer->duration());
		}
		switch (state) {
		case QMediaPlayer::PlayingState:
			_taskbarProgress->show();
			_taskbarProgress->resume();
			break;
		case QMediaPlayer::PausedState:
			_taskbarProgress->show();
			_taskbarProgress->pause();
			break;
		case QMediaPlayer::StoppedState:
			_taskbarProgress->hide();
			break;
		}
	} else {
		_taskbarProgress->hide();
	}

	if (_settings->value("hasOverlayIcon").toBool()) {
		switch (state) {
		case QMediaPlayer::PlayingState:
			_taskbarButton->setOverlayIcon(QIcon(":/player/" + _theme + "/play"));
			break;
		case QMediaPlayer::PausedState:
			_taskbarButton->setOverlayIcon(QIcon(":/player/" + _theme + "/pause"));
			break;
		case QMediaPlayer::StoppedState:
			_taskbarButton->setOverlayIcon(_stop->icon());
			break;
		}
	} else {
		_taskbarButton->clearOverlayIcon();
	}
}

void WindowsToolbar::updateThumbnailToolBar()
{
	if (_mediaPlayer->state() == QMediaPlayer::PlayingState) {
		_playPause->setIcon(QIcon(":/player/" + _theme + "/pause"));
	} else {
		_playPause->setIcon(QIcon(":/player/" + _theme + "/play"));
	}
}
