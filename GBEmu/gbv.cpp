#define GBV_VERSION_MAJOR 1
#define GBV_VERSION_MINOR 3
#define GBV_VERSION_PATCH 0

#define OBJ_NULL 0xff
#define MAX_OBJECTS_PER_SCANLINE 10

#define GBV_MIN(a, b) ((a) < (b)) ? (a) : (b)
#define GBV_MAX(a, b) ((a) > (b)) ? (a) : (b)
#define GBV_SPRITE_MARGIN_LEFT 8
#define GBV_SPRITE_MARGIN_TOP 16

/* internal data pointers */
static gbv_io_mapped_registers_t gbv_reg;
static gbv_u8 * gbv_tile_data;
static gbv_u8 * gbv_tile_map0;
static gbv_u8 * gbv_tile_map1;
static gbv_obj_char * gbv_oam_data;

/* internal tracking of triggerable interrupts during LCD operation */
static struct lcd_stat_trig {
	gbv_u8 ints[4];
} global_lcd_stat_trig;

/* internal functions */
static gbv_u8 get_color(gbv_u8 idx, gbv_u8 pal) {
	gbv_u8 color = (pal >> (2 * idx)) & 0x03;
	return color;
}

static gbv_u8 get_pal_idx_from_tile_row(gbv_u8 row[2], gbv_u8 x) {
	gbv_u8 pal_idx = ((row[0] >> (7 - x)) & 0x01) | ((row[1] >> (7 - x) & 0x01) << 1);
	return pal_idx;
}

static gbv_u8 get_pixel_from_tile_row(gbv_u8 row[2], gbv_u8 x, gbv_u8 pal) {
	gbv_u8 pal_idx = get_pal_idx_from_tile_row(row, x);
	gbv_u8 color = get_color(pal_idx, pal);
	return color;
}

static void fill_memory(void * buffer, gbv_u16 size, gbv_u8 value) {
	gbv_u16 size8 = size / 8;
	gbv_u16 rem = size % 8;
	unsigned long long * buffer8 = (unsigned long long*)buffer;
	if (size8) {
		unsigned long long value8 = (unsigned long long)value;
		value8 |= (value8 << 56) | (value8 << 48) | (value8 << 40) | (value8 << 32) | (value8 << 24) | (value8 << 16) | (value8 << 8);
		for (gbv_u16 i = 0; i < size8; i++) {
			buffer8[i] = value8;
		}
	}
	if (rem) {
		gbv_u8 * buffer_rem = (gbv_u8*)(buffer8 + size8);
		for (gbv_u8 i = 0; i < rem; i++) {
			buffer_rem[i] = value;
		}
	}
}

gbv_u8 * get_tile_from_tilemap(gbv_u8 x, gbv_u8 y, gbv_lcdc_flag map_select) {
	gbv_u8* tile = 0;
	gbv_u8 *tile_map = (*gbv_reg.lcdc & map_select) ? gbv_tile_map1 : gbv_tile_map0;
	gbv_u8 tile_id = tile_map[GBV_BG_TILES_X * y + x];
	if (*gbv_reg.lcdc & GBV_LCDC_BG_DATA_SELECT) {
		gbv_u8* tile_data = gbv_tile_data;
		tile = tile_data + GBV_TILE_SIZE * tile_id;
	}
	else {
		gbv_u8* tile_data = gbv_tile_data + 0x1000;
		tile = tile_data + GBV_TILE_SIZE * (s8)tile_id;
	}

	return tile;
}

void check_for_lcd_interrupts() {
#if 0
	if (gbv_lcdc_int_callback) {
		gbv_lcd_mode mode = gbv_stat_mode();
		if (global_lcd_stat_trig.ints[mode]) {
			gbv_u8 trigger = 0;
			switch (mode) {
			case GBV_LCD_MODE_HBLANK:
				trigger = gbv_io_stat & GBV_STAT_HBLANK_INT;
				break;
			case GBV_LCD_MODE_VBLANK:
				trigger = gbv_io_stat & GBV_STAT_VBLANK_INT;
				break;
			case GBV_LCD_MODE_OAM:
				trigger = gbv_io_stat & GBV_STAT_OAM_INT;
				break;
			case GBV_LCD_MODE_TRANSFER:
				trigger = (gbv_io_stat & GBV_STAT_LYC_INT) && (gbv_io_stat & GBV_STAT_LYC);
				break;
			}
			if (trigger) {
				global_lcd_stat_trig.ints[mode] = 0;
				gbv_lcdc_int_callback();
			}
		}
	}
#endif
}

