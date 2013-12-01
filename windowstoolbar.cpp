#include "windowstoolbar.h"

#include <QFile>
#include <QLabel>
#include <QLibrary>
#include <QMediaContent>
#include <QMediaPlaylist>
#include <QWindow>

#include <cover.h>
#include <filehelper.h>

#include <QtDebug>

WindowsToolbar::WindowsToolbar()
	: _mainWindow(NULL), _skipBackward(NULL), _playPause(NULL), _stop(NULL), _skipForward(NULL),
	  _taskbarButton(NULL), _taskbarProgress(NULL), _thumbbar(NULL)
{
	_settings = Settings::getInstance();
	connect(_settings, &Settings::themeHasChanged, this, &WindowsToolbar::updateThumbnailToolBar);

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
	QWindow *window = new QWindow();
	window->setIcon(QIcon(":/windows-toolbar/mmmmp"));
	_taskbarButton->setWindow(window);
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
void WindowsToolbar::updateCover()
{
    /*FileHelper fh(_mediaPlayer->currentMedia());
	Cover *cover = fh.extractCover();
	if (cover) {
		QPixmap p;
		bool b = p.loadFromData(cover->byteArray(), cover->format());
		if (b) {
			qDebug() << "inner cover was loaded";

			QWindow *window = new QWindow();
			QWidget *w = QWidget::createWindowContainer(window, new QWidget());
			QLayout *l = new QHBoxLayout(w);
			QLabel *imageLabel = new QLabel(w);
			imageLabel->setPixmap(p);
			l->addWidget(imageLabel);
			w->setLayout(l);
			//w->show();
			_thumbbar->setWindow(window);
		}
	} else {
		qDebug() << "we need to look at the current dir if " << _mediaPlayer->currentMedia().canonicalUrl().toLocalFile() << "has some picture";
    }*/
}

void WindowsToolbar::updateOverlayIcon()
{
	if (_settings->value("hasOverlayIcon").toBool()) {
        switch (_mediaPlayer.data()->state()) {
		// Icons are inverted from updateThumbnailToolBar() method because it's reflecting the actual state of the player
		case QMediaPlayer::PlayingState:
			_taskbarButton->setOverlayIcon(QIcon(":/player/" + _settings->theme() + "/play"));
			break;
		case QMediaPlayer::PausedState:
			_taskbarButton->setOverlayIcon(QIcon(":/player/" + _settings->theme() + "/pause"));
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
    switch (_mediaPlayer.data()->state()) {
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
