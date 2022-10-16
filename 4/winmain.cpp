#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
#define WINVER 0x0A00
#define _WIN32_WINNT 0x0A00
#define HEIGHT 16
#define WIDTH 30
#define MINE_NUM 99
#define MINE 9
#define BLANK 0
#define MASK 1
#define FLAG 2
#define QUESTION 3
#define HINT 4
#include <windows.h>
#include <algorithm>
#include <d2d1.h>
#include <dwrite.h>
#include <cstdio>
#include <ctime>
using namespace std;
double scaleX, scaleY;
float margin = 20;
ID2D1Factory *pFactory; // variables related to GUI
ID2D1DCRenderTarget *pRenderTarget;
ID2D1SolidColorBrush *pWhiteBrush, *pBlackBrush, *pGrayBrush, *pDarkGrayBrush;
ID2D1SolidColorBrush *pColorfulBrush[8];
ID2D1StrokeStyle *pStrokeStyle;
IDWriteFactory *pDWriteFactory = NULL;
IDWriteTextFormat *pTextFormat = NULL;

struct position
{
    int i, j;
};
struct board
{
    unsigned char mat[HEIGHT][WIDTH], mask[HEIGHT][WIDTH];
    unsigned char *operator[](int index) { return mat[index]; }
    unsigned char operator[](position pos) { return mat[pos.i][pos.j]; }
};

board brd;  // global variable of board
bool first; // whether click for the first time (to prevent first mine)
int expand_cnt;
position expand_upd[WIDTH * HEIGHT];
bool peek = 0;

// create d2d objects
int OnCreate(HWND hWnd)
{
    HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pFactory);
    if (!SUCCEEDED(hr))
        return -1;
    D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(
        D2D1_RENDER_TARGET_TYPE_DEFAULT,
        D2D1::PixelFormat(
            DXGI_FORMAT_B8G8R8A8_UNORM,
            D2D1_ALPHA_MODE_IGNORE),
        0, 0,
        D2D1_RENDER_TARGET_USAGE_NONE,
        D2D1_FEATURE_LEVEL_DEFAULT);
    hr = pFactory->CreateDCRenderTarget(&props, &pRenderTarget);
    if (!SUCCEEDED(hr))
        return -1;
    HDC hdc = GetDC(hWnd);
    RECT rc;
    GetClientRect(hWnd, &rc);
    hr = pRenderTarget->BindDC(hdc, &rc);
    if (!SUCCEEDED(hr))
        return -1;
    hr = pRenderTarget->CreateSolidColorBrush(
        D2D1::ColorF(D2D1::ColorF::White), &pWhiteBrush);
    if (!SUCCEEDED(hr))
        return -1;
    hr = pRenderTarget->CreateSolidColorBrush(
        D2D1::ColorF(D2D1::ColorF::Black), &pBlackBrush);
    if (!SUCCEEDED(hr))
        return -1;
    hr = pRenderTarget->CreateSolidColorBrush(
        D2D1::ColorF(0.7f, 0.7f, 0.7f), &pGrayBrush);
    if (!SUCCEEDED(hr))
        return -1;
    hr = pRenderTarget->CreateSolidColorBrush(
        D2D1::ColorF(0.5f, 0.5f, 0.5f), &pDarkGrayBrush);
    if (!SUCCEEDED(hr))
        return -1;
    hr = pRenderTarget->CreateSolidColorBrush(
        D2D1::ColorF(D2D1::ColorF::DarkGreen), &pColorfulBrush[0]);
    if (!SUCCEEDED(hr))
        return -1;
    hr = pRenderTarget->CreateSolidColorBrush(
        D2D1::ColorF(D2D1::ColorF::Red), &pColorfulBrush[1]);
    if (!SUCCEEDED(hr))
        return -1;
    hr = pRenderTarget->CreateSolidColorBrush(
        D2D1::ColorF(D2D1::ColorF::Blue), &pColorfulBrush[2]);
    if (!SUCCEEDED(hr))
        return -1;
    hr = pRenderTarget->CreateSolidColorBrush(
        D2D1::ColorF(D2D1::ColorF::Magenta), &pColorfulBrush[3]);
    if (!SUCCEEDED(hr))
        return -1;
    hr = pRenderTarget->CreateSolidColorBrush(
        D2D1::ColorF(D2D1::ColorF::Brown), &pColorfulBrush[4]);
    if (!SUCCEEDED(hr))
        return -1;
    hr = pRenderTarget->CreateSolidColorBrush(
        D2D1::ColorF(D2D1::ColorF::DarkRed), &pColorfulBrush[5]);
    if (!SUCCEEDED(hr))
        return -1;
    hr = pRenderTarget->CreateSolidColorBrush(
        D2D1::ColorF(D2D1::ColorF::DarkCyan), &pColorfulBrush[6]);
    if (!SUCCEEDED(hr))
        return -1;
    hr = pRenderTarget->CreateSolidColorBrush(
        D2D1::ColorF(D2D1::ColorF::Black), &pColorfulBrush[7]);
    if (!SUCCEEDED(hr))
        return -1;
    D2D1_STROKE_STYLE_PROPERTIES strokeStyleProperties = D2D1::StrokeStyleProperties(
        D2D1_CAP_STYLE_FLAT,
        D2D1_CAP_STYLE_FLAT,
        D2D1_CAP_STYLE_FLAT,
        D2D1_LINE_JOIN_MITER,
        2.0f,
        D2D1_DASH_STYLE_DASH,
        0.0f);
    hr = pFactory->CreateStrokeStyle(strokeStyleProperties, NULL, 0, &pStrokeStyle);
    if (!SUCCEEDED(hr))
        return -1;
    hr = DWriteCreateFactory(
        DWRITE_FACTORY_TYPE_SHARED,
        __uuidof(IDWriteFactory),
        reinterpret_cast<IUnknown **>(&pDWriteFactory));
    if (!SUCCEEDED(hr))
        return -1;
    hr = pDWriteFactory->CreateTextFormat(
        L"Segoe UI", NULL, DWRITE_FONT_WEIGHT_NORMAL,
        DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
        20.0f * scaleX, L"", &pTextFormat);
    hr = pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
    hr = pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    if (!SUCCEEDED(hr))
        return -1;
    return 0;
}

