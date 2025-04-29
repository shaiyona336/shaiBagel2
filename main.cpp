#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <iostream>
#include <vector>
#include <cmath>

// Constants
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const float GRAVITY = 0.2f;
const int TERRAIN_SIZE = 10;
const int WORM_SIZE = 30;

// Simple structs for our demo (not using the ECS system for this demo)
struct GameObject {
    float x, y;         // Position
    float vx = 0, vy = 0; // Velocity
    SDL_FRect rect;     // Rendering rectangle

    GameObject(float posX, float posY, float width, float height)
        : x(posX), y(posY), rect{posX, posY, width, height} {}

    void updateRect() {
        rect.x = x;
        rect.y = y;
    }
};

struct Worm : GameObject {
    int health = 100;

    Worm(float posX, float posY)
        : GameObject(posX, posY, WORM_SIZE, WORM_SIZE) {}

    void move(float dx) {
        x += dx;
        updateRect();
    }

    void jump() {
        if (vy == 0) { // Only jump if on ground
            vy = -6.0f;
        }
    }
};

struct Terrain {
    std::vector<std::vector<bool>> blocks;

    Terrain(int width, int height) {
        blocks.resize(width / TERRAIN_SIZE);
        for (auto& column : blocks) {
            column.resize(height / TERRAIN_SIZE, false);
        }

        // Create a simple terrain landscape
        for (size_t x = 0; x < blocks.size(); x++) {
            // Create a rolling hill landscape
            int hillHeight = height / 2 +
                             static_cast<int>(sin(x * 0.1) * 100) +
                             static_cast<int>(sin(x * 0.05) * 50);

            for (size_t y = hillHeight / TERRAIN_SIZE; y < blocks[0].size(); y++) {
                blocks[x][y] = true;
            }
        }
    }

    bool checkCollision(const SDL_FRect& rect) {
        int startX = std::max(0, static_cast<int>(rect.x) / TERRAIN_SIZE);
        int endX = std::min(static_cast<int>(blocks.size() - 1), static_cast<int>(rect.x + rect.w) / TERRAIN_SIZE);
        int startY = std::max(0, static_cast<int>(rect.y) / TERRAIN_SIZE);
        int endY = std::min(static_cast<int>(blocks[0].size() - 1), static_cast<int>(rect.y + rect.h) / TERRAIN_SIZE);

        for (int x = startX; x <= endX; x++) {
            for (int y = startY; y <= endY; y++) {
                if (blocks[x][y]) {
                    SDL_FRect terrainRect = {
                        static_cast<float>(x * TERRAIN_SIZE),
                        static_cast<float>(y * TERRAIN_SIZE),
                        static_cast<float>(TERRAIN_SIZE),
                        static_cast<float>(TERRAIN_SIZE)
                    };

                    // Check for intersection
                    if (SDL_HasRectIntersectionFloat(&rect, &terrainRect)) {
                        return true;
                    }
                }
            }
        }

        return false;
    }

    void render(SDL_Renderer* renderer) {
        SDL_SetRenderDrawColor(renderer, 139, 69, 19, 255); // Brown color

        for (size_t x = 0; x < blocks.size(); x++) {
            for (size_t y = 0; y < blocks[x].size(); y++) {
                if (blocks[x][y]) {
                    SDL_FRect terrainRect = {
                        static_cast<float>(x * TERRAIN_SIZE),
                        static_cast<float>(y * TERRAIN_SIZE),
                        static_cast<float>(TERRAIN_SIZE),
                        static_cast<float>(TERRAIN_SIZE)
                    };
                    SDL_RenderFillRect(renderer, &terrainRect);
                }
            }
        }
    }
};

int main(int argc, char* argv[]) {
    // Initialize SDL
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return -1;
    }

    // Create window and renderer
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;

    if (!SDL_CreateWindowAndRenderer("Worms Game Demo", SCREEN_WIDTH, SCREEN_HEIGHT, 0, &window, &renderer)) {
        std::cerr << "Window or renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return -1;
    }

    // Create game objects
    Terrain terrain(SCREEN_WIDTH, SCREEN_HEIGHT);

    // Create worms at different positions
    std::vector<Worm> worms;
    worms.emplace_back(100, 100);
    worms.emplace_back(300, 100);
    worms.emplace_back(500, 100);

    // Game loop variables
    bool quit = false;
    SDL_Event e;
    int currentWorm = 0;  // Index of the active worm
    int turnTimer = 0;    // Timer to track turn duration
    const int TURN_DURATION = 100; // Number of frames per turn

    // Main game loop
    while (!quit) {
        // Handle events
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_EVENT_QUIT) {
                quit = true;
            }
        }

        // Update turn timer
        turnTimer++;

        // Handle the active worm
        Worm& activeWorm = worms[currentWorm];

        // Simple AI for demonstration (moves randomly and jumps)
        if (turnTimer % 20 == 0) {
            // Randomly move left, right or jump
            int action = rand() % 3;
            if (action == 0) {
                activeWorm.move(-10.0f);
            } else if (action == 1) {
                activeWorm.move(10.0f);
            } else {
                activeWorm.jump();
            }
        }

        // Change to next worm when turn is over
        if (turnTimer >= TURN_DURATION) {
            currentWorm = (currentWorm + 1) % worms.size();
            turnTimer = 0;
        }

        // Update all worms
        for (auto& worm : worms) {
            // Apply gravity
            worm.vy += GRAVITY;

            // Move worm
            worm.y += worm.vy;
            worm.updateRect();

            // Check terrain collision
            if (terrain.checkCollision(worm.rect)) {
                // Move back up until not colliding
                while (terrain.checkCollision(worm.rect)) {
                    worm.y -= 1.0f;
                    worm.updateRect();
                }
                worm.vy = 0;
            }
        }

        // Clear screen
        SDL_SetRenderDrawColor(renderer, 135, 206, 235, 255); // Sky blue
        SDL_RenderClear(renderer);

        // Render terrain
        terrain.render(renderer);

        // Render worms
        for (int i = 0; i < worms.size(); i++) {
            const auto& worm = worms[i];

            // Draw worm body
            if (i == currentWorm) {
                SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Active worm is red
            } else {
                SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); // Inactive worms are green
            }
            SDL_RenderFillRect(renderer, &worm.rect);

            // Draw health bar
            SDL_FRect healthBar = {
                worm.x,
                worm.y - 10,
                WORM_SIZE * (worm.health / 100.0f),
                5
            };
            SDL_SetRenderDrawColor(renderer, 255,
                                    static_cast<Uint8>(worm.health * 2.55f),
                                    0, 255);
            SDL_RenderFillRect(renderer, &healthBar);
        }

        // Render turn indicator
        SDL_FRect turnIndicator = {10, 10, 100, 20};
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderFillRect(renderer, &turnIndicator);

        // Present renderer
        SDL_RenderPresent(renderer);

        // Cap frame rate
        SDL_Delay(16); // ~60 FPS
    }

    // Cleanup
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}