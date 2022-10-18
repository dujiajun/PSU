# Private Set Union

## 测试环境

代码和教程在 Ubuntu 20.04 系统上，使用 g++ 9.4 和 CMake 3.24 构建。
## 安装依赖

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

找一个地方存放编译后的头文件和静态库，这里使用`/path_to_PSU/libOTe/out/install/linux`。如果为空会自动安装到`/usr/local`文件夹中。
```shell
cd libOTe
python build.py --install=/path_to_PSU/libOTe/out/install/linux -- -D ENABLE_RELIC=ON -D ENABLE_NP=ON -D ENABLE_KOS=ON -D ENABLE_IKNP=ON -D ENABLE_SILENTOT=ON
```
如果更改了 install 地址，后文记得修改`CMakeLists.txt`中的`include`和`lib`目录。

## 编译PSU

### Visual Studio 2019 + CMake Preset
1. 使用VS打开PSU文件夹，创建`CMakePresets.json`配置文件。
2. 在默认模板基础上修改`cacheVariables`，添加
```json
"PRESET_NAME": "${presetName}"
```
3. 点击**生成-全部生成**即可。

### 自行调用CMake

自行设定`PRESET_NAME`（如果上节安装到了 `/path_to_PSU/libOTe/out/install/linux`，`PRESET_NAME` 应该填 `linux`）或者修改 `CMakeLists.txt` 到你的地址。

```shell
mkdir build # in PSU dir
cd build
cmake .. -D PRESET_NAME=linux
make
```