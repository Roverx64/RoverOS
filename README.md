# RoverOS

### About
RoverOS is a hobby OS developed by one person.  
It is certain to have bugs and oversights.  
This is an update from legacy x86 RoverOS and the x64 GRUB bootloader.  
RoverOS is named after my dog, and that is where my username comes from as well.  

### Planned for next update
Things I will try to add in the next update
* Buddy & SLAB memory allocator
* Barebones XHCI support
* MSI/X support

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

### Screenshot (x86_64)
![RoverOS-UEFI](https://user-images.githubusercontent.com/111342077/232359539-c0e2b337-8fb0-42e7-8936-c9a702981144.png)

---
### Requirements
Things needed for both architectures
* `gcc` or an x86_64 cross compiler in `/home/your_username/opt/cross/bin`
* `ld`
* `Make`

### x86_64 requirements
Things needed for only the AMD64 architecture
* `ovmf` (I suggest looking up how to get ovmf for your distro)
* `nasm`
* `mtools`
* `xorriso`
* `gnu-efi` (https://github.com/vathpela/gnu-efi is the default repo)
* `qemu-system-x86_64`

### ARM requirements
Things needed for only the ARM64 architecture
* `arm-none-eabi-as`
* `arm-none-eabi-ld`
* `qemu-system-aarch64`

### Other make options
Add to the makefile arguments to use them `make arch=DESIRED_ARCH <arg>=<value>`
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
3. run `make arch=DESIRED_ARCH` to compile and run the EFI application
* `Replace 'DESIRED_ARCH' with either 'x86_64' or 'aarch64'. It is case sensitive`

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
* hd# where # is the HDD number, gpt# where # is the partition number
```
menuentry "RoverOS" {
        set root='hd#,gpt#'
        chainloader /ROSBootloader.efi
}
```
This file is at `/etc/grub.d/` for Ubuntu  
1. update grub config via command (Search up how to do it for your distro)  
2. Reboot and the option 'RoverOS' should be there

# Misc
* `make reset` cleans the directory to the default state
* `make check` checks for compilers and OVMF
