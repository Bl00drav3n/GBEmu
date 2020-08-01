u8 bus_read(u16 addr) {
	if (addr < 0x4000) {
		// rom0
		return device.rom0[addr];
	}
	else if (addr < 0x8000) {
		// rom1
		// TODO: pass through MBC instead?
		return device.rom1[addr - 0x4000];
	}
	else if (addr < 0xA000) {
		// vram
		return device.vram[addr - 0x8000];
	}
	else if (addr < 0xC000) {
		// external ram
		// TODO: pass through MBC instead?
		if (device.eram) {
			return device.eram[addr - 0xA000];
		}
		else {
			fprintf(stderr, "WARNING: Attempted read from unavailable external RAM on address %04Xh at %04Xh.\n", addr, device.ins_ptr);
			return 0xff;
		}
	}
	else if (addr < 0xE000) {
		// wram
		return device.wram[addr - 0xC000];
	}
	else if (addr < 0xFE00) {
		// echo of 0xC000 - 0xDDFF
		return device.wram[addr - 0xE000];
	}
	else if (addr < 0xFEA0) {
		// oam
	}
	else if (addr < 0xFF00) {
		// not usable
	}
	else if (addr < 0xFF80) {
		// I/O ports
		return *device_get_io(addr);
	}
	else if (addr < 0xFFFF) {
		// hram
		return device.hram[addr - 0xFF80];
	}
	else {
		// interrupt enable register
		return device.ie;
	}

	return 0;
}

