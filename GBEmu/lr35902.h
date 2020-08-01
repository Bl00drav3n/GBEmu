typedef enum {
	IME_DISABLE = 0,  // disable all interrupts
	IME_ENABLE = 1    // enable interrupts specified in IE
} ime_flag_t;

typedef enum {
	FLAG_ZF = 1 << 7,
	FLAG_N = 1 << 6,
	FLAG_H = 1 << 5,
	FLAG_CY = 1 << 4
} cpu_flags_t;

typedef struct {
	u32 freq;
	struct {
		union {
			struct {
				u8 f;
				u8 a;
			};
			u16 af;
		};
		union {
			struct {
				u8 c;
				u8 b;
			};
			u16 bc;
		};
		union {
			struct {
				u8 e;
				u8 d;
			};
			u16 de;
		};
		union {
			struct {
				u8 l;
				u8 h;
			};
			u16 hl;
		};
		u16 pc;
		u16 sp;
	} registers;
	u8 ime;

	u8   cycles;
	u16  addr;
	u8   imm8;
	u16  imm16;
	u8* from8;
	u8* to8;
	u16* from16;
	u16* to16;
	u8* reg8;
	u16* reg16;
	u8   cb_bitmask;
	u8* cb_target;

	bool disable_interrupt;
	bool enable_interrupt;
	bool halted;
	bool stopped;
} lr35902_cpu_t;

struct instruction_t {
	const char* name;
	void(*mode)(void);
	u8(*op)(void);
	u8 cycles;
};