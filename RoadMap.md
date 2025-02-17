# AtlasOS Development Roadmap

## Phase 1: Core Setup
- [x] **Kernel Creation**: Develop a minimal kernel capable of basic booting, process management, and setting up interrupt handling.
- [x] **Bootloader Integration**: Finalize integration with Limine for smooth booting into AtlasOS.
- [ ] **Basic Memory Management**: Set up a basic memory manager, starting with simple paging or segmentation.
- [ ] **CLI (AtlasControl)**: Build the command-line interface with basic commands like `Echo`, `Ls`, and `Cat`.

## Phase 2: Stability and Basic Features
- [ ] **Process Scheduling**: Add basic process management with simple multitasking.
- [ ] **Kernel and User Space Separation**: Establish kernel mode vs user mode and handle system calls.
- [ ] **Memory Allocation**: Implement simple heap management or static memory allocation.
- [ ] **Keyboard and Display**: Implement text-based I/O with keyboard input and VGA/text output.

## Phase 3: Feature Expansion
- [ ] **File System (ATFS)**: Implement a basic file system with read and write capabilities. Start with simple files and no directories.
- [ ] **System Calls**: Add fundamental system calls like `Open`, `Close`, `Exit`, `CreateProcess`, and `WaitForProcess`.
- [ ] **File System Expansion**: Enhance ATFS with directories, file permissions, and more robust error handling.
- [ ] **CLI Enhancements**: Add more commands like `CD`, `MKDIR`, `RM`, and support for piping and redirection.

## Phase 4: User and Device Support
- [ ] **User Management**: Implement basic user management (user IDs, multi-user processes).
- [ ] **Peripheral Support**: Add support for additional devices like mouse input and storage devices.
- [x] **Advanced Display**: Implement framebuffer support for graphical display.
- [ ] **Error Handling**: Improve error handling across the system and add logs for debugging.

## Phase 5: Optimization and Usability
- [x] **Optimize Boot Time**: Reduce boot time, and improve kernel responsiveness and performance.
- [ ] **Stability Improvements**: Address stability issues like memory leaks, race conditions, and deadlocks.
- [ ] **Advanced Memory Management**: Implement advanced memory management techniques like virtual memory or paging.

## Phase 6: Testing, Documentation, and Feedback
- [ ] **Testing**: Perform extensive testing in various environments (QEMU, real hardware, etc.) and fix bugs.
- [ ] **Documentation**: Write developer documentation for the kernel, CLI, file system, and system calls.
- [ ] **Community Feedback**: Open source the project and gather feedback from the community to refine the system.

