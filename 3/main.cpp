#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
#define WINVER 0x0A00
#define _WIN32_WINNT 0x0A00
#define INF 2000000000
#define UKN 2000000001
#define ID_BUTTON_STARTOVER 100
#define ID_BUTTON_NEXT 101
#define ID_BUTTON_UP 102
#define ID_BUTTON_DOWN 103
#define ID_BUTTON_LEFT 104
#define ID_BUTTON_RIGHT 105
#include <windows.h>
#include <d2d1.h>
#include <vector>
#include <stack>
#include <algorithm>
#include <string>
using namespace std;
double scaleX, scaleY;
float margin = 20;
ID2D1Factory *pFactory; // GUI相关变量
ID2D1DCRenderTarget *pRenderTarget;
ID2D1SolidColorBrush *pWhiteBrush, *pBlackBrush, *pGrayBrush;
ID2D1StrokeStyle *pStrokeStyle;
RECT rc_button_startover, rc_button_next,
    rc_button_down, rc_button_up,
    rc_button_left, rc_button_right;
RECT rc_text_alpha, rc_text_beta, rc_text_val;
HFONT hDefaultFont;
HWND hWndButtonNext;

struct board
{
    unsigned char mat[15][15];
    unsigned char *operator[](int index) { return mat[index]; }
    board() { memset(mat, 0, sizeof(mat)); }
};
struct position
{
    int i, j;
};
void GenPos(vector<position> &);
int Eval(board &brd);

board brd;                                         // 棋盘全局变量
unsigned char turn = 1;                            // 轮次全局变量
bool over = 0, running = 0, analysing = 0;         // 运行状态
position hlt[2];                                   // 强调标注的位置
vector<position> history_analyse, dashed, history; // 下子历史
int idx_his = -1;                                  // 撤销位置下标
stack<vector<position>> st_dashed;                 // 分析时的临时保存位置
int current, alpha_glb = -INF, beta_glb = INF, value = UKN, depth_glb = 0;

// 创建d2d对象
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
    rc.right = rc.bottom;
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
        D2D1::ColorF(D2D1::ColorF::Gray), &pGrayBrush);
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
    return 0;
}

