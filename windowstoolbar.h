#ifndef WINDOWSTOOLBAR_H
#define WINDOWSTOOLBAR_H

#include "mediaplayerplugininterface.h"

#include "ui_config.h"
#include "win7_include.h"

class WindowsToolbar : public MediaPlayerPluginInterface
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID MediaPlayerPluginInterface_iid)
	Q_INTERFACES(MediaPlayerPluginInterface)
private:
	Ui::ConfigForm *_configPage;

	THUMBBUTTON m_thbButtons[3];
	ITaskbarList3* m_w7toolbar;

	WId _winId;

public:
	WindowsToolbar();

	virtual QString name() const;

	virtual QString version() const;

	QWidget *configPage();

	virtual void setWinId(WId wId) { _winId = wId; }

protected:
	 bool winEvent(MSG * message, long * result);

private:
	 void W7ToolbarInit();
	 void W7ToolbarButtonsInit();
};

#endif // WINDOWSTOOLBAR_H
