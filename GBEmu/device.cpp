static const char* interrupt_str[5] = {
	"VBLANK",
	"LCDC",
	"TIMER",
	"SERIAL_TRANSFER",
	"P1"
};

void device_toggle_mute() {
	device.muted = !device.muted;
}

void device_set_volume_level(u8 level) {
	SDL_assert(level <= MAX_VOLUME_LEVEL);
	if (level <= MAX_VOLUME_LEVEL) {
		device.volume_level = level;
		device.volume = (float)level / (float)MAX_VOLUME_LEVEL;
	}
}

void device_increase_volume() {
	if (device.volume_level < MAX_VOLUME_LEVEL) {
		device_set_volume_level(device.volume_level + 1);
	}
}

void device_decrease_volume() {
	if (device.volume_level > 0) {
		device_set_volume_level(device.volume_level - 1);
	}
}

void device_reset() {
	cpu_reset();
	device.io[IO_TIMA] = 0x00;
	device.io[IO_TMA] = 0x00;
	device.io[IO_TAC] = 0x00;
	device.io[IO_NR10] = 0x80;
	device.io[IO_NR11] = 0xBF;
	device.io[IO_NR12] = 0xF3;
	device.io[IO_NR14] = 0xBF;
	device.io[IO_NR21] = 0x3F;
	device.io[IO_NR22] = 0x00;
	device.io[IO_NR24] = 0xBF;
	device.io[IO_NR30] = 0x7F;
	device.io[IO_NR31] = 0xFF;
	device.io[IO_NR32] = 0x9F;
	device.io[IO_NR33] = 0xBF;
	device.io[IO_NR41] = 0xFF;
	device.io[IO_NR42] = 0x00;
	device.io[IO_NR43] = 0x00;
	device.io[IO_NR30] = 0xBF;
	device.io[IO_NR50] = 0x77;
	device.io[IO_NR51] = 0xF3;
	for (u8 i = IO_PWM_START; i <= IO_PWM_END; i++) {
		device.io[i] = 0xF1;
	}
	device.io[IO_LCDC] = 0x91;
	device.io[IO_SCY] = 0x00;
	device.io[IO_SCX] = 0x00;
	device.io[IO_LYC] = 0x00;
	device.io[IO_BGP] = 0xFC;
	device.io[IO_OBP0] = 0xFF;
	device.io[IO_OBP1] = 0xFF;
	device.io[IO_WY] = 0x00;
	device.io[IO_WX] = 0x00;
	device.ie = 0x00;

	device.io[IO_IF] = 0xE0;
	device.io[IO_STAT] = 0x02;

	device.sweep.enabled = false;
	device.sweep.freq = 0;
	device.sweep.timer = timer_create(0);
	device.audio_seq = sequencer_create(8192, 8);
	for (int i = 0; i < 4; i++) {
		device.channel[i].freq = 0x7FFF;
		device.channel[i].len = (i == AUDIO_WAVE) ? 256 : 64;
		device.channel[i].envelope.period = timer_create(0);
		device.channel[i].envelope.can_update = false;
	}
	device.channel[AUDIO_SQUARE1].pwm = sequencer_create(0, 8);
	device.channel[AUDIO_SQUARE2].pwm = sequencer_create(0, 8);
	device.channel[AUDIO_WAVE].wave.pos = sequencer_create(0, 32);
	device.channel[AUDIO_WAVE].wave.sample = 0;
	device.channel[AUDIO_NOISE].noise.timer = timer_create(0);
	device.channel[AUDIO_NOISE].noise.reg = 0x7FFF;

	device_set_volume_level(DEFAULT_VOLUME_LEVEL);
	device.vblank = false;
}

void device_init() {
	device_reset();

	for (int i = 0; i < AUDIO_CHANNEL_COUNT; i++) {
		device.channel[i].enable_override = true;
	}
}

void device_toggle_channel_enable(audio_channel_type_t channel) {
	device.channel[channel].enable_override = !device.channel[channel].enable_override;
}

u8* device_get_io(u16 addr) {
	return device.io + (addr & 0xff);
}

const char* get_interrupt_str(interrupt_type_t interrupt) {
	switch (interrupt) {
	case INT_VBLANK: return interrupt_str[0];
	case INT_LCDC: return interrupt_str[1];
	case INT_TIMER: return interrupt_str[2];
	case INT_SERIAL_TRANSFER: return interrupt_str[3];
	case INT_P1: return interrupt_str[4];
	}
	return "INVALID";
}

