# Tài liệu lưu trữ EEPROM

## 1. Tổng quan

WM03A sử dụng EEPROM ngoài để lưu trữ thông tin định danh thiết bị, cấu hình, trạng thái vận hành, dữ liệu công tơ và nhật ký sự kiện.

Không gian bộ nhớ được chia thành các vùng có địa chỉ bắt đầu và dung lượng cố định. Việc phân vùng giúp các chức năng truy cập đúng khu vực dữ liệu, tránh ghi đè lên dữ liệu của chức năng khác.

| Thông số | Giá trị |
|---|---|
| EEPROM | M95M02-DRMN6TP |
| Dung lượng | 256 KB (262.144 bytes) |
| Kích thước trang | 256 bytes |
| Phạm vi địa chỉ | `0x00000–0x3FFFF` |

## 2. Bản đồ bộ nhớ

EEPROM được chia thành các vùng lưu trữ liên tiếp theo thứ tự địa chỉ. Địa chỉ bắt đầu và dung lượng của mỗi vùng được xác định bằng các macro cấu hình trong file `eeprom_layout.h`.

| Phân vùng | Địa chỉ bắt đầu | Địa chỉ kết thúc | Dung lượng (bytes) | Tỷ lệ (%) | Mục đích |
|---|---:|---:|---:|---:|---|
| Header | `0x00000` | `0x0000F` | 16 | 0,0061% | Nhận diện EEPROM với số magic `0x02468ACE` và xác định phiên bản bố trí dữ liệu. |
| Device Serial | `0x00010` | `0x0001B` | 12 | 0,0046% | Lưu số serial của thiết bị dạng `uint64_t`. |
| Device Activation Status | `0x0001C` | `0x0001F` | 4 | 0,0015% | Lưu trạng thái kích hoạt thiết bị. |
| Device Password | `0x00020` | `0x0011F` | 256 | 0,0977% | Lưu thông tin xác thực mật khẩu của ba cấp người dùng bằng **PBKDF2-HMAC-SHA-256**. |
| Metadata | `0x00120` | `0x0021F` | 256 | 0,0977% | Quản lý chỉ số đọc/ghi và số lượng bản ghi. |
| Runtime Data | `0x00220` | `0x0035F` | 320 | 0,1221% | Lưu trạng thái vận hành cần duy trì qua các lần khởi động; kết hợp với vùng `RAM NOINIT` để duy trì dữ liệu trong RAM qua một số loại reset. |
| IP Endpoint Configuration | `0x00360` | `0x00377` | 24 | 0,0092% | Lưu địa chỉ IPv4 và cổng máy chủ. |
| Module Configuration | `0x00378` | `0x00383` | 12 | 0,0046% | Lưu chu kỳ ghi chốt, chu kỳ gửi dữ liệu và múi giờ. |
| Charge Configuration | `0x00384` | `0x0038F` | 12 | 0,0046% | Lưu các tham số cấu hình bộ sạc như dòng giới hạn đầu vào, dòng sạc, điện áp sạc, bật/tắt LED hiển thị. |
| Pulse Meter Configuration | `0x00390` | `0x00407` | 120 | 0,0458% | Lưu cấu hình đồng hồ đo sử dụng ngõ vào xung. |
| Modbus Meter Configuration | `0x00408` | `0x004CF` | 200 | 0,0763% | Lưu cấu hình đồng hồ đo giao tiếp Modbus RTU. |
| Pressure Sensor Configuration | `0x004D0` | `0x0051F` | 80 | 0,0305% | Lưu cấu hình cảm biến áp suất. |
| Latch | `0x00520` | `0x24F0F` | 150.000 | 57,2205% | Lưu các bản ghi dữ liệu chốt. |
| Event | `0x24F10` | `0x27DEF` | 12.000 | 4,5776% | Lưu các bản ghi sự kiện. |
| Push Status | `0x27DF0` | `0x2CC0F` | 20.000 | 7,6294% | Lưu trạng thái gửi dữ liệu lên máy chủ. |
| Changed Info Log | `0x2CC10` | `0x302BF` | 14.000 | 5,3406% | Lưu lịch sử thay đổi cài đặt và mật khẩu. |
| Vùng chưa phân bổ | `0x302C0` | `0x3FFFF` | 64.832 | 24,7313% | Dung lượng chưa được phân vùng, dành cho mở rộng trong tương lai. |
| **Tổng dung lượng EEPROM** | `0x00000` | `0x3FFFF` | **262.144** | **100,0000%** | |

## 3. Thuật toán bảo vệ bộ nhớ và tăng tuổi thọ EEPROM

### 3.1. Cơ chế sửa lỗi ECC theo nhóm 4 bytes (ECC x4)

EEPROM **M95M02-D** tích hợp cơ chế **Error Correction Code (ECC)** ở mức phần cứng, hoạt động trong suốt quá trình giao tiếp SPI. ECC được áp dụng cho từng nhóm 4 bytes liên tiếp, tại các địa chỉ từ `4*N` đến `4*N + 3`, trong đó `N` là số nguyên không âm.

