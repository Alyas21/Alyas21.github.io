#include <iostream>
#include <windows.h>
#include <vector>
#include <map>
#include <utility>
#include <cstdlib> 
#include <cmath>
#include <random>

using namespace std;

// g++ slime_mould.cpp -o slime_mould.exe -lgdi32
// ./slime_mould.exe

map<pair<int, int>, pair<int, int>> tem_dis;
vector<vector<float>> plain;
const int gap = 15;
const int width = static_cast<int>(GetSystemMetrics(SM_CXSCREEN) / gap);
const int height = static_cast<int>(GetSystemMetrics(SM_CYSCREEN) / gap);

struct Agent {
    pair<float, float> position;
    float angle;

    Agent(pair<float, float> pos, float ang) : position(pos), angle(ang) {}
};
vector<Agent> agents;
void drawPixel(HDC hdc, pair<int, int>pos, vector<int> rgb) {
    pair<int, int> re_pos = tem_dis[pos];
    RECT pixel = { re_pos.first, re_pos.second, re_pos.first + gap, re_pos.second + gap };
    HBRUSH brush = CreateSolidBrush(RGB(rgb[0], rgb[1], rgb[2]));
    FillRect(hdc, &pixel, brush);  
    DeleteObject(brush);     
}
float cal_angle(int px, int py, int mx, int my) {
    float angle = atan2(my - py, mx - px);
    return angle;
}
pair<int, int> get_mouse() {
    POINT cursorPos;
    GetCursorPos(&cursorPos);
    ScreenToClient(GetActiveWindow(), &cursorPos);
    return make_pair(floor(cursorPos.x / gap), floor(cursorPos.y / gap));
}

void update(int x, int y, double value) {
    plain[y][x] = value;
}
vector<int> grayscale(float value) {
    value = max(0.0f, min(1.0f, value));
    int grayscaleValue = static_cast<int>(value * 255);
    return {grayscaleValue, grayscaleValue, grayscaleValue};
}
float gen_random() {
    static random_device rd; 
    static mt19937 gen(rd());
    static uniform_real_distribution<float> dis(0.0f, 1.0f); // no idea how this works
    return dis(gen);
}
float Ngen_random(float n) {
    static random_device rd; 
    static mt19937 gen(rd());
    static uniform_real_distribution<float> dis(0.0f, n); // no idea how this works
    return dis(gen);
}

float sense(Agent agent, float sensorAngleOffset) {
    float sensorAngle = agent.angle + sensorAngleOffset;
    pair<float, float> sensorDir = make_pair(cos(sensorAngle), sin(sensorAngle));
    pair<int, int> sensorp1 = make_pair(static_cast<int>(agent.position.first + sensorDir.first), 
                                    static_cast<int>(agent.position.second + sensorDir.second));
    pair<int, int> sensorCenter = make_pair(static_cast<int>(sensorp1.first * sensorAngleOffset), 
                                    static_cast<int>(sensorp1.second * sensorAngleOffset));
    float sensorRadius = 1.0;
    float sum = 0;
    for (int dy = -sensorRadius; dy <= sensorRadius; dy++) {
        for (int dx = -sensorRadius; dx <= sensorRadius; dx++) {
            pair<int, int> pos = make_pair(sensorCenter.first + dx, sensorCenter.second + dy);
            if(pos.first >= 0 && pos.first < width && pos.second >= 0 && pos.second < height) {
                sum += plain[pos.second][pos.first];
            }
        }
    }
    return sum;
}

void update_agent(Agent& ag) {
    pair<float, float> direction = make_pair(cos(ag.angle), sin(ag.angle));
    pair<float, float> new_pos = make_pair(ag.position.first + direction.first, ag.position.second + direction.second);

    int new_x = static_cast<int>(new_pos.first);
    int new_y = static_cast<int>(new_pos.second);


    float random_steer = Ngen_random(1);

    if (new_x < 0 || new_x >= width || new_y < 0 || new_y >= height) {
        new_x = min(width - 1, max(0, new_x));
        new_y = min(height - 1, max(0, new_y));
        ag.angle = atan2(-direction.second + (gen_random() - 0.5), -direction.first + (gen_random() - 0.5));
    }else {

        int angleSpace = 2;
        float weightF = sense(ag, 0);
        float weightL = sense(ag, angleSpace);
        float weightR = sense(ag, -angleSpace);

        if(weightF > weightL && weightF > weightR) {
            ag.angle += 0.0;
        }else if(weightF < weightL && weightF < weightR) {
            ag.angle += (random_steer - 0.5) * 2;
        }else if(weightR > weightL) {
            ag.angle -= random_steer * 2;
        }else if(weightL > weightR) {
            ag.angle += random_steer * 2;
        }
    }

    update(new_x, new_y, 1.0);
    ag.position = new_pos;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    vector<pair<float, pair<int, int>>> ccell;
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
   
            for(auto& ag : agents) {
                update_agent(ag);
            }

            float fade_value = 0.005;

            for(int y = 0; y < height; y++) {
                for(int x = 0; x < width; x++) {
                    if(plain[y][x] > 0) {
                        vector<int> gray_value = grayscale(plain[y][x]);
                        drawPixel(memDC, make_pair(x, y), gray_value);
                        plain[y][x] -= fade_value;
                    }
                    else{
                        drawPixel(memDC, make_pair(x, y), {0, 0, 0});
                    }
                }
            }

            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {

                    float sum = 0.0;
                    for (int ny = y - 1; ny <= y + 1; ny++) {
                        for (int nx = x - 1; nx <= x + 1; nx++) {
                            if (ny >= 0 && ny < height && nx >= 0 && nx < width) {
                                sum += plain[ny][nx];
                            }
                        }
                    }
                    sum /= 10;  
                    ccell.push_back(make_pair(sum, make_pair(x, y)));
                }
            }
            for(auto& ccel : ccell) {
                update(ccel.second.first, ccel.second.second, ccel.first);
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
            SetTimer(hwnd, 1, 50, NULL);  // 31 milliseconds interval
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
    const bool random = false;

    const float sx = width / 2.0f;  
    const float sy = height / 2.0f; 
    const float radius = min(width, height) / 4.0f; 


    const int nAgents = 1;

    if(random) {
        for (int i = 0; i < nAgents; i++) {
            int x = static_cast<int>(Ngen_random(width));
            int y = static_cast<int>(Ngen_random(height));
            float angle = Ngen_random(360.0);

            agents.emplace_back(make_pair(x, y), angle);
        }
    }else {
    for (float i = 0.0; i < nAgents; i += 1.0) {
        float angle = i * (M_PI / 180.0);
        float x = sx + radius * cos(angle);
        float y = sy + radius * sin(angle);
        float agentAngle = atan2(sy - y, sx - x);

        agents.emplace_back(make_pair(x, y), agentAngle);
    }
    }
    for(int y = 0; y < height; y++) {
        vector<float> plain_row;
        for(int x = 0; x < width; x++) {
            tem_dis[make_pair(x, y)] = make_pair(x*gap, y*gap);
            plain_row.push_back(0.0);
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