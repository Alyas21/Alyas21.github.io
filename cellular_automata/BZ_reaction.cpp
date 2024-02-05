#include <iostream>
#include <windows.h>
#include <vector>
#include <map>
#include <utility>
#include <cstdlib> 

using namespace std;

// g++ BZ_reaction.cpp -o BZ_reaction.exe -lgdi32
// ./BZ_reaction.exe

map<pair<int, int>, pair<int, int>> tem_dis;
vector<vector<pair<int, bool>>> plain;
int width = 192;
int height = 108;
int gap = 10;

int maxInfection = 100;
int defHealthy = 50;
int rateInfection = 15;

void drawPixel(HDC hdc, pair<int, int>pos, vector<int> rgb) {
    pair<int, int> re_pos = tem_dis[pos];
    RECT pixel = { re_pos.first, re_pos.second, re_pos.first + gap, re_pos.second + gap };
    HBRUSH brush = CreateSolidBrush(RGB(rgb[0], rgb[1], rgb[2]));
    FillRect(hdc, &pixel, brush);  
    DeleteObject(brush);     
}
void update(int x, int y, int value, bool healthy) {
    plain[y][x].first = value;
    plain[y][x].second = healthy;
}

bool noutbounds(int x, int y) {
    if(x < 0 || x >= width || y < 0 || y >=height) {
        return false;
    }
    return true;
}

//average of neighbors
int aveNei(int x, int y) {
    int sumInfection = 0;
    if (noutbounds(x - 1, y + 1)) {
        sumInfection += plain[y + 1][x - 1].first; 
    }
    if (noutbounds(x, y + 1)) {
        sumInfection += plain[y + 1][x].first;
    }
    if (noutbounds(x + 1, y + 1)) {
        sumInfection += plain[y + 1][x + 1].first;
    }
    if (noutbounds(x - 1, y)) {
        sumInfection += plain[y][x - 1].first;
    }
    if (noutbounds(x + 1, y)) {
        sumInfection += plain[y][x + 1].first;
    }
    if (noutbounds(x - 1, y - 1)) {
        sumInfection += plain[y - 1][x - 1].first;
    }
    if (noutbounds(x, y - 1)) {
        sumInfection += plain[y - 1][x].first;
    }
    if (noutbounds(x + 1, y - 1)) {
        sumInfection += plain[y - 1][x + 1].first;
    }
    int averageInfection = static_cast<int>(sumInfection / 8) + rateInfection;
    return averageInfection;
}
int aveInfectedNei(int x, int y) {
    int ninfected = 0;
    int sumInfection = 0;
    if (noutbounds(x - 1, y + 1) && !plain[y+1][x-1].second) {
        ninfected ++;
        sumInfection += plain[y + 1][x - 1].first; 
    }
    if (noutbounds(x, y + 1)&& !plain[y+1][x].second) {
        ninfected ++;
        sumInfection += plain[y + 1][x].first;
    }
    if (noutbounds(x + 1, y + 1)&& !plain[y+1][x+1].second) {
        ninfected ++;
        sumInfection += plain[y + 1][x + 1].first;
    }
    if (noutbounds(x - 1, y) && !plain[y][x-1].second) {
        ninfected ++;
        sumInfection += plain[y][x - 1].first;
    }
    if (noutbounds(x + 1, y) && !plain[y][x+1].second) {
        ninfected ++;
        sumInfection += plain[y][x + 1].first;
    }
    if (noutbounds(x - 1, y - 1) && !plain[y-1][x-1].second) {
        ninfected ++;
        sumInfection += plain[y - 1][x - 1].first;
    }
    if (noutbounds(x, y - 1) && !plain[y-1][x].second) {
        ninfected ++;
        sumInfection += plain[y - 1][x].first;
    }
    if (noutbounds(x + 1, y - 1) && !plain[y-1][x+1].second) {
        ninfected ++;
        sumInfection += plain[y - 1][x + 1].first;
    }
    int averageInfection = static_cast<int>(sumInfection / ninfected);
    return averageInfection;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    vector<pair<pair<int, bool>, pair<int, int>>> change_cells;
    switch (uMsg) {
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            //create memory DC: contect 
            HDC memDC = CreateCompatibleDC(hdc);
            HBITMAP memBitmap = CreateCompatibleBitmap(hdc, width * gap, height * gap);
            HBITMAP hbmOld = (HBITMAP)SelectObject(memDC, memBitmap);

            // Clear/turn black the memory DC
            HBRUSH bgBrush = CreateSolidBrush(RGB(0, 0, 0));
            RECT rect = {0, 0, width * gap, height * gap};
            FillRect(memDC, &rect, bgBrush);
            DeleteObject(bgBrush);

            //draw the cells
            for(int y = 0; y < height; y++) {
                for(int x = 0; x < width; x++){
                    if(plain[y][x].second) {
                        drawPixel(memDC, make_pair(x, y), {255, 255, 255});
                    } else {
                        drawPixel(memDC, make_pair(x, y), {0, 0, 0});
                    }
                }
            }
            cout << "HAHHA" << endl;

            for(int y = 0; y < height; y++) {
                for(int x = 0; x < width; x++){
                    if(plain[y][x].first >= maxInfection) {
                        change_cells.push_back(make_pair(make_pair(0, true), make_pair(x, y)));

                    }else if(plain[y][x].second) {
                        change_cells.push_back(make_pair(make_pair(aveInfectedNei(x, y), false), make_pair(x, y)));

                    }else if(((maxInfection >= plain[y][x].first) && (plain[y][x].first >= 0)) && !plain[y][x].second){
                        change_cells.push_back(make_pair(make_pair(aveNei(x, y), false), make_pair(x, y)));
                    }
                    else{
                        cout << "AHHHHHHHHHH" << endl;
                    }
                
                }
            }
            cout << "doop da" << endl;

            for(auto& cell : change_cells) {
                update(cell.second.first, cell.second.second, cell.first.first, cell.first.second);
            }

            BitBlt(hdc, 0, 0, width * gap, height * gap, memDC, 0, 0, SRCCOPY);

            // Clean up
            SelectObject(memDC, hbmOld);
            DeleteObject(memBitmap);
            DeleteDC(memDC);

            EndPaint(hwnd, &ps);
            return 0;
        }
        case WM_CREATE: {
            SetTimer(hwnd, 1, 31, NULL);  // 31 milliseconds interval
            break;
        }
        case WM_TIMER: {
            RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE);
            break;
        }
        case WM_DESTROY: {
            PostQuitMessage(0);
            return 0;
        }
        default:
            return DefWindowProcW(hwnd, uMsg, wParam, lParam);
    }
}

