#include "elf.h"

int CurrentPid = 1;
/*
 *  If current PID is 1 then its the kernel idle process
 *  PID != 1 && > 1 is a subprocess PID
 */

static void* LoadedPages[4096];
static int LoadedPageCount = 0;

void* SignalBuffer = NULL;
const void* SignalBufferAddress = 0x100000;

#define ET_REL 0x0001U

void LoadKernelRelocatableElf(void* elf_data) {
    Elf64_Ehdr* ehdr = (Elf64_Ehdr*)elf_data;

    if (ehdr->e_ident[0] != 0x7F || ehdr->e_ident[1] != 'E' ||
        ehdr->e_ident[2] != 'L' || ehdr->e_ident[3] != 'F') {
        printk("{ ELF Error } Not an ELF executable\n\r");
        return;
    }

    if (ehdr->e_type != ET_REL) {
        printk("{ ELF Error } Not a relocatable ELF\n\r");
        return;
    }

    Elf64_Shdr* section_table = (Elf64_Shdr*)((uint64_t)elf_data + ehdr->e_shoff);
    Elf64_Shdr* shstrtab_hdr = &section_table[ehdr->e_shstrndx];
    const char* shstrtab = (const char*)elf_data + shstrtab_hdr->sh_offset;

    Elf64_Shdr* symtab_hdr = NULL;
    Elf64_Sym* symtab = NULL;
    size_t symcount = 0;

    void* section_bases[ehdr->e_shnum];
    memset(section_bases, 0, sizeof(section_bases));

	int entry_found = 0;

	for (int i = 0; i < ehdr->e_shnum; i++) {
	    Elf64_Shdr* sh = &section_table[i];

		const char* name = shstrtab + sh->sh_name;
		if (!(sh->sh_flags & SHF_ALLOC) || sh->sh_size == 0 ||
		    strcmp(name, ".note.gnu.property") == 0) continue;

	    if (!(sh->sh_flags & SHF_ALLOC) || sh->sh_size == 0) continue;

	    uint8_t* target = KiPmmAlloc(); // allocate one page or more
	    if (target == NULL) {
	        printk("{ ELF Error } Failed to allocate memory for section %d\n\r", i);
	        return;
	    }

	    section_bases[i] = target;

	    if (sh->sh_type == SHT_NOBITS) {
	        memset(target, 0, sh->sh_size);
	    } else {
	        memcpy(target, (uint8_t*)elf_data + sh->sh_offset, sh->sh_size);
	    }

	    KiMMap(target, target, MMAP_PRESENT | MMAP_RW);

	    printk("{ ELF Info } Mapped section %s to 0x%p (%lu bytes)\n\r", name, target, sh->sh_size);
	}

	if (!entry_found) {
	    printk("{ ELF Error } .text section not found — no entry to run\n\r");
	}

    if (!symtab_hdr || !symtab) {
        printk("{ ELF Error } No symbol table found\n\r");
        return;
    }

    for (int i = 0; i < ehdr->e_shnum; i++) {
        Elf64_Shdr* relsec = &section_table[i];

        if (relsec->sh_type != SHT_RELA) continue;

        Elf64_Shdr* target_sec = &section_table[relsec->sh_info];
        uint8_t* target_base = section_bases[relsec->sh_info];
        Elf64_Rela* relas = (Elf64_Rela*)((uint8_t*)elf_data + relsec->sh_offset);
        size_t rela_count = relsec->sh_size / sizeof(Elf64_Rela);

        for (size_t j = 0; j < rela_count; j++) {
            Elf64_Rela* rela = &relas[j];
            uint64_t* reloc_addr = (uint64_t*)(target_base + rela->r_offset);
            uint32_t sym_idx = ELF64_R_SYM(rela->r_info);
            uint32_t type = ELF64_R_TYPE(rela->r_info);

            if (sym_idx >= symcount) continue;

            Elf64_Sym* sym = &symtab[sym_idx];
            uint64_t S = 0;

            if (sym->st_shndx != SHN_UNDEF && sym->st_shndx < ehdr->e_shnum) {
                S = (uint64_t)section_bases[sym->st_shndx] + sym->st_value;
            }

            uint64_t A = rela->r_addend;
            uint64_t P = (uint64_t)reloc_addr;

            switch (type) {
                case R_X86_64_64:
                    *reloc_addr = S + A;
                    break;
                case R_X86_64_PC32:
                    *(uint32_t*)reloc_addr = (uint32_t)(S + A - P);
                    break;
                default:
                    printk("{ ELF Warning } Unsupported relocation type: %u\n\r", type);
                    break;
            }
        }
    }

    for (int i = 0; i < ehdr->e_shnum; i++) {
        const char* name = shstrtab + section_table[i].sh_name;
        if (strcmp(name, ".text") == 0) {
            void (*entry)(void*) = (void (*)(void*))((uint64_t)section_bases[i]);
            if (SignalBuffer == NULL) SignalBuffer = KiPmmAlloc();
            KiMMap(SignalBuffer, SignalBuffer, MMAP_PRESENT | MMAP_RW);
            *(int*)SignalBuffer = 0xDEADBEEF;
            entry(SignalBuffer);
            break;
        }
    }
}