void lcd_change_mode(gbv_lcd_mode mode) {
	*gbv_reg.stat = (*gbv_reg.stat & ~GBV_STAT_MODE) | (mode & GBV_STAT_MODE);
	device.vblank = (mode == GBV_LCD_MODE_VBLANK);
}

/* API functions */
void gbv_get_version(int * maj, int * min, int * patch) {
	if (maj) {
		*maj = GBV_VERSION_MAJOR;
	}
	if (min) {
		*min = GBV_VERSION_MINOR;
	}
	if (patch) {
		*patch = GBV_VERSION_PATCH;
	}
}

void gbv_init(gbv_u8 *vram, gbv_u8 *oam, gbv_io_mapped_registers_t reg) {
	gbv_tile_data = vram;
	gbv_tile_map0 = vram + 0x1800;
	gbv_tile_map1 = vram + 0x1C00;
	gbv_oam_data  = (gbv_obj_char*)oam;
	gbv_reg = reg;
}

void gbv_lcdc_set(gbv_lcdc_flag flag) {
	*gbv_reg.lcdc = *gbv_reg.lcdc | flag;
}

void gbv_lcdc_reset(gbv_lcdc_flag flag) {
	*gbv_reg.lcdc = *gbv_reg.lcdc & ~flag;
}

void gbv_stat_set(gbv_stat_flag flag) {
	if (flag > GBV_STAT_LYC) {
		*gbv_reg.stat = *gbv_reg.stat | flag;
	}
}

void gbv_stat_reset(gbv_stat_flag flag) {
	if (flag > GBV_STAT_LYC) {
		*gbv_reg.stat = *gbv_reg.stat & ~flag;
	}
}

gbv_lcd_mode gbv_stat_mode() {
	return (gbv_lcd_mode)(*gbv_reg.stat & GBV_STAT_MODE);
}

gbv_u8 gbv_stat_lyc() {
	return *gbv_reg.stat & GBV_STAT_LYC;
}

gbv_io gbv_ly() {
	return *gbv_reg.ly;
}

gbv_u8 * gbv_get_tile_map0() {
	return gbv_tile_map0;
}

gbv_u8 * gbv_get_tile_map1() {
	return gbv_tile_map1;
}

gbv_u8 * gbv_get_tile_data() {
	return gbv_tile_data;
}

gbv_tile * gbv_get_tile(gbv_u8 tile_id) {
	return (gbv_tile*)gbv_tile_data + tile_id;
}

void gbv_transfer_oam_data(gbv_u8* data) {
	memcpy(gbv_oam_data, data, GBV_OBJ_SIZE);
}

void gbv_mode0() {
	lcd_change_mode(GBV_LCD_MODE_HBLANK);
	if (device.io[IO_STAT] & 0x08) {
		device_request_interrupt(INT_LCDC);
	}
}

void gbv_mode1() {
	lcd_change_mode(GBV_LCD_MODE_VBLANK);
	device_request_interrupt(INT_VBLANK);
	if (device.io[IO_STAT] & 0x10) {
		device_request_interrupt(INT_LCDC);
	}
}

static struct {
	gbv_u8 obj_count;
	gbv_u8 objs[MAX_OBJECTS_PER_SCANLINE];
	gbv_u8* buffer;
	gbv_palette* palette;
} gbv_scanline;

void gbv_mode2() {
	lcd_change_mode(GBV_LCD_MODE_OAM);
	if (device.io[IO_STAT] & 0x20) {
		device_request_interrupt(INT_LCDC);
	}
	gbv_scanline.obj_count = 0;
	gbv_u8 sprite_height = (*gbv_reg.lcdc & GBV_LCDC_OBJ_SIZE_SELECT) ? 16 : 8;
	for (gbv_u8 idx = 0; idx < GBV_OBJ_COUNT; idx++) {
		gbv_obj_char* obj = gbv_oam_data + idx;
		if (obj->y <= *gbv_reg.ly + GBV_SPRITE_MARGIN_TOP && obj->y + sprite_height > *gbv_reg.ly + GBV_SPRITE_MARGIN_TOP) {
			if (gbv_scanline.obj_count < MAX_OBJECTS_PER_SCANLINE) {
				gbv_scanline.objs[gbv_scanline.obj_count++] = idx;
			}
		}
	}
}

void gbv_mode3() {
	lcd_change_mode(GBV_LCD_MODE_TRANSFER);
}