// initialize the board
void Initialize()
{
    first = 1;
    expand_cnt = 0;
    for (int i = 0; i < HEIGHT; i++)
        for (int j = 0; j < WIDTH; j++)
            brd.mask[i][j] = MASK;
}

// calculate numbers of mines in squares without mines
void Calculate()
{
    for (int i = 0; i < HEIGHT; i++)
        for (int j = 0; j < WIDTH; j++)
            if (brd[i][j] != MINE)
                brd[i][j] = (i > 0 && j > 0 && brd[i - 1][j - 1] == MINE ? 1 : 0) +
                            (i > 0 && brd[i - 1][j] == MINE ? 1 : 0) +
                            (i > 0 && j < WIDTH - 1 && brd[i - 1][j + 1] == MINE ? 1 : 0) +
                            (j > 0 && brd[i][j - 1] == MINE ? 1 : 0) +
                            (j < WIDTH - 1 && brd[i][j + 1] == MINE ? 1 : 0) +
                            (i < HEIGHT - 1 && j > 0 && brd[i + 1][j - 1] == MINE ? 1 : 0) +
                            (i < HEIGHT - 1 && brd[i + 1][j] == MINE ? 1 : 0) +
                            (i < HEIGHT - 1 && j < WIDTH - 1 && brd[i + 1][j + 1] == MINE ? 1 : 0);
}

// select random squares to put mines
void Randomize(int i0, int j0)
{
    memset(brd.mat, 0, sizeof(brd.mat));
    int k = 0;
    srand(time(NULL));
    while (k < min(MINE_NUM, WIDTH * HEIGHT - 1))
    {
        int i = rand() % HEIGHT, j = rand() % WIDTH;
        if (brd[i][j] == MINE || i == i0 && j == j0)
            continue;
        brd[i][j] = MINE;
        k++;
    }
    Calculate();
}

