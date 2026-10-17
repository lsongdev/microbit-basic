# micro:bit Arduino demo

使用 PlatformIO + Arduino 的 **micro:bit V2** 入门示例，无第三方库依赖。
V1 的 MCU、点阵接线不同，不能直接使用此配置或固件。

## 功能

- 5×5 点阵显示心跳动画，每 400 ms 切换大小心形。
- 按 A 在心形和笑脸之间切换。
- 按 B 暂停或继续心跳动画；暂停期间点阵仍正常刷新。
- 串口以 115200 波特率输出按键事件及每秒运行状态。
- 按键采用 30 ms 消抖，使用 millis()/micros() 实现非阻塞调度。

## 编译与烧录

安装 PlatformIO Core，或在 VS Code 安装 PlatformIO IDE 扩展，打开本目录。
用支持数据传输的 USB 线连接开发板，系统应出现 `MICROBIT` 磁盘。

```sh
pio run
pio run -t upload
pio device monitor -b 115200
```

配置使用 `mbed` 上传方式，将 HEX 固件写入开发板的 USB 磁盘。
如果自动识别磁盘失败，可指定挂载路径（按实际路径替换）：

```sh
pio run -t upload --upload-port /run/media/lsong/MICROBIT
```

也可将 `.pio/build/microbit_v2/firmware.hex` 拖入 `MICROBIT` 磁盘。
烧录会覆盖板上现有程序，完成后自动运行。串口如未自动识别，先运行
`pio device list`，再用 `pio device monitor -p <串口路径> -b 115200` 指定。

## 修改示例

主程序位于 `src/main.cpp`。修改 `kPictures` 可自定义图案：每个图案有五行，
每行五个位，`1` 表示亮，最左侧对应 bit 4。修改 `kFrameMs` 可调整动画速度。
点阵按行扫描，一行保持约 1 ms；不要在 loop() 中添加长时间 delay()，否则会闪烁。
点阵占用的边缘引脚 P3、P4、P6、P7、P10 不应同时用于其他输出。

参考：[PlatformIO V2 板卡配置](https://docs.platformio.org/en/latest/boards/nordicnrf52/bbcmicrobit_v2.html)、
[Arduino V2 引脚映射](https://github.com/sandeepmistry/arduino-nRF5/blob/master/variants/BBCmicrobitV2/variant.cpp)。