int main() {    
    bool random = true;
    bool not_random = false;
    int chance = 3;
    for(int y = 0; y < height; y++) {
        vector<pair<int, bool>> plain_row;
        for(int x = 0; x < width; x++) {
            tem_dis[make_pair(x, y)] = make_pair(x*gap, y*gap);
            if(random) {
            if (rand() % chance == 0) {
                plain_row.push_back(make_pair(10, true));
            } else{
                plain_row.push_back(make_pair(10, true));
            }}
            // else if(not_random){
            // if(x % 5 == 0 || x % 2 == 0){
            //     plain_row.push_back(0);
            // }else {
            //     plain_row.push_back(1);
            // }}
            // if(!random && !not_random) {
            //     plain_row.push_back(0);
            // }
        }
        plain.push_back(plain_row);
    }
    cout << "YAY" << endl;


    const wchar_t* CLASS_NAME = L"SampleWindowClass";

    WNDCLASSW wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandleW(NULL);
    wc.lpszClassName = CLASS_NAME;
    RegisterClassW(&wc);

    HWND hwnd = CreateWindowExW(
        //---------------------------
        0,                   //opitional window styles
        CLASS_NAME,          //window class
        L"ADD NAME PLEASE AHHHHHH",     //window title
        WS_POPUP, //window style

        //---------------------------
        //make fullscreen
        0, //x postion
        0, //y position
        GetSystemMetrics(SM_CXSCREEN),
        GetSystemMetrics(SM_CYSCREEN), 
        //---------------------------
        NULL,   //parent window
        NULL,   //menu
        GetModuleHandleW(NULL), //Instance handle
        NULL    // Addutuibak aplication data
        //---------------------------
    );

    if (hwnd == NULL) {
        return 0;
    }
    HCURSOR hcursor = LoadCursor(NULL, IDC_ARROW);
    SetCursor(hcursor);

    ShowWindow(hwnd, SW_SHOWDEFAULT);
    //Enter message loop
    MSG msg = {};
    while(GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    return 0;
}