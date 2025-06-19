#include "elf.h"

int CurrentPid = 1;
/*
 *  If current PID is 1 then its the kernel idle process
 *  PID != 1 && > 1 is a subprocess PID
 */

static void* LoadedPages[4096];
static int LoadedPageCount = 0;

void LoadKernelElf(void* elf_data, int argc, char** argv, char** envp) {
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

        for (uint64_t off = 0; off < memsz; off += 0x1000) {
            uint64_t virt = vaddr + off;
            uint64_t phys = virt;

            if (virt < 0x400000 || virt > 0x500000) {
                return;
            }


            KiUMap((void*)virt);
            KiMMap((void*)virt, (void*)phys, MMAP_PRESENT | MMAP_RW);

            if (LoadedPageCount < 4096) {
                LoadedPages[LoadedPageCount++] = (void*)virt;
            }

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

    (void)argc;
    (void)argv;
    (void)envp;

    void (*entry)() = (void (*)())ehdr->e_entry;
    entry();
}

void KiEnterUserMode(void (*Entry)) {
    __asm__ volatile (
        "movq %0, %%rax\n\t"
        "movq %%rsp, %%rcx\n\t"
        "swapgs\n\t"
        "movq %%rcx, %%rsp\n\t"
        "pushq $0x23\n\t"         // User data segment selector
        "pushq %%rcx\n\t"         // User mode stack pointer
        "pushfq\n\t"
        "pushq $0x1B\n\t"         // User code segment selector
        "pushq %0\n\t"            // Entry point
        "iretq\n\t"
        :
        : "r"(Entry)
        : "rax", "rcx"
    );
}

void LoadUserElf(void* elf_data, int argc, char** argv, char** envp) {
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

        for (uint64_t off = 0; off < memsz; off += 0x1000) {
            uint64_t virt = vaddr + off;
            uint64_t phys = virt;

            if (virt < 0x400000 || virt > 0x500000) {
                return;
            }


            KiUMap((void*)virt);
            KiMMap((void*)virt, (void*)phys, MMAP_PRESENT | MMAP_RW);

            if (LoadedPageCount < 4096) {
                LoadedPages[LoadedPageCount++] = (void*)virt;
            }

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

    (void)argc;
    (void)argv;
    (void)envp;

    void (*entry)() = (void (*)())ehdr->e_entry;

    KiEnterUserMode(entry);
}

void UnloadKernelElf() {
    for (int i = 0; i < LoadedPageCount; i++) {
        KiUMap(LoadedPages[i]);
        LoadedPages[i] = NULL;
        
    }
    LoadedPageCount = 0;
}

void UnloadUserElf() {
	UnloadKernelElf();
}
