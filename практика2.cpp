#include <windows.h>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cmath>

using namespace std;

// Константы игрыыыыыыыыыыыыыы
const int TILE_SIZE = 30;
const int MAP_WIDTH = 19;
const int MAP_HEIGHT = 21;
const int WINDOW_WIDTH = MAP_WIDTH * TILE_SIZE;
const int WINDOW_HEIGHT = MAP_HEIGHT * TILE_SIZE + 50;

// Карта игры (1 - стена, 0 - пусто, 2 - точка)
int gameMap[MAP_HEIGHT][MAP_WIDTH] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1},
    {1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,2,1},
    {1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,2,1},
    {1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,2,1},
    {1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1},
    {1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,2,1},
    {1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,2,1},
    {1,2,2,2,2,2,2,2,2,0,2,2,2,2,2,2,2,2,1},
    {1,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,1,1,1},
    {1,1,1,1,2,1,1,1,2,1,1,1,2,1,1,1,1,1,1},
    {1,2,2,2,2,2,2,2,2,0,2,2,2,2,2,2,2,2,1},
    {1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,2,1},
    {1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,2,1},
    {1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1},
    {1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,2,1},
    {1,2,1,1,2,1,1,1,2,1,1,1,2,1,1,2,1,2,1},
    {1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};

enum Direction { NONE, UP, DOWN, LEFT, RIGHT };

// Структуры для хранения позиций
struct Point {
    float x, y;
    int gridX, gridY;
};

// Глобальные переменные
Point pacman;
Direction pacmanDir = NONE;
Direction nextDir = NONE;
int score = 0;
int lives = 3;
bool gameRunning = true;

// Призраки
struct Ghost {
    Point pos;
    Direction dir;
    COLORREF color;
};

vector<Ghost> ghosts;

// Кисти для рисования
HBRUSH hBrushWall, hBrushPacman, hBrushGhost1, hBrushGhost2, hBrushGhost3, hBrushDot;
HPEN hPenBlack;
UINT_PTR timerId;
HWND g_hwnd;

// Функции
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void DrawGame(HDC hdc);
void UpdateGame();
void InitGame();

// Функция для проверки, свободна ли клетка (не стена)
bool isCellFree(int x, int y) {
    if (x < 0 || x >= MAP_WIDTH || y < 0 || y >= MAP_HEIGHT) return false;
    return gameMap[y][x] != 1;
}