// 打印棋盘
void PaintBoard(HWND hWnd, int n = 0, position *upd = NULL, int m = 0, position *hlt = NULL)
{
    RECT rc;
    GetClientRect(hWnd, &rc);
    float l = ((rc.bottom - rc.top) - 2 * margin) / 15, stroke = 1,
          radius = 0.4f * l, square_ratio = 0.9, line_len = 0.2 * l;
    pRenderTarget->BeginDraw();
    if (n == 0)
    {
        n = 255;
        upd = new position[255];
        for (int i = 0; i < 15; i++)
            for (int j = 0; j < 15; j++)
            {
                upd[i * 15 + j].i = i;
                upd[i * 15 + j].j = j;
            }
        if (upd == NULL)
        {
            MessageBox(hWnd, L"Fail to allocate memory!", L"error", MB_ICONERROR);
            return;
        }
        pRenderTarget->Clear(D2D1::ColorF(1, 1, 1));
    }
    for (int idx = 0; idx < n; idx++)
    {
        int i = upd[idx].i, j = upd[idx].j;
        if (i < 0 || i > 15 || j < 0 || j > 15)
            continue;
        D2D_POINT_2F top = {(j + 0.5f) * l + margin, i * l + margin},
                     bottom = {(j + 0.5f) * l + margin, (i + 1) * l + margin},
                     left = {j * l + margin, (i + 0.5f) * l + margin},
                     right = {(j + 1) * l + margin, (i + 0.5f) * l + margin},
                     mid = {(j + 0.5f) * l + margin, (i + 0.5f) * l + margin};
        pRenderTarget->FillRectangle({left.x, top.y, right.x, bottom.y}, pWhiteBrush);
        if (i == 0)
            pRenderTarget->DrawLine(mid, bottom, pBlackBrush, scaleX * stroke);
        else if (i == 14)
            pRenderTarget->DrawLine(top, mid, pBlackBrush, scaleX * stroke);
        else
            pRenderTarget->DrawLine(top, bottom, pBlackBrush, scaleX * stroke);
        if (j == 0)
            pRenderTarget->DrawLine(mid, right, pBlackBrush, scaleX * stroke);
        else if (j == 14)
            pRenderTarget->DrawLine(left, mid, pBlackBrush, scaleX * stroke);
        else
            pRenderTarget->DrawLine(left, right, pBlackBrush, scaleX * stroke);
        if (brd[i][j] == 1)
        {
            pRenderTarget->FillEllipse({mid, radius, radius}, pWhiteBrush);
            pRenderTarget->DrawEllipse({mid, radius, radius}, pBlackBrush, scaleX * stroke);
        }
        if (brd[i][j] == 2)
        {
            pRenderTarget->FillEllipse({mid, radius, radius}, pBlackBrush);
            pRenderTarget->DrawEllipse({mid, radius, radius}, pBlackBrush, scaleX * stroke);
        }
    }
    if (n == 0)
        delete[] upd;
    for (int idx = 0; idx < dashed.size(); idx++)
    {
        int i = dashed[idx].i, j = dashed[idx].j;
        if (i < 0 || i > 15 || j < 0 || j > 15)
            continue;
        D2D_POINT_2F top = {(j + 0.5f) * l + margin, i * l + margin},
                     bottom = {(j + 0.5f) * l + margin, (i + 1) * l + margin},
                     left = {j * l + margin, (i + 0.5f) * l + margin},
                     right = {(j + 1) * l + margin, (i + 0.5f) * l + margin},
                     mid = {(j + 0.5f) * l + margin, (i + 0.5f) * l + margin};
        pRenderTarget->FillRectangle({left.x, top.y, right.x, bottom.y}, pWhiteBrush);
        if (i == 0)
            pRenderTarget->DrawLine(mid, bottom, pBlackBrush, scaleX * stroke);
        else if (i == 14)
            pRenderTarget->DrawLine(top, mid, pBlackBrush, scaleX * stroke);
        else
            pRenderTarget->DrawLine(top, bottom, pBlackBrush, scaleX * stroke);
        if (j == 0)
            pRenderTarget->DrawLine(mid, right, pBlackBrush, scaleX * stroke);
        else if (j == 14)
            pRenderTarget->DrawLine(left, mid, pBlackBrush, scaleX * stroke);
        else
            pRenderTarget->DrawLine(left, right, pBlackBrush, scaleX * stroke);
        pRenderTarget->DrawEllipse({mid, radius, radius}, pBlackBrush, scaleX * stroke, pStrokeStyle);
    }
    for (int idx = 0; idx < history_analyse.size(); idx++)
    {
        turn = 3 - turn;
        int i = history_analyse[idx].i, j = history_analyse[idx].j;
        if (i < 0 || i > 15 || j < 0 || j > 15)
            continue;
        D2D_POINT_2F top = {(j + 0.5f) * l + margin, i * l + margin},
                     bottom = {(j + 0.5f) * l + margin, (i + 1) * l + margin},
                     left = {j * l + margin, (i + 0.5f) * l + margin},
                     right = {(j + 1) * l + margin, (i + 0.5f) * l + margin},
                     mid = {(j + 0.5f) * l + margin, (i + 0.5f) * l + margin};
        pRenderTarget->FillRectangle({left.x, top.y, right.x, bottom.y}, pWhiteBrush);
        if (i == 0)
            pRenderTarget->DrawLine(mid, bottom, pBlackBrush, scaleX * stroke);
        else if (i == 14)
            pRenderTarget->DrawLine(top, mid, pBlackBrush, scaleX * stroke);
        else
            pRenderTarget->DrawLine(top, bottom, pBlackBrush, scaleX * stroke);
        if (j == 0)
            pRenderTarget->DrawLine(mid, right, pBlackBrush, scaleX * stroke);
        else if (j == 14)
            pRenderTarget->DrawLine(left, mid, pBlackBrush, scaleX * stroke);
        else
            pRenderTarget->DrawLine(left, right, pBlackBrush, scaleX * stroke);
        if (turn == 1)
        {
            pRenderTarget->FillEllipse({mid, radius, radius}, pWhiteBrush);
            pRenderTarget->DrawEllipse({mid, radius, radius}, pBlackBrush, scaleX * stroke, pStrokeStyle);
        }
        if (turn == 2)
        {
            pRenderTarget->FillEllipse({mid, radius, radius}, pGrayBrush);
            pRenderTarget->DrawEllipse({mid, radius, radius}, pBlackBrush, scaleX * stroke, pStrokeStyle);
        }
    }
    for (int idx = 0; idx < m; idx++)
    {
        int i = hlt[idx].i, j = hlt[idx].j;
        D2D_POINT_2F top = {(j + 0.5f) * l + margin, i * l + margin},
                     bottom = {(j + 0.5f) * l + margin, (i + 1) * l + margin},
                     left = {j * l + margin, (i + 0.5f) * l + margin},
                     right = {(j + 1) * l + margin, (i + 0.5f) * l + margin};
        float delta = (1 - square_ratio) / 2.0f * l;
        pRenderTarget->DrawLine({left.x + delta, top.y + delta},
                                {left.x + delta, top.y + delta + line_len},
                                pBlackBrush, scaleX * stroke);
        pRenderTarget->DrawLine({left.x + delta, top.y + delta},
                                {left.x + delta + line_len, top.y + delta},
                                pBlackBrush, scaleX * stroke);
        pRenderTarget->DrawLine({right.x - delta, top.y + delta},
                                {right.x - delta, top.y + delta + line_len},
                                pBlackBrush, scaleX * stroke);
        pRenderTarget->DrawLine({right.x - delta, top.y + delta},
                                {right.x - delta - line_len, top.y + delta},
                                pBlackBrush, scaleX * stroke);
        pRenderTarget->DrawLine({left.x + delta, bottom.y - delta},
                                {left.x + delta, bottom.y - delta - line_len},
                                pBlackBrush, scaleX * stroke);
        pRenderTarget->DrawLine({left.x + delta, bottom.y - delta},
                                {left.x + delta + line_len, bottom.y - delta},
                                pBlackBrush, scaleX * stroke);
        pRenderTarget->DrawLine({right.x - delta, bottom.y - delta},
                                {right.x - delta, bottom.y - delta - line_len},
                                pBlackBrush, scaleX * stroke);
        pRenderTarget->DrawLine({right.x - delta, bottom.y - delta},
                                {right.x - delta - line_len, bottom.y - delta},
                                pBlackBrush, scaleX * stroke);
    }
    HRESULT hr = pRenderTarget->EndDraw();
    if (!SUCCEEDED(hr))
        MessageBox(hWnd, L"Fail to finish paint!", L"Error", MB_ICONERROR);
}

