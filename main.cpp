#include <iostream>
#include <queue>
#include <vector>
#include <limits>
#include <SDL3/SDL.h>
#include <thread>
#include <chrono>
#define WINDOW_WIDTH 1920
#define WINDOW_HEIGHT 1080
#define BOX_SIZE 20

const int GRID_COLS = WINDOW_WIDTH / BOX_SIZE;
const int GRID_ROWS = WINDOW_HEIGHT / BOX_SIZE;

struct Coordinate {
    int x, y;
    Coordinate(int x = 0, int y = 0) : x(x), y(y) {}
};

class Color {
public:
    int r, g, b, a;
    Color(int r, int g, int b, int a = 255) : r(r), g(g), b(b), a(a) {}
    void Apply(SDL_Renderer* renderer) {
        SDL_SetRenderDrawColor(renderer, r, g, b, a);
    }
};

class Box {
public:
    int gridX, gridY;
    int pixelX, pixelY;
    int size;
    bool isStart = false;
    bool isEnd = false;
    bool isWall = false;
    bool isVisited = false;
    bool isPath = false;
    int distance = std::numeric_limits<int>::max();
    Box* source = nullptr;

    Box(int gx = 0, int gy = 0, int size = BOX_SIZE)
        : gridX(gx), gridY(gy), size(size) {
        pixelX = gx * size;
        pixelY = gy * size;
    }

    void Draw(SDL_Renderer* renderer) {
        SDL_FRect rect = { static_cast<float>(pixelX), static_cast<float>(pixelY),
                           static_cast<float>(size), static_cast<float>(size) };

        if (isStart)
            Color(0, 255, 0).Apply(renderer); // Green
        else if (isEnd)
            Color(255, 0, 0).Apply(renderer); // Red
        else if (isPath)
            Color(255, 255, 0).Apply(renderer); // Yellow for final path
        else if (isVisited)
            Color(this->distance*10, 0, 0).Apply(renderer); // Light blue for visited
        else if (isWall)
            Color(0, 0, 0).Apply(renderer); // Black
        else
            Color(100, 100, 100).Apply(renderer); // Neutral grey

        SDL_RenderFillRect(renderer, &rect);

        // Draw grid border
        Color(255, 255, 255).Apply(renderer);
        SDL_RenderRect(renderer, &rect);
    }
};

class Grid {
public:
    Box boxes[GRID_COLS][GRID_ROWS];
    Coordinate startCoord = { 10, 10 };
    Coordinate endCoord = { 30, 50};

    std::priority_queue<std::pair<int, Box*>, std::vector<std::pair<int, Box*>>, std::greater<>> pq;
    bool algorithmRunning = true;
    bool tracingPath = false;
    Box* pathBox = nullptr;

    std::vector<Coordinate> walls;
   
    Grid() {

        InitWalls();
        for (int i = 0; i < GRID_COLS; ++i) {
            for (int j = 0; j < GRID_ROWS; ++j) {
                boxes[i][j] = Box(i, j);
            }
        }

        for (const auto& wall : walls) {
            if (wall.x >= 0 && wall.x < GRID_COLS && wall.y >= 0 && wall.y < GRID_ROWS) {
                boxes[wall.x][wall.y].isWall = true;
            }
        }

        GetBox(startCoord.x, startCoord.y)->isStart = true;
        GetBox(endCoord.x, endCoord.y)->isEnd = true;

        // Initialize algorithm
        Box* start = GetBox(startCoord.x, startCoord.y);
        start->distance = 0;
        pq.emplace(0, start);
    }


    void InitWalls()
    {
        for (int y = 0; y <= 29; ++y) {
            walls.push_back({ 5, y });
            walls.push_back({ 15, y });
        }

    }

    int  CalculateHeuristic(Box* current )
    {
		Box* end = GetBox(endCoord.x, endCoord.y);
		return abs(current->gridX - end->gridX) + abs(current->gridY - end->gridY);

    }

    void Step() {
        if (algorithmRunning && !pq.empty()) {
			Box* current = pq.top().second;

 
            pq.pop();

            if (current->isVisited)
                return;

            current->isVisited = true;

            if (current == GetBox(endCoord.x, endCoord.y)) {
                algorithmRunning = false;
                tracingPath = true;
                pathBox = current->source;
                return;
            }

            for (Box* neighbor : GetNeighbors(current)) {
                int g = current->distance + 1;
                int h = CalculateHeuristic(neighbor); // not current
                int f = g + h;

                if (g < neighbor->distance) {
                    neighbor->distance = g;
                    neighbor->source = current;
                    pq.emplace(f, neighbor);
                }
            }
        }
        else if (tracingPath && pathBox && !pathBox->isStart) {
            pathBox->isPath = true;
            pathBox = pathBox->source;
        }
        else {
            tracingPath = false;
        }



    }

    void Draw(SDL_Renderer* renderer) {
        for (int i = 0; i < GRID_COLS; ++i) {
            for (int j = 0; j < GRID_ROWS; ++j) {
                boxes[i][j].Draw(renderer);
            }
        }
    }

    Box* GetBox(int x, int y) {
        if (x < 0 || x >= GRID_COLS || y < 0 || y >= GRID_ROWS)
            return nullptr;
        return &boxes[x][y];
    }

    std::vector<Box*> GetNeighbors(Box* box) {
        std::vector<Box*> neighbors;
        //GOOD pATTERN KEEP IN MIND FOR LATER 
        int dx[] = { 0, 0, -1, 1 };
        int dy[] = { -1, 1, 0, 0 };
        for (int i = 0; i < 4; ++i) {
            Box* neighbor = GetBox(box->gridX + dx[i], box->gridY + dy[i]);
            if (neighbor && !neighbor->isWall)
                neighbors.push_back(neighbor);
        }
        return neighbors;
    }
};

int main() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << "\n";
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Dijkstra Pathfinding", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL);
    if (!window) {
        std::cerr << "Window Error: " << SDL_GetError() << "\n";
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);
    if (!renderer) {
        std::cerr << "Renderer Error: " << SDL_GetError() << "\n";
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    Grid grid;
    bool running = true;
    SDL_Event event;

    Uint64 lastStepTime = 0;
    const Uint64 stepDelay = 3;

    while (running) {
        Uint64 currentTime = SDL_GetTicks();

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
        }

        if (currentTime - lastStepTime >= stepDelay) {
            grid.Step();
            lastStepTime = currentTime;
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        grid.Draw(renderer);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}