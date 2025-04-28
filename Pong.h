#pragma once
#include <SDL3/SDL.h>
#include <box2d/box2d.h>

class Pong
{
public:
	Pong();
	~Pong();

	void run();
private:
	static constexpr int FPS = 60;
	static constexpr float BOX_SCALE = 10;
	static constexpr float TEX_SCALE = 0.5f;
	static constexpr SDL_FRect BALL_TEX = {404, 580, 76, 76};

	SDL_Texture* tex;
	SDL_Renderer* ren;
	SDL_Window* win;

	b2WorldId world;
	b2BodyId ballBody;
};

