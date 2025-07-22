#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_DELAY 16 // 60 FPS
#define SCREEN_WIDTH 1920 
#define SCREEN_HEIGHT 1080
#define PONG_WIDTH 20
#define PONG_HEIGHT 140
#define MID_LINE_WIDTH 20
#define BALL_WIDTH 35
#define BALL_HEIGHT 35
#define SEGMENT_THICKNESS 20
#define DIGIT_WIDTH 110
#define DIGIT_HEIGHT 160

typedef struct Box
{
	uint32_t x, y;
	uint32_t width, height;
	uint32_t color;
} box;

typedef struct game_state
{
	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_Texture* texture;
	uint32_t ball_dx = -1, ball_dy = 1, left_score, right_score;
	uint32_t* pixels;
	bool hit_left, hit_right;
	box left_pong, right_pong, mid_line, ball;
	Mix_Music* bounced;
	Mix_Music* point_scored;
	Mix_Music* won;
} game_state;

static game_state state;

void ClearScreen()
{
	for (uint32_t i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; ++i)
	{
		state.pixels[i] = 0xff000000;
	}
}

void DrawPixel(uint32_t x, uint32_t y, uint32_t color)
{
	uint32_t x0, y0;
	x0 = (uint32_t)x;
	y0 = (uint32_t)y;
	
	if (x0 >= 0 && x0 <= SCREEN_WIDTH && y0 >= 0 && y0 <= SCREEN_HEIGHT)
	{
		state.pixels[y0 * SCREEN_WIDTH + x0] = color;
	}
}

void DrawBox(const Box& box)
{
	for (uint32_t y = box.y; y < box.height + box.y; ++y)
	{
		for (uint32_t x = box.x; x < box.width + box.x; ++x)
		{
			DrawPixel(x, y, box.color);
		}
	}
}

void MoveBox(Box& box, uint32_t dx, uint32_t dy)
{
	if ((box.x + dx) >= 0 && (box.x + dx) <= (SCREEN_WIDTH - box.width)) box.x += dx;
	if ((box.y + dy) >= 0 && (box.y + dy) <= (SCREEN_HEIGHT - box.height)) box.y += dy;
}

void MoveBall()
{	
	for (int i = 0; i < 5; ++i)
	{
		MoveBox(state.ball, state.ball_dx, state.ball_dy);
		if (state.ball.y >= state.left_pong.y - (state.ball.height / 2) && state.ball.y <= state.left_pong.y + state.left_pong.height
			&& state.ball.x >= state.left_pong.x - (state.ball.width / 2) && state.ball.x <= state.left_pong.x + state.left_pong.width)
		{
			state.ball_dx = 1;
			if (state.left_pong.y >= SCREEN_HEIGHT / 2)
				state.ball_dy = -1;
			else
				state.ball_dy = 1;

			state.hit_left = true;
			state.hit_right = false;
			Mix_PlayMusic(state.bounced, 0);
		}

		if (state.ball.y >= state.right_pong.y - (state.ball.height / 2) && state.ball.y <= state.right_pong.y + state.right_pong.height
			&& state.ball.x >= state.right_pong.x - (state.ball.width / 2) && state.ball.x <= state.right_pong.x + state.right_pong.width)
		{
			state.ball_dx = -1;
			if (state.right_pong.y >= SCREEN_HEIGHT / 2)
				state.ball_dy = -1;
			else
				state.ball_dy = 1;

			state.hit_left = false;
			state.hit_right = true;
			Mix_PlayMusic(state.bounced, 0);
		}

		if (state.ball.y >= (SCREEN_HEIGHT - BALL_HEIGHT))
		{
			if (state.hit_left)
				state.ball_dx = 1;
			if (state.hit_right)
				state.ball_dx = -1;
			state.ball_dy = -1;
			Mix_PlayMusic(state.bounced, 0);
		}

		if (state.ball.y <= 0)
		{
			if (state.hit_left)
				state.ball_dx = 1;
			if (state.hit_right)
				state.ball_dx = -1;
			state.ball_dy = 1;
			Mix_PlayMusic(state.bounced, 0);
		}

		if (state.ball.x == (SCREEN_WIDTH - BALL_WIDTH))
		{
			++state.left_score;
			state.ball_dx = 1;
			state.ball_dy = 1;
			state.ball.x = (SCREEN_WIDTH / 2) - (BALL_WIDTH / 2);
			state.ball.y = rand() % 720;
			Mix_PlayMusic(state.point_scored, 0);
		}

		if (state.ball.x == 0)
		{
			++state.right_score;
			state.ball_dx = -1;
			state.ball_dy = 1;
			state.ball.x = (SCREEN_WIDTH / 2) - (BALL_WIDTH / 2);
			state.ball.y = rand() % (SCREEN_HEIGHT - BALL_HEIGHT);
			Mix_PlayMusic(state.point_scored, 0);
		}

		if (state.left_score == 11 || state.right_score == 11)
		{
			state.left_score = 0;
			state.right_score = 0;
			state.ball_dx = -1;
			state.ball_dy = 1;
			state.ball.x = (SCREEN_WIDTH / 2) - (BALL_WIDTH / 2);
			state.ball.y = rand() % (SCREEN_HEIGHT - BALL_HEIGHT);
			Mix_PlayMusic(state.won, 0);
			if (Mix_PlayingMusic())
				SDL_Delay((Mix_MusicDuration(state.won) - 2) * 1000.0);
		}
	}
}

