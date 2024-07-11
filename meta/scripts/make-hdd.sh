# Create an empty zeroed-out 64MiB image file.
dd if=/dev/zero bs=1M count=0 seek=64 of=image.hdd

# Create a GPT partition table.
sgdisk image.hdd -n 1:2048 -t 1:ef00

# Download the latest Limine binary release for the 7.x branch.
git clone https://github.com/limine-bootloader/limine.git --branch=v7.x-binary --depth=1

# Build "limine" utility.
make -C limine

# Install the Limine BIOS stages onto the image.
./limine/limine bios-install image.hdd

# Format the image as fat32.
mformat -i image.hdd@@1M

# Make relevant subdirectories.
mmd -i image.hdd@@1M ::/EFI ::/EFI/BOOT ::/boot ::/boot/limine

# Copy over the relevant files.
mcopy -i image.hdd@@1M bin/myos ::/boot
mcopy -i image.hdd@@1M limine.cfg limine/limine-bios.sys ::/boot/limine
mcopy -i image.hdd@@1M limine/BOOTX64.EFI ::/EFI/BOOT
mcopy -i image.hdd@@1M limine/BOOTIA32.EFI ::/EFI/BOOT
