// basic organism cellular automata
#include <iostream>
#include <windows.h>
#include <vector>
#include <map>
#include <utility>
#include <cstdlib>
#include <ctime> 

using namespace std;

// g++ plant_sim.cpp -o plant_sim.exe -lgdi32
// ./plant_sim.exe

map<pair<int, int>, pair<int, int>> tem_dis;
vector<vector<int>> plain;
const int gap = 10;
const int width = static_cast<int>(GetSystemMetrics(SM_CXSCREEN) / gap);
const int height =static_cast<int>(GetSystemMetrics(SM_CYSCREEN) / gap);

//0 = empty cell
//1 = eats food
//2 = proudceses food
//3 = food cell

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

pair<int, vector<pair<int, int>>> check(int x, int y, int type) {
    vector<pair<int, int>> emp_nei;
    int lneigh = 0;
    if (noutbounds(x - 1, y + 1) && plain[y + 1][x - 1] == type) {
        lneigh++;
        emp_nei.push_back(make_pair(x-1, y+1));
    }
    if (noutbounds(x, y + 1) && plain[y + 1][x] == type) {
        lneigh++;
        emp_nei.push_back(make_pair(x, y+1));
    }
    if (noutbounds(x + 1, y + 1) && plain[y + 1][x + 1] == type) {
        lneigh++;
        emp_nei.push_back(make_pair(x+1, y+1));
    }
    if (noutbounds(x - 1, y) && plain[y][x - 1] == type) {
        lneigh++;
        emp_nei.push_back(make_pair(x-1, y));
    }
    if (noutbounds(x + 1, y) && plain[y][x + 1] == type) {
        lneigh++;
        emp_nei.push_back(make_pair(x+1, y));
    }
    if (noutbounds(x - 1, y - 1) && plain[y - 1][x - 1] == type) {
        lneigh++;
        emp_nei.push_back(make_pair(x-1, y-1));
    }
    if (noutbounds(x, y - 1) && plain[y - 1][x] == type) {
        lneigh++;
        emp_nei.push_back(make_pair(x, y-1));
    }
    if (noutbounds(x + 1, y - 1) && plain[y - 1][x + 1] == type) {
        lneigh++;
        emp_nei.push_back(make_pair(x+1, y-1));
    }
    return make_pair(lneigh, emp_nei);
}
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    srand(static_cast<unsigned int>(time(0))); 
    vector<pair<int, pair<int, int>>> change_cells;

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
                    if(plain[y][x] == 1) {
                        drawPixel(memDC, make_pair(x, y), {255, 0, 0});
                    }
                    else if(plain[y][x] == 2) {
                        drawPixel(memDC, make_pair(x, y), {0, 255, 0});
                    }
                    else if(plain[y][x] == 3){
                        drawPixel(memDC, make_pair(x, y), {0, 0, 255});
                    }else {
                        drawPixel(memDC, make_pair(x, y), {0, 0, 0});
                    }
                }
            }

            for(int y = 0; y < height; y++) {
                for(int x = 0; x < width; x++){
                    if (plain[y][x] == 1) { // eats food
                        pair<int, vector<pair<int, int>>> Fdata = check(x, y, 3);
                        int foodNeighbors = Fdata.first;
                        vector<pair<int, int>> FavNeighbours = Fdata.second;
                        if (foodNeighbors > 0) {
                            int findex = rand() % FavNeighbours.size();
                            pair<int, int> pick_food = FavNeighbours[findex];
                            change_cells.push_back(make_pair(1, pick_food));
                        } else {
                            change_cells.push_back(make_pair(2, make_pair(x, y)));
                        }

                    } else if (plain[y][x] == 2) { // makes food

                        pair<int, vector<pair<int, int>>> FCdata = check(x, y, 0);
                        int emptyNeighbors = FCdata.first;
                        vector<pair<int, int>> empPosNeighbours = FCdata.second;
                        if (emptyNeighbors > 0 && emptyNeighbors < suffication ) {
                            for(int nfood = 0; nfood < nfood_create; nfood++) {
                                int cindex = rand() % empPosNeighbours.size();
                                pair<int, int> pick_create = empPosNeighbours[cindex];
                                change_cells.push_back(make_pair(3, pick_create));
                                empPosNeighbours.erase(empPosNeighbours.begin() +  cindex);
                                if(empPosNeighbours.size() <= 0) {
                                    break;
                                }
                            }
                        } else {
                            change_cells.push_back(make_pair(0, make_pair(x, y)));
                        }
                    }
                    else if(plain[y][x] == 3) {
                        change_cells.push_back(make_pair(0, make_pair(x, y)));
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
    bool not_random = 0;
    
    vector<int> table = {1, 2, 2, 3, 0 ,0, 0, 0, 0, 0, 0,};
    int tsize = table.size();

    for(int y = 0; y < height; y++) {
        vector<int> plain_row;
        for(int x = 0; x < width; x++) {
            tem_dis[make_pair(x, y)] = make_pair(x*gap, y*gap);
            if(random) {
            int randIndex = rand() % tsize;
            plain_row.push_back(table[randIndex]);
            }
            else if(not_random){
            if(x % 5 == 0 || y % 5 == 0){
                tem_dis[make_pair(x, y)] = make_pair(x*gap, y*gap);
                int randIndex = rand() % tsize;
                plain_row.push_back(table[randIndex]);
            }else {
                plain_row.push_back(2);
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