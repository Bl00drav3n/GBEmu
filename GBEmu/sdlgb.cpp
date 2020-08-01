#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <SDL2/SDL.h>
#include <stdio.h>

#if 0
#define WINDOW_SCALE  3
#define screen_width  (WINDOW_SCALE * GBV_SCREEN_WIDTH)
#define screen_height (WINDOW_SCALE * GBV_SCREEN_HEIGHT)
#endif

#define TARGET_FRAME_TIME_US 16742LL

/* module globals */
static LARGE_INTEGER freq, last;
static HANDLE timer;
static SDL_Window   * window;
static SDL_Renderer * renderer;
static SDL_Texture  * framebuffer;
static SDL_GameController * controller;
static unsigned char video_mem[GBV_SCREEN_SIZE];
static int screen_width;
static int screen_height;

unsigned char * sdlgb_get_video_mem() {
	return video_mem;
}

/* Windows sleep in 100ns units */
static void microsleep(HANDLE timer, LONGLONG us) {
	LARGE_INTEGER li;
	li.QuadPart = -10LL * us;
	SetWaitableTimer(timer, &li, 0, NULL, NULL, FALSE);
	WaitForSingleObject(timer, INFINITE);
}

static void sdl_graceful_exit(const char *fmt) {
	fprintf(stderr, fmt, SDL_GetError());
	SDL_Quit();
	exit(1);
}

void sdlgb_init(const char *title, sdlgb_scale_t scale) {
	screen_width = scale * GBV_SCREEN_WIDTH;
	screen_height = scale * GBV_SCREEN_HEIGHT;

	/* platform setup */
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER) != 0) {
		sdl_graceful_exit("Error initializing SDL video: %s\n");
	}
	window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screen_width, screen_height, SDL_WINDOW_SHOWN);
	if (!window) {
		sdl_graceful_exit("Error creating window: %s\n");
	}
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (!renderer) {
		sdl_graceful_exit("Error creating renderer: %s\n");
	}
	framebuffer = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, GBV_SCREEN_WIDTH, GBV_SCREEN_HEIGHT);
	if (!framebuffer) {
		sdl_graceful_exit("Error creating framebuffer: %s\n");
	}

	SDL_version sdl_ver;
	SDL_GetVersion(&sdl_ver);
	fprintf(stdout, "Initialized SDL version %d.%d.%d\n", sdl_ver.major, sdl_ver.minor, sdl_ver.patch);
	fprintf(stdout, "  platform:         %s\n", SDL_GetPlatform());
	fprintf(stdout, "  video driver:     %s\n", SDL_GetCurrentVideoDriver());
	fprintf(stdout, "  framebuffer size: %dx%d\n", GBV_SCREEN_WIDTH, GBV_SCREEN_HEIGHT);
	fprintf(stdout, "  display size:     %dx%d\n", screen_width, screen_height);
	fprintf(stdout, "  scale:            %dx\n", scale);

	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&last);
	timer = CreateWaitableTimerA(NULL, TRUE, NULL);
}

