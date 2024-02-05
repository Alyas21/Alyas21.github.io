#include <iostream>
#include <windows.h>
#include <vector>
#include <map>
#include <utility>
#include <cstdlib> 

using namespace std;

// g++ conways.cpp -o conways.exe -lgdi32
// ./conways.exe

map<pair<int, int>, pair<int, int>> tem_dis;
vector<vector<int>> plain;
int width = 192;
int height = 108;
int gap = 10;

void drawPixel(HDC hdc, pair<int, int>pos, vector<int> rgb) {
    pair<int, int> re_pos = tem_dis[pos];
    RECT pixel = { re_pos.first, re_pos.second, re_pos.first + gap, re_pos.second + gap };
    HBRUSH brush = CreateSolidBrush(RGB(rgb[0], rgb[1], rgb[2]));
    FillRect(hdc, &pixel, brush);  
    DeleteObject(brush);     
}
void update(int x, int y, int value) {
    plain[y][x] = value;
}

bool noutbounds(int x, int y) {
    if(x < 0 || x >= width || y < 0 || y >=height) {
        return false;
    }
    return true;
}

int check_live(int x, int y) {
    int lneigh = 0;
    if (noutbounds(x - 1, y + 1) && plain[y + 1][x - 1] == 1) {
        lneigh++;
    }
    if (noutbounds(x, y + 1) && plain[y + 1][x] == 1) {
        lneigh++;
    }
    if (noutbounds(x + 1, y + 1) && plain[y + 1][x + 1] == 1) {
        lneigh++;
    }
    if (noutbounds(x - 1, y) && plain[y][x - 1] == 1) {
        lneigh++;
    }
    if (noutbounds(x + 1, y) && plain[y][x + 1] == 1) {
        lneigh++;
    }
    if (noutbounds(x - 1, y - 1) && plain[y - 1][x - 1] == 1) {
        lneigh++;
    }
    if (noutbounds(x, y - 1) && plain[y - 1][x] == 1) {
        lneigh++;
    }
    if (noutbounds(x + 1, y - 1) && plain[y - 1][x + 1] == 1) {
        lneigh++;
    }
    return lneigh;
}
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    vector<pair<int, pair<int, int>>> change_cells;
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
                    if(plain[y][x] == 1) {
                        drawPixel(memDC, make_pair(x, y), {255, 255, 255});
                    } else {
                        drawPixel(memDC, make_pair(x, y), {0, 0, 0});
                    }
                }
            }

            for(int y = 0; y < height; y++) {
                for(int x = 0; x < width; x++){
                    int liveNeighbors = check_live(x, y);
                    if (plain[y][x] == 1) {
                        if (liveNeighbors < 2 || liveNeighbors >= 4) {
                            change_cells.push_back(make_pair(0, make_pair(x, y)));
                        }
                    } else {
                        if (liveNeighbors == 3) {
                            change_cells.push_back(make_pair(1, make_pair(x, y)));
                        }
                    }
                }
            }

            for(auto& cell : change_cells) {
                update(cell.second.first, cell.second.second, cell.first);
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
    bool random = false;
    bool not_random = false;
    int chance = 3;
    for(int y = 0; y < height; y++) {
        vector<int> plain_row;
        for(int x = 0; x < width; x++) {
            tem_dis[make_pair(x, y)] = make_pair(x*gap, y*gap);
            if(random) {
            if (rand() % chance == 0) {
                plain_row.push_back(1);
            } else{
                plain_row.push_back(0);
            }}
            else if(not_random){
            if(x % 5 == 0 || x % 2 == 0){
                plain_row.push_back(0);
            }else {
                plain_row.push_back(1);
            }}
            if(!random && !not_random) {
                plain_row.push_back(0);
            }
        }
        plain.push_back(plain_row);
    }


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