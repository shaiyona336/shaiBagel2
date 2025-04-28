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
    bool isActive = false;
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

    void aim(float angle) {
        aimAngle = angle;
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

    std::vector<Worm> worms;
    // Create 4 worms at different positions
    worms.emplace_back(100, 100);
    worms.emplace_back(300, 100);
    worms.emplace_back(500, 100);
    worms.emplace_back(700, 100);
    worms[0].isActive = true; // First worm is active

    std::vector<Projectile> projectiles;
    std::vector<Explosion> explosions;

    // Game loop variables
    bool quit = false;
    SDL_Event e;
    int currentWorm = 0;
    bool firing = false;
    int animationStep = 0;

    // Main game loop
    while (!quit) {
        // Handle events
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_EVENT_QUIT) {
                quit = true;
            }
        }

        // Predefined animation sequence for the demo
        animationStep++;

        // Every 60 frames, active worm will fire
        if (animationStep % 60 == 0 && !firing) {
            firing = true;
            Worm& active = worms[currentWorm];

            // Calculate projectile velocity based on aim angle
            float projectileSpeed = 8.0f;
            float projectileVx = cos(active.aimAngle) * projectileSpeed;
            float projectileVy = sin(active.aimAngle) * projectileSpeed;

            // Create a projectile
            projectiles.emplace_back(
                active.x + WORM_SIZE / 2,
                active.y + WORM_SIZE / 2,
                projectileVx,
                projectileVy
            );
        }

        // Every 180 frames, switch to next worm
        if (animationStep % 180 == 0) {
            worms[currentWorm].isActive = false;
            currentWorm = (currentWorm + 1) % worms.size();
            worms[currentWorm].isActive = true;
            firing = false;

            // Set a new random aim angle for variety
            worms[currentWorm].aimAngle = static_cast<float>(rand() % 628) / 100.0f; // 0 to 2π
        }

        // Update worms
        for (auto& worm : worms) {
            // Apply gravity
            worm.vy += GRAVITY;

            // Move worm
            worm.y += worm.vy;
            worm.updateRect();

            // Check collision with terrain
            if (terrain.checkCollision(worm.rect)) {
                // Move back up until not colliding
                while (terrain.checkCollision(worm.rect)) {
                    worm.y -= 1.0f;
                    worm.updateRect();
                }
                worm.vy = 0;
            }

            // Randomly move active worm
            if (worm.isActive && animationStep % 10 == 0) {
                int randomMove = rand() % 3; // 0: left, 1: right, 2: jump
                switch (randomMove) {
                    case 0:
                        worm.move(-2.0f);
                        break;
                    case 1:
                        worm.move(2.0f);
                        break;
                    case 2:
                        worm.jump();
                        break;
                }
            }
        }

        // Update projectiles
        for (auto it = projectiles.begin(); it != projectiles.end();) {
            auto& projectile = *it;

            // Apply gravity and wind
            projectile.vy += GRAVITY;
            projectile.vx += WIND;

            // Move projectile
            projectile.x += projectile.vx;
            projectile.y += projectile.vy;
            projectile.updateRect();

            // Check if projectile is out of bounds
            if (projectile.x < 0 || projectile.x > SCREEN_WIDTH ||
                projectile.y < 0 || projectile.y > SCREEN_HEIGHT) {
                it = projectiles.erase(it);
                continue;
            }

            // Check collision with terrain
            if (terrain.checkCollision(projectile.rect)) {
                // Create explosion at projectile position
                explosions.emplace_back(
                    projectile.x,
                    projectile.y,
                    EXPLOSION_MAX_SIZE / 2
                );

                // Destroy terrain
                terrain.destroy(
                    static_cast<int>(projectile.x),
                    static_cast<int>(projectile.y),
                    EXPLOSION_MAX_SIZE / 2
                );

                // Check if explosion hit any worms
                for (auto& worm : worms) {
                    float dx = worm.x + WORM_SIZE / 2 - projectile.x;
                    float dy = worm.y + WORM_SIZE / 2 - projectile.y;
                    float distance = sqrt(dx * dx + dy * dy);

                    if (distance < EXPLOSION_MAX_SIZE) {
                        // Damage falls off with distance
                        float damage = 30 * (1.0f - distance / EXPLOSION_MAX_SIZE);
                        worm.health -= static_cast<int>(damage);

                        // Apply knockback
                        float knockback = 5.0f * (1.0f - distance / EXPLOSION_MAX_SIZE);
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
        for (const auto& worm : worms) {
            // Render worm body
            if (worm.isActive) {
                SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Active worm is red
            } else {
                SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); // Inactive worms are green
            }
            SDL_RenderFillRect(renderer, &worm.rect);

            // Render health bar
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

            // Render aim line for active worm
            if (worm.isActive) {
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                float lineLength = 30.0f;
                float endX = worm.x + WORM_SIZE / 2 + cos(worm.aimAngle) * lineLength;
                float endY = worm.y + WORM_SIZE / 2 + sin(worm.aimAngle) * lineLength;
                SDL_RenderLine(renderer,
                                worm.x + WORM_SIZE / 2,
                                worm.y + WORM_SIZE / 2,
                                endX, endY);
            }
        }

        // Render projectiles
        SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255); // Yellow
        for (const auto& projectile : projectiles) {
            SDL_RenderFillRect(renderer, &projectile.rect);
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

        // Render UI text (would use SDL_ttf in a real implementation)
        // For this demo, we'll just use colored rectangles to indicate turn info
        SDL_FRect turnIndicator = {10, 10, 100, 20};
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderFillRect(renderer, &turnIndicator);

        // Present rendered content
        SDL_RenderPresent(renderer);

        // Cap frame rate
        SDL_Delay(16); // ~60 FPS
    }

    // Clean up resources
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}