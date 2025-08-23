#!/bin/bash

# Start QEMU in background
qemu-system-aarch64 -M raspi4b -serial null -serial stdio -d int,cpu_reset,guest_errors,unimp -kernel kernel8.img -s -S &
QEMU_PID=$!

# Kill QEMU when script exits
trap "kill $QEMU_PID 2>/dev/null" EXIT

# Wait a sec for QEMU to start
sleep 1

# Start GDB in new terminal window
kitty arm-gnu-toolchain/bin/aarch64-none-elf-gdb -ex "target remote localhost:1234" -ex "tui enable" build/kernel8.elf

# Keep script running until Ctrl+C
wait $QEMU_PID