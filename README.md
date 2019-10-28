# Lightguy
Ethernet powered and controlled LED strip driver and more

# Build instructions:

To bootstrap (on any recent Debian/Ubuntu/LinuxMint distro):

```
sudo apt install build-essential arm-none-eabi* git cmake
git clone https://github.com/tinic/lightkraken.git
cd lightkraken
git submodule init
git submodule update
```

Then to build:

```
./build_release_all.sh
```

Output files will be in the build/ folder. The correct file to flash from the bootloader is 'lightkraken_bootloaded.bin'.
