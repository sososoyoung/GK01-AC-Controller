# 空调型号与红外码库

## 空调

空调页的品牌／协议和遥控器型号由 `/api/hvac/catalog` 从当前编译的 IRremoteESP8266 库生成。只列出 IRac 能发送的空调协议，避免把 NEC 等普通按键协议当作空调。

- 型号 `0`：兼容旧行为。格力、华凌使用原专用编码器，其他协议继续使用原先的库默认型号 `1`。
- 正数型号：使用库的具名遥控器型号。格力选择 YAW1F、YBOFB、YX1FSF 后改走 IRac；默认仍保留原格力编码器。
- 型号随最近空调状态保存到设备配置，MQTT 控制沿用该型号；主机向从机发送时携带型号。旧配置缺少该字段时自动为 `0`。
- 同品牌不同协议仍需手动匹配，具名型号只表示上游提供该选项，不代表已经在具体空调上验证。
- 新型号、电视控制与非 38 kHz RAW 转发需要主从设备都更新。默认空调命令和 38 kHz RAW 保留旧消息格式。

使用方式：查看遥控器背面型号，在空调页选择相应品牌／协议和型号，再测试开关、温度、模式、风速。没有匹配项时使用学习功能或外部固定码集。

## 小米电视

遥控器页内置 11 个按钮：电源、主页、菜单、确认、上下左右、返回、音量加减。默认只由本机发射，可以选择全部设备。

码表来源：[Flipper-IRDB 的 Xiaomi_TV.ir](https://github.com/Lucaslhm/Flipper-IRDB/blob/main/TVs/Xiaomi/Xiaomi_TV.ir)，于 2026-10-16 下载。原文件保存在 `data/ir/xiaomi_tv.ir`，转换后的可导入文件为 `data/ir/xiaomi_tv.json`，固件波形放在 `firmware/src/hw/tv_remote.cpp`。自动测试逐项比较原始码集、JSON 与固件中的全部时序。

这是社区记录的一个小米电视码组，源文件没有注明具体机型；尚未实机验证。原文件的 Home 与 Ok 波形高度相似，保持原记录，不擅自推测替换。电源使用原文件的 Open 信号，作为电源切换候选码；应在实机上确认行为。不同电视、电视盒子、地区型号可能不兼容，不能承诺覆盖所有小米电视。语音、蓝牙配对不在红外能力范围内。

上游 README 对新提交采用 CC0，并明确早期提交不在统一授权范围内；再分发码库时应核对具体文件历史及授权。

## 导入社区码库

转换工具只在本机运行，不请求云服务、不需要额外 Python 包：

```bash
# Flipper RAW，或 parsed NEC（8 位地址、8 位命令）
python3 scripts/convert_ir_codes.py remote.ir remote.json

# SmartIR 的 Broadlink Base64 或 Raw 命令
python3 scripts/convert_ir_codes.py smartir.json remote.json --frequency 38000
```

在网页「遥控器 → 已保存 → 导入码库」选择生成的 `remote.json`。按钮保存在当前浏览器；点击后仅由当前设备发射。清理浏览器数据会丢失保存记录，可用现有备份功能导出。

转换范围与限制：

- Flipper：支持 RAW 和 parsed NEC；其他 parsed 协议需先转为 RAW，避免错误解释位序。
- SmartIR：支持 `commandsEncoding: Raw`，以及 `supportedController: Broadlink` 的 Base64；不支持 Xiaomi、LOOKin 专有格式或 RF 数据包。
- Broadlink 数据包没有明确载波频率，默认使用 38 kHz；请根据设备实际要求传入 `--frequency`。
- SmartIR 中“模式／风速／温度”的每个组合转换为一个固定按钮，不会自动扩展为动态空调协议。
- 单个码集最多 500 个按钮，每条最多 512 个时序，载波范围 20～60 kHz。超长单个时序会插入零时长反相段，保留原波形的极性和总时长；超过总长度则拒绝转换，不截断。
- RAW 的多机广播受现有 1024 字节 UDP 包限制；过长时返回错误，避免本机执行而从机收到截断波形。导入按钮固定本机发送，不受 UDP 长度限制。

可用来源：[SmartIR 空调码库](https://github.com/smartHomeHub/SmartIR/blob/master/docs/CLIMATE.md)、[Flipper-IRDB](https://github.com/Lucaslhm/Flipper-IRDB)、[irdb](https://github.com/probonopd/irdb)。irdb 的协议／设备／功能号需要先用 IrScrutinizer 等工具转换为 RAW，本工具不直接读取其 CSV。

Broadlink 时序解析参照 [python-broadlink 的数据格式实现](https://github.com/mjg59/python-broadlink/blob/master/broadlink/remote.py)。

## API

- `GET /api/hvac/catalog`：空调协议及其具名型号。
- `POST /api/hvac`：原字段基础上增加 `model`（可省略，默认 `0`）。
- `GET /api/hvac/state`：增加 `model`。
- `POST /api/tv/xiaomi`：`key=power|home|menu|ok|up|down|left|right|back|volume_up|volume_down`，`target=LOCAL|ALL`，默认 `LOCAL`。主机也接受现有逗号分隔设备 ID。
- `POST /api/send`：`raw` 为逗号分隔微秒时序，增加 `frequency`（默认 `38000` Hz）和 `target=LOCAL`；省略 target 保留原有广播行为。

`ok:true` 表示固件已完成发射／提交 UDP 消息，不表示家电确认接收。

验证：`python3 scripts/test_ir_codes.py`；安装 `@playwright/test` 后可运行 `node scripts/test_ir_ui.cjs` 做浏览器模拟验收，以及日常 `pio run -e rom0 -e rom1`。尚需在真实空调、小米电视和多机组网上完成硬件验收。
