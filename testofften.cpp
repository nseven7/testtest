#include <iostream>
#include <vector>
#include <thread>
#include <windows.h>
#include <cmath>
#include <chrono>

using namespace std;

int SizeX = 32;
int SizeY = 16;
const int ScreenWidth = GetSystemMetrics(SM_CXSCREEN);
const int ScreenHeight = GetSystemMetrics(SM_CYSCREEN);
const int ScreenWidthHalf = ScreenWidth / 2;
const int ScreenHeightHalf = ScreenHeight / 2;

bool FoundLeftSide = false;
bool FoundRightSide = false;
bool CenterAligned = false;
bool BreakOut = false;
vector<POINT> HitPoints;
double AVGPoint = 0;
int IDX = 0;
int delay = 250;
bool capsLockMode = true;
bool shiftMode = true;
bool isEnglish = true; // Language toggle
int colorMode = 1; // 1 for Color 1, 2 for Color 2

bool GetAsyncKeyStateWrapper(int vKey) {
    return GetAsyncKeyState(vKey) & 0x8000;
}

void SetConsoleColor(WORD color) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
}

void DisplayHelp(bool isEnglish) {
    if (isEnglish) {
        cout << "Instructions:\n";
        cout << "F12: Toggle Language\n";
        cout << "Page Up: Toggle Caps Lock Mode\n";
        cout << "Page Down: Toggle Shift Mode\n";
        cout << "Ctrl + '1': Select Color 1\n";
        cout << "Ctrl + '2': Select Color 2\n";
        cout << "Ctrl + '+': Increase Radius\n";
        cout << "Ctrl + '-': Decrease Radius\n";
        cout << "Alt + '+': Increase Delay\n";
        cout << "Alt + '-': Decrease Delay\n";
    }
    else {
        cout << "Инструкции:\n";
        cout << "F12: Переключение языка\n";
        cout << "Page Up: Переключение режима Caps Lock\n";
        cout << "Page Down: Переключение режима Shift\n";
        cout << "Ctrl + '1': Выбор цвета 1\n";
        cout << "Ctrl + '2': Выбор цвета 2\n";
        cout << "Ctrl + '+': Увеличить радиус\n";
        cout << "Ctrl + '-': Уменьшить радиус\n";
        cout << "Alt + '+': Увеличить задержку\n";
        cout << "Alt + '-': Уменьшить задержку\n";
    }
}

void DisplaySettings(int radius, int delay, bool capsLockMode, bool shiftMode, bool isEnglish, int colorMode) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD coord;
    coord.X = 0;
    coord.Y = 0;
    SetConsoleCursorPosition(hConsole, coord);

    system("cls"); // Clear the console

    DisplayHelp(isEnglish);

    if (isEnglish) {
        cout << "Radius: " << radius << "\n";
        cout << "Delay: " << delay << " ms\n";
        cout << "Caps Lock Mode: ";
    }
    else {
        cout << "Радиус: " << radius << "\n";
        cout << "Задержка: " << delay << " мс\n";
        cout << "Режим Caps Lock: ";
    }

    SetConsoleColor(capsLockMode ? FOREGROUND_GREEN : FOREGROUND_RED);
    cout << (capsLockMode ? (isEnglish ? "Enabled" : "Включен") : (isEnglish ? "Disabled" : "Выключен")) << "\n";
    SetConsoleColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE); // Reset to default color

    if (isEnglish) {
        cout << "Shift Mode: ";
    }
    else {
        cout << "Режим Shift: ";
    }

    SetConsoleColor(shiftMode ? FOREGROUND_GREEN : FOREGROUND_RED);
    cout << (shiftMode ? (isEnglish ? "Enabled" : "Включен") : (isEnglish ? "Disabled" : "Выключен")) << "\n";
    SetConsoleColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE); // Reset to default color

    if (isEnglish) {
        cout << "Color Mode: " << colorMode << " (1: Color 1, 2: Color 2)\n";
    }
    else {
        cout << "Режим цвета: " << colorMode << " (1: Цвет 1, 2: Цвет 2)\n";
    }
}

bool IsColor1(COLORREF color) {
    int R = GetRValue(color);
    int G = GetGValue(color);
    int B = GetBValue(color);
    bool isColor1 = (R > 230) && (B > 230) && (G < 200);

    // Добавим вывод значений цветов в консоль
    if (isColor1) {
        cout << "Color1 Detected - R: " << R << " G: " << G << " B: " << B << "\n";
    }

    return isColor1;
}

bool IsColor2(COLORREF color) {
    int R = GetRValue(color);
    int G = GetGValue(color);
    int B = GetBValue(color);
    // Условие для определения жёлтого цвета

    bool isColor2 = (R > 200) && (G > 200) && (B < 150);

    // Добавим вывод значений цветов в консоль
    if (isColor2) {
        cout << "Color2 Detected - R: " << R << " G: " << G << " B: " << B << "\n";
    }

    return isColor2;
}


