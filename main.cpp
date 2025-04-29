#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <iostream>
#include <vector>
#include <cmath>

// Constants
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const float GRAVITY = 0.2f;
const float WIND = 0.03f;
const int TERRAIN_SIZE = 10;
const int WORM_SIZE = 30;
const int PROJECTILE_SIZE = 8;
const int EXPLOSION_MAX_SIZE = 80;

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
    float aimAngle = 0.0f;

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

struct Projectile : GameObject {
    bool active = true;

    Projectile(float posX, float posY, float velX, float velY)
        : GameObject(posX, posY, PROJECTILE_SIZE, PROJECTILE_SIZE) {
        vx = velX;
        vy = velY;
    }
};

struct Explosion : GameObject {
    int duration = 30;
    int currentFrame = 0;
    int maxRadius;

    Explosion(float posX, float posY, int radius)
        : GameObject(posX, posY, radius * 2, radius * 2), maxRadius(radius) {
        rect.x = posX - radius;
        rect.y = posY - radius;
    }

    bool update() {
        currentFrame++;
        float progress = static_cast<float>(currentFrame) / duration;
        float currentSize = maxRadius * 2 * (progress < 0.5f ? progress * 2 : (1.0f - progress) * 2);

        rect.w = currentSize;
        rect.h = currentSize;
        rect.x = x - currentSize / 2;
        rect.y = y - currentSize / 2;

        return currentFrame < duration;
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

    void destroy(int centerX, int centerY, int radius) {
        int startX = std::max(0, (centerX - radius) / TERRAIN_SIZE);
        int endX = std::min(static_cast<int>(blocks.size() - 1), (centerX + radius) / TERRAIN_SIZE);
        int startY = std::max(0, (centerY - radius) / TERRAIN_SIZE);
        int endY = std::min(static_cast<int>(blocks[0].size() - 1), (centerY + radius) / TERRAIN_SIZE);

        for (int x = startX; x <= endX; x++) {
            for (int y = startY; y <= endY; y++) {
                float dx = (x * TERRAIN_SIZE + TERRAIN_SIZE / 2) - centerX;
                float dy = (y * TERRAIN_SIZE + TERRAIN_SIZE / 2) - centerY;
                float distance = sqrt(dx * dx + dy * dy);

                if (distance < radius) {
                    blocks[x][y] = false;
                }
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

    std::vector<Projectile> projectiles;
    std::vector<Explosion> explosions;

    // Game loop variables
    bool quit = false;
    SDL_Event e;
    int currentWorm = 0;  // Index of the active worm
    bool isFiring = false;
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

        // Simple AI for demonstration (moves randomly and fires)
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

        // Change aim angle periodically
        if (turnTimer % 10 == 0) {
            activeWorm.aimAngle += 0.1f;
            if (activeWorm.aimAngle > 2*M_PI) activeWorm.aimAngle = 0;
        }

        // Fire a projectile near the end of the turn
        if (turnTimer == TURN_DURATION - 20 && !isFiring) {
            isFiring = true;

            // Calculate projectile velocity from aim angle
            float power = 7.0f;
            float projVelX = cos(activeWorm.aimAngle) * power;
            float projVelY = sin(activeWorm.aimAngle) * power;

            // Create projectile
            projectiles.emplace_back(
                activeWorm.x + WORM_SIZE/2,
                activeWorm.y + WORM_SIZE/2,
                projVelX,
                projVelY
            );
        }

        // Change to next worm when turn is over
        if (turnTimer >= TURN_DURATION) {
            currentWorm = (currentWorm + 1) % worms.size();
            turnTimer = 0;
            isFiring = false;
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

        // Update projectiles
        for (auto it = projectiles.begin(); it != projectiles.end();) {
            auto& proj = *it;

            // Apply physics
            proj.vy += GRAVITY;
            proj.vx += WIND;

            // Move projectile
            proj.x += proj.vx;
            proj.y += proj.vy;
            proj.updateRect();

            // Check if out of bounds
            if (proj.x < 0 || proj.x > SCREEN_WIDTH || proj.y < 0 || proj.y > SCREEN_HEIGHT) {
                it = projectiles.erase(it);
                continue;
            }

            // Check terrain collision
            if (terrain.checkCollision(proj.rect)) {
                // Create explosion
                explosions.emplace_back(proj.x, proj.y, 30);

                // Destroy terrain
                terrain.destroy(proj.x, proj.y, 30);

                // Check if explosion hits worms
                for (auto& worm : worms) {
                    float dx = worm.x + WORM_SIZE/2 - proj.x;
                    float dy = worm.y + WORM_SIZE/2 - proj.y;
                    float distance = sqrt(dx*dx + dy*dy);

                    if (distance < 40) {
                        // Apply damage based on distance
                        float damageScale = 1.0f - (distance / 40.0f);
                        worm.health -= static_cast<int>(30 * damageScale);

                        // Apply knockback
                        float knockback = 5.0f * damageScale;
                        worm.vx += (dx / distance) * knockback;
                        worm.vy += (dy / distance) * knockback - 2.0f; // Extra upward boost
                    }
                }

                it = projectiles.erase(it);
            } else {
                ++it;
            }
        }

        // Update explosions
        for (auto it = explosions.begin(); it != explosions.end();) {
            if (!it->update()) {
                it = explosions.erase(it);
            } else {
                ++it;
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

            // Draw aim line for active worm
            if (i == currentWorm) {
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                float lineLength = 30.0f;
                float endX = worm.x + WORM_SIZE/2 + cos(worm.aimAngle) * lineLength;
                float endY = worm.y + WORM_SIZE/2 + sin(worm.aimAngle) * lineLength;
                SDL_RenderLine(renderer,
                              worm.x + WORM_SIZE/2,
                              worm.y + WORM_SIZE/2,
                              endX, endY);
            }
        }

        // Render projectiles
        SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255); // Yellow
        for (const auto& proj : projectiles) {
            SDL_RenderFillRect(renderer, &proj.rect);
        }

        // Render explosions
        for (const auto& explosion : explosions) {
            // Gradient from red to yellow
            float progress = static_cast<float>(explosion.currentFrame) / explosion.duration;
            SDL_SetRenderDrawColor(renderer,
                                   255,
                                   static_cast<Uint8>(255 * progress),
                                   0, 255);
            SDL_RenderFillRect(renderer, &explosion.rect);
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