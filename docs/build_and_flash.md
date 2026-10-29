# Build và Flash Firmware

## 1. Tổng quan

Dự án sử dụng **CMake** và **Ninja** để biên dịch firmware. Makefile cung cấp các lệnh thuận tiện để biên dịch, đóng gói firmware image, nạp firmware và dọn dẹp các tệp build.

Firmware bao gồm ba thành phần:

- **Boot:** Chương trình Bootloader.
- **App A:** Chương trình ứng dụng cho Slot A.
- **App B:** Chương trình ứng dụng cho Slot B.

Dự án hỗ trợ hai cấu hình build:

- **Debug:** Phục vụ quá trình phát triển và gỡ lỗi.
- **Release:** Cấu hình build dùng để tạo firmware phát hành.

## 2. Công cụ cần thiết

Đảm bảo các công cụ sau đã được cài đặt và có thể gọi từ dòng lệnh:

- GNU Arm Embedded Toolchain (`arm-none-eabi-gcc`).
- CMake.
- Ninja.
- Python 3.
- STM32CubeProgrammer CLI (`STM32_Programmer_CLI`).

Tệp cấu hình toolchain được sử dụng là `cmake/gcc-arm-none-eabi.cmake`.

## 3. Các lệnh Build

Chạy các lệnh sau tại thư mục gốc của dự án.

| Lệnh | Chức năng |
|---|---|
| `make build` | Build toàn bộ firmware Release và đóng gói firmware image. |
| `make build-d` | Build Boot, App A và App B ở cấu hình Debug. |
| `make build-r` | Build Boot, App A và App B ở cấu hình Release. |
| `make build-boot-d` | Build Bootloader ở cấu hình Debug. |
| `make build-app-a-d` | Build App A ở cấu hình Debug. |
| `make build-app-b-d` | Build App B ở cấu hình Debug. |
| `make build-boot-r` | Build Bootloader ở cấu hình Release. |
| `make build-app-a-r` | Build App A ở cấu hình Release. |
| `make build-app-b-r` | Build App B ở cấu hình Release. |

### 3.1. Quy trình Build

Mỗi lệnh build sử dụng CMake để cấu hình thư mục build tương ứng, sau đó gọi CMake để biên dịch bằng Ninja.

Cấu trúc thư mục đầu ra:

```text
build/
├── Debug/
│   ├── Boot/
│   ├── App_A/
│   └── App_B/
└── Release/
    ├── Boot/
    ├── App_A/
    └── App_B/
```

Các tệp ELF được tạo tại những vị trí sau:

| Thành phần | Debug | Release |
|---|---|---|
| Boot | `build/Debug/Boot/WM03_BOOT.elf` | `build/Release/Boot/WM03_BOOT.elf` |
| App A | `build/Debug/App_A/WM03_APP_A.elf` | `build/Release/App_A/WM03_APP_A.elf` |
| App B | `build/Debug/App_B/WM03_APP_B.elf` | `build/Release/App_B/WM03_APP_B.elf` |

### 3.2. Đóng gói Firmware Image

Sau khi build thành công cả ba thành phần ở cấu hình Release, lệnh `make build` chạy script:

```bash
python pack_firmware.py
```

Script đọc các tệp firmware đã biên dịch, sắp xếp dữ liệu theo địa chỉ bộ nhớ được cấu hình, cập nhật metadata và tạo ra các firmware image.

Các tệp đầu ra nằm trong `build/Release/`:

| Tệp | Mô tả |
|---|---|
| `WM03_Firmware_Flash.bin` | Ảnh bộ nhớ Flash chứa Bootloader và App A tại các địa chỉ tương ứng. |
| `WM03_Firmware_Ota.bin` | Gói dữ liệu OTA chứa App A, App B và metadata của hai slot. |

Chi tiết về bố cục bộ nhớ, metadata và cách tính CRC được trình bày trong [tài liệu đóng gói firmware image](firmware_image_packing.md).

**Lưu ý:** `make build-r` chỉ build ba thành phần Release, không chạy script đóng gói firmware.

## 4. Flash Firmware

Các lệnh flash sử dụng STM32CubeProgrammer CLI thông qua giao tiếp SWD. Cần kết nối thiết bị với máy tính và đảm bảo công cụ có thể giao tiếp với vi điều khiển.