// print board on hWnd at n squares in array upd
void PaintBoard(HWND hWnd, int n = 0, position *upd = NULL)
{
    RECT rc;
    GetClientRect(hWnd, &rc);
    float l = min(((rc.right - rc.left) - 2 * margin) / WIDTH,
                  ((rc.bottom - rc.top) - 2 * margin) / HEIGHT),
          stroke = 1, radius = 0.45f * l;
    pRenderTarget->BeginDraw();
    if (n == 0)
    {
        n = HEIGHT * WIDTH;
        upd = new position[HEIGHT * WIDTH];
        for (int i = 0; i < HEIGHT; i++)
            for (int j = 0; j < WIDTH; j++)
            {
                upd[i * WIDTH + j].i = i;
                upd[i * WIDTH + j].j = j;
            }
        if (upd == NULL)
        {
            MessageBox(hWnd, L"Fail to allocate memory!", L"error", MB_ICONERROR);
            return;
        }
        pRenderTarget->Clear(D2D1::ColorF(1, 1, 1));
        for (int i = 0; i <= HEIGHT; i++)
            pRenderTarget->DrawLine({margin, i * l + margin},
                                    {WIDTH * l + margin, i * l + margin},
                                    pGrayBrush, scaleX * stroke);
        for (int j = 0; j <= WIDTH; j++)
            pRenderTarget->DrawLine({j * l + margin, margin},
                                    {j * l + margin, HEIGHT * l + margin},
                                    pGrayBrush, scaleX * stroke);
    }
    for (int idx = 0; idx < n; idx++)
    {
        int i = upd[idx].i, j = upd[idx].j;
        if (i < 0 || i > HEIGHT || j < 0 || j > WIDTH)
            continue;
        D2D_POINT_2F mid = {(j + 0.5f) * l + margin, (i + 0.5f) * l + margin};
        D2D1_RECT_F rect_f = {mid.x - radius, mid.y - radius,
                              mid.x + radius, mid.y + radius};
        if (brd.mask[i][j] == MASK && !peek)
            pRenderTarget->FillRectangle(rect_f, pGrayBrush);
        else if (brd.mask[i][j] == QUESTION && !peek)
        {
            pRenderTarget->FillRectangle(rect_f, pGrayBrush);
            pRenderTarget->DrawText(L"?", 1, pTextFormat, rect_f, pBlackBrush);
        }
        else if (brd.mask[i][j] == FLAG && !peek)
        {
            pRenderTarget->FillRectangle(rect_f, pGrayBrush);
            pRenderTarget->DrawText(L"+", 1, pTextFormat, rect_f, pBlackBrush);
        }
        else if (brd.mask[i][j] == HINT && !peek)
            pRenderTarget->FillRectangle(rect_f, pDarkGrayBrush);
        else
        {
            pRenderTarget->FillRectangle(rect_f, pWhiteBrush);
            if (brd[i][j] == MINE)
                pRenderTarget->DrawText(L"*", 1, pTextFormat, rect_f, pBlackBrush);
            else if (brd[i][j] != 0)
            {
                wchar_t ws[3];
                _itow(brd[i][j], ws, 10);
                pRenderTarget->DrawText(ws, 1, pTextFormat, rect_f, pColorfulBrush[brd[i][j] - 1]);
            }
        }
    }
    if (n == 0)
        delete[] upd;
    HRESULT hr = pRenderTarget->EndDraw();
    if (!SUCCEEDED(hr))
        MessageBox(hWnd, L"Fail to finish paint!", L"Error", MB_ICONERROR);
}

// expand a square using recursion
void Expand(HWND hWnd, int i, int j)
{
    if (i < 0 || i >= HEIGHT || j < 0 || j >= WIDTH || brd.mask[i][j] != MASK)
        return;
    brd.mask[i][j] = BLANK;
    expand_upd[expand_cnt] = {i, j};
    expand_cnt++;
    if (brd[i][j] != 0)
        return;
    Expand(hWnd, i - 1, j - 1);
    Expand(hWnd, i - 1, j);
    Expand(hWnd, i - 1, j + 1);
    Expand(hWnd, i, j - 1);
    Expand(hWnd, i, j + 1);
    Expand(hWnd, i + 1, j - 1);
    Expand(hWnd, i + 1, j);
    Expand(hWnd, i + 1, j + 1);
}

