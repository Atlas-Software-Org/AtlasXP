#include "../PIT.h"

static volatile uint64_t PitTicks = 0;

__attribute__((interrupt)) void KiPitHandler(int *__unused) {
	(void)__unused;

	PitTicks++;

	KiPicSendEoi(0);
}

static volatile uint64_t PitTickFreq = 0;

void PitInit(uint32_t Freq) {
	uint32_t divisor = PIT_FREQ / Freq;
	outb(PIT_CMD_PORT, 0x36);
	outb(PIT_CHANNEL0_PORT, (uint8_t)(divisor & 0xFF));
	outb(PIT_CHANNEL0_PORT, (uint8_t)((divisor >> 8) & 0xFF));
	PitTickFreq = Freq;
}

uint64_t PitGetTicks() {
	return PitTicks;
}

uint64_t PitWaitTicks(uint64_t Ticks) {
	uint64_t start = PitGetTicks();
	while ((PitGetTicks() - start) < Ticks);
	return PitGetTicks();
}

void PitWaitMs(uint64_t Ms) {
	uint64_t ticks = (Ms * PitTickFreq) / 1000;
	PitWaitTicks(ticks);
}
