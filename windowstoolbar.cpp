#include "windowstoolbar.h"
#include "model/sqldatabase.h"

#include <QFile>
#include <QGuiApplication>
#include <QLabel>
#include <QLibrary>
#include <QMediaContent>
#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <QWindow>

#include "cover.h"
#include "settings.h"
#include "musicsearchengine.h"

#include <QtDebug>

WindowsToolbar::WindowsToolbar(QObject *parent)
	: MediaPlayerPlugin(parent)
	, _musicSearchEngine(nullptr)
	, _mediaPlayerControl(nullptr)
	, _skipBackward(nullptr)
	, _playPause(nullptr)
	, _stop(nullptr)
	, _skipForward(nullptr)
	, _toggleShuffle(nullptr)
	, _taskbarProgress(nullptr)
	, _thumbbar(new QWinThumbnailToolBar(parent))
	, _taskbarButton(new QWinTaskbarButton(parent))
{
	_thumbbar->setIconicPixmapNotificationsEnabled(true);
	auto settings = Settings::instance();
	connect(settings, &Settings::themeHasChanged, this, &WindowsToolbar::updateThumbnailToolBar);

	// First time (ever) the plugin is loaded
	if (settings->value("WindowsToolbar/hasProgressBarInTaskbar").isNull()) {
		settings->setValue("WindowsToolbar/hasProgressBarInTaskbar", true);
	}
	if (settings->value("WindowsToolbar/hasMediaPlayerButtonsInThumbnail").isNull()) {
		settings->setValue("WindowsToolbar/hasMediaPlayerButtonsInThumbnail", true);
	}
	if (settings->value("WindowsToolbar/hasOverlayIcon").isNull()) {
		settings->setValue("WindowsToolbar/hasOverlayIcon", false);
	}
	_thumbbar->setIconicThumbnailPixmap(QPixmap(":/icons/mp_win32"));
}

WindowsToolbar::~WindowsToolbar()
{
	this->disconnect();
}

void WindowsToolbar::setMediaPlayerControl(AbstractMediaPlayerControl *mediaPlayerControl)
{
	_mediaPlayerControl = mediaPlayerControl;

	connect(_mediaPlayerControl->mediaPlayer(), &MediaPlayer::positionChanged, [=] (qint64 pos, qint64 duration) {
		if (duration > 0) {
			_taskbarProgress->setValue(100 * pos / duration);
		}
	});
	connect(_mediaPlayerControl->mediaPlayer(), &MediaPlayer::stateChanged, this, &WindowsToolbar::updateOverlayIcon);
	connect(_mediaPlayerControl->mediaPlayer(), &MediaPlayer::stateChanged, this, &WindowsToolbar::updateThumbnailToolBar);
	connect(_mediaPlayerControl->mediaPlayer(), &MediaPlayer::stateChanged, this, &WindowsToolbar::updateProgressbarTaskbar);
	connect(_mediaPlayerControl->mediaPlayer(), &MediaPlayer::currentMediaChanged, this, &WindowsToolbar::updateCover);

	// Init visibility of overlay icon
	this->updateOverlayIcon();

	// Init visibility of media buttons
	this->showThumbnailButtons(Settings::instance()->value("WindowsToolbar/hasMediaPlayerButtonsInThumbnail").toBool());

	// Connect each buttons to the main program
	connect(_skipBackward, &QWinThumbnailToolButton::clicked, _mediaPlayerControl, &AbstractMediaPlayerControl::skipBackward);
	connect(_skipForward, &QWinThumbnailToolButton::clicked, _mediaPlayerControl, &AbstractMediaPlayerControl::skipForward);
	connect(_playPause, &QWinThumbnailToolButton::clicked, _mediaPlayerControl, &AbstractMediaPlayerControl::togglePlayback);
	connect(_stop, &QWinThumbnailToolButton::clicked, this, [=]() {
		_mediaPlayerControl->stop();
		_thumbbar->setIconicThumbnailPixmap(QPixmap(":/icons/mp_win32"));
	});
	//connect(_toggleShuffle, &QWinThumbnailToolButton::clicked, _mediaPlayerControl, &MediaPlayerControl::toggleShuffle);
	connect(_toggleShuffle, &QWinThumbnailToolButton::clicked, this, [=]() {
		bool checked = _toggleShuffle->property("clicked").toBool();
		_mediaPlayerControl->toggleShuffle(checked);
		_toggleShuffle->setProperty("clicked", !checked);
		this->updateThumbnailToolBar();
	});
}

QWidget* WindowsToolbar::configPage()
{
	QWidget *widget = new QWidget();
	_ui.setupUi(widget);
	// Init the UI with correct values
	auto settings = Settings::instance();
	_ui.progressBarTaskbar->setChecked(settings->value("WindowsToolbar/hasProgressBarInTaskbar").toBool());
	_ui.overlayIconTaskbar->setChecked(settings->value("WindowsToolbar/hasOverlayIcon").toBool());
	_ui.mediaPlayerButtonsThumbnail->setChecked(settings->value("WindowsToolbar/hasMediaPlayerButtonsInThumbnail").toBool());

	// Connect the UI with the settings
	connect(_ui.progressBarTaskbar, &QCheckBox::stateChanged, [=](int s) {
		settings->setValue("WindowsToolbar/hasProgressBarInTaskbar", (s == Qt::Checked));
		this->updateProgressbarTaskbar();
	});
	connect(_ui.overlayIconTaskbar, &QCheckBox::stateChanged, [=](int s) {
		settings->setValue("WindowsToolbar/hasOverlayIcon", (s == Qt::Checked));
		this->updateOverlayIcon();
	});
	connect(_ui.mediaPlayerButtonsThumbnail, &QCheckBox::stateChanged, [=](int s) {
		settings->setValue("WindowsToolbar/hasMediaPlayerButtonsInThumbnail", (s == Qt::Checked));
		this->showThumbnailButtons(s == Qt::Checked);
	});
	return widget;
}

