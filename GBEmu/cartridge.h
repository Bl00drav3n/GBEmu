#define ROM_BANK_SIZE 0x4000
#define RAM_BANK_SIZE 0x2000

typedef struct {
	u8 entrypoint[4];
	u8 logo[48];
	u8 title[16];
	u8 new_licensee_code[2];
	u8 sgb_flag;
	u8 mbc_type;
	u8 rom_size;
	u8 ram_size;
	u8 destination_code;
	u8 old_licensee_code[2];
	u8 version_number;
	u8 checksum;
	u8 global_checksum[2];
} cartridge_header_t;

typedef struct {
	u8 rom[ROM_BANK_SIZE];
} rom_bank_t;

typedef struct {
	u8 ram[RAM_BANK_SIZE];
} ram_bank_t;

struct mbc1_t {
	u8 ctrl;
	bool ram_enabled;
	bool ram_mode;
};

struct mbc2_t {
	u8 ctrl;
	u8 ram[512];
};

typedef enum {
	ROM_ONLY = 0x00,
	MBC1 = 0x01,
	MBC1_RAM = 0x02,
	MBC1_RAM_BATTERY = 0x03,
	MBC2 = 0x05,
	MBC2_BATTERY = 0x06,
	ROM_RAM = 0x08,
	ROM_RAM_BATTERY = 0x09,
	MMM01 = 0x0B,
	MMM01_RAM = 0x0C,
	MMM01_RAM_BATTERY = 0x0D,
	MBC3_TIMER_BATTERY = 0x0F,
	MBC3_TIMER_RAM_BATTERY = 0x10,
	MBC3 = 0x11,
	MBC3_RAM = 0x12,
	MBC3_RAM_BATTERY = 0x13,
	MBC4 = 0x15,
	MBC4_RAM = 0x16,
	MBC4_RAM_BATTERY = 0x17,
	MBC5 = 0x19,
	MBC5_RAM = 0x1A,
	MBC5_RAM_BATTERY = 0x1B,
	MBC5_RUMBLE = 0x1C,
	MBC5_RUMBLE_RAM = 0x1D,
	MBC5_RUMBLE_RAM_BATTERY = 0x1E,
	POCKET_CAMERA = 0xFC,
	BANDAI_TAMAS = 0xFD,
	HUC3 = 0xFE,
	HUC1_RAM_BATTERY = 0xFF
} mbc_type_t;

typedef void (*mbc_write_fn)(u16 addr, u8 data);

struct mbc_t {
	mbc_type_t type;
	union {
		mbc1_t mbc1;
		mbc2_t mbc2;
	};
};

struct cartridge_t {
	cartridge_header_t* header;

	u8          rom_bank_count;
	rom_bank_t* rom_banks;

	u8          ram_bank_count;
	ram_bank_t* ram_banks;

	mbc_t        mbc;
	mbc_write_fn write;
};