#include "tray_icon.h"
#include <iostream>
#include <Windows.h>

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow)
{
	start_tray_icon();
	MessageBox(0, TEXT("Hello, World!"), TEXT("Message"), 0);
	return 0;
}