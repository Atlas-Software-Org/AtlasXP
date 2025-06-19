# AtlasOS64
Currently at version 0.0.7 (REWRITE) Atlas supports a functional bitmap allocator ({SRC}/PMM/pmm.c), a simple log interface, support for flanterm natively, cleaner code than the older versions, full support for printk [A printf / sprintf Implementation for Embedded Systems](https://github.com/mpaland/printf), and we are planning to add more features, such as full ELF support.

# How to configure the OS before compilation?

To configure Atlas before compilation you need to execute the shell script `configure` in the root directory of the OS workspace:
```bash
./configure
```

After you configure Atlas you need to check for ftctx_setup.c using:
```bash
ls -lh ./kernel/src/flanterm/ftctx_setup.c
```

If the file exists then you can proceed to compile and run Atlas using the following command:
```bash
make run
```

If the file doesnt exist you need to create it using:
```bash
touch ./kernel/src/flanterm/ftctx_setup.c
```
Then you need to put this code in it:
```c
#include "flanterm.h"

struct flanterm_context* global_ft_ctx;

void SetGlobalFtCtx(struct flanterm_context *ctx) {
    global_ft_ctx = ctx;
}

struct flanterm_context *GetGlobalFtCtx() {
    return global_ft_ctx;
}
```

Voila Atlas is prepared for compilation (with ftctx_setup.c prepared manually), you can run it with the same command as written above

# Atlas Software & Microsystems
