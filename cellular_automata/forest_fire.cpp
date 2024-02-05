// basic organism cellular automata
#include <iostream>
#include <windows.h>
#include <vector>
#include <map>
#include <utility>
#include <cstdlib>
#include <ctime> 

using namespace std;

// g++ forest_fire.cpp -o forest_fire.exe -lgdi32
// ./forest_fire.exe

map<pair<int, int>, pair<int, int>> tem_dis;
vector<vector<pair<int, int>>> plain;
int width = 192;
int height = 108;
int gap = 10;

//empty cell = 0
//fire cell = 1
//wood cell = 2
//ash cell = 3

int ash_decay = 12; //time it takes to turn ash into wood
int wood_decay = 10; //time it tales for wood to burn

void drawPixel(HDC hdc, pair<int, int>pos, vector<int> rgb) {
    pair<int, int> re_pos = tem_dis[pos];
    RECT pixel = { re_pos.first, re_pos.second, re_pos.first + gap, re_pos.second + gap };
    HBRUSH brush = CreateSolidBrush(RGB(rgb[0], rgb[1], rgb[2]));
    FillRect(hdc, &pixel, brush);  
    DeleteObject(brush);     
}
void update(int x, int y, int value, int decay) {
    plain[y][x].first = value;
    plain[y][x].second = decay;
}

bool noutbounds(int x, int y) {
    if(x < 0 || x >= width || y < 0 || y >=height) {
        return false;
    }
    return true;
}

pair<int, vector<pair<int, int>>> check(int x, int y, int type) {
    vector<pair<int, int>> emp_nei;
    int lneigh = 0;
    if (noutbounds(x - 1, y + 1) && plain[y + 1][x - 1].first == type) {
        lneigh++;
        emp_nei.push_back(make_pair(x-1, y+1));
    }
    if (noutbounds(x, y + 1) && plain[y + 1][x].first == type) {
        lneigh++;
        emp_nei.push_back(make_pair(x, y+1));
    }
    if (noutbounds(x + 1, y + 1) && plain[y + 1][x + 1].first == type) {
        lneigh++;
        emp_nei.push_back(make_pair(x+1, y+1));
    }
    if (noutbounds(x - 1, y) && plain[y][x - 1].first == type) {
        lneigh++;
        emp_nei.push_back(make_pair(x-1, y));
    }
    if (noutbounds(x + 1, y) && plain[y][x + 1].first == type) {
        lneigh++;
        emp_nei.push_back(make_pair(x+1, y));
    }
    if (noutbounds(x - 1, y - 1) && plain[y - 1][x - 1].first == type) {
        lneigh++;
        emp_nei.push_back(make_pair(x-1, y-1));
    }
    if (noutbounds(x, y - 1) && plain[y - 1][x].first == type) {
        lneigh++;
        emp_nei.push_back(make_pair(x, y-1));
    }
    if (noutbounds(x + 1, y - 1) && plain[y - 1][x + 1].first == type) {
        lneigh++;
        emp_nei.push_back(make_pair(x+1, y-1));
    }
    return make_pair(lneigh, emp_nei);
}
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    srand(static_cast<unsigned int>(time(0))); 
    vector<pair<pair<int, int>, pair<int, int>>> change_cells;

    const int nfood_create = 1;
    const int suffication = 6;
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
                    if(plain[y][x].first == 1) {
                        drawPixel(memDC, make_pair(x, y), {255, 0, 0});
                    }
                    else if(plain[y][x].first == 2) {
                        drawPixel(memDC, make_pair(x, y), {0, 255, 0});
                    }
                    else if(plain[y][x].first == 3){
                        drawPixel(memDC, make_pair(x, y), {0, 0, 0});
                    }else {
                        drawPixel(memDC, make_pair(x, y), {0, 0, 0});
                    }
                }
            }

            for(int y = 0; y < height; y++) {
                for(int x = 0; x < width; x++){
                    if (plain[y][x].first == 1) { // fire
                        pair<int, vector<pair<int, int>>> sfire = check(x, y, 2);
                        int wfnei = sfire.first;

                        if(plain[y][x].second < wood_decay) {
                            if(plain[y][x].second > 0) {
                                plain[y][x].second -= 1;
                            }else {
                                change_cells.push_back(make_pair(make_pair(3, ash_decay), make_pair(x, y)));
                            }
                        }
                        else if(wfnei > 0) {
                            change_cells.push_back(make_pair(make_pair(1, wood_decay-=1), make_pair(x, y)));
                            for(auto& fcell : sfire.second) {
                                change_cells.push_back(make_pair(make_pair(1, wood_decay), make_pair(fcell.first, fcell.second)));
                            }
                        }else {
                            change_cells.push_back(make_pair(make_pair(3, ash_decay), make_pair(x, y)));
                        }
                    }
                    else if (plain[y][x].first == 2) { // wood
                        pair<int, vector<pair<int, int>>> csuffic = check(x, y, 2);
                        int wnei = csuffic.first;
                        int suffic = 5 + rand() % 4;
                        if(wnei > 0 && wnei < suffic) {
                            ;
                        } else {
                            change_cells.push_back(make_pair(make_pair(1, wood_decay), make_pair(x, y)));
                        }
                    }
                    else if(plain[y][x].first == 3) {// ash
                        pair<int, vector<pair<int, int>>> asuffic = check(x, y, 3);

                        if(asuffic.first > 7) {
                            change_cells.push_back(make_pair(make_pair(0, 0), make_pair(x, y)));
                        }


                        else if(plain[y][x].second > 0) {
                            plain[y][x].second -= 1;
                        }else {
                            change_cells.push_back(make_pair(make_pair(2, ash_decay), make_pair(x, y)));
                        }
                    }
                    else if(plain[y][x].first == 0) {
                        pair<int, vector<pair<int, int>>> asuffic = check(x, y, 3);

                        if(asuffic.first > 5) {
                            change_cells.push_back(make_pair(make_pair(3, ash_decay), make_pair(x, y)));
                        }
                    }
                }
            }

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
            SetTimer(hwnd, 1, 100, NULL);  // 31 milliseconds interval
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
    srand(static_cast<unsigned int>(time(0))); 

    bool random = true;
    bool not_random = false;
    
    vector<int> table = {2, 2, 0};
    int tsize = table.size();

    for(int y = 0; y < height; y++) {
        vector<pair<int, int>> plain_row;
        for(int x = 0; x < width; x++) {
            tem_dis[make_pair(x, y)] = make_pair(x*gap, y*gap);
            if(random) {
            int randIndex = rand() % tsize;
            plain_row.push_back(make_pair(table[randIndex], ash_decay));
            }
            // else if(not_random){
            // if(x % 5 == 0 || y % 5 == 0){
            //     tem_dis[make_pair(x, y)] = make_pair(x*gap, y*gap);
            //     int randIndex = rand() % tsize;
            //     plain_row.push_back(table[randIndex]);
            // }else {
            //     plain_row.push_back(2);
            // }}
            if(!random && !not_random) {
                plain_row.push_back(make_pair(0, 0));
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