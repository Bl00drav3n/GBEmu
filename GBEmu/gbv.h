#ifndef __GBV_H__
#define __GBV_H__

/* in case you're compiling this as a library */
#ifndef GBV_API
#define GBV_API
#endif

#define GBV_TILE_MEMORY_SIZE   6144
#define GBV_BG_MAP_MEMORY_SIZE 1024
#define GBV_OAM_MEMORY_SIZE    160
#define GBV_HW_MEMORY_SIZE     (64 * 1024)

#define GBV_SCREEN_WIDTH       160
#define GBV_SCREEN_HEIGHT      144
#define GBV_SCREEN_SIZE        (GBV_SCREEN_WIDTH * GBV_SCREEN_HEIGHT)

#define GBV_BG_TILES_X         32
#define GBV_BG_TILES_Y         32
#define GBV_BG_TILE_COUNT      (GBV_BG_TILES_X * GBV_BG_TILES_Y)

#define GBV_WND_TILES_X        20
#define GBV_WND_TILES_Y        18
#define GBV_WND_TILE_COUNT     (GBV_WND_TILES_X * GBV_WND_TILES_Y)

#define GBV_TILE_WIDTH         8
#define GBV_TILE_HEIGHT        8
#define GBV_TILE_PITCH         2
#define GBV_TILE_SIZE          (GBV_TILE_PITCH * GBV_TILE_HEIGHT)

#define GBV_OBJ_COUNT          40
#define GBV_OBJ_SIZE           (4 * GBV_OBJ_COUNT)

typedef char           gbv_s8;
typedef short          gbv_s16;
typedef unsigned char  gbv_u8;
typedef unsigned short gbv_u16;

typedef unsigned char  gbv_io;

typedef enum {
	GBV_RENDER_MODE_GRAYSCALE_8,	// 0-255
} gbv_render_mode;

typedef struct {
	gbv_u8 colors[4];
} gbv_palette;

typedef enum {
	GBV_LCDC_BG_ENABLE       = 0x01, /* enable bg display */
	GBV_LCDC_OBJ_ENABLE      = 0x02, /* enable obj display */
	GBV_LCDC_OBJ_SIZE_SELECT = 0x04, /* enable 8x16 obj mode */
	GBV_LCDC_BG_MAP_SELECT   = 0x08, /* bg tile map display select */
	GBV_LCDC_BG_DATA_SELECT  = 0x10, /* bg & wnd tile data select */
	GBV_LCDC_WND_ENABLE      = 0x20, /* enable wnd display */
	GBV_LCDC_WND_MAP_SELECT  = 0x40, /* wnd tile map display select */
	GBV_LCDC_CTRL            = 0x80, /* enable lcd */
} gbv_lcdc_flag;

typedef enum {
	GBV_STAT_MODE       = 0x03, /* read-only: mode flag (0: h-blank, 1: v-blank, 2: searching oam, 3: transferring data to lcd driver */
	GBV_STAT_LYC        = 0x04, /* read-only: coincidence flag */
	GBV_STAT_HBLANK_INT = 0x08, /* mode 0 h-blank interrupt */
	GBV_STAT_VBLANK_INT = 0x10, /* mode 1 v-blank interrupt */
	GBV_STAT_OAM_INT    = 0x20, /* mode 2 oam interrupt */
	GBV_STAT_LYC_INT    = 0x40, /* LYC=LY coincidence interrupt */
} gbv_stat_flag;

typedef enum {
	GBV_LCD_MODE_HBLANK   = 0x00,
	GBV_LCD_MODE_VBLANK   = 0x01,
	GBV_LCD_MODE_OAM      = 0x02,
	GBV_LCD_MODE_TRANSFER = 0x03,
} gbv_lcd_mode;

typedef enum {
	GBV_OBJ_ATTR_PALETTE_SELECT  = 0x10, /* specify obj palette */
	GBV_OBJ_ATTR_FLIP_HORIZONTAL = 0x20, /* flip horizontally */
	GBV_OBJ_ATTR_FLIP_VERTICAL   = 0x40, /* flip vertically */
	GBV_OBJ_ATTR_PRIORITY_FLAG   = 0x80, /* display priority flag */
} gbv_obj_attr;

typedef struct {
	gbv_u8 data[8][2];
} gbv_tile;

typedef struct {
	gbv_u8 y;
	gbv_u8 x;
	gbv_u8 id;
	gbv_u8 attr;
} gbv_obj_char;

struct gbv_memory_t {
	gbv_u8* vram;
	gbv_u8* oam;
};

/* user defined callback function used for interrupt handling */
typedef void (*gbv_int_callback)(void);

typedef struct {
	gbv_io *stat;
	gbv_io *lcdc;
	gbv_io *bgp;
	gbv_io *obp0;
	gbv_io *obp1;
	gbv_io *scx;
	gbv_io *scy;
	gbv_io *lyc;
	gbv_io *wx;
	gbv_io *wy;
	gbv_io *ly;
} gbv_io_mapped_registers_t;

extern GBV_API gbv_io_mapped_registers_t gbv_reg;

/*****************************/
/************ API ************/
/*****************************/

/* version information */
extern GBV_API void gbv_get_version(int * maj, int * min, int * patch);

/* initialize system, provide GBV_HW_MEMORY_SIZE (64k) of memory */
extern GBV_API void gbv_init(gbv_u8 *vram, gbv_u8 *oam);

/* utility function for LCD control */
extern GBV_API void gbv_lcdc_set(gbv_lcdc_flag flag);
extern GBV_API void gbv_lcdc_reset(gbv_lcdc_flag flag);

/* utility function for LCD stat register */
extern GBV_API void gbv_stat_set(gbv_stat_flag flag);
extern GBV_API void gbv_stat_reset(gbv_stat_flag flag);

/* return raw pointers for data specification */
extern GBV_API gbv_u8 * gbv_get_tile_map0();
extern GBV_API gbv_u8 * gbv_get_tile_map1();

extern GBV_API gbv_u8   * gbv_get_tile_data();
extern GBV_API gbv_tile * gbv_get_tile(gbv_u8 tile_id);

/* LCD status register */
extern GBV_API void gbv_stat_set(gbv_stat_flag flag);
extern GBV_API void gbv_stat_reset(gbv_stat_flag flag);

/* LCD mode */
extern GBV_API gbv_lcd_mode gbv_stat_mode();

/* LCD Y coincidence */
extern GBV_API gbv_u8 gbv_stat_lyc();

/* LY register */
extern GBV_API gbv_io gbv_ly();

/* set user defined callback for LCDC status interrupt */
extern GBV_API void gbv_lcdc_set_stat_interrupt(gbv_int_callback callback);

/* copy GBV_OBJ_SIZE bytes of data to OAM memory */
extern GBV_API void gbv_transfer_oam_data(gbv_u8 *data);

/* render all data to target buffer */
extern GBV_API void gbv_render(void * render_buffer, gbv_render_mode mode, gbv_palette * palette);

#endif