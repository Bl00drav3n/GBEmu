void audio_enable_channel(int channel) {
	SDL_assert(channel < AUDIO_CHANNEL_COUNT);
	if (channel < AUDIO_CHANNEL_COUNT) {
		device.io[IO_NR52] |= 1 << channel;
	}
}

void audio_disable_channel(int channel) {
	SDL_assert(channel < AUDIO_CHANNEL_COUNT);
	if (channel < AUDIO_CHANNEL_COUNT) {
		u8 mask = 1 << channel;
		device.io[IO_NR52] = device.io[IO_NR52] & ~mask;
	}
}

bool audio_enabled() {
	return device.io[IO_NR52] & 0x80;
}

bool audio_channel_enabled(int channel) {
	SDL_assert(channel < AUDIO_CHANNEL_COUNT);
	if (channel < AUDIO_CHANNEL_COUNT) {
		if (device.io[IO_NR52] & (1 << channel)) {
			switch (channel) {
			case AUDIO_SQUARE1:
				return device.io[IO_NR12] & 0xF8;
			case AUDIO_SQUARE2:
				return device.io[IO_NR22] & 0xF8;
			case AUDIO_WAVE:
				return device.io[IO_NR30] & 0x80;
			case AUDIO_NOISE:
				return device.io[IO_NR42] & 0xF8;
			}
		}
	}
	return false;
}

u8 noise_get_clock_shift() {
	return (device.io[IO_NR43] & 0xF0) >> 4;
}

u8 noise_get_width_mode() {
	return device.io[IO_NR43] & 0x08;
}

u8 noise_get_divider() {
	switch (device.io[IO_NR43] & 0x07) {
	case 0: return   8;
	case 1: return  16;
	case 2: return  32;
	case 3: return  48;
	case 4: return  64;
	case 5: return  80;
	case 6: return  96;
	case 7: return 112;
	}
	return 0;
}

u8 wave_get_volume_shift() {
	switch (device.io[IO_NR32] & 0x60) {
	case 0x00: return 4;
	case 0x20: return 0;
	case 0x40: return 1;
	case 0x60: return 2;
	}
	return 0;
}

u8 sweep_get_shift() {
	return device.io[IO_NR10] & 0x07;
}

u8 sweep_get_period() {
	u8 period = (device.io[IO_NR10] & 0x70) >> 4;
	return period;
}

u8 sweep_get_mode() {
	// NOTE: 0 - add, !0 - sub
	return device.io[IO_NR10] & 0x08;
}

u16 sweep_calculate_freq() {
	u16 freq = device.sweep.freq >> sweep_get_shift();
	if (sweep_get_mode()) {
		return device.sweep.freq - freq;
	}
	else {
		return device.sweep.freq + freq;
	}
}

bool sweep_overflow_check(u16 freq) {
	bool result = false;
	if (freq > 2047) {
		audio_disable_channel(AUDIO_SQUARE1);
		result = true;
	}
	return result;
}

u8* envelope_get_ctrl_register(u8 channel) {
	SDL_assert(channel < AUDIO_CHANNEL_COUNT && channel != AUDIO_WAVE);
	if (channel < AUDIO_CHANNEL_COUNT && channel != AUDIO_WAVE) {
		switch (channel) {
		case AUDIO_SQUARE1: return &device.io[IO_NR12];
		case AUDIO_SQUARE2: return &device.io[IO_NR22];
		case AUDIO_NOISE: return &device.io[IO_NR42];
		}
	}
	return 0;
}

u8 envelope_get_mode(u8 channel) {
	u8 result = 0;
	SDL_assert(channel < AUDIO_CHANNEL_COUNT && channel != AUDIO_WAVE);
	if (channel < AUDIO_CHANNEL_COUNT && channel != AUDIO_WAVE) {
		u8* reg = envelope_get_ctrl_register(channel);
		if (reg) {
			result = (*reg) & 0x08;
		}
	}
	return result;
}