// callback function
LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (uMsg == WM_DESTROY)
        PostQuitMessage(0);
    else if (uMsg == WM_CREATE)
    {
        if (OnCreate(hWnd) < 0)
        {
            MessageBox(hWnd, L"创建D2D对象失败!", L"错误", MB_ICONERROR);
            return DefWindowProc(hWnd, uMsg, wParam, lParam);
        }
    }
    else if (uMsg == WM_GETMINMAXINFO)
    {
        MINMAXINFO *pmmi = (MINMAXINFO *)lParam;
        pmmi->ptMinTrackSize = {(int)(scaleX * 900), (int)(scaleY * 520)};
        pmmi->ptMaxTrackSize = {(int)(scaleX * 900), (int)(scaleY * 520)};
    }
    else if (uMsg == WM_PAINT || uMsg == WM_SIZE)
        PaintBoard(hWnd);
    else if (uMsg == WM_LBUTTONDOWN) // when click squares
    {
        RECT rc;
        GetClientRect(hWnd, &rc);
        int xPos = LOWORD(lParam) / scaleX, yPos = HIWORD(lParam) / scaleY;
        float l = min(((rc.right - rc.left) - 2 * margin) / WIDTH,
                      ((rc.bottom - rc.top) - 2 * margin) / HEIGHT) /
                  scaleX;
        int i = (yPos - margin / scaleX) / l, j = (xPos - margin / scaleX) / l;
        if (i >= 0 && i < HEIGHT && j >= 0 && j < WIDTH)
        {
            if (first)
            {
                Randomize(i, j);
                first = 0;
            }
            if (brd.mask[i][j] == MASK)
                if (brd[i][j] == MINE)
                    MessageBox(hWnd, L"Game over!", L"Notice", MB_OK);
                else
                {
                    Expand(hWnd, i, j);
                    PaintBoard(hWnd, expand_cnt, expand_upd);
                    expand_cnt = 0;
                }
        }
    }
    else if (uMsg == WM_RBUTTONDOWN) // mark squares
    {
        RECT rc;
        GetClientRect(hWnd, &rc);
        int xPos = LOWORD(lParam) / scaleX, yPos = HIWORD(lParam) / scaleY;
        float l = min(((rc.right - rc.left) - 2 * margin) / WIDTH,
                      ((rc.bottom - rc.top) - 2 * margin) / HEIGHT) /
                  scaleX;
        int i = (yPos - margin / scaleX) / l, j = (xPos - margin / scaleX) / l;
        if (i >= 0 && i < HEIGHT && j >= 0 && j < WIDTH)
        {
            if (brd.mask[i][j] == MASK)
                brd.mask[i][j] = FLAG;
            else if (brd.mask[i][j] == FLAG)
                brd.mask[i][j] = QUESTION;
            else if (brd.mask[i][j] == QUESTION)
                brd.mask[i][j] = MASK;
            position upd[1] = {i, j};
            PaintBoard(hWnd, 1, upd);
        }
    }
    else if (uMsg == WM_KEYDOWN) // keyboard down
    {
        if (wParam == VK_F1)
        {
            wchar_t str[] =
                L"按上键载入mines.txt\n按下键保存board.txt\n按Esc初始化\n按右键查看最终结果\n按左键恢复当前局面\n按Enter载入mines.txt\n    并保存board.txt";
            MessageBox(hWnd, str, L"使用说明", MB_OK);
        }
        else if (wParam == VK_ESCAPE)
        {
            Initialize();
            PaintBoard(hWnd);
        }
        else if (wParam == VK_UP)
        {
            // Format:
            // n m
            // blank1_i blank1_j
            // ...
            // mine1_i mine1_j
            // ...
            char filename[] = "mines.txt";
            FILE *fp = fopen(filename, "r");
            int m, n;
            fscanf(fp, "%d%d", &m, &n);
            bool go = 0;
            for (int i = 0; i < m; i++)
            {
                int x, y;
                if (fscanf(fp, "%d%d", &x, &y) != 2)
                    break;
                if (brd[x][y] == MINE)
                    go = 1;
                Expand(hWnd, x, y);
            }
            for (int i = 0; i < n; i++)
            {
                int x, y;
                if (fscanf(fp, "%d%d", &x, &y) != 2)
                    break;
                brd.mask[x][y] = FLAG;
            }
            if (go)
                MessageBox(hWnd, L"Game over!", L"Notice", MB_OK);
            PaintBoard(hWnd);
            fclose(fp);
        }
        else if (wParam == VK_DOWN)
        {
            // Format:
            // _: mask
            // num: number of mines around
            char filename[] = "board.txt";
            FILE *fp = fopen(filename, "w");
            for (int i = 0; i < HEIGHT; i++)
                for (int j = 0; j < WIDTH; j++)
                    if (brd.mask[i][j])
                        fprintf(fp, j == WIDTH - 1 ? "_\n" : "_");
                    else
                        fprintf(fp, j == WIDTH - 1 ? "%d\n" : "%d", brd[i][j]);
            fclose(fp);
        }
        else if (wParam == VK_LEFT || wParam == VK_RIGHT)
        {
            peek = wParam == VK_RIGHT;
            PaintBoard(hWnd);
        }
        else if (wParam == VK_RETURN)
        {
            SendMessage(hWnd, WM_KEYDOWN, VK_UP, 0);
            SendMessage(hWnd, WM_KEYDOWN, VK_DOWN, 0);
        }
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

// program entry
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPreInstance, PWSTR pCmdLine, int nCmdShow)
{
    HDC hdc = GetDC(0);
    scaleX = (double)GetDeviceCaps(hdc, DESKTOPHORZRES) / GetDeviceCaps(hdc, HORZRES);
    scaleY = (double)GetDeviceCaps(hdc, DESKTOPVERTRES) / GetDeviceCaps(hdc, VERTRES);
    SetProcessDPIAware();

    CoInitialize(NULL);

    // register window class
    WNDCLASS wc = {};
    wc.lpszClassName = L"MineSweeper";
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClass(&wc);

    // create window
    HWND hWnd = CreateWindow(
        L"MineSweeper", L"MineSweeper", WS_OVERLAPPEDWINDOW & (~WS_MAXIMIZEBOX),
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL, NULL, hInstance, NULL);

    ShowWindow(hWnd, nCmdShow);
    Initialize();

    MSG msg;

    // message loop
    while (true)
    {
        WINBOOL flag = PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);
        if (msg.message == WM_QUIT)
            break;
        else if (flag)
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    return 0;
}