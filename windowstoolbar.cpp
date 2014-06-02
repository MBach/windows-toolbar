#include "windowstoolbar.h"

#include <QFile>
#include <QGuiApplication>
#include <QLabel>
#include <QLibrary>
#include <QMediaContent>
#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <QWindow>

#include "cover.h"

#include <QtDebug>

WindowsToolbar::WindowsToolbar()
	: QObject(), _skipBackward(NULL), _playPause(NULL), _stop(NULL), _skipForward(NULL),
	  _taskbarProgress(NULL), _thumbbar(NULL)
{
	_settings = Settings::getInstance();
	connect(_settings, &Settings::themeHasChanged, this, &WindowsToolbar::updateThumbnailToolBar);

	// First time (ever) the plugin is loaded
	if (_settings->value("WindowsToolbar/hasProgressBarInTaskbar").isNull()) {
		_settings->setValue("WindowsToolbar/hasProgressBarInTaskbar", true);
	}
	if (_settings->value("WindowsToolbar/hasMediaPlayerButtonsInThumbnail").isNull()) {
		_settings->setValue("WindowsToolbar/hasMediaPlayerButtonsInThumbnail", true);
	}
	if (_settings->value("WindowsToolbar/hasOverlayIcon").isNull()) {
		_settings->setValue("WindowsToolbar/hasOverlayIcon", false);
	}

	_taskbarButton = new QWinTaskbarButton(this);
}

WindowsToolbar::~WindowsToolbar()
{
	_taskbarButton->clearOverlayIcon();
	_taskbarProgress->hide();
	this->showThumbnailButtons(false);
}

void WindowsToolbar::setMediaPlayer(QWeakPointer<MediaPlayer> mediaPlayer)
{
	_mediaPlayer = mediaPlayer;
	this->init();
	connect(_mediaPlayer.data(), &QMediaPlayer::positionChanged, [=] (qint64 pos) {
		if (_mediaPlayer.data()->duration() > 0) {
			_taskbarProgress->setValue(100 * pos / _mediaPlayer.data()->duration());
		}
	});
	connect(_mediaPlayer.data(), &QMediaPlayer::stateChanged, this, &WindowsToolbar::updateOverlayIcon);
	connect(_mediaPlayer.data(), &QMediaPlayer::stateChanged, this, &WindowsToolbar::updateThumbnailToolBar);
	connect(_mediaPlayer.data(), &QMediaPlayer::stateChanged, this, &WindowsToolbar::updateProgressbarTaskbar);
	connect(_mediaPlayer.data(), &QMediaPlayer::currentMediaChanged, this, &WindowsToolbar::updateCover);
}

QWidget* WindowsToolbar::configPage()
{
	QWidget *widget = new QWidget();
	_ui.setupUi(widget);
	// Init the UI with correct values
	_ui.progressBarTaskbar->setChecked(_settings->value("WindowsToolbar/hasProgressBarInTaskbar").toBool());
	_ui.overlayIconTaskbar->setChecked(_settings->value("WindowsToolbar/hasOverlayIcon").toBool());
	_ui.progressBarThumbnail->setChecked(_settings->value("WindowsToolbar/hasProgressBarInThumbnail").toBool());
	_ui.mediaPlayerButtonsThumbnail->setChecked(_settings->value("WindowsToolbar/hasMediaPlayerButtonsInThumbnail").toBool());

	// Connect the UI with the settings
	connect(_ui.progressBarTaskbar, &QCheckBox::stateChanged, [=](int s) {
		_settings->setValue("WindowsToolbar/hasProgressBarInTaskbar", (s == Qt::Checked));
		this->updateProgressbarTaskbar();
	});
	connect(_ui.overlayIconTaskbar, &QCheckBox::stateChanged, [=](int s) {
		_settings->setValue("WindowsToolbar/hasOverlayIcon", (s == Qt::Checked));
		this->updateOverlayIcon();
	});
	connect(_ui.progressBarThumbnail, &QCheckBox::stateChanged, [=](int s) {
		_settings->setValue("WindowsToolbar/hasProgressBarInThumbnail", (s == Qt::Checked));
	});
	connect(_ui.mediaPlayerButtonsThumbnail, &QCheckBox::stateChanged, [=](int s) {
		_settings->setValue("WindowsToolbar/hasMediaPlayerButtonsInThumbnail", (s == Qt::Checked));
		this->showThumbnailButtons(s == Qt::Checked);
	});
	return widget;
}

void WindowsToolbar::init()
{
	// Progress bar in the task bar
	_taskbarButton->setWindow(QGuiApplication::topLevelWindows().first());
	_taskbarProgress = _taskbarButton->progress();

	// If one has switched to another view (like a plugin which brings a new view mode), reroute these buttons
	connect(_taskbarButton->window(), &QWindow::activeChanged, this, [=](){
		foreach (QWindow *w, QGuiApplication::topLevelWindows()) {
			if (w->isVisible()) {
				_taskbarButton->setWindow(w);
				_thumbbar->setWindow(w);
				break;
			}
		}
	});

	// Init visibility of progressBar in the taskBar
	_taskbarProgress->setVisible(_settings->value("WindowsToolbar/hasProgressBarInTaskbar").toBool());

	// Init visibility of overlay icon
	this->updateOverlayIcon();

	// Init visibility of progressBar in the thumbnail

	// Init visibility of media buttons
	this->showThumbnailButtons(_settings->value("WindowsToolbar/hasMediaPlayerButtonsInThumbnail").toBool());
}

