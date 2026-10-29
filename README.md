# WM03A Firmware

## 1. Giới thiệu

Water Datalogger **WM03A** là thiết bị thu thập dữ liệu từ đồng hồ nước và các cảm biến giám sát. Thiết bị hỗ trợ xử lý và lưu trữ dữ liệu cục bộ, đồng thời truyền dữ liệu đến máy chủ trung tâm thông qua mạng di động.

Bên cạnh khả năng truyền thông từ xa, thiết bị hỗ trợ giao tiếp trực tiếp tại hiện trường phục vụ cấu hình, kiểm tra, chẩn đoán và bảo trì. Firmware được thiết kế theo hướng phân tầng và module hóa, nhằm nâng cao khả năng bảo trì, mở rộng chức năng và đảm bảo độ tin cậy trong quá trình vận hành.

## 2. Chức năng chính

- **Thu thập dữ liệu:** Tiếp nhận tín hiệu xung từ đồng hồ nước, đọc dữ liệu từ các thiết bị đo qua giao tiếp RS485 và thu thập dữ liệu cảm biến.
- **Lưu trữ cục bộ:** Lưu dữ liệu đo lường, thông tin cấu hình và nhật ký sự kiện vào bộ nhớ ngoài.
- **Truyền thông từ xa:** Kết nối máy chủ thông qua mạng di động để truyền dữ liệu và trao đổi thông tin với hệ thống quản lý.
- **Giao tiếp tại hiện trường:** Hỗ trợ kết nối trực tiếp để cấu hình, kiểm tra và chẩn đoán thiết bị.
- **Quản lý thời gian:** Sử dụng thông tin thời gian để đóng dấu thời gian cho dữ liệu và nhật ký sự kiện.
- **Giám sát hệ thống:** Theo dõi trạng thái hoạt động, quản lý các sự kiện và xử lý những tình huống bất thường.
- **Cập nhật firmware:** Hỗ trợ cơ chế cập nhật phần mềm từ xa (FOTA), nếu được triển khai trong phiên bản firmware tương ứng.

## 3. Nền tảng phần cứng

- **MCU:** STM32L431RCT6, 32-bit, 256 KB Flash (128 blocks × 2 KB), 64 KB RAM (48 KB SRAM1 + 16 KB SRAM2).
- **Bộ nhớ ngoài:** EEPROM M95M02-DRMN6TP, 256 KB, giao tiếp SPI.
- **Giao tiếp RS485:** Sử dụng IC MAX3485ESA chuyển đổi từ giao tiếp UART.
- **Đo áp suất:** ADC MCP3421A1T-E/CH, 18-bit, giao tiếp I2C.
- **Giám sát hệ thống:** IC Watchdog STM6823ZWY6F.
- **Giám sạc và xả pin:** IC Sạc BQ25895, giao tiếp I2C.
- **Kết nối mạng:** Module 4G A7677S-LANV.

## 4. Nền tảng phát triển

### 4.1. Công cụ và môi trường phát triển

| Thành phần | Thông số |
|---|---|
| Vi điều khiển | STM32L431RCT6 |
| Ngôn ngữ lập trình | C |
| Hệ thống build | CMake |
| Build tool | Ninja |
| Bộ công cụ biên dịch | Arm GNU Toolchain |
| Môi trường phát triển | STM32CubeIDE |
| Công cụ nạp và gỡ lỗi | STM32CubeProgrammer, ST-LINK |
| Tạo tài liệu mã nguồn | Doxygen |

### 4.2. Cấu trúc mã nguồn

Các thư mục và file chính của dự án:

```text
WM03A/
├── cmake/              # Cấu hình hỗ trợ build
├── Core/               # Mã nguồn lõi và khởi tạo MCU
├── Drivers/            # Driver và thư viện hỗ trợ phần cứng
├── Linker/             # Cấu hình linker đang sử dụng
├── Users/              # Mã nguồn ứng dụng và các module chức năng
├── docs/               # Tài liệu kỹ thuật
├── CMakeLists.txt      # Cấu hình build chính
├── Makefile            # Định nghĩa các lệnh build và nạp được sử dụng trong quá trình phát triển
├── WM03A.ioc           # Cấu hình STM32CubeMX
├── Doxyfile            # Cấu hình tạo tài liệu mã nguồn
└── pack_firmware.py    # Công cụ đóng gói firmware sau khi build
```

Ý nghĩa các thành phần:

| Thành phần | Trách nhiệm                                                                                             |
| ---------- | ------------------------------------------------------------------------------------------------------- |
| `Core/`    | Chứa mã nguồn lõi do STM32CubeMX tạo ra hoặc cấu hình khởi tạo MCU.|
| `Drivers/` | Chứa driver ngoại vi và các thư viện hỗ trợ phần cứng được tạo bởi STM32CubeMX.|
| `Users/`   | Chứa mã nguồn do dự án phát triển, bao gồm xử lý ứng dụng và các chức năng riêng của thiết bị.|
| `cmake/`   | Chứa các cấu hình hoặc tập lệnh hỗ trợ quá trình build được tạo bởi STM32CubeMX.|
| `Linker/`  | Chứa cấu hình linker được sử dụng theo cấu hình lựa chọn.|
| `docs/`    | Chứa tài liệu kiến trúc, phần cứng, giao tiếp, lưu trữ và hướng dẫn build.|

## 5. Tài liệu kỹ thuật

Các tài liệu chi tiết được tổ chức riêng trong thư mục `docs/`:

- [Nền tảng phần cứng](docs/hardware.md) — các khối chức năng phần cứng và giao diện ngoại vi.
- [Kiến trúc phần mềm](docs/architecture.md) — cấu trúc module, phân tầng và nguyên tắc phụ thuộc giữa các thành phần.
- [Giao thức truyền thông](docs/communication_protocol.md) — cấu trúc bản tin, định dạng dữ liệu và quy trình trao đổi với thiết bị hoặc máy chủ.
- [Quản lý lưu trữ](docs/storage.md) — tổ chức dữ liệu, cấu hình và nhật ký trong bộ nhớ.
- [Build và nạp firmware](docs/build_and_flash.md) — hướng dẫn cấu hình môi trường, biên dịch, nạp và gỡ lỗi.

## 6. Biên dịch và nạp firmware

Firmware sử dụng CMake kết hợp Ninja và Arm GNU Toolchain để cấu hình, biên dịch mã nguồn.

Quy trình phát triển cơ bản:

1. Cài đặt và cấu hình các công cụ phát triển cần thiết.
2. Cấu hình build theo hướng dẫn của dự án.
3. Biên dịch firmware bằng CMake và Ninja.
4. Nạp firmware vào thiết bị bằng STM32CubeProgrammer hoặc ST-LINK.
5. Kiểm tra hoạt động và gỡ lỗi thông qua giao diện debug được cấu hình trong firmware.

Tham khảo [Build và nạp firmware](docs/build_and_flash.md) để biết các lệnh và cấu hình cụ thể.

## 7. Quản lý phiên bản

Thông tin phiên bản firmware được định nghĩa trong mã nguồn và được cập nhật theo từng bản phát hành.

Khi thay đổi chức năng hoặc giao thức, cần cập nhật phiên bản tương ứng và điều chỉnh các tài liệu liên quan để đảm bảo tính nhất quán giữa mã nguồn, firmware phát hành và tài liệu kỹ thuật.