u8 envelope_get_period(u8 channel) {
	u8 result = 0;
	SDL_assert(channel < AUDIO_CHANNEL_COUNT && channel != AUDIO_WAVE);
	if (channel < AUDIO_CHANNEL_COUNT && channel != AUDIO_WAVE) {
		u8* reg = envelope_get_ctrl_register(channel);
		if (reg) {
			result = (*reg) & 0x07;
		}
	}
	return result;
}

void envelope_reload_volume(u8 channel) {
	u8 result = 0;
	SDL_assert(channel < AUDIO_CHANNEL_COUNT && channel != AUDIO_WAVE);
	if (channel < AUDIO_CHANNEL_COUNT && channel != AUDIO_WAVE) {
		u8* reg = envelope_get_ctrl_register(channel);
		if (reg) {
			device.channel[channel].envelope.volume = ((*reg) & 0xF0) >> 4;
		}
	}
}

void envelope_update(u8 channel) {
	// TODO: Noise channel seems to missbehave?
	SDL_assert(channel < AUDIO_CHANNEL_COUNT && channel != AUDIO_WAVE);
	if (channel < AUDIO_CHANNEL_COUNT && channel != AUDIO_WAVE) {
		if (device.channel[channel].envelope.can_update) {
			u8 volume = device.channel[channel].envelope.volume;
			if (envelope_get_mode(channel)) {
				volume++;
			}
			else {
				volume--;
			}
			if (volume < 16) {	
				device.channel[channel].envelope.volume = volume;
			}
			else {
				device.channel[channel].envelope.can_update = false;
			}
		}
	}
}

void audio_channel_update_freq(int channel) {
	SDL_assert(channel >= 0 && channel < AUDIO_CHANNEL_COUNT);
	if (channel >= 0 && channel < AUDIO_CHANNEL_COUNT) {
		u16 freq = device.channel[channel].freq;
		switch (channel) {
		case AUDIO_SQUARE1:
		case AUDIO_SQUARE2:
			timer_freq(&device.channel[channel].pwm.timer, (2048 - freq) << 2, false);
			break;
		case AUDIO_WAVE:
			timer_freq(&device.channel[channel].wave.pos.timer, (2048 - freq) << 1, false);
			break;
		case AUDIO_NOISE:
			freq = noise_get_divider() << noise_get_clock_shift();
			timer_freq(&device.channel[AUDIO_NOISE].noise.timer, freq, false);
			break;
		}
	}
}

void audio_trigger_channel(int channel) {
	SDL_assert(channel < AUDIO_CHANNEL_COUNT);
	if (channel < AUDIO_CHANNEL_COUNT) {
		audio_enable_channel(channel);
		if (device.channel[channel].len == 0) {
			device.channel[channel].len = (channel == AUDIO_WAVE) ? 256 : 64;
		}
		audio_channel_update_freq(channel);
		switch (channel) {
		case AUDIO_SQUARE1:
		{
			device.sweep.freq = device.channel[AUDIO_SQUARE1].freq;
			device.sweep.enabled = sweep_get_shift() | sweep_get_period();
			timer_freq(&device.sweep.timer, sweep_get_period(), true);
			if (sweep_get_shift()) {
				sweep_overflow_check(sweep_calculate_freq());
			}
		} break;
		case AUDIO_SQUARE2:
			break;
		case AUDIO_WAVE:
			device.channel[AUDIO_WAVE].wave.pos.step = 0;
			break;
		case AUDIO_NOISE:
			device.channel[AUDIO_NOISE].noise.reg = 0x7FFF;
			break;
		}
		
		if (channel != AUDIO_WAVE) {
			device.channel[channel].envelope.can_update = true;
			timer_freq(&device.channel[channel].envelope.period, envelope_get_period(channel), false);
			envelope_reload_volume(channel);
		}
	}
}