void device_request_interrupt(interrupt_type_t interrupt) {
	device.io[IO_IF] = device.io[IO_IF] | interrupt;
#if _ENABLE_DUMP
	fprintf(debug_out, "Requested interrupt: %s (IE:%02Xh, IF:%02Xh, IME:%d)\n", get_interrupt_str(interrupt), (uint32_t)device.ie, (uint32_t)device.io[IO_IF], (uint32_t)device.cpu.ime);
#endif
}

#define CARTRIDGE_HEADER_OFFSET 0x0100
bool device_load_cartdrige(const char *rom_filename) {
	u8* rom = load_file(rom_filename);
	if (!rom) return false;

	device.rom0 = rom;
	device.rom1 = rom + 0x4000;
	device.eram = 0;

	cartridge_header_t* header = (cartridge_header_t*)(rom + CARTRIDGE_HEADER_OFFSET);
	fprintf(stdout, "Loading cartridge:\n");
	fprintf(stdout, "Title: %.16s\n", header->title);
	fprintf(stdout, "Type:  %02Xh\n", (unsigned int)header->mbc_type);
	fprintf(stdout, "ROM:   %02Xh\n", (unsigned int)header->rom_size);
	fprintf(stdout, "RAM:   %02Xh\n", (unsigned int)header->ram_size);

	switch (header->mbc_type) {
	case MBC1:
		fprintf(stdout, "MBC1 cartridge\n");
		break;
	case MBC1_RAM:
		fprintf(stdout, "MBC1 cartridge with static RAM\n");
		break;
	case MBC1_RAM_BATTERY:
		fprintf(stdout, "MBC1 cartridge with battery buffered static RAM\n");
		break;
	case ROM_ONLY:
		fprintf(stdout, "ROM only cartridge\n");
		break;
	default:
		fprintf(stderr, "Unsupported cartridge type %02Xh\n", header->mbc_type);
		return false;
	}

	u8 rom_bank_count, ram_bank_count;
	switch (header->rom_size) {
	case 0x00:
	case 0x01:
	case 0x02:
	case 0x03:
	case 0x04:
	case 0x05:
	case 0x07:
	case 0x08:
		rom_bank_count = 2 << header->rom_size;
		break;
	case 0x52:
		rom_bank_count = 72;
		break;
	case 0x53:
		rom_bank_count = 80;
		break;
	case 0x54:
		rom_bank_count = 96;
		break;
	default:
		fprintf(stderr, "Invalid ROM size %02Xh\n", header->rom_size);
		return false;
	}

	switch (header->ram_size) {
	case 0x00:
		ram_bank_count = 0;
		break;
	case 0x01:
	case 0x02:
		ram_bank_count = 1;
		break;
	case 0x03:
		ram_bank_count = 4;
		break;
	case 0x04:
		ram_bank_count = 16;
		break;
	case 0x05:
		ram_bank_count = 8;
		break;
	default:
		fprintf(stderr, "Invalid RAM size %02Xh\n", header->ram_size);
		return false;
	}

	if (ram_bank_count == 0) {
		// TODO: ??
		ram_bank_count = 1;
	}
	cartridge_t* cartridge = allocate_cartridge(rom_bank_count, ram_bank_count);
	if (cartridge) {
		memcpy(cartridge->rom_banks, rom, sizeof(rom_bank_t) * cartridge->rom_bank_count);
		cartridge->header = (cartridge_header_t*)(cartridge->rom_banks[0].rom + CARTRIDGE_HEADER_OFFSET);
		cartridge_update_mbc(cartridge);

		device.cartridge = cartridge;
		fprintf(stdout, "Cartridge loaded\n");
		return true;
	}

	return false;
}

void unload_cartridge() {
	free_cartridge(device.cartridge);
	device.cartridge = 0;
	fprintf(stdout, "Cartridge unloaded\n");
}

void dma_transfer() {
	u16 addr = device.io[IO_DMA] << 8;
	u8* mem = 0;
	if (addr < 0x4000) {
		mem = device.rom0 + addr;
	}
	else if (addr < 0x8000) {
		mem = device.rom1 + addr - 0x4000;
	}
	else if (addr < 0xA000) {
		mem = device.vram + addr - 0x8000;
	}
	else if (addr < 0xC000) {
		mem = device.eram + addr - 0xA000;
		if (!device.eram) {
			fprintf(stderr, "ERROR: Attempted dma from unavailable external RAM on address %04Xh at %04Xh.\n", addr, device.ins_ptr);
		}
	}
	else if (addr < 0xE000) {
		mem = device.wram + addr - 0xC000;
	}
	else if (addr <= 0xF100) {
		// echo of 0xC000 - 0xDDFF
		mem = device.wram + addr - 0xE000;
	}

	if (mem) {
		gbv_transfer_oam_data(mem);
	}
}