| Lệnh | Chức năng |
|---|---|
| `make flash` | Build Release, đóng gói firmware image, xóa Flash, nạp ảnh Flash tổng hợp, xác minh dữ liệu và reset thiết bị. |
| `make flash-r` | Build Release, xóa Flash, nạp riêng Bootloader và App A từ các tệp ELF, sau đó reset thiết bị. |
| `make flash-d` | Build Debug, xóa Flash, nạp riêng Bootloader và App A từ các tệp ELF, sau đó reset thiết bị. |

### `make flash`

Đây là quy trình flash mặc định. Lệnh thực hiện `make build`, bao gồm build toàn bộ firmware Release và đóng gói firmware image.

Sau đó, thiết bị được xóa Flash và nạp tệp `WM03_Firmware_Flash.bin` tại địa chỉ `0x08000000`.

```bash
STM32_Programmer_CLI -c port=SWD mode=UR -e all
STM32_Programmer_CLI -c port=SWD mode=UR -w "build/Release/WM03_Firmware_Flash.bin" 0x08000000 -v
STM32_Programmer_CLI -c port=SWD -rst
```

Quy trình gồm các bước:

1. Xóa toàn bộ Flash của thiết bị.
2. Nạp firmware image từ địa chỉ `0x08000000`.
3. Xác minh dữ liệu sau khi nạp bằng tùy chọn `-v`.
4. Reset thiết bị bằng tùy chọn `-rst`.

### `make flash-r`

Lệnh này build các thành phần Release, xóa Flash, sau đó nạp riêng Bootloader và App A từ các tệp ELF.

```bash
STM32_Programmer_CLI -c port=SWD mode=UR -e all
STM32_Programmer_CLI -c port=SWD mode=UR -w "build/Release/Boot/WM03_BOOT.elf" -v
STM32_Programmer_CLI -c port=SWD mode=UR -w "build/Release/App_A/WM03_APP_A.elf" -v
STM32_Programmer_CLI -c port=SWD -rst
```

### `make flash-d`

Lệnh này thực hiện quy trình tương tự `make flash-r`, nhưng sử dụng các tệp ELF ở cấu hình Debug.

```bash
STM32_Programmer_CLI -c port=SWD mode=UR -e all
STM32_Programmer_CLI -c port=SWD mode=UR -w "build/Debug/Boot/WM03_BOOT.elf" -v
STM32_Programmer_CLI -c port=SWD mode=UR -w "build/Debug/App_A/WM03_APP_A.elf" -v
STM32_Programmer_CLI -c port=SWD -rst
```

**Lưu ý:**

- `make flash` sử dụng firmware image `.bin` đã đóng gói và nạp tại địa chỉ `0x08000000`.
- `make flash-r` và `make flash-d` nạp trực tiếp các tệp ELF, sử dụng thông tin địa chỉ có trong từng tệp.
- Cả `make flash-r` và `make flash-d` chỉ nạp Bootloader và App A, không nạp App B.
- Hai lệnh này không chạy `pack_firmware.py`, do đó không tạo firmware image đóng gói trong quá trình flash.

## 5. Dọn dẹp Build

| Lệnh | Chức năng |
|---|---|
| `make clean` | Gọi mục tiêu `clean` của CMake trong cả sáu thư mục build Debug và Release. |
| `make clear` | Xóa toàn bộ thư mục `build/`. |
| `make all` | Thực hiện `make clean`, sau đó chạy `make build`. |

### `make clean`

Xóa các sản phẩm build được tạo bởi CMake trong sáu thư mục build Debug và Release. Thư mục và các tệp cấu hình build được giữ lại nếu trình sinh build hỗ trợ cách hoạt động này.

Lệnh này không xóa toàn bộ thư mục `build/`.

### `make clear`

Xóa toàn bộ thư mục `build/`, bao gồm các sản phẩm build và dữ liệu cấu hình CMake đã tạo. Cần chạy lại bước cấu hình CMake trước khi build tiếp.

### `make all`

Mục tiêu `all` được định nghĩa trong Makefile như sau:

```makefile
all: clean build
```

Lệnh này dọn dẹp các sản phẩm build cũ, build lại toàn bộ firmware Release và đóng gói firmware image. Lệnh không xóa toàn bộ thư mục `build/` như `make clear`.