u8 audio_get_duty_cycle(int channel) {
	SDL_assert(channel == AUDIO_SQUARE1 || channel == AUDIO_SQUARE2);
	u8* NRx1 = (channel == AUDIO_SQUARE1) ? &device.io[IO_NR11] : &device.io[IO_NR21];
	u8 duty = ((*NRx1) & 0xC0) >> 6;
	SDL_assert(duty < 4);
	return duty;
}

bool audio_len_ctr_enabled(int channel) {
	SDL_assert(channel < AUDIO_CHANNEL_COUNT);
	switch (channel) {
	case AUDIO_SQUARE1:
		return device.io[IO_NR14] & 0x40;
	case AUDIO_SQUARE2:
		return device.io[IO_NR24] & 0x40;
	case AUDIO_WAVE:
		return device.io[IO_NR34] & 0x40;
	case AUDIO_NOISE:
		return device.io[IO_NR44] & 0x40;
	}
	return false;
}

void audio_clock_sweep() {
	MEASURE_TIMING("SWP");
	if (timer_clock(&device.sweep.timer) && device.sweep.enabled && sweep_get_period()) {
		u16 freq = sweep_calculate_freq();
		if (!sweep_overflow_check(freq) && sweep_get_shift()) {
			device.sweep.freq = freq;
			device.channel[AUDIO_SQUARE1].freq = freq;
			audio_channel_update_freq(AUDIO_SQUARE1);
			sweep_overflow_check(sweep_calculate_freq());
		}
	}
}

void audio_clock_len_ctr() {
	MEASURE_TIMING("LEN");
	for (int i = 0; i < 4; i++) {
		if (audio_len_ctr_enabled(i) && device.channel[i].len) {
			if (--device.channel[i].len == 0) {
				audio_disable_channel(i);
			}
		}
	}
}

void audio_clock_vol_env() {
	MEASURE_TIMING("ENV");
	audio_channel_type_t channels[3] = { AUDIO_SQUARE1, AUDIO_SQUARE2, AUDIO_NOISE };
	for (int i = 0; i < 3; i++) {
		audio_channel_type_t channel = channels[i];
		if (timer_clock(&device.channel[channel].envelope.period) && envelope_get_period(channel)) {
			envelope_update(channel);
		}
	}
}

float audio_highpass(float in, int idx)
{
	// TODO: this thing is ugly garbage
	SDL_assert(idx >= 0 && idx < 2);
	static float capacitor[2] = {};
	float out = in - capacitor[idx];
	// TODO: Precalculate depending on sample rate
	// capacitor slowly charges to 'in' via their difference
	capacitor[idx] = in - out * 0.99601330891084907605152232417604f; // 44100Hz
	//capacitor[idx] = in - out * 0.97872512056998715043213287116672f; // 8192Hz
	return out;
}

void audio_dac_read(u8 samples_in[2], float samples_out[2]) {
	samples_out[0] = -1.0f + 2.0f * ((float)samples_in[0] / 15.0f);
	samples_out[1] = -1.0f + 2.0f * ((float)samples_in[1] / 15.0f);
}

void audio_clock_sqr_wave_pwm(int channel) {
	SDL_assert(channel == AUDIO_SQUARE1 || channel == AUDIO_SQUARE2);
	sequencer_t* pwm = &device.channel[channel].pwm;
	if (timer_clock(&pwm->timer)) {
		sequencer_next(pwm);
	}
	u8 *sample = (channel == AUDIO_SQUARE1) ? device.audio_samples.sqr1 : device.audio_samples.sqr2;
	sample[0] = sample[1] = 0x00;
	if (device.channel[channel].enable_override && audio_channel_enabled(channel)) {
		u8 duty = audio_get_duty_cycle(channel);
		u8 mask[4] = { 0x01, 0x81, 0x87, 0x7E };
		u8 output = (mask[duty] & (0x80 >> pwm->step)) ? device.channel[channel].envelope.volume : 0x00;
		if (device.io[IO_NR51] & (0x01 << channel)) {
			sample[0] = output;
		}
		if (device.io[IO_NR51] & (0x10 << channel)) {
			sample[1] = output;
		}
		device.channel[channel].envelope.volume;
	}
	else {
		audio_disable_channel(channel);
	}
}

