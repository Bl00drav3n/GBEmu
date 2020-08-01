#define MAX_VOLUME_LEVEL 10
#define DEFAULT_VOLUME_LEVEL (MAX_VOLUME_LEVEL / 2)

typedef enum {
	IO_P1 = 0x00,
	IO_SB = 0x01,
	IO_SC = 0x02,
	IO_DIV = 0x04,
	IO_TIMA = 0x05,
	IO_TMA = 0x06,
	IO_TAC = 0x07,
	IO_IF = 0x0F,
	IO_NR10 = 0x10,
	IO_NR11 = 0x11,
	IO_NR12 = 0x12,
	IO_NR13 = 0x13,
	IO_NR14 = 0x14,
	IO_NR21 = 0x16,
	IO_NR22 = 0x17,
	IO_NR23 = 0x18,
	IO_NR24 = 0x19,
	IO_NR30 = 0x1A,
	IO_NR31 = 0x1B,
	IO_NR32 = 0x1C,
	IO_NR33 = 0x1D,
	IO_NR34 = 0x1E,
	IO_NR41 = 0x20,
	IO_NR42 = 0x21,
	IO_NR43 = 0x22,
	IO_NR44 = 0x23,
	IO_NR50 = 0x24,
	IO_NR51 = 0x25,
	IO_NR52 = 0x26,
	IO_PWM_START = 0x30,
	IO_PWM_END = 0x3F,
	IO_LCDC = 0x40,
	IO_STAT = 0x41,
	IO_SCY = 0x42,
	IO_SCX = 0x43,
	IO_LY = 0x44,
	IO_LYC = 0x45,
	IO_DMA = 0x46,
	IO_BGP = 0x47,
	IO_OBP0 = 0x48,
	IO_OBP1 = 0x49,
	IO_WY = 0x4A,
	IO_WX = 0x4B
} io_register_t;

typedef enum {
	INT_VBLANK = 0x01,
	INT_LCDC = 0x02,
	INT_TIMER = 0x04,
	INT_SERIAL_TRANSFER = 0x08,
	INT_P1 = 0x10,
	INT_MASK = 0x1F
} interrupt_type_t;

struct device_t {
	u32 counter;
	lr35902_cpu_t cpu;

	struct vpu_t {
		bool disabled;
		u16 cycles;
	} vpu;

	sequencer_t audio_seq;
	struct {
		u8 sqr1[2];
		u8 sqr2[2];
		u8 wave[2];
		u8 noise[2];
	} audio_samples;

	struct {
		u32 len;	// NOTE: len of 63 means 1/256 sec (64 - len)/256
		u16 freq;
		sequencer_t pwm;
		struct {
			bool can_update;
			timer_t period;
			u8 volume;
		} envelope;
		struct {
			sequencer_t pos;
			u8 data[16];
			u8 sample;
		} wave;
		struct {
			timer_t timer;
			u16 reg;
		} noise;
		bool enable_override;
	} channel[AUDIO_CHANNEL_COUNT];

	struct {
		bool enabled;
		timer_t timer;
		u16 freq;
	} sweep;

	u8* rom0;         // 16kB rom bank 0 fixed
	u8* rom1;         // 16kB rom bank 1 switchable
	u8  vram[0x2000]; //  8kb of vram
	u8* eram;         //  8kB external ram
	u8  wram[0x2000]; //  8kB work ram
	u8  oam[0xA0];    // 160B of sprite memory (40 sprites)
	u8  io[0x80];     // i/o registers
	u8  hram[0x7F];   // 127B of high ram 
	u8  ie;           // interrupt enable register

	bool dma;         // dma started
	u16 ins_ptr;

	sdlgb_input_t input;
	cartridge_t* cartridge;

	bool vblank;
	gbv_palette palette;
	u8* render_buffer;
		
	u8 volume_level;
	float volume;
	bool muted;
};

u8* device_get_io(u16 addr);
void device_request_interrupt(interrupt_type_t type);

static device_t device = {};