# csxarch
IST 108: A 64-bit minicomputer instruction set architecture and emulator

About time this made it to GitHub so I can stop emailing myself zip files.
This is being completely redone as a Reduced Instruction Set Computer (RISC) architecture with the intention of implementing
it in hardware or FPGA. See here: https://docs.google.com/spreadsheets/d/1IfMkAmnkQNDln73CEYVMPOwn-ksKOTF6_v55dq888YE

## Build instructions
`make` (requires GNU make)

## Run
run `./obj/cpu108k path/to/ipl.bin`.

## Infrequently Asked Questions
### Q: Why are you using GNU make extensions? My system will only run pure POSIX make!
A: Do not attempt to run `cpu108k` on a bagel, please upgrade to at least a VAX-11 (1977) and install at least gmake 1.0
### Q: What do the blinkenlights mean?
A: In order:
- Program counter
- Current instruction
- Processor Status Word (see spreadsheet)
- Stack pointer
- Link register
- Selected general-purpose register, use up and down keys to select
- Current selection index
