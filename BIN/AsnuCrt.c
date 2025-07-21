#include "LibAsnuApi/AsnuApi.h"

extern int main(int argc, char** argv, char** envp);

void* gSignalBuffer;

void _start(void* SignalBuffer) {
	gSignalBuffer = SignalBuffer;
    asnu_api_exit(main(0, 0, 0));
}
