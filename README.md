# ESP32-S3 LuaRTOS SD Card Project

一个基于ESP32-S3的嵌入式Lua运行时系统，集成了SD卡存储、LVGL图形界面和完整的文件系统支持。

## 项目概述

本项目实现了一个完整的ESP32-S3嵌入式系统，具备以下核心功能：

- **Lua 5.4 脚本引擎** - 支持动态脚本执行和交互式编程
- **SD卡文件系统** - 通过SPI接口实现完整的文件读写操作
- **LVGL图形界面** - 现代化的嵌入式GUI框架
- **模块化架构** - 清晰的组件分离和API设计

## 硬件配置

### 主控制器

- **芯片**: ESP32-S3
- **开发框架**: ESP-IDF 5.x
- **编程语言**: C + Lua

### SD卡模块

- **接口**: SPI3_HOST
- **引脚配置**:

```text
CS   (片选)   → GPIO 9
SCK  (时钟)   → GPIO 11
MOSI (数据出) → GPIO 42
MISO (数据入) → GPIO 41
VCC           → 3.3V
GND           → GND
```

- **上拉电阻**: 每个信号线需要10kΩ上拉至3.3V
- **支持格式**: FAT32, ≤32GB

### 显示器

- 支持LVGL兼容的各种显示器
- 详细配置参见 `DISPLAY_README.md`

## 软件架构

### 目录结构

```text
luartos/
├── main/                    # 主程序
│   ├── main.c              # 系统主入口
│   ├── main_simple_lua.h   # 嵌入式Lua演示脚本
│   └── main.lua            # 外部Lua脚本文件
├── components/              # 组件库
│   ├── sdcard/             # SD卡驱动组件
│   │   ├── sdcard_driver.c # SPI SD卡驱动实现
│   │   ├── sdcard_driver.h # 驱动接口定义
│   │   ├── sdcard_lua_bindings.c # Lua API绑定
│   │   └── CMakeLists.txt  # 组件构建配置
│   ├── lua/                # Lua引擎
│   ├── lvgl/               # LVGL图形库
│   └── lvgl_esp32_drivers/ # ESP32显示驱动
└── build/                  # 构建输出目录
```

### 核心组件

#### 1. SD卡驱动 (`components/sdcard/`)

- **驱动层**: 基于ESP-IDF的SDSPI接口实现
- **文件系统**: FAT32支持，完整的VFS集成
- **特性**:
  - 低速模式初始化 (400kHz)
  - 自动错误检测和恢复
  - 热插拔支持

#### 2. Lua引擎集成

- **版本**: Lua 5.4
- **绑定**: 完整的SD卡操作API
- **执行方式**:
  - 嵌入式脚本 (编译时内置)
  - 外部脚本文件 (从SD卡加载)

#### 3. LVGL图形界面

- **版本**: LVGL 8.x
- **集成**: Lua绑定支持
- **特性**: 现代化UI组件、主题支持

## API 参考

### SD卡 Lua API

#### 基础操作

```lua
-- 初始化SD卡
local success, error = sdcard.init()

-- 挂载文件系统
local success, error = sdcard.mount(mount_point, max_files)
-- mount_point: 挂载点 (默认 "/sdcard")
-- max_files: 最大文件数 (默认 5)

-- 卸载文件系统
sdcard.unmount()

-- 检查挂载状态
local mounted = sdcard.is_mounted()
```

#### 文件操作

```lua
-- 写入文件
local success, bytes_written = sdcard.write_file(filename, data)

-- 读取文件
local data = sdcard.read_file(filename)

-- 删除文件
local success = sdcard.delete_file(filename)

-- 列出文件
local files = sdcard.list_files(path)
-- 返回: {{name="file1.txt", size=123}, ...}
```

#### 系统信息

```lua
-- 获取SD卡信息
local info = sdcard.get_info()
-- 返回: {name="...", type="...", capacity_mb=..., speed="..."}

-- 获取磁盘使用情况
local usage = sdcard.get_usage()
-- 返回: {total=..., used=..., free=...} (字节)

-- 格式化SD卡 (慎用!)
local success = sdcard.format()
```

### LVGL Lua API

详细的LVGL Lua API参考请查看 `LUA_LVGL_README.md`

## 开发指南

### 环境搭建

1. **安装ESP-IDF**

   ```bash
   # 安装ESP-IDF 5.x
   git clone -b v5.1 https://github.com/espressif/esp-idf.git
   cd esp-idf && ./install.sh
   source export.sh
   ```

2. **克隆项目**

   ```bash
   git clone <your-repo-url> luartos
   cd luartos
   ```

3. **配置项目**

   ```bash
   idf.py menuconfig
   # 配置显示器、WiFi等选项
   ```

