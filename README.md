# RoverOS

### About
RoverOS is a hobby OS developed by one person that is certain to have some bugs and oversights.  
The 'Rover' in RoverOS is taken from my dogs name, and that is where my username comes from as well.
There is no specific goal of this OS/Kernel, it is just for fun and to learn more about OS internals.  

### Planned for next update
Things I will try to add in the next update
* Moving away from GNUEFI and onto my own UEFI abstraction and headers
* General frameworks for future updates (IE. SLAB, Buddy, AHCI, Mutlitasking)

### Planned features
These are things I will likely add/do in the near future
* AHCI support
* Generic USB device drivers
* Reduced amount of C files

### Unsupported hardware
* Non UEFI machines
* CSM only UEFI machines
* Non x64 (64 bit) CPUs
* Aarch32 only CPUs

---
### Requirements
Things needed for both architectures
* `gcc` or an x86_64 cross compiler in `/home/your_username/opt/cross/bin`
* `ld`
* `Make`

### x86_64 requirements
Tools needed for AMD64
* `ovmf` (I suggest looking up how to get ovmf for your distro)
* `nasm`
* `mtools`
* `xorriso`
* `gnu-efi` (https://github.com/vathpela/gnu-efi is the default repo)
* `qemu-system-x86_64`

### ARM requirements
Tools needed for Aarch64
* `arm-none-eabi-as`
* `arm-none-eabi-ld`
* `qemu-system-aarch64`
* `xorriso`

### Other make options
Add to the makefile arguments to use them `make arch=<target_arch> <arg>=<value>`
* `kvm` set to `1` to enable kvm emulator
* `hdd` set to `1` to enable copying to a real hdd
* `gdb` set to `1` to enable the use of gdb with qemu
---
# Compiling
#### Setting vars
Do not include the '' or "" unless specified and avoid adding a space or / after a variable  
##### Edit the `config.mk` file to set these vars
* set `OVMF_PATH` to where your OVMF.fd file is (This is likely to be in `/usr/share/qemu`)  
Example: `OVMF_PATH = /use/share/qemu`  
#### RoverOS
1. Ensure all vars are set as required above
2. cd to the base of the RoverOS folder (where the Makefile is)
3. run `make arch=<target_arch>` to compile and run the EFI application
* `Replace '<target_arch>' with either 'x86_64' or 'aarch64'. It is case sensitive`

# Real hardware (Empty FAT32 Partiton needed)
#### Requirements for AMD64
* GPT Formatted HDD
* 30 MB FAT32 Partition
* GRUB Bootloader
#### Requirements for Aarch64
* Unknown (I do not have a raspberrypi to test with yet)
#### Setting vars
* set `HDD_PATH` to a FAT32 partition and ensure the partition is mounted  
Example: `HDD_PATH = /media/rover/C4D3-DC2C`
#### Compiling
* run `make arch=DESIRED_ARCH hdd=1`
#### GRUB
* Put the following in your `40_custom` grub file.
* set the root to your FAT32 partition 
* hd# where # is the HDD/SSD number, gpt# where # is the partition number
```
menuentry "RoverOS" {
        set root='hd#,gpt#'
        chainloader /Polarboot.efi
}
```
This file is at `/etc/grub.d/` for Ubuntu  
1. update grub config via command (Search up how to do it for your distro)  
2. Reboot and the option 'RoverOS' should be there

# Misc
* `make reset` cleans the directory to the default state
* `make check` checks for compilers and OVMF
