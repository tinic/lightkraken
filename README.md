# Lightkraken
Ethernet powered and controlled LED strip driver and more

# Build instructions for the firmware binary blob:

To bootstrap (on any recent Debian/Ubuntu/LinuxMint distro):

```
sudo apt install build-essential *arm-none-eabi* git cmake ninja-build
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

# Build instructions for the Web UI, this will update fsdata.c with the new compressed JS/HTML/CSS data

```
sudo apt install build-essential git npm
git clone https://github.com/tinic/lightkraken-ui.git
cd lightkraken-ui
npm install
npm run build
cd ..
git clone https://github.com/tinic/lightkraken.git
cd lightkraken
git submodule init
git submodule update
cd fs
rm -r *
cp -R ../../lightkraken-ui/dist/* ./
cd ..
gcc -I lwip-ajax/src/include/ -I ./ -I GD32F10x_lwip/ lwip-ajax/src/apps/http/makefsdata/makefsdata.c -o makefsdata
makefsdata -defl
```
