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

#include <QtDebug>

WindowsToolbar::WindowsToolbar()
	: QObject(), _skipBackward(NULL), _playPause(NULL), _stop(NULL), _skipForward(NULL),
	  _taskbarProgress(NULL), _thumbbar(NULL)
{
	_settings = Settings::instance();
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

void WindowsToolbar::setMediaPlayer(MediaPlayer *mediaPlayer)
{
	_mediaPlayer = mediaPlayer;
	this->init();
	connect(_mediaPlayer, &MediaPlayer::positionChanged, [=] (qint64 pos, qint64 duration) {
		if (duration > 0) {
			_taskbarProgress->setValue(100 * pos / duration);
		}
	});
	connect(_mediaPlayer, &MediaPlayer::stateChanged, this, &WindowsToolbar::updateOverlayIcon);
	connect(_mediaPlayer, &MediaPlayer::stateChanged, this, &WindowsToolbar::updateThumbnailToolBar);
	connect(_mediaPlayer, &MediaPlayer::stateChanged, this, &WindowsToolbar::updateProgressbarTaskbar);
	connect(_mediaPlayer, &MediaPlayer::currentMediaChanged, this, &WindowsToolbar::updateCover);
	this->updateCover(QString());
}

QWidget* WindowsToolbar::configPage()
{
	QWidget *widget = new QWidget();
	_ui.setupUi(widget);
	// Init the UI with correct values
	_ui.progressBarTaskbar->setChecked(_settings->value("WindowsToolbar/hasProgressBarInTaskbar").toBool());
	_ui.overlayIconTaskbar->setChecked(_settings->value("WindowsToolbar/hasOverlayIcon").toBool());
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

	QApplication *app = static_cast<QApplication*>(QApplication::instance());
	connect(app, &QApplication::focusChanged, this, [=]() {
		// qDebug() << "focus changed";
		if (!_thumbbar) {
			return;
		}
		if (QScreen *screen = QGuiApplication::primaryScreen()) {
			if (QWindow *w = QApplication::topLevelWindows().first()) {
				QPixmap originalPixmap = screen->grabWindow(w->winId());
				_thumbbar->setIconicLivePreviewPixmap(originalPixmap);
			}
		}
	});

	SqlDatabase *db = SqlDatabase::instance();
	connect(db, &SqlDatabase::aboutToLoad, this, [=]() {
		_mediaPlayer->blockSignals(true);
		if (_mediaPlayer->state() != QMediaPlayer::StoppedState) {
			_taskbarProgress->setProperty("previousVisible", true);
			_taskbarProgress->setProperty("previousPausedState", _taskbarProgress->isPaused());
			_taskbarProgress->setProperty("previousValue", _taskbarProgress->value());
			_taskbarProgress->setValue(0);
			_taskbarProgress->setPaused(false);
		}
	});
	connect(db, &SqlDatabase::progressChanged, _taskbarProgress, &QWinTaskbarProgress::setValue);
	connect(db, &SqlDatabase::loaded, this, [=]() {
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

	// Init visibility of progressBar in the taskBar
	_taskbarProgress->setVisible(_settings->value("WindowsToolbar/hasProgressBarInTaskbar").toBool());

	// Init visibility of overlay icon
	this->updateOverlayIcon();

	// Init visibility of media buttons
	this->showThumbnailButtons(_settings->value("WindowsToolbar/hasMediaPlayerButtonsInThumbnail").toBool());
}

void WindowsToolbar::showThumbnailButtons(bool visible)
{
	if (visible) {
		_thumbbar = new QWinThumbnailToolBar(this);
		_thumbbar->setIconicPixmapNotificationsEnabled(true);
		connect(_thumbbar, &QWinThumbnailToolBar::iconicLivePreviewPixmapRequested, this, [=]() {
			qDebug() << "iconicLivePreviewPixmapRequested";
			/*QScreen *screen = QGuiApplication::primaryScreen();
			if (screen) {
				QPixmap originalPixmap = screen->grabWindow(0);
				_thumbbar->setIconicLivePreviewPixmap(originalPixmap);
			}*/
		});
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
		connect(_skipBackward, &QWinThumbnailToolButton::clicked, _mediaPlayer, &MediaPlayer::skipBackward);
		connect(_skipForward, &QWinThumbnailToolButton::clicked, _mediaPlayer, &MediaPlayer::skipForward);
		connect(_playPause, &QWinThumbnailToolButton::clicked, this, [=]() {
			if (_mediaPlayer->state() == QMediaPlayer::PlayingState) {
				_mediaPlayer->pause();
			} else {
				_mediaPlayer->play();
			}
		});
		connect(_stop, &QWinThumbnailToolButton::clicked, _mediaPlayer, &MediaPlayer::stop);
	} else if (_thumbbar) {
		///XXX the thumbnail window is not resizing properly when removing buttons?
		_thumbbar->clear();
		delete _thumbbar;
		_thumbbar = NULL;
	}
}

/** Update the cover when the current media in the player has changed. */
void WindowsToolbar::updateCover(const QString &uri)
{
	if (!_thumbbar) {
		return;
	}

	SqlDatabase *db = SqlDatabase::instance();
	Cover *c = db->selectCoverFromURI(uri);
	if (c) {
		QPixmap p = QPixmap::fromImage(QImage::fromData(c->byteArray(), c->format()));
		_thumbbar->setIconicThumbnailPixmap(p);
		delete c;
	} else {
		_thumbbar->setIconicThumbnailPixmap(QPixmap(":/icons/mp_win32"));
	}
}

void WindowsToolbar::updateOverlayIcon()
{
	if (_settings->value("WindowsToolbar/hasOverlayIcon").toBool()) {
		qDebug() << (_stop == NULL);
		switch (_mediaPlayer->state()) {
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
	switch (_mediaPlayer->state()) {
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
		_thumbbar->setIconicThumbnailPixmap(QPixmap(":/icons/mp_win32"));
		break;
	}
}

/** Update icons for buttons. */
void WindowsToolbar::updateThumbnailToolBar()
{
	_skipBackward->setIcon(QIcon(":/player/" + _settings->theme() + "/skipBackward"));
	if (_mediaPlayer->state() == QMediaPlayer::PlayingState) {
		_playPause->setIcon(QIcon(":/player/" + _settings->theme() + "/pause"));
	} else {
		_playPause->setIcon(QIcon(":/player/" + _settings->theme() + "/play"));
	}
	_stop->setIcon(QIcon(":/player/" + _settings->theme() + "/stop"));
	_skipForward->setIcon(QIcon(":/player/" + _settings->theme() + "/skipForward"));
}
