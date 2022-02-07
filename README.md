# Private Set Union

## Install Dependencies

```shell
git clone https://github.com/dujiajun/PSU --recursive
cd libOTe
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

Choose a place to store compiled headers and static libraries, denoted by `prefix`. If empty, it will be installed in `/usr/`.

```shell
cd libOTe
python build.py -- -D ENABLE_RELIC=ON -D ENABLE_ALL_OT=ON --install=<prefix>
```
Do not forget to modify relevant directories in PSU's `CMakeLists.txt`.

## Compile PSU (two ways)

### Visual Studio 2019/2022 + CMake Preset
1. Open `PSU` folder with VS.
2. Modify the `CMakePresets.json`, adding following item to `cacheVariables`
```json
"PRESET_NAME": "${presetName}"
```
3. Click **Build - Build All** Menu.

### Only CMake

Set the variable `PRESET_NAME` or modify `CMakeLists.txt`.

```shell
mkdir build
cd
cmake .. -D PRESET_NAME=<PRESET_NAME>
make
```