#/bin/sh
export PATH="/opt/gcc-arm-none-eabi-9-2019-q4-major/bin:$PATH"

export GITHUB_REPO=lightkraken

git submodule init
git submodule update

./build_release_all.sh

#if [[ $? -ne 0 ]] ; then
#  exit 1
#fi

arm-none-eabi-objdump > build/lightkraken_bootloader.dump.txt -D build/lightkraken_bootloader.elf 
arm-none-eabi-objdump > build/lightkraken_bootloaded.dump.txt -D build/lightkraken_bootloaded.elf 
arm-none-eabi-objdump > build/lightkraken_bootloader.annotated.txt -S build/lightkraken_bootloader.elf 
arm-none-eabi-objdump > build/lightkraken_bootloaded.annotated.txt -S build/lightkraken_bootloaded.elf 

buildnumber=$(git rev-list HEAD --count)

zip -j build.zip build/lightkraken_bootloader.bin build/lightkraken_bootloaded.bin
github-release delete --tag latest || true
git remote rm origin
git remote add origin https://${GITHUB_USER}:${GITHUB_TOKEN}@github.com/tinic/lightkraken.git
git push --delete origin latest || true
github-release release --tag latest --name "latest"
#fix race on github.com
sleep 5
github-release upload --tag latest --name "lightkraken-bin-$(date +"%Y-%m-%d")-r${buildnumber}.zip" --file build.zip
