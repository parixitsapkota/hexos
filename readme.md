# HexOs

HexOs is a minimal, experimental x86 operating system built from scratch for learning and exploring low-level systems architecture, interrupt handling, and hardware control.

---

## 🛠️ Toolchain Dependencies

To compile and emulate HexOs, ensure your host machine has the following packages installed:

*   **Compiler & Linker:** `clang`, `lld` (LLVM Linker), `clang-format`, `llvm-objcopy`
*   **Build Automation:** `make`
*   **Image Management & Deployment:** `dd`, `fdisk`, `mtools`
*   **Emulation:** `qemu-system-x86_64`

---

## Building and Running

The build system utilizes LLVM toolchains and `make` to compile the kernel and generate a bootable disk image.

### Compile and run

```bash
make run
```
