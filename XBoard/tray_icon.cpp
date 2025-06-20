#include "tray_icon.h"
#include "resource.h"
#include <Windows.h>
#include <winrt/base.h>

extern "C" IMAGE_DOS_HEADER __ImageBase;

namespace
{
	HWND tray_icon_hwnd = NULL;

	enum
	{
		wm_icon_notify = WM_APP
	};

	UINT wm_taskbar_restart = 0;

	NOTIFYICONDATAW tray_icon_data;
	bool tray_icon_created = false;

	HMENU h_menu = nullptr;
	HMENU h_sub_menu = nullptr;
	POINT tray_icon_click_point;
}

void handle_tray_command(HWND window, const WPARAM command_id, LPARAM lParam)
{
	switch (command_id)
	{
	case ID_EXIT_MENU_COMMAND:
		if (h_menu)
			DestroyMenu(h_menu);
		DestroyWindow(window);
		break;
	}
}

LRESULT __stdcall tray_icon_window_proc(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CREATE:
		if (wm_taskbar_restart == 0)
		{
			tray_icon_hwnd = window;
			wm_taskbar_restart = RegisterWindowMessageW(L"TaskbarCreated");
		}
		break;
	case WM_DESTROY:
		if (tray_icon_created)
		{
			Shell_NotifyIcon(NIM_DELETE, &tray_icon_data);
			tray_icon_created = false;
		}
		PostQuitMessage(0);
		break;
	case WM_CLOSE:
		DestroyWindow(window);
		break;
	case WM_WINDOWPOSCHANGING:
		if (!tray_icon_created)
		{
			tray_icon_created = Shell_NotifyIcon(NIM_ADD, &tray_icon_data) == TRUE;
		}
		break;
	case WM_COMMAND:
		handle_tray_command(window, wParam, lParam);
		break;
	default:
		if (message == wm_icon_notify)
		{
			switch (lParam)
			{
			case WM_LBUTTONUP:
			case WM_RBUTTONUP:
			case WM_CONTEXTMENU:
				if (!h_menu)
					h_menu = LoadMenu(reinterpret_cast<HINSTANCE>(&__ImageBase), MAKEINTRESOURCE(ID_TRAY_MENU));

				if (!h_sub_menu)
					h_sub_menu = GetSubMenu(h_menu, 0);

				POINT mouse_pointer;
				GetCursorPos(&mouse_pointer);
				SetForegroundWindow(window);
				TrackPopupMenu(h_sub_menu, TPM_CENTERALIGN | TPM_BOTTOMALIGN, mouse_pointer.x, mouse_pointer.y, 0, window, nullptr);
				break;
			}
		}
		else if (message == wm_taskbar_restart)
		{
			tray_icon_created = Shell_NotifyIcon(NIM_ADD, &tray_icon_data) == TRUE;
			break;
		}
	}
	return DefWindowProc(window, message, wParam, lParam);
}

void start_tray_icon()
{
	auto h_instance = reinterpret_cast<HINSTANCE>(&__ImageBase);
	auto icon = LoadIcon(h_instance, MAKEINTRESOURCE(APPICON));
	if (icon)
	{
		UINT id_tray_icon = 1;

		WNDCLASS wc = {};
		wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wc.hInstance = h_instance;
		wc.lpszClassName = tray_icon_window_class;
		wc.style = CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc = tray_icon_window_proc;
		wc.hIcon = icon;
		RegisterClass(&wc);
		auto hwnd = CreateWindow(wc.lpszClassName,
			tray_icon_window_class,
			WS_OVERLAPPEDWINDOW | WS_POPUP,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			nullptr,
			nullptr,
			wc.hInstance,
			nullptr);
		WINRT_VERIFY(hwnd);
		memset(&tray_icon_data, 0, sizeof(tray_icon_data));
		tray_icon_data.cbSize = sizeof(tray_icon_data);
		tray_icon_data.hIcon = icon;
		tray_icon_data.hWnd = hwnd;
		tray_icon_data.uID = id_tray_icon;
		tray_icon_data.uCallbackMessage = wm_icon_notify;
		tray_icon_data.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
		ChangeWindowMessageFilterEx(hwnd, WM_COMMAND, MSGFLT_ALLOW, nullptr);

		tray_icon_created = Shell_NotifyIcon(NIM_ADD, &tray_icon_data) == TRUE;
	}
}

void set_tray_icon_visible(bool shouldIconBeVisible)
{
	tray_icon_data.uFlags |= NIF_STATE;
	tray_icon_data.dwStateMask = NIS_HIDDEN;
	tray_icon_data.dwState = shouldIconBeVisible ? 0 : NIS_HIDDEN;
	Shell_NotifyIcon(NIM_MODIFY, &tray_icon_data);
}

void stop_tray_icon()
{
	if (tray_icon_created)
	{
		SendMessage(tray_icon_hwnd, WM_CLOSE, 0, 0);
	}
}