void LoadKernelElf(void* elf_data, int argc, char** argv, char** envp) {
    Elf64_Ehdr* ehdr = (Elf64_Ehdr*)elf_data;

    if (ehdr->e_ident[0] != 0x7F || ehdr->e_ident[1] != 'E' ||
        ehdr->e_ident[2] != 'L' || ehdr->e_ident[3] != 'F') {
        printk("{ ELF Error } Not an ELF executable\n\r");
        return;
    }

	Elf64_Shdr* section_table = (Elf64_Shdr*)((uint64_t)elf_data + ehdr->e_shoff);
	Elf64_Shdr* shstrtab_hdr = &section_table[ehdr->e_shstrndx];
	const char* shstrtab = (const char*)elf_data + shstrtab_hdr->sh_offset;

	for (int i = 0; i < ehdr->e_shnum; i++) {
	    Elf64_Shdr* shdr_i = &section_table[i];
	    const char* name = shstrtab + shdr_i->sh_name;

	    printk("SECTION FOUND WITH NAME: %s\n\r", name);
	}

    Elf64_Phdr* phdrs = (Elf64_Phdr*)((uint8_t*)elf_data + ehdr->e_phoff);

	switch (ehdr->e_type) {
	    case ET_REL: {
	        int has_reloc = 0;
	        Elf64_Shdr* section_table = (Elf64_Shdr*)((uint64_t)elf_data + ehdr->e_shoff);
	        for (int i = 0; i < ehdr->e_shnum; i++) {
	            uint32_t shtype = section_table[i].sh_type;
	            if (shtype == SHT_REL || shtype == SHT_RELA) {
	                has_reloc = 1;
	                break;
	            }
	        }

	        if (!has_reloc) {
	            printk("{ ELF Warning } ET_REL file has no relocation sections\n\r");
	        }

	        printk("{ ELF Info } Loading relocatable ELF / UNAVAILABLE AS OF CURRENT VERSION\n\r");
	        //LoadKernelRelocatableElf(elf_data);
	        return;
	    }

	    case ET_EXEC:
	        printk("{ ELF Info } Loading statically linked executable\n\r");
	        break;

	    case ET_DYN:
	        if (ehdr->e_entry != 0) {
	            printk("{ ELF Info } Loading PIE executable (ET_DYN + entry)\n\r");
	        } else {
	            printk("{ ELF Info } Shared object detected (ET_DYN)\n\r");
	            printk("{ ELF Error } Shared objects not supported in this context\n\r");
	            return;
	        }
	        break;

	    case ET_CORE:
	        printk("{ ELF Error } Core dump file provided, unsupported\n\r");
	        return;

	    case ET_NONE:
	    default:
	        printk("{ ELF Error } Unknown or invalid ELF type: 0x%x\n\r", ehdr->e_type);
	        return;
	}

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

	if (SignalBuffer == NULL) SignalBuffer = SignalBufferAddress;

	KiMMap(SignalBuffer, SignalBuffer, MMAP_PRESENT | MMAP_RW);

	void (*entry)(void* sigBuf) = (void (*)(void*))ehdr->e_entry;
	*(int*)SignalBuffer = 0xDEADBEEF;

	entry(SignalBuffer);
}

/* USERMODE DOESNT WORK PROBABLY AS OF THIS VERSION... (ASNU (0.0.8-0.1.0))*/

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
            KiMMap((void*)virt, (void*)phys, MMAP_PRESENT | MMAP_RW | MMAP_USER);

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
