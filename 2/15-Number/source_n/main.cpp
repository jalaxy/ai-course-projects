#ifndef UNICODE
#define UNICODE
#endif
#define WINVER 0x0A00
#define _WIN32_WINNT 0x0A00
#include "game_new.h"

double scaleX = 1, scaleY = 1;
RECT rectButton1, rectButton2, rectButtonAdd, rectButtonSub;
RECT rectSelect1, rectSelect2, rectSelect3, rectSelect4;
wchar_t buttonLabel[5] = L"Calc";

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (uMsg == WM_DESTROY)
        PostQuitMessage(0);
    else if (uMsg == WM_GETMINMAXINFO)
    {
        MINMAXINFO *pmmi = reinterpret_cast<MINMAXINFO *>(lParam);
        POINT sz_min = {(int)(scaleX * 400), (int)(scaleY * 223)};
        POINT sz_max = {(int)(scaleX * 400), (int)(scaleY * 223)};
        pmmi->ptMinTrackSize = sz_min;
        pmmi->ptMaxTrackSize = sz_max;
    }
    else if (uMsg == WM_PAINT)
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        HFONT hFont = CreateFont(195 / n * scaleY, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE,
                                 DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS,
                                 CLEARTYPE_QUALITY, VARIABLE_PITCH, TEXT("Courier New"));
        if (hFont == NULL)
            return 0;
        SelectObject(hdc, hFont);
        FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));
        RECT Rect = ps.rcPaint;
        POINT sz = {Rect.bottom - Rect.top, Rect.bottom - Rect.top};
        int paddingX = scaleX * 1, paddingY = scaleY * 1;
        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++)
            {
                Rect.top = i * sz.x / n + paddingX;
                Rect.bottom = (i + 1) * sz.x / n - paddingX;
                Rect.left = j * sz.y / n + paddingY;
                Rect.right = (j + 1) * sz.y / n - paddingY;
                FillRect(hdc, &Rect, mat[i * n + j] == n * n ? (HBRUSH)(COLOR_WINDOW + 1) : (HBRUSH)(1));
                SetTextColor(hdc, RGB(0x00, 0x00, 0x00));
                SetBkMode(hdc, TRANSPARENT);
                wchar_t s[32];
                if (n < 5)
                    _itow_s(mat[i * n + j], s, 32, 16);
                else
                    _itow_s(mat[i * n + j], s, 32, 10);
                if (mat[i * n + j] < n * n)
                    DrawText(hdc, s, -1, &Rect, DT_CENTER);
            }
        DeleteObject(hFont);
        hFont = CreateFont(20 * scaleY, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE,
                           DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS,
                           CLEARTYPE_QUALITY, VARIABLE_PITCH, TEXT("Courier New"));
        if (hFont == NULL)
            return 0;
        SelectObject(hdc, hFont);
        GetClientRect(hWnd, &Rect);
        Rect.left = Rect.bottom - Rect.top + paddingX * 5;
        Rect.top += paddingY * 5;
        wchar_t s[64] = L"Steps:\n  ", num[32];
        _itow_s(steps, num, -1, 10);
        wcscat_s(s, num);
        wcscat_s(s, L"\n\nNode\nvisited:\n  ");
        _itow_s(node_expanded, num, -1, 10);
        wcscat_s(s, num);
        wcscat_s(s, L"\nto visit:\n  ");
        _itow_s(node_to_expand, num, -1, 10);
        wcscat_s(s, num);
        DrawText(hdc, s, -1, &Rect, DT_LEFT);
        FillRect(hdc, &rectButton1, (HBRUSH)(1));
        DrawText(hdc, L"Play", -1, &rectButton1, DT_CENTER);
        if (finished)
            wcscpy_s(buttonLabel, L"Over");
        FillRect(hdc, &rectButton2, (HBRUSH)(1));
        DrawText(hdc, buttonLabel, -1, &rectButton2, DT_CENTER);
        FillRect(hdc, &rectButtonAdd, (HBRUSH)(1));
        DrawText(hdc, L"+", -1, &rectButtonAdd, DT_CENTER);
        FillRect(hdc, &rectButtonSub, (HBRUSH)(1));
        DrawText(hdc, L"-", -1, &rectButtonSub, DT_CENTER);
        if (method == 0)
            FillRect(hdc, &rectSelect1, (HBRUSH)(1));
        DrawText(hdc, L"g+0", -1, &rectSelect1, DT_LEFT);
        if (method == 1)
            FillRect(hdc, &rectSelect2, (HBRUSH)(1));
        DrawText(hdc, L"g+h", -1, &rectSelect2, DT_LEFT);
        if (method == 2)
            FillRect(hdc, &rectSelect3, (HBRUSH)(1));
        DrawText(hdc, L"g+2h", -1, &rectSelect3, DT_LEFT);
        if (method == 3)
            FillRect(hdc, &rectSelect4, (HBRUSH)(1));
        DrawText(hdc, L"g+10h", -1, &rectSelect4, DT_LEFT);
        EndPaint(hWnd, &ps);
        DeleteObject(hFont);
        HINSTANCE hInstance;
    }
    else if (uMsg == WM_LBUTTONDOWN)
    {
        RECT rect;
        GetClientRect(hWnd, &rect);
        int xPos = HIWORD(lParam),
            yPos = LOWORD(lParam),
            i = xPos * n / (rect.bottom - rect.top),
            j = yPos * n / (rect.bottom - rect.top);
        if (wcscmp(buttonLabel, L"Calc") == 0 && i < n && j < n)
        {
            finished = move(i, j);
            InvalidateRect(hWnd, NULL, FALSE);
        }
        if (yPos < rectButton1.right && yPos > rectButton1.left)
            if (xPos < rectButton1.bottom && xPos > rectButton1.top)
            {
                randomize();
                initialize();
                InvalidateRect(hWnd, NULL, FALSE);
            }
        if (yPos < rectButton2.right && yPos > rectButton2.left)
            if (xPos < rectButton2.bottom && xPos > rectButton2.top)
                if (wcscmp(buttonLabel, L"Over") != 0)
                {
                    if (seq.empty())
                        if (A_star())
                            wcscpy_s(buttonLabel, L"Next");
                        else
                            wcscpy_s(buttonLabel, L"Over");
                    else
                    {
                        mat = seq.top();
                        seq.pop();
                        if (seq.empty())
                            wcscpy_s(buttonLabel, L"Over");
                        steps--;
                    }
                    InvalidateRect(hWnd, NULL, FALSE);
                }
        if (yPos < rectButtonAdd.right && yPos > rectButtonAdd.left)
            if (xPos < rectButtonAdd.bottom && xPos > rectButtonAdd.top)
            {
                if (n < 10)
                {
                    n++;
                    randomize();
                    initialize();
                    InvalidateRect(hWnd, NULL, FALSE);
                }
            }
        if (yPos < rectButtonSub.right && yPos > rectButtonSub.left)
            if (xPos < rectButtonSub.bottom && xPos > rectButtonSub.top)
            {
                if (n > 2)
                {
                    n--;
                    randomize();
                    initialize();
                    InvalidateRect(hWnd, NULL, FALSE);
                }
            }
        if (yPos < rectSelect1.right && yPos > rectSelect1.left)
            if (xPos < rectSelect1.bottom && xPos > rectSelect1.top)
            {
                method = 0;
                initialize();
                wcscpy_s(buttonLabel, L"Next");
                A_star();
                InvalidateRect(hWnd, NULL, FALSE);
            }
        if (yPos < rectSelect2.right && yPos > rectSelect2.left)
            if (xPos < rectSelect2.bottom && xPos > rectSelect2.top)
            {
                method = 1;
                initialize();
                wcscpy_s(buttonLabel, L"Next");
                A_star();
                InvalidateRect(hWnd, NULL, FALSE);
            }
        if (yPos < rectSelect3.right && yPos > rectSelect3.left)
            if (xPos < rectSelect3.bottom && xPos > rectSelect3.top)
            {
                method = 2;
                initialize();
                wcscpy_s(buttonLabel, L"Next");
                A_star();
                InvalidateRect(hWnd, NULL, FALSE);
            }
        if (yPos < rectSelect4.right && yPos > rectSelect4.left)
            if (xPos < rectSelect4.bottom && xPos > rectSelect4.top)
            {
                method = 3;
                initialize();
                wcscpy_s(buttonLabel, L"Next");
                A_star();
                InvalidateRect(hWnd, NULL, FALSE);
            }
    }
    else if (uMsg == WM_SIZE)
        InvalidateRect(hWnd, NULL, FALSE);
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPreInstance, PWSTR pCmsLine, int nCmdShow)
{
    HDC hdc = GetDC(0);
    scaleX = (double)GetDeviceCaps(hdc, DESKTOPHORZRES) / GetDeviceCaps(hdc, HORZRES);
    scaleY = (double)GetDeviceCaps(hdc, DESKTOPVERTRES) / GetDeviceCaps(hdc, VERTRES);
    SetProcessDPIAware();
    rectButton1 = {(int)(305 * scaleX), (int)(10 * scaleY), (int)(375 * scaleX), (int)(30 * scaleY)};
    rectButtonSub = {(int)(305 * scaleX), (int)(35 * scaleY), (int)(325 * scaleX), (int)(55 * scaleY)};
    rectButtonAdd = {(int)(355 * scaleX), (int)(35 * scaleY), (int)(375 * scaleX), (int)(55 * scaleY)};
    rectButton2 = {(int)(305 * scaleX), (int)(60 * scaleY), (int)(375 * scaleX), (int)(80 * scaleY)};
    rectSelect1 = {(int)(315 * scaleX), (int)(90 * scaleY), (int)(375 * scaleX), (int)(110 * scaleY)};
    rectSelect2 = {(int)(315 * scaleX), (int)(110 * scaleY), (int)(375 * scaleX), (int)(130 * scaleY)};
    rectSelect3 = {(int)(315 * scaleX), (int)(130 * scaleY), (int)(375 * scaleX), (int)(150 * scaleY)};
    rectSelect4 = {(int)(315 * scaleX), (int)(150 * scaleY), (int)(375 * scaleX), (int)(170 * scaleY)};

    // register window class
    const wchar_t CLASS_NAME[] = L"8-Number";
    WNDCLASS wc = {};
    wc.lpszClassName = CLASS_NAME;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClass(&wc);

    // create window handle
    HWND hWnd = CreateWindowEx(
        0, CLASS_NAME, L"8-Numbers",
        WS_OVERLAPPEDWINDOW & (~WS_MAXIMIZEBOX),
        CW_USEDEFAULT, CW_USEDEFAULT, 400 * scaleX, 223 * scaleY,
        NULL, NULL, hInstance, NULL);
    if (hWnd == NULL)
        return 0;
    const TCHAR *data = TEXT("hello"); // LPCTSTR

    ShowWindow(hWnd, nCmdShow);

    srand(time(NULL));
    initialize();
    randomize();

    // handle messages
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    release();
    return 0;
}