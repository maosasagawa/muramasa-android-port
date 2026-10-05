# 村正 Android 引擎移植 / Muramasa Android Engine Port

这是 Nitroplus N2 引擎的 Android 引擎级移植，用于《装甲悪鬼村正》。项目基于 FGRE 的 `libnpa`、`libnpengine` 和 `muramasa`，并参考 NitroSharp。它不包含游戏内容；请仅使用自己合法拥有的游戏数据。

## 项目内容

- GLES2 渲染、基于 `stb_truetype` 的竖排文字、`stb_vorbis` 与 SDL 音频、内置 GBK 转换表及 NPA 读取修复。
- Muramasa NSS 方言支持、存档/读档恢复、水平文字模式和配置切换。
- Android 适配：信箱缩放、触摸输入及外部存储游戏目录。
- `patches/` 只保存修改脚本的 unified diff；不要提交完整的原版或修改版游戏脚本。

水平文字模式由引擎在 `NSBInterpreter::CallFunction` 的脚本函数调用入口实现：启用横排时，把对白、开窗、立绘和名牌等竖排函数调用改派到游戏已有的横排函数，并按目标函数调整参数；不改写场景文本，也不使用纹理位置重映射。方向切换从下一个 `SetText*` 调用起清除相反方向的文字框和立绘；项目只分发统一脚本差异，不分发完整游戏脚本。

## 下载与安装

1. 到本仓库 [Releases](https://github.com/maosasagawa/muramasa-android-port/releases) 下载 `muramasa-android-v*.apk`（同时包含 arm64-v8a 与 x86_64，debug 签名），或在 [Actions](https://github.com/maosasagawa/muramasa-android-port/actions) 中下载每次提交构建的 APK。
2. 把自己正版游戏目录（含全部 `.npa` 与散文件）复制到下方目录。
3. 如需在游戏「设置」中切换横排/竖排，先用 `scripts/apply_script_patches.sh` 生成打过补丁的 `sys_config.nss` 放进游戏目录；不打补丁也能运行，只是设置界面没有「文字方向」一项。

## 当前进度

- 已可运行：开场、警告页、标题菜单、新游戏、正文推进，背景/立绘/特效/BGM/音效，竖排正文。
- 存档/读档：普通槽位、快速存读档和标题「从中继续」此前已在桌面版验证；一次普通槽位读档重放中观察到循环 SE01 重新创建并播放，但本轮尚未在指定说话人台词处完成槽 1 往返。
- 横排模式：引擎在脚本函数调用入口按目标函数可解析性重定向到游戏内建的横排实现，不改写场景文本。桌面运行确认下一文本块可在横排/竖排间切换；横排底框、文字位置和另一场景的说话人标签可见。指定指挥官台词的布局、完整名牌及旧朝向立绘清理仍未完成验收。
- 界面与标点：此前对白活动时的设置界面截图没有对白文字覆盖；本轮系统菜单截图仍可见变暗的场景文字。旧的无框截图状态未能复现，根因尚未确定；垂直标点的放大裁图和位置测量仍待完成。
- 标题「从中继续」与方向持久化此前已在桌面版验证；本轮未在最后一次中途切换后重新运行此流程。
- 未实现：视频（OP）播放。

## CI 与发布

`.github/workflows/android.yml` 在每次推送到 `main`、PR 和手动触发时用 NDK r25c 从源码编译 SDL2、libpng、libjpeg-turbo 和引擎，生成 APK 并上传为 Actions 构建产物。推送 `v*` 标签（如 `git tag v0.1.0 && git push origin v0.1.0`）时自动创建 GitHub Release 并附上 APK。CI 不接触任何游戏数据。

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
