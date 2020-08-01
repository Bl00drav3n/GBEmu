#define WINDOW_TITLE "TinyGB"
#define AUDIO_SAMPLE_RATE 44100
#define _DEBUG_MEASURE_TIMINGS 0

#define _CRT_SECURE_NO_WARNINGS
#include "sdlgb.h"
#include "gbv.h"
#include "gbemu.h"
#include "utility.h"
#include "lr35902.h"
#include "sound.h"
#include "cartridge.h"
#include "bus.h"
#include "device.h"

#define _ENABLE_DUMP 0

#include "sdlgb.cpp"
#include "gbv.cpp"
#include "utility.cpp"
#include "sound.cpp"
#include "lr35902.cpp"
#include "video.cpp"
#include "cartridge.cpp"
#include "bus.cpp"
#include "device.cpp"
#include "dbg.cpp"

#define ROM_FILENAME "sml.gb"
//#define ROM_FILENAME "tests/cpu_instrs.gb"
//#define ROM_FILENAME "tests/individual/01-special.gb"
//#define ROM_FILENAME "tests/individual/02-interrupts.gb"
//#define ROM_FILENAME "tests/individual/03-op sp,hl.gb"
//#define ROM_FILENAME "tests/individual/04-op r,imm.gb"
//#define ROM_FILENAME "tests/individual/05-op rp.gb"
//#define ROM_FILENAME "tests/individual/06-ld r,r.gb"
//#define ROM_FILENAME "tests/individual/07-jr,jp,call,ret,rst.gb"
//#define ROM_FILENAME "tests/individual/08-misc instrs.gb"
//#define ROM_FILENAME "tests/individual/09-op r,r.gb"
//#define ROM_FILENAME "tests/individual/10-bit ops.gb"
//#define ROM_FILENAME "tests/individual/11-op a,(hl).gb"
// PASSED: 4, 5, 6, 9, 10

void sdlgb_process_events(int* running, SDL_Window *main, SDL_Window *dbg) {
	SDL_Event evt;
	while (SDL_PollEvent(&evt)) {
		switch (evt.type) {
		case SDL_QUIT:
			*running = false;
			break;
		case SDL_WINDOWEVENT:
			if (evt.window.event == SDL_WINDOWEVENT_CLOSE) {
				if (evt.window.windowID == SDL_GetWindowID(main)) {
					*running = false;
				}
				else {
					SDL_DestroyWindow(dbg);
				}
			}
			break;
		case SDL_CONTROLLERDEVICEADDED:
			if (!controller) {
				controller = SDL_GameControllerOpen(evt.cdevice.which);
				fprintf(stdout, "Game controller added: %s\n", SDL_GameControllerName(controller));
			}
			break;
		case SDL_CONTROLLERDEVICEREMOVED:
			if (!SDL_GameControllerGetAttached(controller)) {
				fprintf(stdout, "Game controller removed: %s\n", SDL_GameControllerName(controller));
				SDL_GameControllerClose(controller);
				controller = 0;
			}
			break;
		case SDL_KEYDOWN:
			switch (evt.key.keysym.sym) {
			case SDLK_F5:
				device_toggle_channel_enable(AUDIO_SQUARE1);
				break;
			case SDLK_F6:
				device_toggle_channel_enable(AUDIO_SQUARE2);
				break;
			case SDLK_F7:
				device_toggle_channel_enable(AUDIO_WAVE);
				break;
			case SDLK_F8:
				device_toggle_channel_enable(AUDIO_NOISE);
				break;
			case SDLK_KP_PLUS:
				device_increase_volume();
				break;
			case SDLK_KP_MINUS:
				device_decrease_volume();
				break;
			case SDLK_KP_MULTIPLY:
				device_toggle_mute();
				break;
			} break;
		}
	}
}

double get_frame_time_us(LARGE_INTEGER *timer_last)
{
	LARGE_INTEGER cur;
	double us = 0;
	double elapsed_us = 0;
	QueryPerformanceCounter(&cur);
	LONGLONG cycles = cur.QuadPart - timer_last->QuadPart;
	elapsed_us = (double)(1000000LL * cycles) / (double)freq.QuadPart;
	*timer_last = cur;
	return elapsed_us;
}