void bus_write(u16 addr, u8 data) {
	if (addr < 0x8000) {
		// write to ROM = configure MBC
		device.cartridge->write(addr, data);
	}
	else if (addr < 0xA000) {
		// vram
		device.vram[addr - 0x8000] = data;
	}
	else if (addr < 0xC000) {
		// external ram
		// TODO: pass through MBC instead?
		if (device.eram) {
			device.eram[addr - 0xA000] = data;
		}
		else {
			//fprintf(stderr, "WARNING: Attempted write to unavailable external RAM on address %04Xh at %04Xh.\n", addr, device.ins_ptr);
		}
	}
	else if (addr < 0xE000) {
		// wram
		device.wram[addr - 0xC000] = data;
	}
	else if (addr < 0xFE00) {
		// echo of 0xC000 - 0xDDFF
		device.wram[addr - 0xE000] = data;
	}
	else if (addr < 0xFEA0) {
		// oam
		device.oam[addr - 0xFE00] = data;
	}
	else if (addr < 0xFF00) {
		// not usable
		//fprintf(stderr, "WARNING: Attempted write to unmapped address %04Xh at %04Xh.\n", addr, device.ins_ptr);
	}
	else if (addr < 0xFF80) {
		// I/O ports
		// TODO
		//SDL_assert(0);
		u8 reg = addr - 0xFF00;
		switch (reg) {
		case IO_P1:
			device.io[reg] = (device.io[reg] & 0x0F) | (data & 0x30);
			break;
		case IO_DIV:
			device.io[reg] = 0;
			break;
		case IO_NR11:
			device.io[reg] = data & 0xC0;
			device.channel[AUDIO_SQUARE1].len = 0x40 - data & 0x3F;
			break;
		case IO_NR12:
			device.io[reg] = data;
			envelope_reload_volume(AUDIO_SQUARE1);
			break;
		case IO_NR13:
			device.channel[AUDIO_SQUARE1].freq = (device.channel[AUDIO_SQUARE1].freq & 0x0700) | data;
			audio_channel_update_freq(AUDIO_SQUARE1);
			break;
		case IO_NR14:
			device.io[reg] = (device.io[reg] & ~0x40) | (data & 0x40);
			device.channel[AUDIO_SQUARE1].freq = (device.channel[AUDIO_SQUARE1].freq & 0x00FF) | ((data & 0x0007) << 8);
			audio_channel_update_freq(AUDIO_SQUARE1);
			if (data & 0x80) audio_trigger_channel(AUDIO_SQUARE1);
			break;
		case IO_NR21:
			device.io[reg] = data & 0xC0;
			device.channel[AUDIO_SQUARE2].len = 0x40 - data & 0x3F;
			break;
		case IO_NR22:
			device.io[reg] = data;
			envelope_reload_volume(AUDIO_SQUARE2);
			break;
		case IO_NR23:
			device.channel[AUDIO_SQUARE2].freq = (device.channel[AUDIO_SQUARE2].freq & 0x0700) | data;
			audio_channel_update_freq(AUDIO_SQUARE2);
			break;
		case IO_NR24:
			device.io[reg] = (device.io[reg] & ~0x40) | (data & 0x40);
			device.channel[AUDIO_SQUARE2].freq = (device.channel[AUDIO_SQUARE2].freq & 0x00FF) | ((data & 0x0007) << 8);
			audio_channel_update_freq(AUDIO_SQUARE2);
			if (data & 0x80) audio_trigger_channel(AUDIO_SQUARE2);
			break;
		case IO_NR30:
			device.io[reg] = (device.io[reg] & ~0x80) | (data & 0x80);
			break;
		case IO_NR31:
			device.io[reg] = data;
			device.channel[AUDIO_WAVE].len = -data;
			break;
		case IO_NR32:
			device.io[reg] = (device.io[reg] & ~0x60) | (data & 0x60);
			break;
		case IO_NR33:
			device.channel[AUDIO_WAVE].freq = (device.channel[AUDIO_WAVE].freq & 0x0700) | data;
			audio_channel_update_freq(AUDIO_WAVE);
			break;
		case IO_NR34:
			device.io[reg] = (device.io[reg] & ~0x40) | (data & 0x40);
			device.channel[AUDIO_WAVE].freq = (device.channel[AUDIO_WAVE].freq & 0x00FF) | ((data & 0x0007) << 8);
			if (data & 0x80) audio_trigger_channel(AUDIO_WAVE);
			break;
		case IO_NR41:
			device.io[reg] = data & 0x3F;
			device.channel[AUDIO_NOISE].len = 0x40 - data & 0x3F;
			break;
		case IO_NR42:
			device.io[reg] = data;
			envelope_reload_volume(AUDIO_NOISE);
			break;
		case IO_NR43:
			device.io[reg] = data;
			audio_channel_update_freq(AUDIO_NOISE);
			break;
		case IO_NR44:
			device.io[reg] = (device.io[reg] & ~0x40) | (data & 0x40);
			if (data & 0x80) audio_trigger_channel(AUDIO_NOISE);
			break;
		case IO_NR50:
			// NOTE: S01/S02 master volume control
			device.io[reg] = (device.io[reg] & ~0x77) | (data & 0x77);
			break;
		case IO_NR51:
			// NOTE: S01/S02 channel output selection
			device.io[reg] = data;
			break;
		case IO_NR52:
			if ((data & 0x80)) {
				audio_master_enable();
			}
			else {
				audio_master_disable();
			}
			break;
		case IO_PWM_START + 0x0: case IO_PWM_START + 0x1: case IO_PWM_START + 0x2: case IO_PWM_START + 0x3:
		case IO_PWM_START + 0x4: case IO_PWM_START + 0x5: case IO_PWM_START + 0x6: case IO_PWM_START + 0x7:
		case IO_PWM_START + 0x8: case IO_PWM_START + 0x9: case IO_PWM_START + 0xa: case IO_PWM_START + 0xb:
		case IO_PWM_START + 0xc: case IO_PWM_START + 0xd: case IO_PWM_START + 0xe: case IO_PWM_START + 0xf:
			device.channel[AUDIO_WAVE].wave.data[reg - 0x30] = data;
			break;
		case IO_LY:
			vpu_reset_ly();
			break;
		case IO_STAT:
			device.io[reg] = (device.io[reg] & 0x07) | (data & 0x78);
			break;
		case IO_IF:
			device.io[reg] = (device.io[reg] & 0xE0) | (data & 0x1F);
			break;
		case IO_DMA:
			device.io[reg] = data;
			if (data <= 0xF1) {
				device.dma = true;
			}
			break;
		default:
			device.io[reg] = data;
		}
	}
	else if (addr < 0xFFFF) {
		// hram
		device.hram[addr - 0xFF80] = data;
	}
	else {
		// interrupt enable register
		device.ie = data;
	}
}
