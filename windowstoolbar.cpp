#include "windowstoolbar.h"

#include <QtUiTools/QUiLoader>
#include <QFile>

#define IDTB_FIRST 3000

WindowsToolbar::WindowsToolbar()
{
	_configPage = new Ui::ConfigForm();
}

QString WindowsToolbar::name() const
{
	return "WindowsToolBar";
}

QString WindowsToolbar::version() const
{
	return "0.1";
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

#include <QtDebug>

bool WindowsToolbar::winEvent(MSG * message, long * result)
{
	static UINT taskBarCreatedId = WM_NULL;
	if (taskBarCreatedId == WM_NULL) {
		taskBarCreatedId = RegisterWindowMessage(L"TaskbarButtonCreated");
	}

	if (message->message == taskBarCreatedId) {
		W7ToolbarInit();
		//W7ToolbarSetImages();
		W7ToolbarButtonsInit();

	} else switch (message->message){
	case WM_COMMAND:
		{
			int buttonId = LOWORD(message->wParam) - IDTB_FIRST;

			if ((buttonId >= 0) && (buttonId < 3)) {

				qDebug() << "Button " << buttonId << " was pressed";
				if (buttonId == 1) { //if "Play|Pause" was pressed

					if (m_thbButtons[1].iBitmap == 1) {
						m_thbButtons[1].iBitmap = 3;
						wcscpy(m_thbButtons[1].szTip, L"Pause");
					} else {
						m_thbButtons[1].iBitmap = 1;
						wcscpy(m_thbButtons[1].szTip, L"Play");
					}

					//W7ToolbarButtonsUpdate();
				}

			}


			break;
		}

	default:
		return false;

	}


	return false;
}

void WindowsToolbar::W7ToolbarInit() {
	HRESULT hr = CoCreateInstance(CLSID_TaskbarList, NULL, CLSCTX_INPROC_SERVER, IID_ITaskbarList3,
								  reinterpret_cast<void**> (&(m_w7toolbar)));
	if (SUCCEEDED(hr)) {
		hr = m_w7toolbar->HrInit();

		if (FAILED(hr)) {
			m_w7toolbar->Release();
			m_w7toolbar = NULL;
		}
	}
}

void WindowsToolbar::W7ToolbarButtonsInit() {
	QString tooltips[3] = {"Prev", "Play", "Next"};

	for (int index = 0; index < 3; index++) {
		wcscpy(m_thbButtons[index].szTip, tooltips[index].toStdWString().c_str());

		m_thbButtons[index].iId = IDTB_FIRST + index;
		m_thbButtons[index].iBitmap = index;
		m_thbButtons[index].dwMask = (THUMBBUTTONMASK)(THB_BITMAP | THB_FLAGS | THB_TOOLTIP);
		m_thbButtons[index].dwFlags = (THUMBBUTTONFLAGS)(THBF_ENABLED);
	}

	if (m_w7toolbar) {
		//m_w7toolbar->ThumbBarAddButtons(this->winId(), 3, m_thbButtons);
		HWND truc = 0;
		m_w7toolbar->ThumbBarAddButtons(truc, 3, m_thbButtons);
	}
}

