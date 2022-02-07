# Private Set Union

## 安装依赖

```shell
git clone https://github.com/dujiajun/PSU --recursive
cd libOTe
python build.py --setup --boost --relic
```

## 编译libOTe

### Visual Studio 2019 + CMake Preset
使用CMake Preset目的是在同一个目录下存放多个平台的库文件。

1. 使用VS打开libOTe文件夹，创建`CMakePresets.json`配置文件。
2. 在默认模板基础上修改`cacheVariables`，添加
```json
"ENABLE_NP": "ON",
"ENABLE_KOS": "ON",
"ENABLE_IKNP": "ON",
"ENABLE_SILENTOT": "ON",
"ENABLE_RELIC": "ON"
```
3. 点击**生成-安装libOTe**

### libOTe自带脚本

找一个地方存放编译后的头文件和静态库，记为`prefix`。如果为空会自动安装到`/usr/`文件夹中。
```shell
cd libOTe
python build.py -- -D ENABLE_RELIC=ON -D ENABLE_ALL_OT=ON --install=<prefix>
```
后文记得修改`CMakeLists.txt`中的`include`和`lib`目录。

## 编译PSU

### Visual Studio 2019 + CMake Preset
1. 使用VS打开PSU文件夹，创建`CMakePresets.json`配置文件。
2. 在默认模板基础上修改`cacheVariables`，添加
```json
"PRESET_NAME": "${presetName}"
```
3. 点击**生成-全部生成**即可。

### 自行调用CMake

自行设定`PRESET_NAME`或者修改`CMakeLists.txt`

```shell
mkdir build
cd
cmake .. -D PRESET_NAME=<>
make
```