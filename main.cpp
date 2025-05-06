#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <iostream>
#include <vector>
using namespace std;
#define GROUND_R 140
#define GROUND_G 70
#define GROUND_B 0
#define GROUND_A 255

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const float GRAVITY = 0.2f;
const int TERRAIN_SIZE = 10;
const int WORM_SIZE = 30;
const int TURN_DURATION = 200;
const int LEFT_MOVE_LENGTH = -10.0f;
const int RIGHT_MOVE_LENGTH = 10.0f;
const int FLOOR_HEIGHT = 500;


struct Worm {
    float x, y;
    float vx = 0, vy = 0;
    SDL_FRect rect;
    int health = 100;

    Worm(float posX, float posY): x(posX), y(posY), rect{posX, posY, WORM_SIZE, WORM_SIZE} {}

    void move(float dx)
    {
        x += dx;
        //if out of boundery, return worm to screen
        if (x < 0)
        {
            x = 0;
        }
        if (x > SCREEN_WIDTH - WORM_SIZE)
        {
            x = SCREEN_WIDTH - WORM_SIZE;
        }
        updateRect();

    }
        void updateRect() {
            rect.x = x;
            rect.y = y;
        }


    void jump() {
        if (vy == 0) { //can only jump if worm on ground
            vy = -6.0f;
        }
    }
};

struct Terrain {
    std::vector<std::vector<bool>> blocks; //if there is floor in (x,y) pixel

    Terrain(int width, int height) {
        //make the size of blocks relative to pixels
        blocks.resize(width);
        for (auto& column : blocks) {
            column.resize(height, false);
        }
        //create surface, in the future can make a more complicated terrain
        for (int x = 0; x < blocks.size(); x++) {
            for (int y = FLOOR_HEIGHT; y < blocks[0].size(); y++) {
                blocks[x][y] = true;
            }
        }
    }

    bool checkCollision(const SDL_FRect& rect) { //will need to improve in the future to check for collution not only in y axis
        return rect.y + rect.h > FLOOR_HEIGHT;
    }

    void render(SDL_Renderer* renderer) { //will need to improve in the future to take account for where there are blocks, i tried but it ran too slow, maybe define bigger blocks to draw chanks at a time
        SDL_SetRenderDrawColor(renderer, GROUND_R, GROUND_G, GROUND_B, GROUND_A);
        //draw basic surface
        SDL_FRect terrainRect = {0, static_cast<float>(FLOOR_HEIGHT), static_cast<float>(SCREEN_WIDTH), static_cast<float>(SCREEN_HEIGHT - FLOOR_HEIGHT)};
        SDL_RenderFillRect(renderer, &terrainRect);
    }

};

int main(int argc, char* argv[]) {
    Terrain terrain(SCREEN_WIDTH, SCREEN_HEIGHT);
    std::vector<Worm> worms;
    int currentWorm = 0;  //current worm turn
    int turnTimer = 0;    //track how much time left for current turn
    worms.emplace_back(100, FLOOR_HEIGHT - WORM_SIZE);
    worms.emplace_back(300, FLOOR_HEIGHT - WORM_SIZE);
    worms.emplace_back(500, FLOOR_HEIGHT - WORM_SIZE);
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;

    if (!SDL_Init(SDL_INIT_VIDEO)) {
		cout << SDL_GetError() << endl;
        return -1;
    }
    if (!SDL_CreateWindowAndRenderer("Worms", SCREEN_WIDTH, SCREEN_HEIGHT, 0, &window, &renderer)) {
        cout << SDL_GetError() << endl;
        return -1;
    }

    while (true) {
        //timer for turn increase
        turnTimer++;
        Worm& activeWorm = worms[currentWorm];
        //for simulation, randomally make worm do one of three moves, move right, move left or jump
        if (turnTimer % (TURN_DURATION/10) == 0) {
            int action = rand() % 3;
            if (action == 0) {
                activeWorm.move(LEFT_MOVE_LENGTH);
            } else if (action == 1) {
                activeWorm.move(RIGHT_MOVE_LENGTH);
            } else {
                activeWorm.jump();
            }
        }
        //switch to next worm if turn duration passed
        if (turnTimer >= TURN_DURATION) {
            currentWorm = (currentWorm + 1) % worms.size();
            turnTimer = 0;
        }
        //apply physics
        for (auto& worm : worms) {
            worm.vy += GRAVITY;
            //move worm in y axis
            worm.y += worm.vy;
            worm.updateRect();
            //check for collision with terrain
            if (terrain.checkCollision(worm.rect)) {
                //if below the ground, move to be on top of platform
                worm.y = FLOOR_HEIGHT - WORM_SIZE;
                worm.updateRect();
                worm.vy = 0;
            }
        }
        //clear screen
        SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255); //blue sky
        SDL_RenderClear(renderer);
        terrain.render(renderer);
        for (int i = 0; i < worms.size(); i++) {
            const auto& worm = worms[i];
            if (i == currentWorm) {  //red for worm that it his turn, green for other worms
                SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            } else {
                SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
            }
            SDL_RenderFillRect(renderer, &worm.rect);
        }
        SDL_RenderPresent(renderer);
        SDL_Delay(10);
    }
    return 0;
}