void AimBot() {
    while (true) {
        // Check if Caps Lock and Shift conditions are met based on the modes
        bool capsCondition = capsLockMode ? (GetKeyState(VK_CAPITAL) & 0x0001) : true;
        bool shiftCondition = shiftMode ? GetAsyncKeyStateWrapper(VK_SHIFT) : true;

        if (capsCondition && shiftCondition) {
            if (!GetAsyncKeyStateWrapper(VK_XBUTTON2)) {
                HitPoints.clear();
                FoundLeftSide = false;
                CenterAligned = false;
                FoundRightSide = false;
                BreakOut = false;
                AVGPoint = 0;
                IDX = 0;

                HDC hdcScreen = GetDC(NULL);
                HDC hdcMem = CreateCompatibleDC(hdcScreen);
                HBITMAP hbmScreen = CreateCompatibleBitmap(hdcScreen, SizeX, SizeY);
                SelectObject(hdcMem, hbmScreen);

                BitBlt(hdcMem, 0, 0, SizeX, SizeY, hdcScreen, ScreenWidthHalf - (SizeX / 2), ScreenHeightHalf - (SizeY / 2), SRCCOPY);

                BITMAPINFO bmi = { 0 };
                bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
                bmi.bmiHeader.biWidth = SizeX;
                bmi.bmiHeader.biHeight = -SizeY;
                bmi.bmiHeader.biPlanes = 1;
                bmi.bmiHeader.biBitCount = 32;
                bmi.bmiHeader.biCompression = BI_RGB;

                vector<COLORREF> pixels(SizeX * SizeY);
                GetDIBits(hdcMem, hbmScreen, 0, SizeY, pixels.data(), &bmi, DIB_RGB_COLORS);

                for (int i = 0; i < SizeX; i++) {
                    for (int j = 0; j < SizeY; j++) {
                        COLORREF pixelColor = pixels[j * SizeX + i];

                        bool colorDetected = (colorMode == 1) ? IsColor1(pixelColor) : IsColor2(pixelColor);

                        if (colorDetected) {
                            if (i < (SizeX / 2)) {
                                FoundLeftSide = true;
                            }
                            else if (i > (SizeX / 2)) {
                                FoundRightSide = true;
                            }

                            HitPoints.push_back(POINT{ i, j });

                            for (const POINT& p : HitPoints) {
                                IDX++;
                                AVGPoint += sqrt(pow(p.x - (SizeX / 2), 2) + pow(p.y - (SizeY / 2), 2));
                                if (((AVGPoint / IDX) < 12) && (IDX > 4)) {
                                    CenterAligned = true;
                                    break;
                                }
                            }

                            if (FoundRightSide && FoundLeftSide && CenterAligned) {
                                keybd_event('K', 0, 0, 0);
                                keybd_event('K', 0, KEYEVENTF_KEYUP, 0);
                                BreakOut = true;
                                break;
                            }
                        }
                    }
                    if (BreakOut) {
                        break;
                    }
                }

                ReleaseDC(NULL, hdcScreen);
                DeleteDC(hdcMem);
                DeleteObject(hbmScreen);
                this_thread::sleep_for(chrono::milliseconds(delay));
            }
        }
        this_thread::sleep_for(chrono::milliseconds(6));
    }
}

void AdjustSettings() {
    while (true) {
        if (GetAsyncKeyStateWrapper(VK_CONTROL)) {
            if (GetAsyncKeyStateWrapper(VK_OEM_PLUS) & 0x0001) {
                SizeX++;
                SizeY++;
                DisplaySettings(SizeX / 2, delay, capsLockMode, shiftMode, isEnglish, colorMode);
            }
            else if (GetAsyncKeyStateWrapper(VK_OEM_MINUS) & 0x0001) {
                if (SizeX > 1 && SizeY > 1) {
                    SizeX--;
                    SizeY--;
                    DisplaySettings(SizeX / 2, delay, capsLockMode, shiftMode, isEnglish, colorMode);
                }
            }
            else if (GetAsyncKeyStateWrapper('1') & 0x0001) {
                colorMode = 1;
                DisplaySettings(SizeX / 2, delay, capsLockMode, shiftMode, isEnglish, colorMode);
            }
            else if (GetAsyncKeyStateWrapper('2') & 0x0001) {
                colorMode = 2;
                DisplaySettings(SizeX / 2, delay, capsLockMode, shiftMode, isEnglish, colorMode);
            }
        }
        else if (GetAsyncKeyStateWrapper(VK_MENU)) {
            if (GetAsyncKeyStateWrapper(VK_OEM_PLUS) & 0x0001) {
                delay += 10;
                DisplaySettings(SizeX / 2, delay, capsLockMode, shiftMode, isEnglish, colorMode);
            }
            else if (GetAsyncKeyStateWrapper(VK_OEM_MINUS) & 0x0001) {
                if (delay > 10) {
                    delay -= 10;
                    DisplaySettings(SizeX / 2, delay, capsLockMode, shiftMode, isEnglish, colorMode);
                }
            }
        }
        else if (GetAsyncKeyStateWrapper(VK_PRIOR) & 0x0001) { // Page Up
            capsLockMode = !capsLockMode;
            DisplaySettings(SizeX / 2, delay, capsLockMode, shiftMode, isEnglish, colorMode);
        }
        else if (GetAsyncKeyStateWrapper(VK_NEXT) & 0x0001) { // Page Down
            shiftMode = !shiftMode;
            DisplaySettings(SizeX / 2, delay, capsLockMode, shiftMode, isEnglish, colorMode);
        }
        else if (GetAsyncKeyStateWrapper(VK_F12) & 0x0001) { // F12
            isEnglish = !isEnglish;
            DisplaySettings(SizeX / 2, delay, capsLockMode, shiftMode, isEnglish, colorMode);
        }
        this_thread::sleep_for(chrono::milliseconds(100));
    }
}

int main() {
    // Initial display of settings
    DisplaySettings(SizeX / 2, delay, capsLockMode, shiftMode, isEnglish, colorMode);

    thread aimThread(AimBot);
    thread adjustThread(AdjustSettings);

    aimThread.join();
    adjustThread.join();

    return 0;
}