void WindowsToolbar::init()
{
	/// XXX: if not set to false, it seems there's a glitch when one is hovering the thumbnail in the taskbar.
	/// The resulting preview is a Window cursor "searching for something" indefinitely
	QtWin::setWindowDisallowPeek(QGuiApplication::topLevelWindows().first(), true);

	// Progress bar in the task bar
	_taskbarButton->setWindow(QGuiApplication::topLevelWindows().first());
	_taskbarProgress = _taskbarButton->progress();

	// If one has switched to another view (like a plugin which brings a new view mode), reroute these buttons
	connect(_taskbarButton->window(), &QWindow::activeChanged, this, [=](){
		for (QWindow *w : QGuiApplication::topLevelWindows()) {
			if (w->isVisible()) {
				_taskbarButton->setWindow(w);
				if (_thumbbar) {
					_thumbbar->setWindow(w);
				}
				break;
			}
		}
	});
}

void WindowsToolbar::showThumbnailButtons(bool visible)
{
	for (auto button : _thumbbar->buttons()) {
		button->deleteLater();
	}
	_thumbbar->clear();

	if (visible) {

		// Five buttons are enough
		_skipBackward = new QWinThumbnailToolButton(_thumbbar);
		_playPause = new QWinThumbnailToolButton(_thumbbar);
		_stop = new QWinThumbnailToolButton(_thumbbar);
		_skipForward = new QWinThumbnailToolButton(_thumbbar);
		_toggleShuffle = new QWinThumbnailToolButton(_thumbbar);

		// Must init this property!
		_toggleShuffle->setProperty("clicked", true);

		_thumbbar->addButton(_skipBackward);
		_thumbbar->addButton(_playPause);
		_thumbbar->addButton(_stop);
		_thumbbar->addButton(_skipForward);
		_thumbbar->addButton(_toggleShuffle);

		_thumbbar->setWindow(QGuiApplication::topLevelWindows().first());

		auto settings = Settings::instance();
		_skipBackward->setIcon(QIcon(":/player/" + settings->theme() + "/skipBackward"));
		this->updateThumbnailToolBar();
	}
}

/** Update the cover when the current media in the player has changed. */
void WindowsToolbar::updateCover(const QString &uri)
{
	SqlDatabase db;
	Cover *cover = db.selectCoverFromURI(uri);
	if (cover) {
		QPixmap p = QPixmap::fromImage(QImage::fromData(cover->byteArray(), cover->format()));
		_thumbbar->setIconicThumbnailPixmap(p);
		delete cover;
	}
}

void WindowsToolbar::updateOverlayIcon()
{
	auto settings = Settings::instance();
	if (settings->value("WindowsToolbar/hasOverlayIcon").toBool()) {
		switch (_mediaPlayerControl->mediaPlayer()->state()) {
		// Icons are inverted from updateThumbnailToolBar() method because it's reflecting the actual state of the player
		case QMediaPlayer::PlayingState:
			_taskbarButton->setOverlayIcon(QIcon(":/player/" + settings->theme() + "/play"));
			break;
		case QMediaPlayer::PausedState:
			_taskbarButton->setOverlayIcon(QIcon(":/player/" + settings->theme() + "/pause"));
			break;
		case QMediaPlayer::StoppedState:
			_taskbarButton->setOverlayIcon(QIcon(":/player/" + settings->theme() + "/stop"));
			break;
		}
	} else if (!_taskbarButton->overlayIcon().isNull()) {
		_taskbarButton->clearOverlayIcon();
	}
}

void WindowsToolbar::updateProgressbarTaskbar()
{
	auto settings = Settings::instance();
	switch (_mediaPlayerControl->mediaPlayer()->state()) {
	case QMediaPlayer::PlayingState:
		_taskbarProgress->resume();
		_taskbarProgress->setVisible(settings->value("WindowsToolbar/hasProgressBarInTaskbar").toBool());
		break;
	case QMediaPlayer::PausedState:
		_taskbarProgress->pause();
		_taskbarProgress->setVisible(settings->value("WindowsToolbar/hasProgressBarInTaskbar").toBool());
		break;
	case QMediaPlayer::StoppedState:
		_taskbarProgress->hide();
		break;
	}
}

/** Update icons for buttons. */
void WindowsToolbar::updateThumbnailToolBar()
{
	if (_skipBackward == nullptr) {
		return;
	}
	auto settings = Settings::instance();
	_skipBackward->setIcon(QIcon(":/player/" + settings->theme() + "/skipBackward"));
	if (_mediaPlayerControl->mediaPlayer()->state() == QMediaPlayer::PlayingState) {
		_playPause->setIcon(QIcon(":/player/" + settings->theme() + "/pause"));
	} else {
		_playPause->setIcon(QIcon(":/player/" + settings->theme() + "/play"));
	}
	_stop->setIcon(QIcon(":/player/" + settings->theme() + "/stop"));
	_skipForward->setIcon(QIcon(":/player/" + settings->theme() + "/skipForward"));
	if (_mediaPlayerControl->isInShuffleState()) {
		_toggleShuffle->setIcon(QIcon(":/player/" + settings->theme() + "/shuffle"));
	} else {
		_toggleShuffle->setIcon(QIcon(":/player/" + settings->theme() + "/sequential"));
	}
}
