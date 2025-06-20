#pragma once

void start_tray_icon();
void set_tray_icon_visible(bool shouldIconBeVisible);
void stop_tray_icon();

const inline wchar_t* tray_icon_window_class = L"XBTrayIconWindow";