int main(int argc, char* argv[]) {
	sdlgb_init("GBEmu", SDLGB_SCALE_4X);

	const char* rom_name = 0;
	debug_out = fopen("dump.txt", "w");
	if (argc == 1) {
		fprintf(stdout, "No parameters passed, trying to load %s\n", ROM_FILENAME);
		rom_name = ROM_FILENAME;
	}
	else if (argc == 2) {
		rom_name = argv[1];
	}
	else {
		fprintf(stdout, "Usage: GBEmu <rom_path>\n");
		return 1;
	}

	if (!device_load_cartdrige(rom_name)) return 1;
	device_init();
	device.render_buffer = sdlgb_get_video_mem();
	device.palette = { 0xFF, 0xAA, 0x55, 0x00 };

	gbv_io_mapped_registers_t reg = {};
	reg.stat = device.io + IO_STAT;
	reg.lcdc = device.io + IO_LCDC;
	reg.bgp = device.io + IO_BGP;
	reg.obp0 = device.io + IO_OBP0;
	reg.obp1 = device.io + IO_OBP1;
	reg.scx = device.io + IO_SCX;
	reg.scy = device.io + IO_SCY;
	reg.lyc = device.io + IO_LYC;
	reg.wx = device.io + IO_WX;
	reg.wy = device.io + IO_WY;
	reg.ly = device.io + IO_LY;
	gbv_init(device.vram, device.oam, reg);

	//SDL_Window *dbg_window = dbg_init();

	// NOTE: Device is synchronized to realtime via audio thread's sample rate
	SDL_AudioSpec audio_spec_req, audio_spec;
	SDL_memset(&audio_spec_req, 0, sizeof(audio_spec_req));
	audio_spec_req.freq = AUDIO_SAMPLE_RATE;
	audio_spec_req.channels = 2;
	audio_spec_req.format = AUDIO_F32;
	audio_spec_req.samples = 512;
	audio_spec_req.callback = device_audio_callback;
	audio_spec_req.userdata = &audio_spec;
	SDL_AudioDeviceID dev = SDL_OpenAudioDevice(0, 0, &audio_spec_req, &audio_spec, 0);
	SDL_PauseAudioDevice(dev, 0);

	const int FRAMES = 64;
	int agg_idx = 0;
	double frame_times[FRAMES] = {};

	int running = true;
	LARGE_INTEGER timer_last;
	QueryPerformanceCounter(&timer_last);
	while (running) {
		// NOTE: sync video (to monitor refresh rate)
		if (device.vblank) {
			/* transfer data to framebuffer */
			void* video_mem_rgba;
			int video_mem_pitch;

			sdlgb_process_events(&running, window, 0/*dbg_window*/);
			device.input = sdlgb_get_button_state();

			// TODO: ensure that we don't write to this buffer during the copy
			SDL_LockTexture(framebuffer, 0, &video_mem_rgba, &video_mem_pitch);
			for (int y = 0; y < GBV_SCREEN_HEIGHT; y++) {
				for (int x = 0; x < GBV_SCREEN_WIDTH; x++) {
					unsigned char* src_px = video_mem + GBV_SCREEN_WIDTH * y + x;
					unsigned char* dst_px = (unsigned char*)video_mem_rgba + y * video_mem_pitch + 4 * x;
					dst_px[0] = 0xFF;
					dst_px[1] = src_px[0];
					dst_px[2] = src_px[0];
					dst_px[3] = src_px[0];
				}
			}
			SDL_UnlockTexture(framebuffer);

			/* transfer framebuffer to screen */
			SDL_Rect dest = { 0, 0, screen_width, screen_height };
			SDL_RenderCopy(renderer, framebuffer, 0, &dest);
			// NOTE: hope that vsync solves our problems
			SDL_RenderPresent(renderer);
			device.vblank = false;

			_mm_mfence();

			double frame_time_avg = 0.0;
			{
				frame_times[agg_idx++] = get_frame_time_us(&timer_last) / 1e3;
				for (int i = 0; i < FRAMES; i++) {
					frame_time_avg += frame_times[i];
				}
				frame_time_avg /= (double)FRAMES;
				agg_idx = agg_idx % FRAMES;
			}

			char buffer[256];
			const char* on = "ON ";
			const char* off = "OFF";
			snprintf(buffer, sizeof(buffer), "%s - CH1:%s CH2:%s CH3:%s CH4:%s VOLUME:%3.0f%% %.2fms/frame (%.2ffps)",
				WINDOW_TITLE,
				device.channel[AUDIO_SQUARE1].enable_override ? on : off,
				device.channel[AUDIO_SQUARE2].enable_override ? on : off,
				device.channel[AUDIO_WAVE].enable_override ? on : off,
				device.channel[AUDIO_NOISE].enable_override ? on : off,
				device.muted ? 0.f : device.volume * 100.0,
				frame_time_avg,
				1000.0 / frame_time_avg
			);
			SDL_SetWindowTitle(window, buffer);
		}

	}

	return 0;
}