void gbv_render_scanline(gbv_u8 *buffer, gbv_palette *palette) {
	SDL_assert(*gbv_reg.ly < GBV_SCREEN_HEIGHT);
	gbv_scanline.buffer = buffer + *gbv_reg.ly * GBV_SCREEN_WIDTH;
	gbv_scanline.palette = palette;
	*gbv_reg.stat = (*gbv_reg.lcdc == *gbv_reg.ly) ? *gbv_reg.stat | GBV_STAT_LYC : *gbv_reg.stat = (*gbv_reg.stat & ~GBV_STAT_LYC);
	for (gbv_u8 lcd_x = 0; lcd_x < GBV_SCREEN_WIDTH; lcd_x++) {
		gbv_u8 pal = 0x00, color = 0x00;
		if (*gbv_reg.lcdc & GBV_LCDC_WND_ENABLE && lcd_x >= *gbv_reg.wx - 7 && *gbv_reg.ly >= *gbv_reg.wy) {
			gbv_u8 win_x = lcd_x + 7 - *gbv_reg.wx;
			gbv_u8 win_y = *gbv_reg.ly - *gbv_reg.wy;
			gbv_u8 tx = win_x / GBV_TILE_WIDTH;
			gbv_u8 ty = win_y / GBV_TILE_HEIGHT;
			gbv_u8 px = win_x % GBV_TILE_WIDTH;
			gbv_u8 py = win_y % GBV_TILE_HEIGHT;
			gbv_u8* tile = get_tile_from_tilemap(tx, ty, GBV_LCDC_WND_MAP_SELECT);
			gbv_u8* row = tile + GBV_TILE_PITCH * py;
			pal = *gbv_reg.bgp;
			color = get_pal_idx_from_tile_row(row, px);
		}
		else if (*gbv_reg.lcdc & GBV_LCDC_BG_ENABLE) {
			/* map lcd screen position to tilemap position */
			gbv_u8 bg_x = lcd_x + *gbv_reg.scx;
			gbv_u8 bg_y = *gbv_reg.ly + *gbv_reg.scy;
			gbv_u8 tx = bg_x / GBV_TILE_WIDTH;
			gbv_u8 ty = bg_y / GBV_TILE_HEIGHT;
			gbv_u8 px = bg_x % GBV_TILE_WIDTH;
			gbv_u8 py = bg_y % GBV_TILE_HEIGHT;
			gbv_u8* tile = get_tile_from_tilemap(tx, ty, GBV_LCDC_BG_MAP_SELECT);
			gbv_u8* row = tile + GBV_TILE_PITCH * py;
			pal = *gbv_reg.bgp;
			color = get_pal_idx_from_tile_row(row, px);
		}
		if (*gbv_reg.lcdc & GBV_LCDC_OBJ_ENABLE) {
			// TODO: properly support order of sprites with coinciding x values
			gbv_u8 mode16x8 = (*gbv_reg.lcdc & GBV_LCDC_OBJ_SIZE_SELECT);
			for (gbv_u8 idx = 0; idx < gbv_scanline.obj_count; idx++) {
				gbv_obj_char* obj = gbv_oam_data + gbv_scanline.objs[gbv_scanline.obj_count - idx - 1];
				if (obj->x <= lcd_x + GBV_SPRITE_MARGIN_LEFT && obj->x + 8 > lcd_x + GBV_SPRITE_MARGIN_LEFT) {
					gbv_u8 px = lcd_x + GBV_SPRITE_MARGIN_LEFT - obj->x;
					gbv_u8 py = *gbv_reg.ly + GBV_SPRITE_MARGIN_TOP - obj->y;
					if (obj->attr & GBV_OBJ_ATTR_FLIP_VERTICAL) {
						py = GBV_TILE_HEIGHT - 1 - py + (mode16x8 ? GBV_TILE_HEIGHT : 0);
					}	if (obj->attr & GBV_OBJ_ATTR_FLIP_HORIZONTAL) {
						px = GBV_TILE_WIDTH - 1 - px;
					}
					gbv_u8* tile = gbv_tile_data + GBV_TILE_SIZE * (mode16x8 ? obj->id & 0xFE : obj->id);
					gbv_u8* row = tile + GBV_TILE_PITCH * py;
					gbv_u8 new_pal = (obj->attr & GBV_OBJ_ATTR_PALETTE_SELECT) ? *gbv_reg.obp1 : *gbv_reg.obp0;
					gbv_u8 new_color = get_pal_idx_from_tile_row(row, px);
					if (obj->attr & GBV_OBJ_ATTR_PRIORITY_FLAG) {
						if (color == 0 && new_color != 0) {
							color = new_color;
							pal = new_pal;
						}
					}
					else if(new_color != 0) {
						color = new_color;
						pal = new_pal;
					}
				}
			}
		}
		gbv_scanline.buffer[lcd_x] = gbv_scanline.palette->colors[get_color(color, pal)];
	}
}