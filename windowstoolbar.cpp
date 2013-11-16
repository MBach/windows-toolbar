#include "windowstoolbar.h"

#include <QtUiTools/QUiLoader>
#include <QFile>

#include <QtDebug>

#include <QWindow>

WindowsToolbar::WindowsToolbar()
	: _mainWindow(NULL)
{
	_configPage = new Ui::ConfigForm();
}

WindowsToolbar::~WindowsToolbar()
{
	delete _configPage;
}

void WindowsToolbar::setMainWindow(QMainWindow *mainWindow)
{
	_mainWindow = mainWindow;
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
	QWinThumbnailToolBar *thumbbar = new QWinThumbnailToolBar(_mainWindow);
	thumbbar->setWindow(_mainWindow->windowHandle());

	QWinThumbnailToolButton *skipBackward = new QWinThumbnailToolButton(thumbbar);
	QWinThumbnailToolButton *playPause = new QWinThumbnailToolButton(thumbbar);
	QWinThumbnailToolButton *stop = new QWinThumbnailToolButton(thumbbar);
	QWinThumbnailToolButton *skipForward = new QWinThumbnailToolButton(thumbbar);

	QSettings settings("MmeMiamMiam", "MmeMiamMiamMusicPlayer");
	QString theme = settings.value("theme").toString();
	if (theme.isEmpty()) {
		theme = "oxygen";
	}

	skipBackward->setIcon(QIcon(":/player/" + theme + "/skipBackward"));
	playPause->setIcon(QIcon(":/player/" + theme + "/play"));
	stop->setIcon(QIcon(":/player/" + theme + "/stop"));
	skipForward->setIcon(QIcon(":/player/" + theme + "/skipForward"));

	thumbbar->addButton(skipBackward);
	thumbbar->addButton(playPause);
	thumbbar->addButton(stop);
	thumbbar->addButton(skipForward);

	connect(skipBackward, &QWinThumbnailToolButton::clicked, [=] () {
		emit skip(false);
	});
	connect(skipForward, &QWinThumbnailToolButton::clicked, [=] () {
		emit skip(true);
	});
	connect(playPause, &QWinThumbnailToolButton::clicked, this, &WindowsToolbar::play);
	connect(stop, &QWinThumbnailToolButton::clicked, this, &WindowsToolbar::stop);
}