void DrawNumber(int number, uint32_t x, uint32_t y, uint32_t color)
{
	Box top = { x, y - 40, 70, SEGMENT_THICKNESS, color };
	Box middle = { x, y + 40, 70, SEGMENT_THICKNESS, color };
	Box bottom = { x, y + 120, 70, SEGMENT_THICKNESS, color };

	Box left_top = { x - 20, y - 20, SEGMENT_THICKNESS, 60, color };
	Box left_bottom = { x - 20, y + 60, SEGMENT_THICKNESS, 60, color };
	Box right_top = { x + 70, y - 20, SEGMENT_THICKNESS, 60, color };
	Box right_bottom = { x + 70, y + 60, SEGMENT_THICKNESS, 60, color };

	switch (number)
	{
	case 0:
		DrawBox(left_top);
		DrawBox(left_bottom);
		DrawBox(top);
		DrawBox(right_top);
		DrawBox(right_bottom);
		DrawBox(bottom);
		break;
	case 1:
		DrawBox(right_top);
		DrawBox(right_bottom);
		break;
	case 2:
		DrawBox(top);
		DrawBox(right_top);
		DrawBox(middle);
		DrawBox(left_bottom);
		DrawBox(bottom);
		break;
	case 3:
		DrawBox(top);
		DrawBox(right_top);
		DrawBox(middle);
		DrawBox(right_bottom);
		DrawBox(bottom);
		break;
	case 4:
		DrawBox(left_top);
		DrawBox(middle);
		DrawBox(right_top);
		DrawBox(right_bottom);
		break;
	case 5:
		DrawBox(top);
		DrawBox(left_top);
		DrawBox(middle);
		DrawBox(right_bottom);
		DrawBox(bottom);
		break;
	case 6:
		DrawBox(top);
		DrawBox(left_top);
		DrawBox(middle);
		DrawBox(left_bottom);
		DrawBox(right_bottom);
		DrawBox(bottom);
		break;
	case 7:
		DrawBox(top);
		DrawBox(right_top);
		DrawBox(right_bottom);
		break;
	case 8:
		DrawBox(top);
		DrawBox(middle);
		DrawBox(bottom);
		DrawBox(left_top);
		DrawBox(left_bottom);
		DrawBox(right_top);
		DrawBox(right_bottom);
		break;
	case 9:
		DrawBox(top);
		DrawBox(left_top);
		DrawBox(right_top);
		DrawBox(middle);
		DrawBox(right_bottom);
		DrawBox(bottom);
		break;
	case 10:
		DrawNumber(1, x - 120, y, color);
		DrawNumber(0, x + 20, y, color);
		break;
	}
}