// 下子修改局面并打印
int PutPiece(HWND hWnd, position pos)
{
    int i = pos.i, j = pos.j;
    if (i < 0 || j < 0 || i > 14 || j > 14 || brd[i][j] != 0)
    {
        PaintBoard(hWnd, 1, hlt, 0, NULL);
        return -1;
    }
    brd[i][j] = turn;
    while (idx_his < history.size() - 1)
        history.pop_back();
    history.push_back({i, j});
    idx_his++;
    hlt[1] = pos;
    PaintBoard(hWnd, 2, hlt, 1, hlt + 1);
    hlt[0] = hlt[1];
    int cnt = 0;
    bool flag = 0;
    for (int ii = 0; ii < 15; ii++)
    {
        cnt = brd[ii][j] == turn ? cnt + 1 : 0;
        flag = cnt >= 5;
        if (flag)
            goto label;
    }
    cnt = 0;
    for (int jj = 0; jj < 15; jj++)
    {
        cnt = brd[i][jj] == turn ? cnt + 1 : 0;
        flag = cnt >= 5;
        if (flag)
            goto label;
    }
    cnt = 0;
    for (int ii = 0; ii < 15; ii++)
    {
        cnt = i + j - ii >= 0 && i + j - ii <= 14 && brd[ii][i + j - ii] == turn ? cnt + 1 : 0;
        flag = cnt >= 5;
        if (flag)
            goto label;
    }
    cnt = 0;
    for (int ii = 0; ii < 15; ii++)
    {
        cnt = ii + j - i >= 0 && ii + j - i <= 14 && brd[ii][ii + j - i] == turn ? cnt + 1 : 0;
        flag = cnt >= 5;
        if (flag)
            goto label;
    }
label:
    wchar_t msg[5] = L"白方赢!";
    msg[0] = turn == 1 ? L'白' : L'黑';
    if (flag)
    {
        MessageBox(hWnd, msg, L"本局结束", MB_OK);
        over = 1;
    }
    return 0;
}

// 修改alpha, beta, value显示值
void EvalHistory(HWND hWnd)
{
    for (int i = 0; i < history_analyse.size(); i++)
    {
        turn = 3 - turn;
        brd[history_analyse[i].i][history_analyse[i].j] = turn;
    }
    value = Eval(brd);
    if (turn == 1 && value > alpha_glb)
    {
        alpha_glb = value;
        InvalidateRect(hWnd, &rc_text_alpha, FALSE);
    }
    if (turn == 2 && value < beta_glb)
    {
        beta_glb = value;
        InvalidateRect(hWnd, &rc_text_beta, FALSE);
    }
    for (int i = 0; i < history_analyse.size(); i++)
    {
        turn = 3 - turn;
        brd[history_analyse[i].i][history_analyse[i].j] = 0;
    }
    InvalidateRect(hWnd, &rc_text_val, FALSE);
}

