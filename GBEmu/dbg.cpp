#define DBG_FONT "fonts/Consolas.ttf"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

SDL_Window* dbg_init() {
	union rgba8_t {
		struct {
			u8 r;
			u8 g;
			u8 b;
			u8 a;
		};
		u8 ch[4];
		u32 packed;
	};
	const u8 num_glyphs_x = 16;
	const u8 num_glyphs_y = 16;
	const u8 glyph_width = 32;
	const u8 glyph_height = 32;
	const u8 bpp = 4;
	u8 cache[glyph_height][glyph_width];
	SDL_Window* dbg = SDL_CreateWindow("GBEmu DBG", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1280, 720, SDL_WINDOW_SHOWN);
	SDL_Renderer* dbg_renderer = SDL_CreateRenderer(dbg, -1, SDL_RENDERER_ACCELERATED);
	SDL_Texture* font_tex = SDL_CreateTexture(dbg_renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, num_glyphs_x * glyph_width, num_glyphs_y * glyph_height);
	SDL_SetTextureColorMod(font_tex, 0xff, 0xff, 0xff);
	SDL_SetTextureBlendMode(font_tex, SDL_BLENDMODE_NONE);

	void* tex_buffer;
	int pitch;
	u8* data = load_file(DBG_FONT);
	stbtt_fontinfo info = {};
	if (data) {
		stbtt_InitFont(&info, data, 0);
		float scale = stbtt_ScaleForPixelHeight(&info, glyph_height);
		SDL_LockTexture(font_tex, 0, &tex_buffer, &pitch);
		for (u8 y = 0; y < num_glyphs_y; y++) {
			for (u8 x = 0; x < num_glyphs_x; x++) {
				u8* glyph = (u8*)tex_buffer + y * glyph_height * pitch + x * glyph_width * bpp;
				stbtt_MakeCodepointBitmap(&info, cache[0], glyph_width, glyph_height, glyph_width, scale, scale, y * num_glyphs_x + x);
				for (u8 pixel_y = 0; pixel_y < glyph_height; pixel_y++) {
					for (u8 pixel_x = 0; pixel_x < glyph_width; pixel_x++) {
						rgba8_t* pixel = (rgba8_t*)(glyph + pixel_y * pitch + pixel_x * bpp);
						pixel->r = cache[pixel_y][pixel_x];
						pixel->g = cache[pixel_y][pixel_x];
						pixel->b = cache[pixel_y][pixel_x];
						pixel->a = cache[pixel_y][pixel_x];
					}
				}
			}
		}
		SDL_UnlockTexture(font_tex);
	}

	SDL_RenderClear(renderer);
	SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);

	int advance, left_side_bearing;
	int x0, x1, y0, y1;
	stbtt_GetFontBoundingBox(&info, &x0, &y0, &x1, &y1);
	float scale = stbtt_ScaleForPixelHeight(&info, glyph_height);
	const char* hello = "Hello World!";
	float x = 0, y = -scale * y0;
	for (; *hello; hello++) {
		int ix0, ix1, iy0, iy1;
		int gx, gy;
		gx = *hello & 0x0F;
		gy = (*hello & 0xF0) >> 4;
		stbtt_GetGlyphBitmapBox(&info, *hello, scale, scale, &ix0, &iy0, &ix1, &iy1);
		stbtt_GetGlyphHMetrics(&info, *hello, &advance, &left_side_bearing);
		SDL_Rect src = { gx * glyph_width, gy * glyph_height, glyph_width, glyph_height };
		SDL_Rect dst = { (int)(x + scale * ix0), (int)(scale * iy0), glyph_width, glyph_height };
		SDL_RenderCopy(dbg_renderer, font_tex, &src, &dst);
		x += scale * advance;
	}
	SDL_RenderPresent(dbg_renderer);
	free(data);

	return dbg;
}