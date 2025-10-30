#!/bin/bash
# Terminal 1: Start QEMU with -S (wait for GDB)
kitty qemu-system-aarch64 -M raspi4b -serial null -serial stdio \
  -semihosting -semihosting-config enable=on,target=native \
  -kernel kernel8.img -s -S &

QEMU_PID=$!
trap "kill $QEMU_PID 2>/dev/null" EXIT

sleep 1

# Terminal 2: Start GDB
kitty arm-gnu-toolchain/bin/aarch64-none-elf-gdb \
  -ex "target remote localhost:1234" \
  -ex "add-symbol-file build/user_debug.elf 0x0"\
  # -ex "tui enable" \
  build/kernel8.elf &

wait

# add-symbol-file build/user_debug.elf 0x0
# x/i 0xffff000000081068