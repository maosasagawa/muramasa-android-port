# 村正 Android 引擎移植 / Muramasa Android Engine Port

这是 Nitroplus N2 引擎的 Android 引擎级移植，用于《装甲悪鬼村正》。项目基于 FGRE 的 `libnpa`、`libnpengine` 和 `muramasa`，并参考 NitroSharp。它不包含游戏内容；请仅使用自己合法拥有的游戏数据。

## 项目内容

- GLES2 渲染、基于 `stb_truetype` 的竖排文字、`stb_vorbis` 与 SDL 音频、内置 GBK 转换表及 NPA 读取修复。
- Muramasa NSS 方言支持、存档/读档恢复、水平文字模式和配置切换。
- Android 适配：信箱缩放、触摸输入及外部存储游戏目录。
- `patches/` 只保存修改脚本的 unified diff；不要提交完整的原版或修改版游戏脚本。

游戏数据安装目录：

```text
/sdcard/Android/data/org.devin.muramasa/files/game
```

Android 版本目前不支持视频播放。

## 构建

### 桌面版

需要 CMake、C++17 编译器、Flex/Bison、SDL2、GLEW、JPEG、PNG、Boost program_options、GStreamer、PangoCairo、线程库和 OpenGL 开发文件。

```sh
./scripts/build_desktop.sh
```

生成的可执行文件位于 `muramasa/build/muramasa`。从游戏目录启动，并让该目录作为当前工作目录。

### Android APK

需要 Android SDK（API 33 / 对应 Build Tools）、Android NDK r25c、CMake、Ninja 或 Make，以及 SDL2 2.30.12、libpng 1.6.43 和 libjpeg-turbo 3.0.2 的源码。先设置构建依赖路径：

```sh
export ANDROID_NDK_ROOT=/path/to/android-ndk-r25c
export SDL2_SOURCE_DIR=/path/to/SDL
export LIBPNG_SOURCE_DIR=/path/to/libpng-1.6.43
export LIBJPEG_TURBO_SOURCE_DIR=/path/to/libjpeg-turbo-3.0.2
./scripts/build_android.sh
```

APK 输出位于 `android/app/app/build/outputs/apk/debug/`。`scripts/build_android_xlibs.sh` 中的 PNG/JPEG 参数可由现有 CMake 缓存复现；SDL2 的原始 CMake 缓存和 shell 历史不可用，因此 SDL2 的配置选项在脚本中标为推断项，发布前应与原构建环境核对。Android 工具链、SDK、SDL2、libpng 和 libjpeg-turbo 源码及其构建依赖不会随本仓库提供。

### 应用脚本补丁

脚本补丁以用户提供的 UTF-8 `sys_config_patched.nss` 为唯一源文件生成。在另一台机器上，可从原版 `sys_config.nss` 或原版 `nss.npa` 应用补丁并输出 GBK 文件：

```sh
./scripts/apply_script_patches.sh --game-dir /path/to/original-game --output /path/to/output/sys_config.nss
# 或
./scripts/apply_script_patches.sh --npa /path/to/original/nss.npa --output /path/to/output/sys_config.nss
```

从 NPA 提取时需要 `npaextract`（或设置 `NPAEXTRACT_BIN`）；该工具及游戏数据不包含在仓库中。

## 许可与来源

- `libnpa/`：MIT；本地基准上游提交 `5bdf666`。
- `libnpengine/`：LGPL-3.0；本地基准上游提交 `da1a175`。`vendor/` 保留第三方文件中的许可证声明。
- `android/app/` 使用 SDL Android Java 源码，随附 SDL 许可证。随应用提供的 Noto Sans CJK 字体与系统安装的 `NotoSansCJK-Regular.ttc` SHA-256 相同；字体按 SIL Open Font License 1.1 提供，许可证/版权信息见 `android/app/FONT-LICENSE.txt`。
- 检查本地 FGRE `muramasa` 源树时未发现 LICENSE、README 或源文件版权/许可声明；其上游许可状态未能确认。发布或再分发该目录前，请先向上游核实并添加相应许可文件。该目录没有被擅自重新许可；详情见 `muramasa/LICENSE-STATUS.md`。
- Nitroplus 游戏数据、原版脚本、图像、音频和存档均不包含在本仓库内。用户必须拥有游戏的合法副本。

## English summary

An engine-level Android port of Nitroplus N2 for *Full Metal Daemon Muramasa*, based on FGRE `libnpa`, `libnpengine`, and `muramasa`, with NitroSharp as a reference. Game data is not included; users must own the game. Install it under `/sdcard/Android/data/org.devin.muramasa/files/game`. Video playback is not supported. See the Chinese sections above for build steps and licensing notes.
