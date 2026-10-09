# 村正 Android 引擎移植 / Muramasa Android Engine Port

这是 Nitroplus N2 引擎的 Android 引擎级移植，用于《装甲悪鬼村正》。项目基于 FGRE 的 `libnpa`、`libnpengine` 和 `muramasa`，并参考 NitroSharp。它不包含游戏内容；请仅使用自己合法拥有的游戏数据。

## 项目内容

- GLES2 渲染、基于 `stb_truetype` 的竖排文字、`stb_vorbis` 与 SDL 音频、内置 GBK 转换表及 NPA 读取修复。
- Muramasa NSS 方言支持、存档/读档恢复、水平文字模式和配置切换。
- Android 适配：信箱缩放、触摸输入及外部存储游戏目录。
- `patches/` 只保存修改脚本的 unified diff；不要提交完整的原版或修改版游戏脚本。

水平文字模式由引擎在 `NSBInterpreter::CallFunction` 的脚本函数调用入口实现：启用横排时，仅当目标符号可解析，才把对白、开窗、立绘和名牌等竖排函数调用改派到游戏已有的横排函数，并适配参数；不改写场景文本，也不使用纹理位置重映射。方向切换从下一个 `SetText*` 调用起清除相反方向的文字框和立绘；项目只分发统一脚本差异，不分发完整游戏脚本。

## 下载与安装

1. 到本仓库 [Releases](https://github.com/maosasagawa/muramasa-android-port/releases) 下载 `muramasa-android-v*.apk`（同时包含 arm64-v8a 与 x86_64，debug 签名），或在 [Actions](https://github.com/maosasagawa/muramasa-android-port/actions) 中下载每次提交构建的 APK。
2. 首选：把自己正版游戏目录（含全部 `.npa` 与散文件）复制到手机内部存储的 `Muramasa/` 目录（`cg.npa` 直接放在该目录内），并在首次启动时按提示授予“所有文件访问权限”。也可复制到 `Android/data/org.devin.muramasa/files/game/`。
3. 如需在游戏「设置」中切换横排/竖排，先用 `scripts/apply_script_patches.sh` 生成打过补丁的 `sys_config.nss` 放进游戏目录；不打补丁也能运行，只是设置界面没有「文字方向」一项。

## 当前进度

- 已可运行：开场、警告页、标题菜单、新游戏、正文推进，背景/立绘/特效/BGM/音效，竖排正文。
- 桌面自动验收：`ma00_000` 横排、竖排分别推进至 `text0080`；捕获的活动文本行均有 alpha=1000 的对应文字框。`text0030` 横排 `boxTH` 和竖排 `boxCT` 的新游戏/自动读档截图均显示 `部隊長` 名牌。
- 名牌句柄：拦截器不再把空字符串字面量解析为空变量名；自动读档日志中的 `$FwCName`、`$ナット名` 和纹理句柄均为 `FwC01`，水平模式使用 `FwH*` 句柄。新游戏与读档日志中的 `cg/fw/nw部隊長.png` 路径字节相同（`63672f66772f6e77e983a8e99a8ae995b72e706e67`），且两种运行都会出现归档读取失败提示；因此现有证据不支持“仅读档时发生路径字符集变化”。
- 存档/读档：横排槽 1 在 `text0030` 的读档首帧与保存前画面像素差为 `0.0000%`，并继续推进至 `text0090`。读档后的 `SE01` 循环播放已在自动读档日志中记录。按 Q/P 对快速槽 9998 的存读档已验证；标题「从中继续」与方向持久化此前已在桌面版验证。
- 横排切换：真实设置切换后的方向变化与相反方向文字框隐藏有日志证据；但两个方向的无菜单 hook 截图以及读档后重新启动/继续的最新完整流程仍需复验。横排系统菜单截图已由用户验收为正确；对白活动时的设置界面截图未见对白文字覆盖。
- 竖排标点：独立 `fonttest` 对尺寸 20、24 输出逐字单元与墨迹测量，指定标点位置及竖排字形检查通过。
- 尚未完成：非 replay 的 IFO 与 `*I` 文本框恢复路径复测、两个方向的严格中途切换截图、以及 Android AVD 验收。
- 未实现：视频（OP）播放。

## 调试

桌面调试钩子仅在设置对应环境变量时启用：

- `NP_AUTO_ADVANCE=<ms>`：文字完全显示并等待指定毫秒后，注入一次左键推进。
- `NP_SHOT_DIR=<dir>`：推进前保存游戏画面，并记录文字框、立绘和文本对象的状态。
- `NP_AUTOLOAD=<slot>`：到达标题后自动读入指定存档槽。
- `NP_STOP_AT=<texthandle>`：捕获指定文本句柄后停止自动推进。
- `NP_TEXT_H=1`：仅供调试时覆盖文字方向；正常运行和验收应使用游戏设置。
- `NP_FACE_DBG=1`、`NP_TEX_DBG=1`、`NP_SND_DBG=1`：输出立绘变量/纹理及音频对象的诊断信息。
- `libnpengine` 的 `fonttest` 是独立构建目标，不链接进游戏，可输出竖排字形图像和逐字几何测量。

## CI 与发布

`.github/workflows/android.yml` 在每次推送到 `main`、PR 和手动触发时用 NDK r25c 从源码编译 SDL2、libpng、libjpeg-turbo 和引擎，生成 APK 并上传为 Actions 构建产物。推送 `v*` 标签（如 `git tag v0.1.0 && git push origin v0.1.0`）时自动创建 GitHub Release 并附上 APK。CI 不接触任何游戏数据。

游戏数据安装目录（首选目录；也可使用应用专属目录）：

```text
/storage/emulated/0/Muramasa
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

An engine-level Android port of Nitroplus N2 for *Full Metal Daemon Muramasa*, based on FGRE `libnpa`, `libnpengine`, and `muramasa`, with NitroSharp as a reference. Game data is not included; users must own the game. Install it in shared storage at `/storage/emulated/0/Muramasa` (grant all-files access on first launch), or under `/sdcard/Android/data/org.devin.muramasa/files/game`. Video playback is not supported. See the Chinese sections above for build steps and licensing notes.
