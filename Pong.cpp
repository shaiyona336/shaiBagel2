#include "Pong.h"
#include <iostream>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <box2d/box2d.h>
using namespace std;

Pong::Pong()
{
	if (!SDL_Init(SDL_INIT_VIDEO)) {
		cout << SDL_GetError() << endl;
		return;
	}

	if (!SDL_CreateWindowAndRenderer(
		"Pong", 800,600, 0, &win, &ren)) {
		cout << SDL_GetError() << endl;
		return;
		}
	SDL_Surface *surf = IMG_Load("res/pong.png");
	if (surf == nullptr) {
		cout << SDL_GetError() << endl;
		return;
	}

	tex = SDL_CreateTextureFromSurface(ren, surf);
	if (tex == nullptr) {
		cout << SDL_GetError() << endl;
		return;
	}
	SDL_DestroySurface(surf);

	b2WorldDef worldDef = b2DefaultWorldDef();
	worldDef.gravity = {0,0};
	world = b2CreateWorld(&worldDef);

	b2BodyDef bodyDef = b2DefaultBodyDef();
	bodyDef.type = b2_dynamicBody;
	bodyDef.position = {400/BOX_SCALE,300/BOX_SCALE};
	ballBody = b2CreateBody(world, &bodyDef);

	b2ShapeDef shapeDef = b2DefaultShapeDef();
	shapeDef.density = 1;
	shapeDef.material.friction = 0.f;
	shapeDef.material.restitution = 2.5f;
	b2Circle circle = {0,0,BALL_TEX.w*TEX_SCALE/BOX_SCALE};
	b2CreateCircleShape(ballBody, &shapeDef, &circle);

	b2Body_SetLinearVelocity(ballBody, {2,-5});
	b2Body_SetAngularVelocity(ballBody, 1.1f);

	bodyDef.type = b2_staticBody;
	bodyDef.position = {800/2/BOX_SCALE,-5.5f};
	b2BodyId wall = b2CreateBody(world, &bodyDef);

	b2Polygon box = b2MakeBox(800/2/BOX_SCALE,1);
	b2CreatePolygonShape(wall, &shapeDef, &box);
}

Pong::~Pong()
{
	if (tex != nullptr)
		SDL_DestroyTexture(tex);
	if (ren != nullptr)
		SDL_DestroyRenderer(ren);
	if (win != nullptr)
		SDL_DestroyWindow(win);

	SDL_Quit();
}

void Pong::run()
{
	SDL_SetRenderDrawColor(ren, 0,0,0,255);
	SDL_FRect r{0,0,
		BALL_TEX.w*TEX_SCALE,
		BALL_TEX.h*TEX_SCALE};

	constexpr float STEP = 1.f/FPS;
	constexpr float RAD_TO_DEG = 57.2958f;

	for (int i = 0; i < 1000; ++i) {
		b2World_Step(world, STEP, 4);

		b2Vec2 p = b2Body_GetPosition(ballBody);
		r.x = p.x*BOX_SCALE;
		r.y = p.y*BOX_SCALE;

		b2Rot rot = b2Body_GetRotation(ballBody);
		float a = RAD_TO_DEG * b2Rot_GetAngle(rot);

		SDL_RenderClear(ren);
		SDL_RenderTextureRotated(
			ren, tex, &BALL_TEX, &r, a,
			nullptr, SDL_FLIP_NONE);
		SDL_RenderPresent(ren);

		SDL_Delay(5);
	}
}
