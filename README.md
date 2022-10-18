# Private Set Union

## Tested Environment

This code and following instruction is tested on Ubuntu 20.04, with g++ 9.4 and CMake 3.24.

## Install Dependencies

```shell
git clone https://github.com/dujiajun/PSU
cd PSU

git clone https://github.com/osu-crypto/libOTe
cd libOTe
git checkout 3a40823f0507710193d5b90e6917878853a2f836

git clone https://github.com/ladnir/cryptotools
cd cryptotools
git checkout 4a83de286d05669678364173f9fdfe45a44ddbc6

cd ..
# in PSU/libOTe
python build.py --setup --boost --relic
```

## Compile libOTe (two ways)

### Visual Studio 2019/2022 + CMake Preset
Using CMake Preset is to store multi-platform libraries in single directories. 

Make sure you enables **Use CMakePresets.json to drive CMake configure, build, and test** in Visual Studio settings.

1. Open `libOTe` folder with Visual Studio. VS will automatically create `CMakePresets.json` file.
2. Modify the `CMakePresets.json`, adding following items to `cacheVariables`
```json
"ENABLE_NP": "ON",
"ENABLE_KOS": "ON",
"ENABLE_IKNP": "ON",
"ENABLE_SILENTOT": "ON",
"ENABLE_RELIC": "ON"
```
3. Click **Build - Install libOTe** menu.

### Scripts from libOTe

Choose a place to store compiled headers and static libraries, denoted by `/path_to_PSU/libOTe/out/install/linux`. If empty, it will be installed in `/usr/local`.

```shell
cd libOTe
python build.py --install=/path_to_PSU/libOTe/out/install/linux -- -D ENABLE_RELIC=ON -D ENABLE_NP=ON -D ENABLE_KOS=ON -D ENABLE_IKNP=ON -D ENABLE_SILENTOT=ON
```
If you change the install dir, do not forget to modify relevant directories in PSU's `CMakeLists.txt`.

## Compile PSU (two ways)

### Visual Studio 2019/2022 + CMake Preset
1. Open `PSU` folder with VS.
2. Modify the `CMakePresets.json`, adding following item to `cacheVariables`
```json
"PRESET_NAME": "${presetName}"
```
3. Click **Build - Build All** Menu.

### Only CMake

Set the variable `PRESET_NAME` (If install dir is `/path_to_PSU/libOTe/out/install/linux`, `PRESET_NAME` will be `linux`) or modify `CMakeLists.txt`.

```shell
mkdir build # in PSU dir
cd build
cmake .. -D PRESET_NAME=linux
make -j
```