// 窗体事件回调函数
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
        pmmi->ptMinTrackSize = {(int)(scaleX * 648), (int)(scaleY * 576)};
        pmmi->ptMaxTrackSize = {(int)(scaleX * 648), (int)(scaleY * 576)};
    }
    else if (uMsg == WM_PAINT)
    {
        RECT rc;
        GetClientRect(hWnd, &rc);
        rc.left = rc.bottom;
        PAINTSTRUCT ps;
        ps.rcPaint = rc;
        HDC hdc = BeginPaint(hWnd, &ps);
        COLORREF color = RGB(0xec, 0xec, 0xec);
        FillRect(hdc, &rc, CreateSolidBrush(color));
        SelectObject(hdc, hDefaultFont);
        SetBkColor(hdc, color);
        rc_text_alpha = {(int)(rc.bottom + 10 * scaleX), (int)(rc.top + 150 * scaleY),
                         (int)(rc.bottom + 110 * scaleX), (int)(rc.top + 180 * scaleY)};
        wstring ws = L"α: ";
        if (alpha_glb == INF || alpha_glb == -INF)
            ws += alpha_glb == INF ? L"INF" : L"-INF";
        else
        {
            wchar_t ws_tmp[16];
            _itow(alpha_glb, ws_tmp, 10);
            ws += ws_tmp;
        }
        DrawText(hdc, ws.c_str(), -1, &rc_text_alpha, DT_LEFT);
        rc_text_beta = {(int)(rc.bottom + 10 * scaleX), (int)(rc.top + 180 * scaleY),
                        (int)(rc.bottom + 100 * scaleX), (int)(rc.top + 210 * scaleY)};
        ws = L"β: ";
        if (beta_glb == INF || beta_glb == -INF)
            ws += beta_glb == INF ? L"INF" : L"-INF";
        else
        {
            wchar_t ws_tmp[16];
            _itow(beta_glb, ws_tmp, 10);
            ws += ws_tmp;
        }
        DrawText(hdc, ws.c_str(), -1, &rc_text_beta, DT_LEFT);
        rc_text_val = {(int)(rc.bottom + 10 * scaleX), (int)(rc.top + 210 * scaleY),
                       (int)(rc.bottom + 100 * scaleX), (int)(rc.top + 240 * scaleY)};
        ws = L"val: ";
        if (value == INF || value == -INF || value == UKN)
            ws += value == INF ? L"INF" : (value == -INF ? L"-INF" : L"UKN");
        else
        {
            wchar_t ws_tmp[16];
            _itow(value, ws_tmp, 10);
            ws += ws_tmp;
        }
        DrawText(hdc, ws.c_str(), -1, &rc_text_val, DT_LEFT);
        EndPaint(hWnd, &ps);
        PaintBoard(hWnd);
    }
    else if (uMsg == WM_LBUTTONDOWN)
    {
        RECT rc;
        GetClientRect(hWnd, &rc);
        int xPos = LOWORD(lParam) / scaleX, yPos = HIWORD(lParam) / scaleY;
        float l = (rc.bottom - rc.top - 2 * margin) / 15 / scaleY;
        int i = (yPos - margin / scaleY) / l, j = (xPos - margin / scaleY) / l;
        if (!running && !over)
        {
            turn = 3 - turn;
            if (PutPiece(hWnd, {i, j}) < 0)
                turn = 3 - turn;
        }
    }
    else if (uMsg == WM_COMMAND)
    {
        int nc = HIWORD(wParam), id = LOWORD(wParam);
        if (nc == BN_CLICKED)
            if (id == ID_BUTTON_STARTOVER)
            {
                running = 0;
                idx_his = -1;
                vector<position>().swap(history);
                memset(brd.mat, 0, sizeof(brd.mat));
                turn = 1;
                over = 0;
                depth_glb = 0;
                vector<position>().swap(dashed);
                vector<position>().swap(history_analyse);
                PaintBoard(hWnd);
                alpha_glb = -INF;
                beta_glb = INF;
                value = UKN;
                InvalidateRect(hWnd, &rc_text_alpha, FALSE);
                InvalidateRect(hWnd, &rc_text_beta, FALSE);
                InvalidateRect(hWnd, &rc_text_val, FALSE);
            }
            else if (id == ID_BUTTON_NEXT)
                if (!over && !running && !analysing)
                    running = 1;
                else if (analysing)
                {
                    analysing = 0;
                    SetWindowText(hWndButtonNext, L"▶");
                    vector<position>().swap(dashed);
                    vector<position>().swap(history_analyse);
                    PaintBoard(hWnd);
                    alpha_glb = -INF;
                    beta_glb = INF;
                    value = UKN;
                    InvalidateRect(hWnd, &rc_text_alpha, FALSE);
                    InvalidateRect(hWnd, &rc_text_beta, FALSE);
                    InvalidateRect(hWnd, &rc_text_val, FALSE);
                }
                else
                    running = 0;
            else if (id == ID_BUTTON_UP)
            {
                if (depth_glb > 0)
                {
                    turn = 3 - turn;
                    depth_glb--;
                    vector<position>().swap(dashed);
                    dashed = st_dashed.top();
                    if (!history_analyse.empty())
                        history_analyse.pop_back();
                    if (!history_analyse.empty())
                        history_analyse.pop_back();
                    current = 0;
                    if (!dashed.empty())
                        history_analyse.push_back(dashed[0]);
                    st_dashed.pop();
                    EvalHistory(hWnd);
                    PaintBoard(hWnd);
                }
            }
            else if (id == ID_BUTTON_DOWN)
            {
                if (!over && !running)
                {
                    turn = 3 - turn;
                    analysing = 1;
                    current = 0;
                    depth_glb++;
                    SetWindowText(hWndButtonNext, L"▷");
                    for (int i = 0; i < history_analyse.size(); i++)
                    {
                        brd[history_analyse[i].i][history_analyse[i].j] = turn;
                        turn = 3 - turn;
                    }
                    st_dashed.push(dashed);
                    vector<position>().swap(dashed);
                    GenPos(dashed);
                    for (int i = 0; i < history_analyse.size(); i++)
                    {
                        brd[history_analyse[i].i][history_analyse[i].j] = 0;
                        turn = 3 - turn;
                    }
                    history_analyse.push_back(dashed[0]);
                    PaintBoard(hWnd);
                    EvalHistory(hWnd);
                }
            }
            else if (id == ID_BUTTON_RIGHT)
                if (analysing)
                {
                    history_analyse.pop_back();
                    current++;
                    if (!dashed.empty())
                        history_analyse.push_back(dashed[current % dashed.size()]);
                    PaintBoard(hWnd);
                    EvalHistory(hWnd);
                }
                else
                {
                    if (idx_his < (int)history.size() - 1)
                    {
                        turn = 3 - turn;
                        idx_his++;
                        brd[history[idx_his].i][history[idx_his].j] = turn;
                        PaintBoard(hWnd);
                    }
                }
            else if (id == ID_BUTTON_LEFT)
                if (analysing)
                {
                    history_analyse.pop_back();
                    current--;
                    history_analyse.push_back(dashed[current % dashed.size()]);
                    PaintBoard(hWnd);
                    EvalHistory(hWnd);
                }
                else
                {
                    if (idx_his >= 0)
                    {
                        turn = 3 - turn;
                        brd[history[idx_his].i][history[idx_his].j] = 0;
                        idx_his--;
                        PaintBoard(hWnd);
                    }
                }
        SetFocus(hWnd);
    }
    else if (uMsg == WM_KEYDOWN)
    {
        if (wParam == VK_RETURN)
            SendMessage(hWnd, WM_COMMAND,
                        ((UINT)BN_CLICKED << 16) + over ? ID_BUTTON_STARTOVER : ID_BUTTON_NEXT,
                        (LPARAM)NULL);
        else if (wParam == VK_UP)
            SendMessage(hWnd, WM_COMMAND,
                        ((UINT)BN_CLICKED << 16) + ID_BUTTON_UP, (LPARAM)NULL);
        else if (wParam == VK_DOWN)
            SendMessage(hWnd, WM_COMMAND,
                        ((UINT)BN_CLICKED << 16) + ID_BUTTON_DOWN, (LPARAM)NULL);
        else if (wParam == VK_RIGHT)
            SendMessage(hWnd, WM_COMMAND,
                        ((UINT)BN_CLICKED << 16) + ID_BUTTON_RIGHT, (LPARAM)NULL);
        else if (wParam == VK_LEFT)
            SendMessage(hWnd, WM_COMMAND,
                        ((UINT)BN_CLICKED << 16) + ID_BUTTON_LEFT, (LPARAM)NULL);
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

int MaxValue(board &, int, int, int, position &), MinValue(board &, int, int, int, position &);

// 产生待评估的下子位置
void GenPos(vector<position> &vPos)
{
    // 考虑非空白位置周围的8个位置
    for (int i = 0; i < 15; i++)
        for (int j = 0; j < 15; j++)
            if (!brd[i][j] && (j < 14 && brd[i][j + 1] ||
                               j > 0 && brd[i][j - 1] ||
                               i < 14 && brd[i + 1][j] ||
                               i > 0 && brd[i - 1][j] ||
                               i < 14 && j < 14 && brd[i + 1][j + 1] ||
                               i < 14 && j > 0 && brd[i + 1][j - 1] ||
                               i > 0 && j < 14 && brd[i - 1][j + 1] ||
                               i > 0 && j > 0 && brd[i - 1][j - 1]))
                vPos.push_back({i, j});
    if (vPos.empty())
    {
        srand(GetTickCount());
        vPos.push_back({7 + rand() % 2, 7 + rand() % 2});
    }
}

// 评价函数
int Eval(board &brd)
{
    // 只统计可活动的不同连子数量对应的个数
    int num[3][16]; // num[i][j]表示i(0: 空白, 1: 白棋, 2: 黑棋)种类的棋子可活动连子数量为j的个数
    memset(num, 0, sizeof(num));
    for (int i = 0; i < 15; i++)
    {
        unsigned char cur, pre;
        int cnt = 1;
        cur = brd[i][0];
        pre = 3 - brd[i][0];
        for (int j = 1; j < 15; j++)
        {
            if (brd[i][j] == cur)
                cnt++;
            else
            {
                if (cur)
                    num[cur][cnt] += (int)(!brd[i][j]) + (int)(!pre) + (cnt >= 5);
                pre = cur;
                cur = brd[i][j];
                cnt = 1;
            }
        }
        if (cur && !pre)
            num[cur][cnt]++;
        num[cur][cnt] += cnt >= 5;
    }
    for (int j = 0; j < 15; j++)
    {
        unsigned char cur, pre;
        int cnt = 1;
        cur = brd[0][j];
        pre = 3 - brd[0][j];
        for (int i = 1; i < 15; i++)
        {
            if (brd[i][j] == cur)
                cnt++;
            else
            {
                if (cur)
                    num[cur][cnt] += (int)(!brd[i][j]) + (int)(!pre) + (cnt >= 5);
                pre = cur;
                cur = brd[i][j];
                cnt = 1;
            }
        }
        if (cur && !pre)
            num[cur][cnt]++;
        num[cur][cnt] += cnt >= 5;
    }
    for (int s = 0; s < 29; s++)
    {
        unsigned char cur, pre;
        int cnt = 1;
        cur = brd[s > 14 ? s - 14 : 0][s > 14 ? 14 : s];
        pre = 3 - brd[s > 14 ? s - 14 : 0][s > 14 ? 14 : s];
        for (int i = s > 14 ? s - 13 : 1; i <= (s > 14 ? 14 : s); i++)
        {
            if (brd[i][s - i] == cur)
                cnt++;
            else
            {
                if (cur)
                    num[cur][cnt] += (int)(!brd[i][s - i]) + (int)(!pre) + (cnt >= 5);
                pre = cur;
                cur = brd[i][s - i];
                cnt = 1;
            }
        }
        if (cur && !pre)
            num[cur][cnt]++;
        num[cur][cnt] += cnt >= 5;
    }
    for (int s = -14; s < 15; s++)
    {
        unsigned char cur, pre;
        int cnt = 1;
        cur = brd[s > 0 ? s : 0][s > 0 ? 0 : -s];
        pre = 3 - brd[14 - s][14];
        for (int i = s > 0 ? s + 1 : 1; i <= (s > 0 ? 14 : 14 + s); i++)
        {
            if (brd[i][i - s] == cur)
                cnt++;
            else
            {
                if (cur)
                    num[cur][cnt] += (int)(!brd[i][i - s]) + (int)(!pre) + (cnt >= 5);
                pre = cur;
                cur = brd[i][i - s];
                cnt = 1;
            }
        }
        if (cur && !pre)
            num[cur][cnt]++;
        num[cur][cnt] += cnt >= 5;
    }
    if (num[1][5] > 0)
        return INF;
    if (num[2][5] > 0)
        return -INF;
    return num[1][2] + 8 * num[1][3] -
           num[2][2] - 8 * num[2][3];
}

HWND hWnd;

// 极大值函数
int MaxValue(board &brd, int beta, int depth, int total, position &pos_ret)
{
    int eva = Eval(brd);
    if (!running || depth == total)
        return eva;
    if (eva == -INF)
        return -INF + depth;
    vector<position> pos_set;
    GenPos(pos_set);
    int alpha = -INF;
    for (int i = 0; i < pos_set.size(); i++) // 子结点循环
    {
        position pos = pos_set[i];
        brd[pos.i][pos.j] = turn;
        turn = 3 - turn;
        int v = MinValue(brd, alpha, depth + 1, total, pos_ret);
        turn = 3 - turn;
        brd[pos.i][pos.j] = 0;
        if (v > alpha)
        {
            alpha = v;
            if (depth == 0)
                pos_ret = pos;
        }
        if (alpha >= beta) // beta截断
            break;
    }
    return alpha;
}

// 极小值函数
int MinValue(board &brd, int alpha, int depth, int total, position &pos_ret)
{
    int eva = Eval(brd);
    if (!running || depth == total)
        return eva;
    if (eva == INF)
        return INF - depth;
    vector<position> pos_set;
    GenPos(pos_set);
    int beta = INF;
    for (int i = 0; i < pos_set.size(); i++)
    {
        position pos = pos_set[i];
        brd[pos.i][pos.j] = turn;
        turn = 3 - turn;
        int v = MaxValue(brd, beta, depth + 1, total, pos_ret);
        turn = 3 - turn;
        brd[pos.i][pos.j] = 0;
        if (v < beta)
        {
            beta = v;
            if (depth == 0)
                pos_ret = pos;
        }
        if (beta <= alpha) // alpha截断
            break;
    }
    return beta;
}

// 产生当前最好的下子位置
position BestPos()
{
    position bestpos, pos;
    for (int d = 2; d < 7; d++)
    {
        (turn == 1 ? MaxValue : MinValue)(brd, turn == 1 ? INF - d : -INF + d, 0, d, pos);
        if (running)
            bestpos = pos;
        else
        {
            running = 1;
            break;
        }
    }
    return bestpos;
}

// 等待线程回调函数
DWORD WINAPI ThreadFunction(LPVOID lParam)
{
    position *pBestPos = (position *)lParam;
    *pBestPos = BestPos();
    return 0;
}

// 程序入口
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPreInstance, PWSTR pCmdLine, int nCmdShow)
{
    HDC hdc = GetDC(0);
    scaleX = (double)GetDeviceCaps(hdc, DESKTOPHORZRES) / GetDeviceCaps(hdc, HORZRES);
    scaleY = (double)GetDeviceCaps(hdc, DESKTOPVERTRES) / GetDeviceCaps(hdc, VERTRES);
    SetProcessDPIAware();

    CoInitialize(NULL);

    // 注册窗体类
    WNDCLASS wc = {};
    wc.lpszClassName = L"Gomoku";
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClass(&wc);

    // 创建窗体们
    hWnd = CreateWindow(
        L"Gomoku", L"五子棋", WS_OVERLAPPEDWINDOW & (~WS_MAXIMIZEBOX),
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL, NULL, hInstance, NULL);

    hDefaultFont = CreateFont(20 * scaleY, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE,
                              DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS,
                              CLEARTYPE_QUALITY, VARIABLE_PITCH, TEXT("Segoe UI"));
    if (hDefaultFont == NULL)
    {
        MessageBox(hWnd, L"Fail to create font!", L"error", MB_ICONERROR);
        return 0;
    }

    RECT rc;
    GetClientRect(hWnd, &rc);
    rc_button_startover = {(int)(rc.bottom + 50 * scaleX), (int)(rc.top + 30 * scaleY),
                           (int)(rc.bottom + 80 * scaleX), (int)(rc.top + 60 * scaleY)};
    HWND hWndButtonStartOver = CreateWindow(
        L"BUTTON", L"🚫",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        rc_button_startover.left, rc_button_startover.top,
        rc_button_startover.right - rc_button_startover.left,
        rc_button_startover.bottom - rc_button_startover.top,
        hWnd, (HMENU)ID_BUTTON_STARTOVER,
        hInstance, NULL);
    if (hWndButtonStartOver == NULL)
    {
        MessageBox(hWnd, L"Fail to create button!", L"error", MB_ICONERROR);
        return 0;
    }
    SendMessage(hWndButtonStartOver, WM_SETFONT, (WPARAM)hDefaultFont, TRUE);
    rc_button_next = {(int)(rc.bottom + 10 * scaleX), (int)(rc.top + 30 * scaleY),
                      (int)(rc.bottom + 40 * scaleX), (int)(rc.top + 60 * scaleY)};
    hWndButtonNext = CreateWindow(
        L"BUTTON", L"▶",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        rc_button_next.left, rc_button_next.top,
        rc_button_next.right - rc_button_next.left,
        rc_button_next.bottom - rc_button_next.top,
        hWnd, (HMENU)ID_BUTTON_NEXT,
        hInstance, NULL);
    if (hWndButtonNext == NULL)
    {
        MessageBox(hWnd, L"Fail to create button!", L"error", MB_ICONERROR);
        return 0;
    }
    SendMessage(hWndButtonNext, WM_SETFONT, (WPARAM)hDefaultFont, TRUE);
    rc_button_up = {(int)(rc.bottom + 10 * scaleX), (int)(rc.top + 70 * scaleY),
                    (int)(rc.bottom + 40 * scaleX), (int)(rc.top + 100 * scaleY)};
    HWND hWndButtonUp = CreateWindow(
        L"BUTTON", L"↑",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        rc_button_up.left, rc_button_up.top,
        rc_button_up.right - rc_button_up.left,
        rc_button_up.bottom - rc_button_up.top,
        hWnd, (HMENU)ID_BUTTON_UP,
        hInstance, NULL);
    if (hWndButtonUp == NULL)
    {
        MessageBox(hWnd, L"Fail to create button!", L"error", MB_ICONERROR);
        return 0;
    }
    SendMessage(hWndButtonUp, WM_SETFONT, (WPARAM)hDefaultFont, TRUE);
    rc_button_down = {(int)(rc.bottom + 50 * scaleX), (int)(rc.top + 70 * scaleY),
                      (int)(rc.bottom + 80 * scaleX), (int)(rc.top + 100 * scaleY)};
    HWND hWndButtonDown = CreateWindow(
        L"BUTTON", L"↓",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        rc_button_down.left, rc_button_down.top,
        rc_button_down.right - rc_button_down.left,
        rc_button_down.bottom - rc_button_down.top,
        hWnd, (HMENU)ID_BUTTON_DOWN,
        hInstance, NULL);
    if (hWndButtonDown == NULL)
    {
        MessageBox(hWnd, L"Fail to create button!", L"error", MB_ICONERROR);
        return 0;
    }
    SendMessage(hWndButtonDown, WM_SETFONT, (WPARAM)hDefaultFont, TRUE);
    rc_button_right = {(int)(rc.bottom + 50 * scaleX), (int)(rc.top + 110 * scaleY),
                       (int)(rc.bottom + 80 * scaleX), (int)(rc.top + 140 * scaleY)};
    HWND hWndButtonRight = CreateWindow(
        L"BUTTON", L"→",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        rc_button_right.left, rc_button_right.top,
        rc_button_right.right - rc_button_right.left,
        rc_button_right.bottom - rc_button_right.top,
        hWnd, (HMENU)ID_BUTTON_RIGHT,
        hInstance, NULL);
    if (hWndButtonRight == NULL)
    {
        MessageBox(hWnd, L"Fail to create button!", L"error", MB_ICONERROR);
        return 0;
    }
    SendMessage(hWndButtonRight, WM_SETFONT, (WPARAM)hDefaultFont, TRUE);
    rc_button_left = {(int)(rc.bottom + 10 * scaleX), (int)(rc.top + 110 * scaleY),
                      (int)(rc.bottom + 40 * scaleX), (int)(rc.top + 140 * scaleY)};
    HWND hWndButtonLeft = CreateWindow(
        L"BUTTON", L"←",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        rc_button_left.left, rc_button_left.top,
        rc_button_left.right - rc_button_left.left,
        rc_button_left.bottom - rc_button_left.top,
        hWnd, (HMENU)ID_BUTTON_LEFT,
        hInstance, NULL);
    if (hWndButtonLeft == NULL)
    {
        MessageBox(hWnd, L"Fail to create button!", L"error", MB_ICONERROR);
        return 0;
    }
    SendMessage(hWndButtonLeft, WM_SETFONT, (WPARAM)hDefaultFont, TRUE);

    ShowWindow(hWnd, nCmdShow);

    MSG msg;
    HANDLE hThread = NULL;
    DWORD dwThreadID;
    position bestpos;
    ULONGLONG t_start;

    // 消息队列循环及等待计算线程
    while (true)
    {
        if (running)
        {
            if (hThread == NULL)
            {
                t_start = GetTickCount64();
                hThread = CreateThread(NULL, 0, ThreadFunction, &bestpos, 0, &dwThreadID);
                if (hThread == NULL)
                {
                    MessageBox(hWnd, L"Fail to create thread!", L"error", MB_ICONERROR);
                    continue;
                }
            }
            if (WaitForSingleObject(hThread, 1) == WAIT_OBJECT_0)
            {
                turn = 3 - turn;
                if (PutPiece(hWnd, bestpos) < 0)
                    turn = 3 - turn;
                running = 0;
                CloseHandle(hThread);
                hThread = NULL;
                SetWindowText(hWndButtonNext, L"▶");
            }
            else
            {
                if ((GetTickCount64() - t_start) % 1000 < 500)
                    SetWindowText(hWndButtonNext, L"▷");
                else
                    SetWindowText(hWndButtonNext, L"▶");
            }
        }
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