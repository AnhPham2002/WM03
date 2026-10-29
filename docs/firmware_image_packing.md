# Firmware Image Packing

## 1. Tổng quan

Quá trình Firmware Image Packing tạo ra các firmware image từ các file binary đã biên dịch. Script `pack_firmware.py` thực hiện việc đọc cấu hình bộ nhớ, ghép dữ liệu firmware vào đúng địa chỉ, cập nhật metadata và tính CRC32 cho từng firmware.

Quá trình đóng gói tạo ra hai file:

- `WM03_Firmware_Flash.bin`: Firmware image dùng để nạp trực tiếp vào bộ nhớ Flash.
- `WM03_Firmware_Ota.bin`: Firmware image dùng cho quá trình cập nhật firmware qua OTA.

## 2. Đầu vào và Đầu ra

### Đầu vào

| Tệp | Mục đích |
|---|---|
| `Users/Configuration/flash_layout.h` | Cung cấp địa chỉ bộ nhớ Flash, vị trí metadata và kích thước tối đa của mỗi slot. |
| `Users/Application/app_storage.h` | Cung cấp giá trị trạng thái firmware hợp lệ và phiên bản firmware. |
| `build/Release/Boot/WM03_BOOT.bin` | Binary của Bootloader. |
| `build/Release/App_A/WM03_APP_A.bin` | Binary của App A. |
| `build/Release/App_B/WM03_APP_B.bin` | Binary của App B. |

### Đầu ra

| Tệp | Mục đích |
|---|---|
| `build/Release/WM03_Firmware_Flash.bin` | File dùng để nạp chương trình lần đầu, địa chỉ bắt đầu `0x08000000`. |
| `build/Release/WM03_Firmware_Ota.bin` | File dùng để đưa lên server hoặc PC tool với mục đích OTA. |

## 3. Cấu hình Bộ nhớ

Script đọc các hằng số từ `flash_layout.h` để xác định vị trí đặt firmware và metadata. Các địa chỉ và giá trị thực tế được lấy từ mã nguồn cấu hình, không được định nghĩa độc lập trong script. Các hằng số chính bao gồm:

| Hằng số | Ý nghĩa |
|---|---|
| `BOOTLOADER_START_ADDR` | Địa chỉ bắt đầu của Bootloader. |
| `SLOT_A_METADATA_ADDR` | Địa chỉ metadata của Slot A. |
| `SLOT_A_START_ADDR` | Địa chỉ bắt đầu của firmware Slot A. |
| `SLOT_B_METADATA_ADDR` | Địa chỉ metadata của Slot B. |
| `SLOT_B_START_ADDR` | Địa chỉ bắt đầu của firmware Slot B. |
| `FIRMWARE_MAX_SIZE` | Kích thước tối đa được quy định cho firmware trong mỗi slot. |
| `FIRMWARE_VALID` | Giá trị trạng thái biểu thị firmware hợp lệ. |
| `FIRMWARE_VERSION` | Phiên bản firmware được ghi vào metadata. |

## 4. Cấu trúc Firmware Metadata

Mỗi firmware slot có một vùng metadata dài 28 byte (được định nghĩa bởi `Firmware_Metadata_t` trong `flash_layout.h`). Script ghi metadata tại vùng metadata tương ứng của slot.

| Offset | Kích thước | Trường | Mô tả |
|---:|---:|---|---|
| 0 | 4 byte | Sequence | Giá trị khởi tạo bằng `0`. |
| 4 | 12 byte | Version | Phiên bản firmware dạng ASCII, phần còn lại được đệm bằng `0x00`. |
| 16 | 4 byte | Status | Trạng thái firmware, lấy từ `FIRMWARE_VALID`. |
| 20 | 4 byte | Firmware Size | Kích thước firmware tính bằng byte. |
| 24 | 4 byte | CRC32 | Giá trị CRC32 của firmware binary. |
| **Tổng** | **28 byte** | | |

Phiên bản firmware sử dụng định dạng `Vxx.xx.xxx`, với độ dài tối đa 10 ký tự. Phiên bản được lưu dưới dạng ASCII trong trường Version có kích thước 12 byte; các byte còn lại được đệm bằng 0x00 để kết thúc chuỗi.

## 5. Tính CRC32

Firmware sử dụng thuật toán CRC32 tương thích với bộ CRC phần cứng (Hardware CRC) của STM32 để kiểm tra tính toàn vẹn dữ liệu firmware. Script `pack_firmware.py` tính CRC32 trên máy tính trong quá trình đóng gói; giá trị CRC được ghi vào metadata để firmware có thể kiểm tra lại khi cần.

### Tham số thuật toán

| Tham số | Giá trị |
|---|---|
| Polynomial | `0x04C11DB7` |
| Giá trị khởi tạo | `0xFFFFFFFF` |
| Độ rộng dữ liệu | 32 bit |
| Thứ tự xử lý bit | MSB-first |
| Final XOR | Không sử dụng |

