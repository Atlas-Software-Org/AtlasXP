#pragma once

struct InterruptFrame;
__attribute__((interrupt)) void KeyboardInt_Hndlr(struct InterruptFrame* frame);