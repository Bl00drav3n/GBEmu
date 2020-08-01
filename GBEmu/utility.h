struct timer_t {
	u32 init;
	u32 value;
};

struct sequencer_t {
	timer_t timer;
	u8 frames;
	u8 step;
};

#if _DEBUG_MEASURE_TIMINGS
#define MEASURE_TIMING(TIMER) {\
	static u32 counter = device.counter;\
	u32 delta = device.counter - counter;\
	fprintf(stdout, "%s: cycles: %6u - freq: %9.3fHz\n", TIMER, delta, (float)(4096 * 1024) / (float)delta);\
	counter = device.counter;\
}
#else
#define MEASURE_TIMING(TIMER)
#endif

#define MINIMUM(a, b) ((a) < (b) ? (a) : (b))
#define MAXIMUM(a, b) ((a) > (b) ? (a) : (b))
#define CLAMP(x, a, b) MINIMUM(MAXIMUM(x, a), b)