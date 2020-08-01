u8* load_file(const char* filename) {
	u8* result = 0;
	FILE* f = fopen(filename, "rb");
	if (f) {
		size_t len;
		fseek(f, 0L, SEEK_END);
		len = ftell(f);
		fseek(f, 0L, SEEK_SET);
		u8* data = (u8*)malloc(len);
		if (data) {
			if (fread(data, 1, len, f) == len) {
				fprintf(stdout, "File %s loaded\n", filename);
				result = data;
			}
			else {
				fprintf(stderr, "Error reading file %s\n", filename);
				free(data);
			}
		}
		else {
			fprintf(stderr, "Error allocating %lld bytes of memory for file %s\n", (uint64_t)len, filename);
		}
	}
	else {
		fprintf(stderr, "Error opening file %s\n", filename);
	}
	return result;
}

void timer_reset(timer_t* t) {
	t->value = t->init;
}

bool timer_clock(timer_t* t) {
	if (t->value == 0) {
		timer_reset(t);
		return true;
	}
	else {
		t->value--;
	}
	return false;
}

void timer_freq(timer_t* timer, u32 freq, bool reset = false) {
	timer->init = freq;
	if (reset)
		timer_reset(timer);
}

timer_t timer_create(u32 freq) {
	timer_t timer = {};
	timer_freq(&timer, freq, true);
	return timer;
}

sequencer_t sequencer_create(u32 freq, u8 frames) {
	sequencer_t seq = {};
	seq.timer = timer_create(freq);
	seq.frames = frames;
	return seq;
}

void sequencer_next(sequencer_t* seq) {
	seq->step = (seq->step + 1) % seq->frames;
}