#pragma once

#include "locale/encoding.h"
#include "main-win/main-win-windows.h"

#include <mciapi.h>

extern MCI_OPEN_PARMSW mci_open_parms;
extern MCI_PLAY_PARMS mci_play_parms;
extern utf16_string mci_device_type;

void setup_mci(HWND hWnd);
