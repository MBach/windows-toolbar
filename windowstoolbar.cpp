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
	, _db(nullptr)
	, _skipBackward(nullptr)
	, _playPause(nullptr)
	, _stop(nullptr)
	, _skipForward(nullptr)
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
{}

void WindowsToolbar::setMediaPlayer(MediaPlayer *mediaPlayer)
{
	_mediaPlayer = mediaPlayer;
	connect(_mediaPlayer, &MediaPlayer::positionChanged, [=] (qint64 pos, qint64 duration) {
		if (duration > 0) {
			_taskbarProgress->setValue(100 * pos / duration);
		}
	});
	connect(_mediaPlayer, &MediaPlayer::stateChanged, this, &WindowsToolbar::updateOverlayIcon);
	connect(_mediaPlayer, &MediaPlayer::stateChanged, this, &WindowsToolbar::updateThumbnailToolBar);
	connect(_mediaPlayer, &MediaPlayer::stateChanged, this, &WindowsToolbar::updateProgressbarTaskbar);
	connect(_mediaPlayer, &MediaPlayer::currentMediaChanged, this, &WindowsToolbar::updateCover);

	// Init visibility of overlay icon
	this->updateOverlayIcon();

	// Init visibility of media buttons
	this->showThumbnailButtons(Settings::instance()->value("WindowsToolbar/hasMediaPlayerButtonsInThumbnail").toBool());
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

	/// FIXME
	connect(_db, &SqlDatabase::aboutToLoad, this, [=]() {
		_mediaPlayer->blockSignals(true);
		if (_mediaPlayer->state() != QMediaPlayer::StoppedState) {
			_taskbarProgress->setProperty("previousVisible", true);
			_taskbarProgress->setProperty("previousPausedState", _taskbarProgress->isPaused());
			_taskbarProgress->setProperty("previousValue", _taskbarProgress->value());
			_taskbarProgress->setValue(0);
			_taskbarProgress->setPaused(false);
		}
	});
	connect(_db->musicSearchEngine(), &MusicSearchEngine::progressChanged, _taskbarProgress, &QWinTaskbarProgress::setValue);
	connect(_db->musicSearchEngine(), &MusicSearchEngine::searchHasEnded, this, [=]() {
		_mediaPlayer->blockSignals(false);
		bool b = _taskbarProgress->property("previousVisible").toBool();
		if (b) {
			_taskbarProgress->show();
			_taskbarProgress->setPaused(_taskbarProgress->property("previousPausedState").toBool());
			_taskbarProgress->setValue(_taskbarProgress->property("previousValue").toInt());
		} else {
			_taskbarProgress->setValue(0);
			_taskbarProgress->hide();
		}
	});

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
	if (visible) {
		// Four buttons are enough
		_skipBackward = new QWinThumbnailToolButton(_thumbbar);
		_playPause = new QWinThumbnailToolButton(_thumbbar);
		_stop = new QWinThumbnailToolButton(_thumbbar);
		_skipForward = new QWinThumbnailToolButton(_thumbbar);

		_thumbbar->addButton(_skipBackward);
		_thumbbar->addButton(_playPause);
		_thumbbar->addButton(_stop);
		_thumbbar->addButton(_skipForward);
		_thumbbar->setWindow(QGuiApplication::topLevelWindows().first());

		auto settings = Settings::instance();
		_skipBackward->setIcon(QIcon(":/player/" + settings->theme() + "/skipBackward"));
		this->updateThumbnailToolBar();
		_stop->setIcon(QIcon(":/player/" + settings->theme() + "/stop"));
		_skipForward->setIcon(QIcon(":/player/" + settings->theme() + "/skipForward"));

		// Connect each buttons to the main program
		connect(_skipBackward, &QWinThumbnailToolButton::clicked, _mediaPlayer, &MediaPlayer::skipBackward);
		connect(_skipForward, &QWinThumbnailToolButton::clicked, _mediaPlayer, &MediaPlayer::skipForward);
		connect(_playPause, &QWinThumbnailToolButton::clicked, this, [=]() {
			if (_mediaPlayer->state() == QMediaPlayer::PlayingState) {
				_mediaPlayer->pause();
			} else {
				_mediaPlayer->play();
			}
		});
		connect(_stop, &QWinThumbnailToolButton::clicked, this, [=]() {
			_mediaPlayer->stop();
			_thumbbar->setIconicThumbnailPixmap(QPixmap(":/icons/mp_win32"));
		});
	} else {
		_thumbbar->removeButton(_skipBackward);
		_thumbbar->removeButton(_playPause);
		_thumbbar->removeButton(_stop);
		_thumbbar->removeButton(_skipForward);
	}
}

/** Update the cover when the current media in the player has changed. */
void WindowsToolbar::updateCover(const QString &uri)
{
	SqlDatabase db;
	db.open();
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
		switch (_mediaPlayer->state()) {
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
	switch (_mediaPlayer->state()) {
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
	if (_mediaPlayer->state() == QMediaPlayer::PlayingState) {
		_playPause->setIcon(QIcon(":/player/" + settings->theme() + "/pause"));
	} else {
		_playPause->setIcon(QIcon(":/player/" + settings->theme() + "/play"));
	}
	_stop->setIcon(QIcon(":/player/" + settings->theme() + "/stop"));
	_skipForward->setIcon(QIcon(":/player/" + settings->theme() + "/skipForward"));
}