void WindowsToolbar::showThumbnailButtons(bool visible)
{
	if (visible) {
		_thumbbar = new QWinThumbnailToolBar(this);
		_thumbbar->setWindow(QGuiApplication::topLevelWindows().first());

		// Four buttons are enough
		_skipBackward = new QWinThumbnailToolButton(_thumbbar);
		_playPause = new QWinThumbnailToolButton(_thumbbar);
		_stop = new QWinThumbnailToolButton(_thumbbar);
		_skipForward = new QWinThumbnailToolButton(_thumbbar);

		_thumbbar->addButton(_skipBackward);
		_thumbbar->addButton(_playPause);
		_thumbbar->addButton(_stop);
		_thumbbar->addButton(_skipForward);

		_skipBackward->setIcon(QIcon(":/player/" + _settings->theme() + "/skipBackward"));
		this->updateThumbnailToolBar();
		_stop->setIcon(QIcon(":/player/" + _settings->theme() + "/stop"));
		_skipForward->setIcon(QIcon(":/player/" + _settings->theme() + "/skipForward"));

		// Connect each buttons to the main program
		connect(_skipBackward, &QWinThumbnailToolButton::clicked, _mediaPlayer.data(), &MediaPlayer::skipBackward);
		connect(_skipForward, &QWinThumbnailToolButton::clicked, _mediaPlayer.data(), &MediaPlayer::skipForward);
		connect(_playPause, &QWinThumbnailToolButton::clicked, [=]() {
			if (_mediaPlayer.data()->state() == QMediaPlayer::PlayingState) {
				_mediaPlayer.data()->pause();
			} else {
				_mediaPlayer.data()->play();
			}
		});
		connect(_stop, &QWinThumbnailToolButton::clicked, _mediaPlayer.data(), &QMediaPlayer::stop);
	} else if (_thumbbar) {
		///XXX the thumbnail window is not resizing properly when removing buttons?
		_thumbbar->clear();
		delete _thumbbar;
		_thumbbar = NULL;
	}
}

/** Update the cover when the current media in the player has changed. */
void WindowsToolbar::updateCover(const QMediaContent &)
{
	/// TODO Qt 5.4
}

void WindowsToolbar::updateOverlayIcon()
{
	if (_settings->value("WindowsToolbar/hasOverlayIcon").toBool()) {
		qDebug() << (_stop == NULL);
		switch (_mediaPlayer.data()->state()) {
		// Icons are inverted from updateThumbnailToolBar() method because it's reflecting the actual state of the player
		case QMediaPlayer::PlayingState:
			_taskbarButton->setOverlayIcon(QIcon(":/player/" + _settings->theme() + "/play"));
			break;
		case QMediaPlayer::PausedState:
			_taskbarButton->setOverlayIcon(QIcon(":/player/" + _settings->theme() + "/pause"));
			break;
		case QMediaPlayer::StoppedState:
			_taskbarButton->setOverlayIcon(QIcon(":/player/" + _settings->theme() + "/stop"));
			break;
		}
	} else if (!_taskbarButton->overlayIcon().isNull()) {
		_taskbarButton->clearOverlayIcon();
	}
}

void WindowsToolbar::updateProgressbarTaskbar()
{
	switch (_mediaPlayer.data()->state()) {
	case QMediaPlayer::PlayingState:
		_taskbarProgress->resume();
		_taskbarProgress->setVisible(_settings->value("WindowsToolbar/hasProgressBarInTaskbar").toBool());
		break;
	case QMediaPlayer::PausedState:
		_taskbarProgress->pause();
		_taskbarProgress->setVisible(_settings->value("WindowsToolbar/hasProgressBarInTaskbar").toBool());
		break;
	case QMediaPlayer::StoppedState:
		_taskbarProgress->hide();
		break;
	}
}

/** Update icons for buttons. */
void WindowsToolbar::updateThumbnailToolBar()
{
	_skipBackward->setIcon(QIcon(":/player/" + _settings->theme() + "/skipBackward"));
	if (_mediaPlayer.data()->state() == QMediaPlayer::PlayingState) {
		_playPause->setIcon(QIcon(":/player/" + _settings->theme() + "/pause"));
	} else {
		_playPause->setIcon(QIcon(":/player/" + _settings->theme() + "/play"));
	}
	_stop->setIcon(QIcon(":/player/" + _settings->theme() + "/stop"));
	_skipForward->setIcon(QIcon(":/player/" + _settings->theme() + "/skipForward"));
}
