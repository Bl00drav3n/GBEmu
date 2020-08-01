#ifndef __SDLGB_H__
#define __SDLGB_H__

#ifdef SDLGB_EXPORT_DLL
#define SDLGB_API __declspec(dllexport)
#elif SDLGB_IMPORT_DLL
#define SDLGB_API __declspec(dllimport)
#else
#define SDLGB_API
#endif

#undef main

typedef enum {
	BUTTON_LEFT,
	BUTTON_RIGHT,
	BUTTON_UP,
	BUTTON_DOWN,
	BUTTON_A,
	BUTTON_B,
	BUTTON_START,
	BUTTON_SELECT,

	NUM_BUTTONS
} sdlgb_button_t;

typedef enum {
	SDLGB_SCALE_1X = 1,
	SDLGB_SCALE_2X,
	SDLGB_SCALE_3X,
	SDLGB_SCALE_4X
} sdlgb_scale_t;

typedef struct
{
	int state[NUM_BUTTONS];
} sdlgb_input_t;

extern SDLGB_API void sdlgb_init(const char *title, sdlgb_scale_t scale);
extern SDLGB_API void sdlgb_quit();

extern SDLGB_API void sdlgb_render_present();
extern SDLGB_API void sdlgb_sync();

extern SDLGB_API sdlgb_input_t sdlgb_get_input_for_frame(int * running);

#endif