sdlgb_input_t sdlgb_get_button_state() {
	sdlgb_input_t input = {};
	if (controller) {
		input.state[BUTTON_A]      = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_A) | SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_B);
		input.state[BUTTON_B]      = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_X) | SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_Y);
		input.state[BUTTON_SELECT] = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_BACK);
		//input.state[BUTTON_A]    = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_GUIDE);
		input.state[BUTTON_START]  = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_START);
		//input.state[BUTTON_A]    = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_LEFTSTICK);
		//input.state[BUTTON_A]    = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_RIGHTSTICK);
		//input.state[BUTTON_A]    = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_LEFTSHOULDER);
		//input.state[BUTTON_A]    = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);
		input.state[BUTTON_UP]     = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_UP);
		input.state[BUTTON_DOWN]   = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_DOWN);
		input.state[BUTTON_LEFT]   = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_LEFT);
		input.state[BUTTON_RIGHT]  = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
		Sint16 sx = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTX);
		Sint16 sy = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTY);
		float x = -1.f + 2.f * ((sx - SDL_JOYSTICK_AXIS_MIN) / (float)(SDL_JOYSTICK_AXIS_MAX - SDL_JOYSTICK_AXIS_MIN));
		float y = -1.f + 2.f * ((sy - SDL_JOYSTICK_AXIS_MIN) / (float)(SDL_JOYSTICK_AXIS_MAX - SDL_JOYSTICK_AXIS_MIN));
		float r = sqrtf(x * x + y * y);
		float phi = atan2f(y / r, x / r);
		if (r > 0.5f) {
			if (phi < M_PI / 4.f && phi >= -M_PI / 4.f) {
				input.state[BUTTON_RIGHT] = true;
			}
			if (phi < 3.f * M_PI / 4.f && phi >= M_PI / 4.f) {
				input.state[BUTTON_DOWN] = true;
			}
			if (phi >= 3.f * M_PI / 4.f || phi < -3.f * M_PI / 4.f) {
				input.state[BUTTON_LEFT] = true;
			}
			if (phi < -M_PI / 4.f && phi >= -3.f * M_PI / 4.f) {
				input.state[BUTTON_UP] = true;
			}
		}
	}
	const Uint8* keys = SDL_GetKeyboardState(0);
	if (keys[SDL_SCANCODE_S]) {
		input.state[BUTTON_A] = true;
	}
	if (keys[SDL_SCANCODE_A]) {
		input.state[BUTTON_B] = true;
	}
	if (keys[SDL_SCANCODE_RETURN]) {
		input.state[BUTTON_START] = true;
	}
	if (keys[SDL_SCANCODE_SPACE]) {
		input.state[BUTTON_SELECT] = true;
	}
	if (keys[SDL_SCANCODE_UP]) {
		input.state[BUTTON_UP] = true;
	}
	if (keys[SDL_SCANCODE_DOWN]) {
		input.state[BUTTON_DOWN] = true;
	}
	if (keys[SDL_SCANCODE_RIGHT]) {
		input.state[BUTTON_RIGHT] = true;
	}
	if (keys[SDL_SCANCODE_LEFT]) {
		input.state[BUTTON_LEFT] = true;
	}
	return input;
}

sdlgb_input_t sdlgb_get_input_for_frame(int * running) {
	SDL_Event evt;
	sdlgb_input_t input = {};
	while (SDL_PollEvent(&evt)) {
		switch (evt.type) {
		case SDL_QUIT:
			*running = false;
			break;
		case SDL_KEYDOWN:
			switch (evt.key.keysym.sym) {
			case SDLK_a:
				input.state[BUTTON_A] = true;
				break;
			case SDLK_s:
				input.state[BUTTON_B] = true;
				break;
			case SDLK_UP:
				input.state[BUTTON_UP] = true;
				break;
			case SDLK_DOWN:
				input.state[BUTTON_DOWN] = true;
				break;
			case SDLK_LEFT:
				input.state[BUTTON_LEFT] = true;
				break;
			case SDLK_RIGHT:
				input.state[BUTTON_RIGHT] = true;
				break;
			case SDLK_RETURN:
			case SDLK_RETURN2:
				input.state[BUTTON_START] = true;
				break;
			case SDLK_SPACE:
				input.state[BUTTON_SELECT] = true;
				break;
			} break;
		}
	}

	return input;
}

void sdlgb_clear() {
	memset(video_mem, 0xFF, GBV_SCREEN_SIZE);
}

void sdlgb_sync() {
	_ReadWriteBarrier();
	_mm_mfence();

	LARGE_INTEGER cur;
	QueryPerformanceCounter(&cur);
	LONGLONG cycles = cur.QuadPart - last.QuadPart;
	LONGLONG elapsed_us = (1000000LL * cycles) / freq.QuadPart;
	LONGLONG to_sleep = TARGET_FRAME_TIME_US - elapsed_us;
	if (to_sleep > 0LL) {
		microsleep(timer, to_sleep);
	}

	_ReadWriteBarrier();
	_mm_mfence();

	last = cur;
}

void sdlgb_quit() {
	CloseHandle(timer);
	SDL_Quit();
}