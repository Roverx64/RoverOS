# RoverOS
### About
RoverOS is a hobby OS developed by one person.  
It is certain to have bugs and oversights.  
This is an update from legacy x86 RoverOS and the x64 GRUB bootloader.  
RoverOS is named after my dog, and that is where my username comes from as well.  

### Planned for next update
Things I will try to add in the next update
* Apple TTF support
* PNG support
* New stack
* Cleaned up/Simplier makefile
* Cleaned up directory structure
* More in-depth fault handlers

### Planned features
These are things I will likely add in the future
* Multitasking
* Networking
* Custom libc (Likely not POSIX complaint)
* Auto makefile config
* ARM (aarch64) support, raspberrypi 3 will be the intended platform
* Proper UI implementation over legacy x86 RoverOS

### Unsupported hardware
* Non UEFI machines
* CSM only UEFI machines
* Non x64 (64 bit) CPUs
* ARM CPUs (for now)

### Issues
* Shift key acts as an enter key on real hardware
* Aligning .text to 2MB causes the kernel to not call other funcs properly
* dumpPMAP/dumpEFI_PMAP command stops reporting accurate info with higher addresses

### x86_64 Requirements
* `gcc` or an x86_64 cross compiler in `/home/your_username/opt/cross/bin`
* `ld`
* `ovmf` (I suggest looking up how to get ovmf for your distro)
* `Make`
* `nasm`
* `mtools`
* `xorriso`
* `gnu-efi` (This will have to be compiled from source)
* `qemu-system-x86_64` (x86_64 version)

### ARM Requirements
* `arm-none-eabi-as`
* `arm-none-eabi-ld`
* `Make`
* `qemu-system-aarch64`

# Compiling
#### Setting vars
Do not include the '' or "" unless specified and avoid adding a space or / after a variable  
**Correct:** `EXMAPLE_VAR = /path/to/something` **Incorrect:** `EXMAPLE_VAR = /path/to/something/ `  
* set `GNU_EFI_PATH` to the absolute path of the gnu-efi source directory  
Example: `GNU_EFI_PATH = /home/rover/Documents/gnu-efi`
* set `OVMF_PATH` to where your OVMF.fd file is (This is likely to be in `/usr/share/qemu`)  
Example: `OVMF_PATH = /use/share/qemu`  
Note: You may have to chown this file to allow Qemu to use it
* set `TARGET` to desired arch (x86_64, aarch64)
#### Gnu-efi (x86_64 only)
* cd to your Documents folder
* download gnu-efi with `git clone https://git.code.sf.net/p/gnu-efi/code gnu-efi`
* cd into gnu-efi and run `make`
#### RoverOS
1. Ensure all vars are set as required above
2. cd to the base of the RoverOS folder (where the Makefile is)
3. run `make required` to copy all needed files from gnu-efi
4. run `make` to compile and run the EFI application

# Real hardware (Empty FAT32 Partiton needed)
#### Requirements
* GPT Formatted HDD
* 30 MB FAT32 Partition
* GRUB Bootloader
#### Issues
* Allocating pages with AllocatePages() may freeze the bootloader
#### Setting vars
* set `USE_HDD` to `1`
* set `HDD_PATH` to a FAT32 partition and ensure the partition is mounted  
Example: `HDD_PATH = /media/rover/C4D3-DC2C`
#### Compiling
* run `make` or `make hdw` if RoverOS has already been compiled
#### GRUB
* Put the following in your `40_custom` grub file.
* set the root to your FAT32 partition 
* hd# where # is the HDD number, gpt# where # is the partition number
```
menuentry "RoverOS" {
        set root='hd#,gpt#'
        chainloader /RoverOS.efi
}
```
This file is at `/etc/grub.d/` for Ubuntu  
1. update grub config via command (Search up how to do it for your distro)  
2. Reboot and the option 'RoverOS' should be there

# Misc
* `make reset` cleans the directory to the default state
* `make check` checks for compilers and OVMF