- **Sửa lỗi khi đọc:** Nếu một bit trong nhóm 4 bytes bị lỗi, ECC có thể phát hiện và tự động sửa bit đó về giá trị đúng, giúp tăng độ tin cậy của dữ liệu đọc.
- **Ảnh hưởng khi ghi dữ liệu:** Mặc dù có thể ghi riêng lẻ từng byte, thao tác ghi vẫn ảnh hưởng đến chu kỳ ghi/xóa (*cycling*) của cả nhóm 4 bytes chứa byte đó.
- **Căn chỉnh dữ liệu 4 bytes:** Các vùng dữ liệu nên được bố trí sao cho địa chỉ bắt đầu và kích thước được căn theo bội số của 4 bytes. Việc căn chỉnh giúp ranh giới dữ liệu phù hợp với các nhóm ECC, tránh để dữ liệu của các vùng khác nhau dùng chung một nhóm 4 bytes và bị ảnh hưởng bởi tần suất cập nhật của nhau.
- **Phân bổ tuổi thọ:** Ngân sách chu kỳ ghi/xóa được tính trên toàn nhóm 4 bytes, không phải từng byte độc lập. Tổng số chu kỳ ghi/xóa của 4 bytes trong cùng nhóm phải nằm trong giới hạn quy định của datasheet.

Do đó, firmware sử dụng cơ chế căn chỉnh kích thước packet bằng `ALIGN4()` và cần bảo đảm địa chỉ bắt đầu của từng vùng dữ liệu cũng được căn chỉnh phù hợp. Lưu ý rằng căn chỉnh không làm tăng trực tiếp tuổi thọ EEPROM, nhưng giúp bố trí dữ liệu phù hợp với cơ chế ECC và quản lý tần suất ghi hiệu quả hơn.

### 3.2. Cơ chế lưu luân phiên (Rotating Slots)

Các vùng dữ liệu được cập nhật thường xuyên sử dụng nhiều khu vực lưu trữ (*slots/areas*) để phân tán số lần ghi và tăng độ tin cậy khi cập nhật dữ liệu.

- **Metadata:** Chia thành 4 khu vực, vùng này lưu các biến index đọc, ghi các bản ghi.
- **Runtime Data:** Chia thành 4 khu vực, vùng này lưu các biến liên quan đến việc đo đếm đồng hồ xung.
- **Kiểm tra khi khởi tạo:** Kiểm tra CRC của các gói dữ liệu để xác định tính hợp lệ trước khi sử dụng. Trường `u32Sequence` hỗ trợ theo dõi thứ tự cập nhật giữa các bản ghi. Giá trị metadata khi khởi tạo sẽ là dữ liệu đọc từ EEPROM có CRC hợp lệ và có `u32Sequence` lớn nhất.

### 3.3. Bộ đệm vòng (Circular Buffer)

Các vùng dữ liệu lưu trữ bản ghi có thể sử dụng cơ chế bộ đệm vòng để quản lý dung lượng hữu hạn của EEPROM.

- **Quản lý bản ghi:** Metadata lưu các chỉ số ghi, chỉ số đọc và số lượng bản ghi tương ứng cho các vùng Latch, Event, Push Status và Changed Info Log.
- **Cập nhật chỉ số:** Các chỉ số được cập nhật khi ghi hoặc đọc bản ghi theo logic của từng vùng dữ liệu.
- **Khi vùng lưu trữ đầy:** Khi chỉ số ghi `SaveIndex` đạt đến giới hạn `MAX_COUNT`, con trỏ được đưa về `0` để tiếp tục ghi theo cơ chế vòng. Đối với hai vùng Latch và Event, nếu `SaveIndex` trùng với `LoadIndex`, bản tin cũ nhất sẽ bị loại bỏ bằng cách tăng `LoadIndex` lên 1 (quay về `0` nếu đạt giới hạn), sau đó lưu bản tin mới tại vị trí `SaveIndex`.

### 3.4. Kiểm tra tính toàn vẹn dữ liệu bằng CRC

Các vùng dữ liệu được bảo vệ bằng mã kiểm tra **CRC-16/MODBUS**. CRC dài 2 bytes và được lưu theo thứ tự little-endian (LSB trước, MSB sau).

- **Khi ghi dữ liệu:** Tính CRC trên phần Data, ghi Data và CRC vào EEPROM, sau đó đọc lại để kiểm tra CRC của dữ liệu đã lưu.
- **Khi đọc dữ liệu:** Đọc Data cùng 2 bytes CRC, tính lại CRC trên Data và so sánh với giá trị đã lưu.
- **Khi CRC hợp lệ:** Dữ liệu vượt qua bước kiểm tra tính toàn vẹn và có thể được sử dụng sau các bước xác thực khác nếu cần.
- **Khi CRC không hợp lệ:** Không sử dụng dữ liệu như dữ liệu đáng tin cậy; firmware xử lý lỗi theo cơ chế tương ứng của vùng lưu trữ.

CRC giúp phát hiện dữ liệu bị sai lệch hoặc hư hỏng, nhưng không tự khôi phục dữ liệu và không bảo đảm tuyệt đối rằng mọi lỗi đều được phát hiện.

## 4. Tài liệu liên quan

- [Kiến trúc phần mềm](../architecture.md)
- [Giao thức](../communication_protocol.md)
