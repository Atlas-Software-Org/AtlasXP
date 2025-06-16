#include "elf.h"

void LoadKernelElf(void* elf_data) {
    Elf64_Ehdr* ehdr = (Elf64_Ehdr*)elf_data;

    if (ehdr->e_ident[0] != 0x7F || ehdr->e_ident[1] != 'E' ||
        ehdr->e_ident[2] != 'L' || ehdr->e_ident[3] != 'F') {
        printk("{ ELF Error } Not an ELF executable\n");
        return;
    }

    Elf64_Phdr* phdrs = (Elf64_Phdr*)((uint8_t*)elf_data + ehdr->e_phoff);

    for (int i = 0; i < ehdr->e_phnum; i++) {
        Elf64_Phdr* ph = &phdrs[i];
        if (ph->p_type != 1) continue; // PT_LOAD

        uint64_t file_base = (uint64_t)elf_data + ph->p_offset;
        uint64_t vaddr = ph->p_vaddr;
        uint64_t memsz = ph->p_memsz;
        uint64_t filesz = ph->p_filesz;

        printk("vaddr: 0x%016x\n", ph->p_vaddr);

        for (uint64_t off = 0; off < memsz; off += 0x1000) {
            uint64_t virt = vaddr + off;
            uint64_t phys = virt;

            if (virt < 0x400000 || virt > 0x500000) {
                printk("Bad virtual address { 0x%016x }\n", virt);
                continue;
            }

            printk("Mapping 0x%016x", virt);

            KiUMap(virt);
            KiMMap(virt, phys, MMAP_PRESENT | MMAP_RW);

            uint64_t copy_size = 0;
            if (off < filesz) {
                copy_size = (filesz - off >= 0x1000) ? 0x1000 : (filesz - off);
                memcpy((void*)virt, (void*)(file_base + off), copy_size);
            }

            if (copy_size < 0x1000) {
                memset((void*)(virt + copy_size), 0, 0x1000 - copy_size);
            }
        }
    }

    void (*entry)() = (void (*)())ehdr->e_entry;
    entry();
}