// Функция для поиска свободной клетки рядом
void findFreeCell(int& x, int& y) {
    // Проверяем исходную клетку
    if (isCellFree(x, y)) return;

    // Ищем вокруг по спирали
    for (int radius = 1; radius < 5; radius++) {
        for (int dy = -radius; dy <= radius; dy++) {
            for (int dx = -radius; dx <= radius; dx++) {
                int newX = x + dx;
                int newY = y + dy;
                if (isCellFree(newX, newY)) {
                    x = newX;
                    y = newY;
                    return;
                }
            }
        }
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Регистрация класса окна
    const wchar_t CLASS_NAME[] = L"SimplePacman";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    RegisterClass(&wc);

    // Создание окна
    g_hwnd = CreateWindowEx(
        0, CLASS_NAME, L"Simple Pacman",
        WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME,
        CW_USEDEFAULT, CW_USEDEFAULT,
        WINDOW_WIDTH + 16, WINDOW_HEIGHT + 39,
        NULL, NULL, hInstance, NULL
    );

    if (!g_hwnd) return 0;

    ShowWindow(g_hwnd, nCmdShow);

    // Создание кистей
    hBrushWall = CreateSolidBrush(RGB(0, 0, 255));
    hBrushPacman = CreateSolidBrush(RGB(255, 255, 0));
    hBrushGhost1 = CreateSolidBrush(RGB(255, 0, 0));      // Красный
    hBrushGhost2 = CreateSolidBrush(RGB(255, 192, 203));  // Розовый
    hBrushGhost3 = CreateSolidBrush(RGB(0, 255, 255));    // Голубой
    hBrushDot = CreateSolidBrush(RGB(255, 255, 255));

    srand(time(NULL));
    InitGame();

    timerId = SetTimer(g_hwnd, 1, 16, NULL);

    // Цикл сообщений
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Очистка
    KillTimer(g_hwnd, timerId);
    DeleteObject(hBrushWall);
    DeleteObject(hBrushPacman);
    DeleteObject(hBrushGhost1);
    DeleteObject(hBrushGhost2);
    DeleteObject(hBrushGhost3);
    DeleteObject(hBrushDot);

    return 0;
}

void InitGame() {
    // Находим свободную клетку для Пакмана (центр внизу)
    int pacX = 9, pacY = 15;
    findFreeCell(pacX, pacY);

    pacman.gridX = pacX;
    pacman.gridY = pacY;
    pacman.x = pacman.gridX * TILE_SIZE + TILE_SIZE / 2;
    pacman.y = pacman.gridY * TILE_SIZE + TILE_SIZE / 2;
    pacmanDir = NONE;
    nextDir = NONE;

    ghosts.clear();

    // Красный призрак (первый)
    int ghostX = 9, ghostY = 9;
    findFreeCell(ghostX, ghostY);
    Ghost g1;
    g1.pos.gridX = ghostX;
    g1.pos.gridY = ghostY;
    g1.pos.x = g1.pos.gridX * TILE_SIZE + TILE_SIZE / 2;
    g1.pos.y = g1.pos.gridY * TILE_SIZE + TILE_SIZE / 2;
    g1.dir = RIGHT;
    g1.color = RGB(255, 0, 0);  // Красный
    ghosts.push_back(g1);

    // Розовый призрак (второй)
    ghostX = 8; ghostY = 10;  // Немного другая начальная позиция
    findFreeCell(ghostX, ghostY);
    Ghost g2;
    g2.pos.gridX = ghostX;
    g2.pos.gridY = ghostY;
    g2.pos.x = g2.pos.gridX * TILE_SIZE + TILE_SIZE / 2;
    g2.pos.y = g2.pos.gridY * TILE_SIZE + TILE_SIZE / 2;
    g2.dir = LEFT;
    g2.color = RGB(255, 192, 203);  // Розовый
    ghosts.push_back(g2);

    // Голубой призрак (третий)
    ghostX = 10; ghostY = 10;  // Еще одна позиция
    findFreeCell(ghostX, ghostY);
    Ghost g3;
    g3.pos.gridX = ghostX;
    g3.pos.gridY = ghostY;
    g3.pos.x = g3.pos.gridX * TILE_SIZE + TILE_SIZE / 2;
    g3.pos.y = g3.pos.gridY * TILE_SIZE + TILE_SIZE / 2;
    g3.dir = UP;
    g3.color = RGB(0, 255, 255);  // Голубой
    ghosts.push_back(g3);

    score = 0;
    lives = 3;
    gameRunning = true;
}

void UpdateGame() {
    if (!gameRunning) return;

    // Проверка смены направления
    if (nextDir != NONE) {
        int testX = pacman.gridX;
        int testY = pacman.gridY;

        if (nextDir == RIGHT) testX++;
        else if (nextDir == LEFT) testX--;
        else if (nextDir == DOWN) testY++;
        else if (nextDir == UP) testY--;

        if (isCellFree(testX, testY)) {
            pacmanDir = nextDir;
            nextDir = NONE;
        }
    }

    // Движение Пакмана
    if (pacmanDir != NONE) {
        float newX = pacman.x, newY = pacman.y;

        if (pacmanDir == RIGHT) newX += 3;
        else if (pacmanDir == LEFT) newX -= 3;
        else if (pacmanDir == DOWN) newY += 3;
        else if (pacmanDir == UP) newY -= 3;

        int tileX = (int)(newX / TILE_SIZE);
        int tileY = (int)(newY / TILE_SIZE);

        // Коррекция границ
        if (newX < 0) {
            newX = MAP_WIDTH * TILE_SIZE - 1;
            tileX = MAP_WIDTH - 1;
        }
        if (newX >= MAP_WIDTH * TILE_SIZE) {
            newX = 1;
            tileX = 0;
        }

        if (tileX >= 0 && tileX < MAP_WIDTH && tileY >= 0 && tileY < MAP_HEIGHT) {
            if (gameMap[tileY][tileX] != 1) {
                pacman.x = newX;
                pacman.y = newY;
                pacman.gridX = tileX;
                pacman.gridY = tileY;
            }
        }
    }

    // Сбор точек
    if (gameMap[pacman.gridY][pacman.gridX] == 2) {
        gameMap[pacman.gridY][pacman.gridX] = 0;
        score += 10;
    }

    // Движение призрака
    for (size_t i = 0; i < ghosts.size(); i++) {
        Ghost& g = ghosts[i];

        // Простой AI - случайное движение
        if (rand() % 100 < 2) { // 2% шанс сменить направление
            vector<Direction> dirs;

            // Проверяем доступные направления
            if (isCellFree(g.pos.gridX, g.pos.gridY - 1)) dirs.push_back(UP);
            if (isCellFree(g.pos.gridX, g.pos.gridY + 1)) dirs.push_back(DOWN);
            if (isCellFree(g.pos.gridX - 1, g.pos.gridY)) dirs.push_back(LEFT);
            if (isCellFree(g.pos.gridX + 1, g.pos.gridY)) dirs.push_back(RIGHT);

            if (!dirs.empty()) {
                g.dir = dirs[rand() % dirs.size()];
            }
        }

        // Движение
        float newX = g.pos.x, newY = g.pos.y;

        if (g.dir == RIGHT) newX += 2;
        else if (g.dir == LEFT) newX -= 2;
        else if (g.dir == DOWN) newY += 2;
        else if (g.dir == UP) newY -= 2;

        int tileX = (int)(newX / TILE_SIZE);
        int tileY = (int)(newY / TILE_SIZE);

        // Коррекция границ
        if (newX < 0) {
            newX = MAP_WIDTH * TILE_SIZE - 1;
            tileX = MAP_WIDTH - 1;
        }
        if (newX >= MAP_WIDTH * TILE_SIZE) {
            newX = 1;
            tileX = 0;
        }

        if (tileX >= 0 && tileX < MAP_WIDTH && tileY >= 0 && tileY < MAP_HEIGHT) {
            if (gameMap[tileY][tileX] != 1) {
                g.pos.x = newX;
                g.pos.y = newY;
                g.pos.gridX = tileX;
                g.pos.gridY = tileY;
            }
        }
    }

    // Проверка столкновения
    for (size_t i = 0; i < ghosts.size(); i++) {
        float dx = pacman.x - ghosts[i].pos.x;
        float dy = pacman.y - ghosts[i].pos.y;
        float dist = sqrt(dx * dx + dy * dy);

        if (dist < TILE_SIZE / 2) {
            lives--;
            if (lives <= 0) {
                gameRunning = false;
            }
            else {
                // Возврат на старт
                int pacX = 9, pacY = 15;
                findFreeCell(pacX, pacY);
                pacman.gridX = pacX;
                pacman.gridY = pacY;
                pacman.x = pacman.gridX * TILE_SIZE + TILE_SIZE / 2;
                pacman.y = pacman.gridY * TILE_SIZE + TILE_SIZE / 2;
                pacmanDir = NONE;
                nextDir = NONE;

                int ghostX = 9, ghostY = 9;
                findFreeCell(ghostX, ghostY);
                ghosts[i].pos.gridX = ghostX;
                ghosts[i].pos.gridY = ghostY;
                ghosts[i].pos.x = ghosts[i].pos.gridX * TILE_SIZE + TILE_SIZE / 2;
                ghosts[i].pos.y = ghosts[i].pos.gridY * TILE_SIZE + TILE_SIZE / 2;

                Sleep(500);
            }
        }
    }
}

void DrawGame(HDC hdc) {
    // Очистка
    RECT client;
    GetClientRect(g_hwnd, &client);
    FillRect(hdc, &client, (HBRUSH)GetStockObject(BLACK_BRUSH));

    // Рисование карты
    for (int i = 0; i < MAP_HEIGHT; i++) {
        for (int j = 0; j < MAP_WIDTH; j++) {
            if (gameMap[i][j] == 1) {
                // Стены
                RECT wall;
                wall.left = j * TILE_SIZE;
                wall.top = i * TILE_SIZE;
                wall.right = wall.left + TILE_SIZE;
                wall.bottom = wall.top + TILE_SIZE;
                FillRect(hdc, &wall, hBrushWall);
            }
            else if (gameMap[i][j] == 2) {
                // Точки
                RECT dot;
                dot.left = j * TILE_SIZE + TILE_SIZE / 2 - 2;
                dot.top = i * TILE_SIZE + TILE_SIZE / 2 - 2;
                dot.right = dot.left + 4;
                dot.bottom = dot.top + 4;
                FillRect(hdc, &dot, hBrushDot);
            }
        }
    }

    // Рисование Пакмана
    SelectObject(hdc, hBrushPacman);
    SelectObject(hdc, hPenBlack);
    Ellipse(hdc,
        (int)(pacman.x - TILE_SIZE / 2),
        (int)(pacman.y - TILE_SIZE / 2),
        (int)(pacman.x + TILE_SIZE / 2),
        (int)(pacman.y + TILE_SIZE / 2));

    // Рисование призраков
    for (size_t i = 0; i < ghosts.size(); i++) {
        Ghost& g = ghosts[i];

        // Выбираем кисть в зависимости от цвета призрака
        if (g.color == RGB(255, 0, 0)) {
            SelectObject(hdc, hBrushGhost1);  // Красный
        }
        else if (g.color == RGB(255, 192, 203)) {
            SelectObject(hdc, hBrushGhost2);  // Розовый
        }
        else if (g.color == RGB(0, 255, 255)) {
            SelectObject(hdc, hBrushGhost3);  // Голубой
        }

        SelectObject(hdc, hPenBlack);
        Ellipse(hdc,
            (int)(g.pos.x - TILE_SIZE / 2),
            (int)(g.pos.y - TILE_SIZE / 2),
            (int)(g.pos.x + TILE_SIZE / 2),
            (int)(g.pos.y + TILE_SIZE / 2));
    }

    // Счет и жизни
    wchar_t buffer[100];
    wsprintf(buffer, L"Score: %d  Lives: %d", score, lives);

    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(255, 255, 255));
    TextOut(hdc, 10, MAP_HEIGHT * TILE_SIZE + 10, buffer, wcslen(buffer));

    if (!gameRunning) {
        SetTextColor(hdc, RGB(255, 0, 0));
        TextOut(hdc, WINDOW_WIDTH / 2 - 40, WINDOW_HEIGHT / 2, L"GAME OVER", 9);
    }
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        DrawGame(hdc);
        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_TIMER:
        UpdateGame();
        InvalidateRect(hwnd, NULL, TRUE);
        return 0;

    case WM_KEYDOWN:
        if (gameRunning) {
            switch (wParam) {
            case VK_RIGHT: nextDir = RIGHT; break;
            case VK_LEFT: nextDir = LEFT; break;
            case VK_UP: nextDir = UP; break;
            case VK_DOWN: nextDir = DOWN; break;
            }
        }
        return 0;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}