### 编译和烧录

```bash
# 编译项目
idf.py build

# 烧录到设备
idf.py flash

# 查看串口输出
idf.py monitor
```

### 添加自定义Lua脚本

#### 方法1: 嵌入式脚本

1. 编辑 `main/main_simple_lua.h`
2. 修改 `main_simple_lua_script` 字符串
3. 重新编译项目

#### 方法2: 外部脚本文件

1. 将Lua脚本保存到SD卡
2. 在主程序中使用 `dofile()` 加载

```lua
-- 从SD卡加载脚本
dofile("/sdcard/my_script.lua")
```

### 硬件调试指南

#### SD卡连接问题

如果遇到SD卡通信错误，请检查：

1. **硬件连接**
   - 确认所有引脚连接正确
   - 使用万用表测试连通性
   - 检查焊接质量

2. **电源质量**
   - 3.3V电压稳定 (±0.1V)
   - 添加去耦电容 (100nF + 10µF)

3. **上拉电阻**
   - 每个信号线必须有10kΩ上拉电阻
   - 这是SPI通信的关键要求

4. **SD卡兼容性**
   - 使用高质量SD卡 (SanDisk, Samsung)
   - 确保≤32GB，FAT32格式
   - 检查写保护开关

## 使用示例

### 基础文件操作

```lua
-- 初始化SD卡
print("Initializing SD card...")
local init_ok, init_err = sdcard.init()
if not init_ok then
    print("Init failed:", init_err)
    return
end

-- 挂载文件系统
local mount_ok, mount_err = sdcard.mount()
if not mount_ok then
    print("Mount failed:", mount_err)
    return
end

-- 写入文件
local data = "Hello from ESP32-S3!\nTime: " .. os.date()
local write_ok, bytes = sdcard.write_file("hello.txt", data)
if write_ok then
    print("File written:", bytes, "bytes")
else
    print("Write failed")
end

-- 读取文件
local content = sdcard.read_file("hello.txt")
if content then
    print("File content:", content)
end
```

### LVGL界面示例

```lua
-- 创建主屏幕
local scr = lvgl.scr_act()
lvgl.obj_set_style_bg_color(scr, 0x001122, lvgl.PART_MAIN())

-- 添加标题
local title = lvgl.label_create(scr)
lvgl.label_set_text(title, "ESP32-S3 LuaRTOS")
lvgl.obj_set_style_text_color(title, lvgl.color_white(), lvgl.PART_MAIN())
lvgl.obj_align(title, lvgl.ALIGN_TOP_MID(), 0, 10)

-- 添加按钮
local btn = lvgl.btn_create(scr)
lvgl.obj_set_size(btn, 120, 40)
lvgl.obj_align(btn, lvgl.ALIGN_CENTER(), 0, 30)

local btn_label = lvgl.label_create(btn)
lvgl.label_set_text(btn_label, "Click Me")
lvgl.obj_center(btn_label)
```

## 问题排除

### 常见问题

1. **SD卡初始化失败**
   - 检查硬件连接
   - 确认上拉电阻
   - 验证电源稳定性

2. **文件写入失败**
   - 检查SD卡写保护
   - 确认文件系统格式 (FAT32)
   - 验证剩余空间

3. **显示器不工作**
   - 参考 `DISPLAY_README.md`
   - 检查显示器驱动配置

4. **Lua脚本错误**
   - 查看串口输出的错误信息
   - 检查语法和API使用

### 性能优化

1. **SD卡性能**
   - 使用高速SD卡 (Class 10)
   - 合理设置SPI时钟频率
   - 批量文件操作

2. **内存使用**
   - 监控堆内存使用
   - 及时释放Lua对象
   - 使用PSRAM存储大数据

## 贡献指南

1. Fork本项目
2. 创建功能分支 (`git checkout -b feature/AmazingFeature`)
3. 提交更改 (`git commit -m 'Add some AmazingFeature'`)
4. 推送到分支 (`git push origin feature/AmazingFeature`)
5. 打开Pull Request

## 许可证

本项目采用MIT许可证 - 详见 `LICENSE` 文件

## 致谢

- ESP-IDF团队提供的优秀开发框架
- LVGL团队的现代化GUI库
- Lua社区的脚本引擎支持
- 参考了B站作者[颜along](https://www.bilibili.com/opus/1022316329467641890)的SD卡SPI实现经验

## 联系方式

如有问题或建议，请通过以下方式联系：

- 提交Issue
- 发起Discussion
- 邮箱: [your-email@example.com]

---

**项目状态**: ✅ 稳定版本  
**最后更新**: 2025年8月3日  
**版本**: v1.0.0