void audio_clock_wave() {
	if (timer_clock(&device.channel[AUDIO_WAVE].wave.pos.timer)) {
		sequencer_next(&device.channel[AUDIO_WAVE].wave.pos);
	}
	u8 *sample_out = device.audio_samples.wave;
	sample_out[0] = sample_out[1] = 0x00;
	if (device.channel[AUDIO_WAVE].enable_override && audio_channel_enabled(AUDIO_WAVE)) {
		u8 sample = device.channel[AUDIO_WAVE].wave.sample >> wave_get_volume_shift();
		if (device.io[IO_NR51] & 0x04) {
			sample_out[0] = sample;
		}
		if (device.io[IO_NR51] & 0x40) {
			sample_out[1] = sample;
		}
		u8 idx = device.channel[AUDIO_WAVE].wave.pos.step >> 1;
		u8 sample_pair = device.channel[AUDIO_WAVE].wave.data[idx];
		switch (device.channel[AUDIO_WAVE].wave.pos.step % 2) {
		case 0:
			device.channel[AUDIO_WAVE].wave.sample = sample_pair >> 4;
			break;
		case 1:
			device.channel[AUDIO_WAVE].wave.sample = sample_pair & 0x0F;
			break;
		}
	}
	else {
		audio_disable_channel(AUDIO_WAVE);
	}
}

void audio_clock_noise() {
	if (timer_clock(&device.channel[AUDIO_NOISE].noise.timer)) {
		u16 value = device.channel[AUDIO_NOISE].noise.reg & 0x7FFF;
		u16 bit = (value & 0x01) ^ ((value & 0x02) >> 1);
		value = (value >> 1) | (bit << 14);
		if (noise_get_width_mode()) {
			value = (value & ~0x0040) | (bit << 6);
		}
		device.channel[AUDIO_NOISE].noise.reg = value;
	}
	u8 *sample_out = device.audio_samples.noise;
	sample_out[0] = sample_out[1] = 0x00;
	if (device.channel[AUDIO_NOISE].enable_override && audio_channel_enabled(AUDIO_NOISE)) {
		u8 sample = ((~device.channel[AUDIO_NOISE].noise.reg) & 0x01) ? device.channel[AUDIO_NOISE].envelope.volume : 0x00;
		if (device.io[IO_NR51] & 0x08) {
			sample_out[0] = sample;
		}
		if (device.io[IO_NR51] & 0x80) {
			sample_out[1] = sample;
		}
	}
	else {
		audio_disable_channel(AUDIO_NOISE);
	}
}

void audio_master_enable() {
	device.io[IO_NR52] |= 0x80;
	device.audio_seq.step = 0;
	device.channel[AUDIO_SQUARE1].pwm.step = 0;
	device.channel[AUDIO_SQUARE2].pwm.step = 0;
	device.channel[AUDIO_WAVE].wave.pos.step = 0;
}

void audio_master_disable() {
	device.io[IO_NR52] = 0x00;
}

void audio_clock() {
	bool frame_tick = timer_clock(&device.audio_seq.timer);
	if (audio_enabled()) {
		if (frame_tick) {
			MEASURE_TIMING("SEQ");
			switch (device.audio_seq.step) {
			case 0:
				audio_clock_len_ctr();
				break;
			case 2:
				audio_clock_len_ctr();
				audio_clock_sweep();
				break;
			case 4:
				audio_clock_len_ctr();
				break;
			case 6:
				audio_clock_len_ctr();
				audio_clock_sweep();
				break;
			case 7:
				audio_clock_vol_env();
				break;
			}
			sequencer_next(&device.audio_seq);
		}
		audio_clock_sqr_wave_pwm(AUDIO_SQUARE1);
		audio_clock_sqr_wave_pwm(AUDIO_SQUARE2);
		audio_clock_wave();
		audio_clock_noise();
	}
}