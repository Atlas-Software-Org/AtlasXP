/* Tell the linker that we want an x86_64 ELF64 output file */
OUTPUT_FORMAT(elf64-x86-64)

/* We want the symbol kmain to be our entry point */
ENTRY(KiStartupInit)

/* Define the program headers we want so the bootloader gives us the right */
/* MMU permissions; this also allows us to exert more control over the linking */
/* process. */
PHDRS
{
    limine_requests PT_LOAD;
    text PT_LOAD;
    rodata PT_LOAD;
    data PT_LOAD;
}

SECTIONS
{
    /* We want to be placed in the topmost 2GiB of the address space, for optimisations */
    /* and because that is what the Limine spec mandates. */
    /* Any address in this region will do, but often 0xffffffff80000000 is chosen as */
    /* that is the beginning of the region. */
    . = 0xffffffff80000000;

    /* Define a section to contain the Limine requests and assign it to its own PHDR */
    .limine_requests : {
        KEEP(*(.limine_requests_start))
        KEEP(*(.limine_requests))
        KEEP(*(.limine_requests_end))
    } :limine_requests

    /* Move to the next memory page for .text */
    . = ALIGN(CONSTANT(MAXPAGESIZE));

    .text : {
        *(.text .text.*)
    } :text

    /* Move to the next memory page for .rodata */
    . = ALIGN(CONSTANT(MAXPAGESIZE));

    .rodata : {
        *(.rodata .rodata.*)
    } :rodata

    /* Move to the next memory page for .data */
    . = ALIGN(CONSTANT(MAXPAGESIZE));

    .data : {
        *(.data .data.*)
    } :data

    /* NOTE: .bss needs to be the last thing mapped to :data, otherwise lots of */
    /* unnecessary zeros will be written to the binary. */
    /* If you need, for example, .init_array and .fini_array, those should be placed */
    /* above this. */
	.bss : ALIGN(0x1000) {
	    *(.bss .bss.*);
	    *(COMMON);
	} :data

	VirtualEndKernel = ALIGN(0x1000);

    /* Discard .note.* and .eh_frame* since they may cause issues on some hosts. */
    /DISCARD/ : {
        *(.debug_info .debug_info*)
        *(.debug_abbrev .debug_abbrev.*)
        *(.debug_line .debug_line.*)
        *(.debug_str .debug_str.*)
        *(.debug_aranges .debug_aranges.*)
        *(.debug_ranges .debug_ranges.*)
        *(.debug_loc .debug_loc.*)
        *(.debug_macinfo .debug_macinfo.*)
        *(.debug_macro .debug_macro.*)
        *(.debug_pubnames .debug_pubnames.*)
        *(.debug_pubtypes .debug_pubtypes.*)
        *(.debug_frame .debug_frame.*)
        *(.eh_frame .eh_frame.*)
        *(.debug_addr .debug_addr.*)
        *(.debug_cu_index .debug_cu_index.*)
        *(.debug_tu_index .debug_tu_index.*)
        *(.gdb_index .gdb_index.*)
        *(.note .note.*)
    }

    a_endKernel = .;
    a_endKernelA = ALIGN(4096);
}