### Quy trình tính

1. Đọc nội dung firmware binary gốc.
2. Chia dữ liệu thành các word 32 bit theo thứ tự byte big-endian.
3. Nếu word cuối chưa đủ 4 byte, đệm `0x00` vào bên phải.
4. Xử lý các word bằng thuật toán CRC32 tương thích với cấu hình CRC phần cứng STM32.
5. Ghi kết quả CRC32 vào trường `CRC32` trong metadata.

CRC được tính trên dữ liệu firmware binary gốc, trước khi firmware được ghép vào image và trước khi metadata được cập nhật.

**Lưu ý:** Kết quả CRC chỉ tương thích khi script và bộ CRC phần cứng STM32 sử dụng cùng polynomial, giá trị khởi tạo, quy tắc đảo bit, thứ tự byte và cách xử lý dữ liệu cuối. Cần đối chiếu cấu hình thực tế của peripheral CRC trên STM32 với implementation trong script.

## 6. Tạo Flash Image

File đầu ra:

`build/Release/WM03_Firmware_Flash.bin`

### Quy trình

1. Xác định địa chỉ bắt đầu image từ `BOOTLOADER_START_ADDR`.
2. Khởi tạo vùng image có kích thước cố định 256 KiB, toàn bộ byte được đặt thành `0xFF`.
3. Ghép `WM03_BOOT.bin` tại địa chỉ bắt đầu của Bootloader.
4. Ghép `WM03_APP_A.bin` tại `SLOT_A_START_ADDR`.
5. Cập nhật metadata của Slot A, bao gồm phiên bản, trạng thái hợp lệ, kích thước và CRC32.
6. Ghi dữ liệu image ra file `WM03_Firmware_Flash.bin`.

Đây là firmware image dùng để nạp firmware lần đầu hoặc khôi phục firmware qua giao tiếp SWD. Image chứa Bootloader và App A, được bố trí theo địa chỉ bộ nhớ Flash và nạp bắt đầu từ địa chỉ `0x08000000`.

## 7. Tạo OTA Image

File đầu ra:

`build/Release/WM03_Firmware_Ota.bin`

### Quy trình

1. Xác định địa chỉ bắt đầu image từ `SLOT_A_METADATA_ADDR`.
2. Xác định địa chỉ kết thúc giới hạn từ `SLOT_B_END_ADDR`, với:

   `SLOT_B_END_ADDR = SLOT_B_START_ADDR + FIRMWARE_MAX_SIZE`

3. Tính kích thước image bằng hiệu giữa địa chỉ kết thúc và địa chỉ bắt đầu. Địa chỉ kết thúc là giới hạn loại trừ, không thuộc vùng image.
4. Khởi tạo toàn bộ image bằng `0xFF`.
5. Ghép `WM03_APP_A.bin` tại `SLOT_A_START_ADDR`.
6. Ghép `WM03_APP_B.bin` tại `SLOT_B_START_ADDR`.
7. Cập nhật metadata cho cả Slot A và Slot B.
8. Ghi dữ liệu ra file `WM03_Firmware_Ota.bin`.

Đây là firmware image dùng để cập nhật firmware qua OTA. Image bao gồm metadata của Slot A và Slot B cùng dữ liệu App A và App B, được bố trí theo địa chỉ bộ nhớ tương ứng để Bootloader sử dụng trong quy trình cập nhật OTA. Các vùng địa chỉ không chứa dữ liệu firmware hoặc metadata được điền bằng `0xFF`.

## 8. Kiểm tra và Giới hạn

Script thực hiện các kiểm tra chính sau:

- Địa chỉ ghép dữ liệu không được nằm trước địa chỉ bắt đầu image.
- Dữ liệu binary phải nằm trong phạm vi image.
- Vùng metadata phải nằm hoàn toàn trong image.
- Phiên bản firmware phải nhỏ hơn 12 byte.
- Kích thước firmware và CRC32 được ghi vào metadata.

Tuy nhiên, cần lưu ý rằng kiểm tra giới hạn của image không đồng nghĩa với kiểm tra kích thước tối đa của từng firmware slot. Cần đảm bảo kích thước của App A và App B không vượt quá vùng nhớ được cấp phát trong `flash_layout.h`.

Ngoài ra, script chỉ tạo và đóng gói firmware image; việc xác minh firmware có thể khởi động thành công, tương thích với Bootloader hoặc đáp ứng quy trình cập nhật OTA cần được kiểm tra riêng.

## 9. Tích hợp với Quy trình Build

Lệnh:

```bash
make build
```

thực hiện hai bước:

1. Build Boot, App A và App B ở cấu hình Release.
2. Chạy `pack_firmware.py` để tạo Flash image và OTA image.

Lệnh `make build-r` chỉ build các thành phần Release mà không thực hiện đóng gói. Để tạo firmware image sau khi build riêng, chạy:

```bash
python pack_firmware.py
```

Các file đầu ra được lưu trong `build/Release/`.