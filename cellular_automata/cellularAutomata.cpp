#include <iostream>
#include <windows.h>
#include <vector>
#include <map>
#include <utility>
#include <cstdlib> 
#include <ctime>

using namespace std;

// g++ cellularAutomata.cpp -o cellularAutomata.exe -lgdi32
// ./cellularAutomata.exe

map<pair<int, int>, pair<int, int>> tem_dis;
vector<vector<pair<int, int>>> plain;
//how many live neighbour cells are needed for a live cell to stay alive
vector<int> survive = {2, 8};
//how many live neighbour cells are needed for a dead cell to become alive
vector<int> turn_alive = {2, 8};
//how many frames it takes for a cell to die
const int ndecay = 10;
//type of checking neighours
const string nei_type = "M";

int decay = ndecay;
int max_decay = ndecay;

int width = 192;
int height = 108;
int gap = 10;

void drawPixel(HDC hdc, pair<int, int>pos, int decay) {
    pair<int, int> re_pos = tem_dis[pos];
    RECT pixel = { re_pos.first, re_pos.second, re_pos.first + gap, re_pos.second + gap };
    int colourValue;
    if (decay > 0) {
        colourValue =  (decay * (255 / max_decay));
        if (colourValue < 0) colourValue = 0;
    } else {
        colourValue = 0;
    }


    HBRUSH brush = CreateSolidBrush(RGB(colourValue, colourValue, colourValue));
    FillRect(hdc, &pixel, brush);
    DeleteObject(brush);
}
void update(int x, int y, int value) {
    plain[y][x].first = value;
}

bool noutbounds(int x, int y) {
    if(x < 0 || x >= width || y < 0 || y >=height) {
        return false;
    }
    return true;
}

int check_liveM(int x, int y) {
    int lneigh = 0;
    if (noutbounds(x - 1, y + 1) && plain[y + 1][x - 1].first == 1) {
        lneigh++;
    }
    if (noutbounds(x, y + 1) && plain[y + 1][x].first == 1) {
        lneigh++;
    }
    if (noutbounds(x + 1, y + 1) && plain[y + 1][x + 1].first == 1) {
        lneigh++;
    }
    if (noutbounds(x - 1, y) && plain[y][x - 1].first == 1) {
        lneigh++;
    }
    if (noutbounds(x + 1, y) && plain[y][x + 1].first == 1) {
        lneigh++;
    }
    if (noutbounds(x - 1, y - 1) && plain[y - 1][x - 1].first == 1) {
        lneigh++;
    }
    if (noutbounds(x, y - 1) && plain[y - 1][x].first == 1) {
        lneigh++;
    }
    if (noutbounds(x + 1, y - 1) && plain[y - 1][x + 1].first == 1) {
        lneigh++;
    }
    return lneigh;
}

int check_liveVN(int x, int y) {
    int lneigh = 0;
    if (noutbounds(x, y + 1) && plain[y + 1][x].first == 1) {
        lneigh++;
    }
    if (noutbounds(x - 1, y) && plain[y][x - 1].first == 1) {
        lneigh++;
    }
    if (noutbounds(x + 1, y) && plain[y][x + 1].first == 1) {
        lneigh++;
    }
    if (noutbounds(x, y - 1) && plain[y - 1][x].first == 1) {
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
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    if(plain[y][x].second < 0) {
                        plain[y][x].second = 0;
                    }
                    int decayLevel = plain[y][x].second;
                    drawPixel(memDC, make_pair(x, y), decayLevel);
                }
            }

            for(int y = 0; y < height; y++) {
                for(int x = 0; x < width; x++){
                    int liveNeighbours;
                    if(nei_type == "VN") {
                        liveNeighbours = check_liveVN(x, y);
                    }else {
                        liveNeighbours = check_liveM(x, y);
                    }
                    if(plain[y][x].first == 1) {
                        bool stay_alive = false;
                        for(auto& sur_cell : survive) {
                            if(liveNeighbours == sur_cell) {
                                stay_alive = true;
                            }
                        }
                        if(!stay_alive || plain[y][x].second < max_decay) {
                            plain[y][x].second -= 1;
                            if(plain[y][x].second <= 0) {
                                change_cells.push_back(make_pair(0, make_pair(x, y)));   
                            } 
                        }
                    }
                    else if(plain[y][x].first == 0) {
                        bool become_alive = false;
                        for(auto& al_cell : turn_alive) {
                            if(liveNeighbours == al_cell) {
                                become_alive = true;
                            }
                        }
                        if(become_alive) {
                            plain[y][x].second = max_decay;
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
    srand(static_cast<unsigned int>(time(0))); 
    bool random = true;
    bool not_random = false;
    int chance = 5;
    for(int y = 0; y < height; y++) {
        vector<pair<int, int>> plain_row;
        for(int x = 0; x < width; x++) {
            tem_dis[make_pair(x, y)] = make_pair(x*gap, y*gap);
            if (random) {
                if (rand() % chance == 0) {
                    plain_row.push_back(make_pair(1, max_decay));
                } else {
                    plain_row.push_back(make_pair(0, 0));
                }
            }
            else if(not_random){
            if(x % 5 || x % 5){
                plain_row.push_back(make_pair(1, max_decay));
            }else {
                plain_row.push_back(make_pair(0, 0));
            }}
            if(!random && !not_random) {
                plain_row.push_back(make_pair(0, max_decay));
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