void device_clock() {
	device.counter++;
	u8 p1 = device.io[IO_P1];
	if ((p1 & 0x30) == 0x10) {
		u8 value = 0;
		value |= device.input.state[BUTTON_A] ? 0x01 : 0x00;
		value |= device.input.state[BUTTON_B] ? 0x02 : 0x00;
		value |= device.input.state[BUTTON_SELECT] ? 0x04 : 0x00;
		value |= device.input.state[BUTTON_START] ? 0x08 : 0x00;
		p1 = (p1 & 0x30) | (~value & 0x0F);
	}
	else if ((p1 & 0x30) == 0x20) {
		u8 value = 0;
		value |= device.input.state[BUTTON_RIGHT] ? 0x01 : 0x00;
		value |= device.input.state[BUTTON_LEFT]  ? 0x02 : 0x00;
		value |= device.input.state[BUTTON_UP]    ? 0x04 : 0x00;
		value |= device.input.state[BUTTON_DOWN]  ? 0x08 : 0x00;
		p1 = (p1 & 0x30) | (~value & 0x0F);
	}
	else {
		p1 = (p1 & 0x30) | 0x0F;
	}
	device.io[IO_P1] = p1;

	if (device.cpu.stopped) {
		device.cpu.stopped = device.input.state == 0;
	}

	if (!device.cpu.stopped) {
		if (device.counter / 256) {
			device.io[IO_DIV]++;
		}
		if (device.io[IO_TAC] & 0x04) {
			int div;
			switch (device.io[IO_TAC] & 0x03) {
			case 0:
				div = 1024;
				break;
			case 1:
				div = 16;
				break;
			case 2:
				div = 64;
				break;
			case 3:
				div = 256;
				break;
			}
			if (device.counter % div == 0) {
				if (device.io[IO_TIMA] == 0xff) {
					device_request_interrupt(INT_TIMER);
					device.io[IO_TIMA] = device.io[IO_TMA];
				}
				else {
					device.io[IO_TIMA]++;
				}
			}
		}

		if (device.dma) {
			dma_transfer();
			device.dma = false;
		}

		audio_clock();
		cpu_clock();
		vpu_clock();
	}
	else {
		sdlgb_clear();
		SDL_Delay(10);
	}
}

void device_audio_callback(void* userdata, Uint8* buffer, int len) {
	SDL_AudioSpec* spec = (SDL_AudioSpec*)userdata;
	// TODO: Allow mono
	SDL_assert(spec->channels == 2);
	// TODO: Allow different formats
	SDL_assert(spec->format == AUDIO_F32);
	// NOTE: divider is cpu clocks per sample
	u32 divider = device.cpu.freq / spec->freq;
	for (int i = 0; i < spec->samples; i++) {
		// NOTE: device is synchronized to audio
		do {
			device_clock();
		} while (device.counter % divider != 0);

		u32 sample_idx = spec->channels * i;
		SDL_assert(sample_idx * sizeof(float) < len);
		float* sample = (float*)buffer + sample_idx;
		if (!device.muted) {
			// NOTE: return in range -1:1
			float sqr1[2] = {}, sqr2[2] = {}, wave[2] = {}, noise[2] = {};
			audio_dac_read(device.audio_samples.sqr1, sqr1);
			audio_dac_read(device.audio_samples.sqr2, sqr2);
			audio_dac_read(device.audio_samples.wave, wave);
			audio_dac_read(device.audio_samples.noise, noise);

			// NOTE: mix the channels
			float mixed[2] = {
				sqr1[0] + sqr2[0] + wave[0] + noise[0],
				sqr1[1] + sqr2[1] + wave[1] + noise[1],
			};
			sample[0] = audio_highpass(CLAMP(mixed[0], -4, 4) / 4.f, 0) * device.volume;
			sample[1] = audio_highpass(CLAMP(mixed[1], -4, 4) / 4.f, 0) * device.volume;
		}
		else {
			sample[0] = sample[1] = 0.f;
		}
	}
}