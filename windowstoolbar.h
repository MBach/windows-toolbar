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
	Ui::ConfigForm *_configPage;

	QMainWindow *_mainWindow;

public:
	WindowsToolbar();

	virtual ~WindowsToolbar();

	virtual QString name() const { return "WindowsToolBar"; }

	virtual QString version() const { return "0.1"; }

	QWidget *configPage();

	void setMainWindow(QMainWindow *mainWindow);

private:
	void init();
};

#endif // WINDOWSTOOLBAR_H