int main()
{
	srand((unsigned int)time(NULL));
	SDL_Init(SDL_INIT_VIDEO);

	if (Mix_Init(MIX_INIT_MP3 | MIX_INIT_WAVPACK) == 0) 
	{
		printf("Failed to initialise SDL_Mixer");
		return 1;
	}

	if (Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096) < 0)
	{
		printf("Failed to open audio: SDL_Mixer");
		return 1;
	}

	state.bounced = Mix_LoadMUS("bounced.wav");
	state.point_scored = Mix_LoadMUS("point_scored.mp3");
	state.won = Mix_LoadMUS("won.mp3");

	state.window = SDL_CreateWindow("Title", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
	state.renderer = SDL_CreateRenderer(state.window, -1, 0);
	state.texture = SDL_CreateTexture(state.renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);

	state.right_pong.width = PONG_WIDTH;
	state.right_pong.height = PONG_HEIGHT;
	state.right_pong.x = SCREEN_WIDTH - (PONG_WIDTH * 5);
	state.right_pong.y = (SCREEN_HEIGHT / 2) - (PONG_HEIGHT / 2);
	state.right_pong.color = 0xffffffff;

	state.left_pong.width = PONG_WIDTH;
	state.left_pong.height = PONG_HEIGHT;
	state.left_pong.x = PONG_WIDTH * 5;
	state.left_pong.y = (SCREEN_HEIGHT / 2) - (PONG_HEIGHT / 2);
	state.left_pong.color = 0xffffffff;

	state.mid_line.width = MID_LINE_WIDTH;
	state.mid_line.height = SCREEN_HEIGHT;
	state.mid_line.x = (SCREEN_WIDTH / 2) - (MID_LINE_WIDTH / 2);
	state.mid_line.y = 0;
	state.mid_line.color = 0xffffffff;

	state.ball.width = BALL_WIDTH;
	state.ball.height = BALL_HEIGHT;
	state.ball.x = (SCREEN_WIDTH / 2) - (BALL_WIDTH / 2);
	state.ball.y = (SCREEN_HEIGHT / 2) - (BALL_HEIGHT / 2);
	state.ball.color = 0xff828282;

	state.pixels = (uint32_t*)malloc(SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(uint32_t));
	SDL_memset(state.pixels, 0, SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(uint32_t));

	bool running = true;
	SDL_Event event;
	while (running)
	{
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT)
				running = false;
		}

		MoveBall();

		const uint8_t* keystate = SDL_GetKeyboardState(NULL);
		if (keystate[SDL_SCANCODE_W])
			MoveBox(state.left_pong, 0, -8);
		if (keystate[SDL_SCANCODE_S])
			MoveBox(state.left_pong, 0, 8);
		if (keystate[SDL_SCANCODE_UP])
			MoveBox(state.right_pong, 0, -8);
		if (keystate[SDL_SCANCODE_DOWN])
			MoveBox(state.right_pong, 0, 8);

		ClearScreen();
		DrawNumber(state.left_score, 50, 50, 0xffff0000);
		DrawNumber(state.right_score, SCREEN_WIDTH - 50 - DIGIT_WIDTH, 50, 0xffff0000);
		DrawBox(state.left_pong);
		DrawBox(state.mid_line);
		DrawBox(state.ball);
		DrawBox(state.right_pong);

		SDL_UpdateTexture(state.texture, NULL, state.pixels, SCREEN_WIDTH * 4);
		SDL_RenderClear(state.renderer);
		SDL_RenderCopy(state.renderer, state.texture, NULL, NULL);
		SDL_RenderPresent(state.renderer);
	}

	free(state.pixels);
	SDL_DestroyTexture(state.texture);
	SDL_DestroyRenderer(state.renderer);
	SDL_DestroyWindow(state.window);
	SDL_Quit();

	return 0;
}