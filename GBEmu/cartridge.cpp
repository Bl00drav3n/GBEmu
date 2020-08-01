u8 mbc1_get_active_rom_bank(mbc1_t mbc1) {
	u8 bank_lo = mbc1.ctrl & 0x1F;
	u8 bank_hi = mbc1.ctrl & (mbc1.ram_mode ? 0x00 : 0x60);
	bank_lo = (bank_lo) ? bank_lo : 0x01;
	u8 bank = (bank_hi << 5) | bank_lo;
	return bank;
}

void mbc1_update_rom_bank() {
	u8 bank = mbc1_get_active_rom_bank(device.cartridge->mbc.mbc1);
	SDL_assert(bank < device.cartridge->rom_bank_count);
	if (bank < device.cartridge->rom_bank_count) {
		device.rom1 = device.cartridge->rom_banks[bank].rom;
	}
}

u8 mbc1_get_active_ram_bank(mbc1_t mbc1) {
	u8 bank = (mbc1.ram_mode) ? (mbc1.ctrl & 0x60) >> 5 : 0x00;
	return bank;
}

void mbc1_update_ram_bank() {
	if (device.cartridge->mbc.mbc1.ram_enabled) {
		u8 bank = mbc1_get_active_ram_bank(device.cartridge->mbc.mbc1);
		SDL_assert(bank < device.cartridge->ram_bank_count);
		if (bank < device.cartridge->ram_bank_count) {
			device.eram = device.cartridge->ram_banks[bank].ram;
		}
	}
	else {
		device.eram = 0;
	}
}

void mbc1_write(u16 addr, u8 data) {
	// NOTE: 2bit + 5bit ROM bank
	// NOTE: 2bit RAM bank
	// NOTE: can select between upper 2 bits of ROM or RAM bank
	// NOTE: during ROM mode, RAM bank 0 is selected; during RAM mode, only banks 0x01 - 0x1F (lower 5 bits) can be selected
	// NOTE: ROM bank 0x00, 0x20, 0x40, 0x60 translate to 0x01, 0x21, 0x41, 0x61 respectively => 125 addressable banks
	// NOTE: this happens when the lower 5 bits are 0
	if (addr < 0x2000) {
		device.cartridge->mbc.mbc1.ram_enabled = data & 0x0A;
		mbc1_update_ram_bank();
	}
	else if (addr < 0x4000) {
		// NOTE: update lower 5 bits of ROM bank
		device.cartridge->mbc.mbc1.ctrl = (device.cartridge->mbc.mbc1.ctrl & 0x60) | (data & 0x1F);
		mbc1_update_rom_bank();
	}
	else if (addr < 0x6000) {
		// NOTE: RAM bank number or upper 2 bits of ROM bank number
		device.cartridge->mbc.mbc1.ctrl = (device.cartridge->mbc.mbc1.ctrl & 0x1F) | ((data & 0x03) << 5);
		mbc1_update_ram_bank();
		mbc1_update_rom_bank();
	}
	else if (addr < 0x8000) {
		// NOTE: select RAM or ROM mode
		device.cartridge->mbc.mbc1.ram_mode = data & 0x01;
		mbc1_update_ram_bank();
		mbc1_update_rom_bank();
	}
}

void cartridge_write_stub(u16 addr, u8 data) {
	fprintf(stderr, "WARNING: Attemped write on read only cartridge on address %04Xh at %04Xh.\n", addr, device.ins_ptr);
}

void cartridge_update_mbc(cartridge_t *cartridge) {
	cartridge->mbc.type = (mbc_type_t)cartridge->header->mbc_type;
	switch (cartridge->mbc.type) {
	case MBC1:
	case MBC1_RAM:
	case MBC1_RAM_BATTERY:
		cartridge->write = mbc1_write;
		break;
	default:
		cartridge->write = cartridge_write_stub;
	}
}

cartridge_t* allocate_cartridge(u8 rom_bank_count, u8 ram_bank_count) {
	cartridge_t* result = 0;
	cartridge_t* c = (cartridge_t*)malloc(sizeof(*c));
	if (c) {
		memset(c, 0, sizeof(cartridge_t));
		rom_bank_t* rom_banks = 0;
		ram_bank_t* ram_banks = 0;
		rom_banks = (rom_bank_t*)malloc(rom_bank_count * sizeof(rom_bank_t));
		if (rom_banks) {
			if (ram_bank_count) {
				ram_banks = (ram_bank_t*)malloc(ram_bank_count * sizeof(ram_bank_t));
				if (ram_banks) {
					c->rom_bank_count = rom_bank_count;
					c->rom_banks = rom_banks;
					c->ram_bank_count = ram_bank_count;
					c->ram_banks = ram_banks;
					result = c;
					memset(c->ram_banks, 0, sizeof(ram_bank_t) * c->ram_bank_count);
				}
				else {
					fprintf(stderr, "Error allocating %u RAM bank(s)\n", (unsigned int)ram_bank_count);
					free(rom_banks);
				}
			}
			else {
				c->rom_bank_count = rom_bank_count;
				c->rom_banks = rom_banks;
				result = c;
			}
		}
		else {
			fprintf(stderr, "Error allocating %u ROM bank(s)\n", (unsigned int)rom_bank_count);
		}
	}
	return result;
}

void free_cartridge(cartridge_t* c) {
	if (c) {
		if (c->rom_banks) free(c->rom_banks);
		if (c->ram_banks) free(c->ram_banks);
		free(c);
	}
}