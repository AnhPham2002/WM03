# Giao thức truyền thông

## 1. Tổng quan và cấu trúc khung truyền

### 1.1. Kênh truyền

Thiết bị WM03A giao tiếp qua hai kênh với cùng một định dạng khung truyền.

| Kênh truyền | Giao thức vật lý |
|---|---|
| Cổng RS485 | Sử dụng cổng **UART1** với IC chuyển đổi MAX3485ESA |
| Cổng 4G | Sử dụng cổng **UART3** kết nối với TCP qua module A7677S-LANV |

### 1.2. Định dạng khung truyền

```text
Byte:   0      1      2..9      10    11    12..13     14..N      N+1..N+2
      +-----+-----+----------+-----+-----+--------+-------------+---------+
      | STX |TYPE |  SERIAL  | CMD | ID  | LEN    | MESSAGE     | CRC16   |
      | 1B  | 1B  |8B uint64 | 1B  | 1B  | 2B LE  | nB          | 2B LE   |
      +-----+-----+----------+-----+-----+--------+-------------+---------+
      |<---------------- Header 14B ------------->|<- MESSAGE ->|<--CRC-->|
```


<a id="table_frame_format"></a>

| Trường dữ liệu | Offset | Kích thước | Giá trị / Định dạng | Mô tả chức năng |
|---|---|---|---|---|
| STX | 0 | 1 Byte | `0x5A` (`PROTOCOL_START_OF_FRAME`) | Byte bắt đầu khung truyền |
| TYPE | 1 | 1 Byte | `0x03` (`PROTOCOL_MODULE_TYPE`) | Định danh module WM03A |
| SERIAL | 2 | 8 Bytes | `uint64` LE | Số Serial định danh thiết bị duy nhất |
| CMD | 10 | 1 Byte | Enum `Protocol_Cmd_Code_t` |	Mã lệnh chức năng |
| ID | 11 | 1 Byte | Byte ID trong Namespace CMD | Mã định danh tham số / sub-command |
| LEN | 12 | 2 Bytes | `uint16` LE | Độ dài phần MESSAGE (Nếu dùng AES, là bội số của 16) |
| MESSAGE | 14 | n Bytes | Ciphertext AES-128 hoặc Plaintext ACK server | Khối dữ liệu mang thông tin |
| CRC16 | 14+n | 2 Bytes | `uint16` LE (Modbus CRC16) | Mã kiểm tra toàn vẹn từ STX đến hết MESSAGE |

**Hằng số kích thước**
- **Header:** 14 bytes.
- **Khung truyền có độ dài nhỏ nhất:** 16 bytes gồm header 14 bytes + 2 bytes CRC (khi message rỗng).

## 2. Mã hoá và kiểm tra tính toàn vẹn

### 2.1. Thuật toán mã hoá

### 2.2. Dữ liệu trước mã hoá (Plaintext)

Khung truyền được định nghĩa ở mục [Bảng chi tiết các trường dữ liệu trong frame](#table_frame_format), phần message trước mã hoá có dạng như sau:

```text
      +----------+---------+---------+----------+
      | LEN      | PAYLOAD | CRC16   | PADDING  |
      | 2B       | nB      | 2B LE   | mB       |
      +----------+---------+---------+----------+
```

- **LEN:** Độ dài PAYLOAD (`uint16_t` 2 bytes LE).
- **PAYLOAD:** Dữ liệu thực, độ dài dược xác định bởi **LEN**.
- **CRC16:** Mã kiểm tra tính toàn vẹn của message khi giải mã tính từ **LEN** đến hết **PAYLOAD** (`uint16_t` 2 bytes LE).
- **PADDING:** Chèn thêm m bytes bất kỳ (m≥0) sao cho `2 + n + 2 + m` là bội của 16.

### 2.3. Dữ liệu bản tin phản hồi (ACK)

Để phản hồi (ACK) cho bản tin nào thì `CMD` và `ID` của bản tin ACK giống với `CMD` và `ID` của bản tin đó. Message được đóng gói như ở mục [2.2](#22-dữ-liệu-trước-mã-hoá-plaintext), payload của bản tin ACK bao gồm:

```text
      +-----------+-------------+
      | DATE_TIME | RESULT_CODE |
      | 6B        | 1B          |
      +-----------+-------------+
```

- **DATE_TIME:** Ngày và giờ gửi bản tin ACK có dạng `YY MM DD HH mm SS`.
- **RESULT_CODE:** Kết quả của mã lỗi trả về ở [Bảng 2-1](#table_error_code)

<a id="table_error_code"></a>

| Hex | Mã lỗi | Ý nghĩa |
|---:|---|---|
| `0x00` | `PROTOCOL_ERR_SUCCESS` | Thành công (không có bất kỳ lỗi nào) |
| `0x01` | `PROTOCOL_ERR_CMD_INVALID` | Mã `CMD` hoặc `ID` không hợp lệ |
| `0x02` | `PROTOCOL_ERR_CRC16_INVALID` | CRC16 ở cuối khung truyền không hợp lệ |
| `0x03` | `PROTOCOL_ERR_FRAME_INVALID` | Khung truyền không đúng với format ở [Định dạng khung truyền](#12-định-dạng-khung-truyền) |
| `0x04` | `PROTOCOL_ERR_FW_NO_SPACE` | Dung lượng file cập nhật lớn hơn dung lượng sẵn có của thiết bị (khi có yêu cầu OTA) |
| `0x05` | `PROTOCOL_ERR_FW_CRC32_FAILED` | CRC32 không hợp lệ sau khi ghi và tính toán gói tin cuối cùng (khi thực hiện OTA) |
| `0x10` | `PROTOCOL_ERR_PASSWORD_INCORRECT` | Sai mật khẩu thiết bị khi truy cập thiết bị qua cổng RS485 |
| `0x11` | `PROTOCOL_ERR_ACCESS_DENIED` | Bị từ chối truy cập qua cổng RS485 do quyền truy cập yêu cầu lớn hơn quyền truy cập hiện tại |

**Ngoại lệ:** Để giảm tải xử lý trên server thì gói tin server ack (`CMD = 0x04`, `ID = 0xFF`) sẽ gửi message dưới dạng plaintext (không mã hoá), message khi đó sẽ không còn m bytes padding. Tất cả ACK của các lệnh `CMD` khác vẫn mã hoá đầy đủ.

## 3. Tập lệnh CMD và các ID theo CMD tương ứng

| Hex | CMD | Mô tả | Kênh áp dụng |
|---:|---|---|:---:|
| `0x00` | `CMD_ACCESS` | Xác thực / thiết lập quyền truy cập | RS485 |
| `0x01` | `CMD_GET` | Đọc cấu hình | RS485 / 4G |
| `0x02` | `CMD_SET` | Ghi cấu hình hoặc thực hiện lệnh | RS485 / 4G |
| `0x03` | `CMD_QUERY` | Truy vấn dữ liệu | RS485 / 4G |
| `0x04` | `CMD_PUSH` | Dữ liệu đẩy từ thiết bị | 4G |
| `0x05` | `CMD_OTA` | Cập nhật firmware OTA | RS485 / 4G |

### 3.1. ACCESS (`0x00`) — Truy cập thiết bị

Lệnh `0x00` chỉ áp dụng cho giao tiếp qua cổng RS485. Với giao tiếp 4G, thiết bị bỏ qua cơ chế phân quyền truy cập.

Thời gian timeout được định nghĩa bởi `PROTOCOL_ACCESS_TIMEOUT` là 5 phút. Khi hết thời gian này, cấp quyền truy cập được đưa về `ACCESS_LEVEL_0`.

Mật khẩu có độ dài cho phép là từ 4 đến 32 ký tự.

| Hex | ID | Payload | Ý nghĩa |
|---:|---|---|---|
| `0x00` | `ACCESS_LV_0` | [Bắt tay](#411-bắt-tay-với-thiết-bị) | Bắt tay với thiết bị và truy cập mức 0 (Operator) |
| `0x01` | `ACCESS_LV_1` | [Đăng nhập](#412-đăng-nhập-vào-thiết-bị) | Xác thực và cấp quyền truy cập mức 1 (Supervisor) |
| `0x02` | `ACCESS_LV_2` | [Đăng nhập](#412-đăng-nhập-vào-thiết-bị) | Xác thực và cấp quyền truy cập mức 2 (Engineer) |
| `0x03` | `ACCESS_LV_3` | [Đăng nhập](#412-đăng-nhập-vào-thiết-bị) | Xác thực và cấp quyền truy cập mức 3 (Factory / Admin) |

#### 3.1.1. Bắt tay với thiết bị

#### 3.1.2. Đăng nhập vào thiết bị

Payload bao gồm độ dài mật khẩu và mật khẩu đăng nhập vào thiết bị với quyền xác định bởi `ID`:
- **LEN:** Độ dài mật khẩu (1 byte), khoảng giá trị từ `4` đến `32`.
- **PASSWORD:** Mật khẩu tương ứng với quyền truy cập (có độ dài xác định bởi **LEN**).

Sau khi gửi lệnh đăng nhập từ PC tool xuống thiết bị, thiết bị xác nhận mật khẩu nhận được tại quyền truy cập đó. Nếu đúng mật khẩu sẽ gửi bản tin ACK với mã `PROTOCOL_ERR_SUCCESS` (`0x00`). Nếu sai mật khẩu sẽ gửi bản tin ACK với mã `PROTOCOL_ERR_PASSWORD_INCORRECT` (`0x10`).

### 3.2. GET (`0x01`) & SET (`0x02`) — Đọc và ghi cấu hình hệ thống

Các lệnh `GET` (`0x01`) và `SET` (`0x02`) được sử dụng để đọc và ghi cấu hình hệ thống.

Cột **Config ID (Hex)** trong các bảng dưới đây là mã của tham số cấu hình nằm trong payload, không phải mã lệnh `GET`/`SET`. Quyền truy cập tương ứng với từng lệnh theo mức Lv1, Lv2, Lv3 (xem ghi chú 5 ở cuối mục).

<a id="table_get_set"></a>

| Hex | ID | GET | SET | Payload GET (yêu cầu) | Payload GET (phản hồi) | Payload SET (yêu cầu) | Ý nghĩa |
|---:|---|---|---|---|---|---|---|
| `0x00` | `CONFIG_MODULE_SERIAL` | Lv1 | Lv3 | `NULL` | `uint64_t` — 8 bytes (LE) | `uint64_t` — 8 bytes (LE) | Đọc hoặc ghi số serial của module |
| `0x01` | `CONFIG_MODULE_ACTIVATION_STATUS` | Lv1 | Lv3 | `NULL` | `0x00` hoặc `0x01` — 1 byte | `0x00` hoặc `0x01` — 1 byte | Đọc hoặc ghi trạng thái kích hoạt module (`0x00`: Chưa kích hoạt / `0x01`: Đã kích hoạt) |
| `0x02` | `CONFIG_TIME` | Lv1 | Lv2 | `NULL` | `YY MM DD HH mm SS` — 6 bytes hex | `YY MM DD HH mm SS` — 6 bytes hex | Đọc hoặc ghi thời gian của module |
| `0x03` | `CONFIG_IP_ENDPOINT` | Lv1 | Lv2 | `NULL` | [IP Port](#321-cài-đặt-ip-port) — 20 bytes ASCII | [IP Port](#321-cài-đặt-ip-port) — 20 bytes ASCII | Đọc hoặc ghi địa chỉ IP và cổng kết nối |
| `0x04` | `CONFIG_MODULE` | Lv1 | Lv2 | [Danh sách ID](#list_parameter_id) | [Danh sách cặp ID - Value](#list_parameter_id_value) | [Danh sách cặp ID - Value](#list_parameter_id_value) | Đọc hoặc ghi [cấu hình module](#322-cài-đặt-module) |
| `0x05` | `CONFIG_CHARGE` | Lv3 | Lv3 | [Danh sách ID](#list_parameter_id) | [Danh sách cặp ID - Value](#list_parameter_id_value) | [Danh sách cặp ID - Value](#list_parameter_id_value) | Đọc hoặc ghi [cấu hình sạc](#323-cài-đặt-sạc) |
| `0x06` | `CONFIG_PULSE_METER` | Lv1 | Lv2 | [Thứ tự đồng hồ + Danh sách ID](#device_list_parameter_id) | [Thứ tự đồng hồ + Danh sách cặp ID - value](#device_list_parameter_id_value) | [Thứ tự đồng hồ + Danh sách cặp ID - value](#device_list_parameter_id_value) | Đọc hoặc ghi [cấu hình đồng hồ xung](#324-cài-đặt-đồng-hồ-xung) |
| `0x07` | `CONFIG_MODBUS_METER` | Lv1 | Lv2 | [Thứ tự đồng hồ + Danh sách ID](#device_list_parameter_id) | [Thứ tự đồng hồ + Danh sách cặp ID - value](#device_list_parameter_id_value) | [Thứ tự đồng hồ + Danh sách cặp ID - value](#device_list_parameter_id_value) | Đọc hoặc ghi [cấu hình đồng hồ Modbus](#325-cài-đặt-đồng-hồ-modbus) |
| `0x08` | `CONFIG_PRESSURE_SENSOR` | Lv1 | Lv2 | [Thứ tự cảm biến + Danh sách ID](#device_list_parameter_id) | [Thứ tự cảm biến + Danh sách cặp ID - value](#device_list_parameter_id_value)  | [Thứ tự cảm biến + Danh sách cặp ID - value](#device_list_parameter_id_value) | Đọc hoặc ghi [cấu hình cảm biến áp suất](#device_list_parameter_id) |
| `0x09` | `CONFIG_REBOOT` | — | Lv3 | Không hỗ trợ | Không hỗ trợ | `NULL` | Khởi động lại thiết bị |
| `0x0A` | `CONFIG_POWER_OFF` | — | Lv3 | Không hỗ trợ | Không hỗ trợ | `NULL` | Tắt nguồn thiết bị |
| `0x0B` | `CONFIG_RESET_SETTING` | — | Lv3 | Không hỗ trợ | Không hỗ trợ | `NULL` | Khôi phục cài đặt |
| `0x0C` | `CONFIG_CHANGE_PASSWORD` | — | Lv1 | Không hỗ trợ | Không hỗ trợ | [Mật khẩu cũ + Quyền cần đổi mật khẩu + Mật khẩu mới](#327-đổi-mật-khẩu) | Đổi mật khẩu |
| `0x10` | `CONFIG_RESET_PASSWORD` | — | Lv3 | Không hỗ trợ | Không hỗ trợ | `NULL` | Đặt lại mật khẩu mặc định ở tất cả các quyền |
| `0x11` | `CONFIG_MCU_RESET_COUNT` | Lv3 | Lv3 | `NULL` | `uint8_t` — 1 byte | `uint8_t` — 1 byte | Đọc hoặc ghi số lần reset MCU |
| `0x13` | `CONFIG_LATCH_IMMEDIATELY` | — | Lv3 | Không hỗ trợ | Không hỗ trợ | `NULL` | Chốt dữ liệu ngay lập tức |
| `0x14` | `CONFIG_PUSH_IMMEDIATELY` | — | Lv3 | Không hỗ trợ | Không hỗ trợ | `NULL` | Đẩy dữ liệu ngay lập tức |
| `0x15` | `CONFIG_EVENT_CREATE` | — | Lv3 | Không hỗ trợ | Không hỗ trợ | [Loại thiết bị + Thứ tự thiết bị + Sự kiện](#328-tạo-sự-kiện-giả) — 3 bytes | Tạo sự kiện giả |
| `0x16` | `CONFIG_ERASE_MEASUREMENT_DATA` | — | Lv3 | Không hỗ trợ | Không hỗ trợ | `NULL` | Xóa dữ liệu đo lường |
| `0x17` | `CONFIG_ERASE_EVENT_DATA` | — | Lv3 | Không hỗ trợ | Không hỗ trợ | `NULL` | Xóa dữ liệu sự kiện |
| `0x18` | `CONFIG_ERASE_PUSH_STATUS_DATA` | — | Lv3 | Không hỗ trợ | Không hỗ trợ | `NULL` | Xóa dữ liệu trạng thái push |
| `0x19` | `CONFIG_ERASE_CHANGE_INFO_LOG_DATA` | — | Lv3 | Không hỗ trợ | Không hỗ trợ | `NULL` | Xóa nhật ký thay đổi thông tin |
| `0x1A` | `CONFIG_ERASE_MCU_RESET_DATA` | — | Lv3 | Không hỗ trợ | Không hỗ trợ | `NULL` | Xóa dữ liệu reset MCU |
| `0x1B` | `CONFIG_ERASE_LOG_DATA` | — | Lv3 | Không hỗ trợ | Không hỗ trợ | `NULL` | Xóa dữ liệu nhật ký |

**Quy ước chung**

- Các lệnh có quyền truy cập `—` là các `ID` không được hỗ trợ với `CMD` tương ứng ở [Bảng các ID của lệnh GET và SET](#table_get_set).
- Phía yêu cầu là server hoặc PC tool, phía phản hồi là thiết bị.
- Đối với lệnh `SET`, sau khi nhận được yêu cầu thì thiết bị sẽ phản hồi `ACK` báo thành công hoặc lỗi lên phía yêu cầu.

#### **Payload SET (yêu cầu) hoặc payload GET (phản hồi) cho các cấu hình đặc biệt** <a id="list_parameter_id_value"></a>

Đối với các lệnh cấu hình sử dụng `Parameter ID` bao gồm các lệnh `0x04` (`CONFIG_MODULE`), `0x05` (`CONFIG_CHARGE`), `0x06` (`CONFIG_PULSE_METER`), `0x07` (`CONFIG_MODBUS_METER`) và `0x08` (`CONFIG_PRESSURE_SENSOR`), Payload Data gồm một hoặc nhiều cặp `Parameter ID` và `Parameter Value` nối tiếp nhau:

```text
[Parameter ID][Parameter Value][Parameter ID][Parameter Value]...[Parameter ID][Parameter Value]
```

Trong đó:

- `Parameter ID`: mã định danh tham số cần cấu hình.
- `Parameter Value`: giá trị tương ứng với tham số. Kiểu dữ liệu và độ dài phụ thuộc vào `Parameter ID`.

Các tham số không xuất hiện trong Payload Data sẽ giữ nguyên giá trị hiện tại của thiết bị đối với lệnh `SET` hoặc không gửi lên phía yêu cầu đối với lệnh `GET`.

#### **Payload GET (yêu cầu) cho các cấu hình đặc biệt** <a id="list_parameter_id"></a>

Các lệnh được hỗ trợ gồm `0x04` (`CONFIG_MODULE`), `0x05` (`CONFIG_CHARGE`), `0x06` (`CONFIG_PULSE_METER`), `0x07` (`CONFIG_MODBUS_METER`) và `0x08` (`CONFIG_PRESSURE_SENSOR`), Payload Data gồm danh sách các `Parameter ID` cần đọc:

```text
[Parameter ID][Parameter ID]...[Parameter ID]
```

Thiết bị trả về dữ liệu tương ứng với các tham số được yêu cầu. Thứ tự và định dạng dữ liệu phản hồi tuân theo quy định của từng lệnh.

#### **Các nhóm cấu hình có nhiều thiết bị**

Đối với các nhóm cấu hình đồng hồ xung `0x06` (`CONFIG_PULSE_METER`), đồng hồ Modbus `0x07` (`CONFIG_MODBUS_METER`) và cảm biến áp suất `0x08` (`CONFIG_PRESSURE_SENSOR`), Payload Data bắt đầu bằng một byte `Device Index` để xác định đồng hồ / cảm biến cần thao tác.

- **Lệnh SET (yêu cầu) hoặc GET (phản hồi):** <a id="device_list_parameter_id_value"></a>

```text
[Device Index][Parameter ID][Parameter Value][Parameter ID][Parameter Value]...[Parameter ID][Parameter Value]
```

- **Lệnh GET (yêu cầu):** <a id="device_list_parameter_id"></a>

```text
[Device Index][Parameter ID][Parameter ID]...[Parameter ID]
```

Với đồng hồ xung và đồng hồ Modbus, `Device Index` có giá trị từ `0` đến `3`, tương ứng với thiết bị thứ nhất đến thiết bị thứ tư. Với cảm biến áp suất, `Device Index` có giá trị `0` hoặc `1`, tương ứng với cảm biến áp suất thứ nhất và thứ hai.

Độ dài của từng `Parameter Value` được xác định bởi kiểu dữ liệu của tham số tương ứng. Các trường dữ liệu nhiều byte sử dụng thứ tự little-endian (LE).

#### 3.2.1. Cài đặt IP Port

| Offset | Trường | Kích thước | Định dạng |
|---:|---|---:|---|
| 0 | IP Address | 15 bytes | ASCII |
| 15 | Port | 5 bytes | ASCII |

Nếu dữ liệu của mỗi trường ngắn hơn kích thước quy định, phần còn thiếu được bổ sung bằng byte `0x00` ở cuối trường đó.

**Ví dụ:** Địa chỉ IP `14.225.244.63`, Port `4399`.
```text
31 34 2E 32 32 35 2E 32 34 34 2E 36 33 00 00 34 33 39 39 00
```

#### 3.2.2. Cài đặt module

Các tham số cấu hình module được định nghĩa bởi kiểu liệt kê `Config_Module_Id_t`.

| Parameter ID (Hex) | Tên | Kiểu dữ liệu | Độ dài dữ liệu | Chú thích |
|---:|---|---|---:|---|
| `0x00` | `CONFIG_MODULE_LATCH_PERIOD` | `uint16_t` | 2 bytes (LE) | Chu kỳ chốt dữ liệu (phút) |
| `0x01` | `CONFIG_MODULE_PUSH_PERIOD` | `uint16_t` | 2 bytes (LE) | Chu kỳ đẩy dữ liệu lên máy chủ (phút) |
| `0x02` | `CONFIG_MODULE_TIMEZONE` | `float` | 4 bytes (LE) | Múi giờ, độ phân giải 0,25 giờ (15 phút) |

**Ví dụ:** Bên phía yêu cầu cần lấy chu kỳ chốt và chu kỳ đẩy của thiết bị, (thiết bị có chu kỳ chốt là 15 phút và chu kỳ đẩy là 60 phút), khi đó hai bên sẽ có cùng mã lệnh `CMD` là `GET` và `ID` là `CONFIG_MODULE`, payload truyền và nhận giữa hai bên như sau:
```text
[Host -> Device]: 00 01
[Device -> Host]: 00 0F 00 01 3C 00
```

**Ví dụ:** Bên phía yêu cầu muốn cài đặt chu kỳ chốt 30 phút và chu kỳ gửi là 6 giờ, thiết bị phản hồi thành công vào lúc 19:25:35 20-10-2026, khi đó hai bên sẽ có cùng mã lệnh `CMD` là `SET` và `ID` là `CONFIG_MODULE`, payload truyền và nhận giữa hai bên như sau:
```text
[Host -> Device]: 00 1E 00 01 68 01
[Device -> Host]: 1A 0A 14 13 19 23 00
```

#### 3.2.3. Cài đặt sạc

Các tham số cấu hình sạc được định nghĩa bởi kiểu liệt kê `Config_Charge_Id_t`.

| Parameter ID (Hex) | Tên | Kiểu dữ liệu | Độ dài dữ liệu | Chú thích |
|---:|---|---|---:|---|
| `0x00` | `CONFIG_CHARGE_IINLIM` | `uint16_t` | 2 bytes (LE) | Giới hạn dòng điện đầu vào của bộ sạc (mA) |
| `0x01` | `CONFIG_CHARGE_VOLTAGE` | `uint16_t` | 2 bytes (LE) | Điện áp sạc pin (mV) |
| `0x02` | `CONFIG_CHARGE_CURRENT` | `uint16_t` | 2 bytes (LE) | Dòng sạc pin (mA) |
| `0x03` | `CONFIG_CHARGE_LED` | `uint8_t` | 1 byte | Bật/tắt LED báo sạc (`0x00`: Tắt, `0x01`: Bật) |

Payload truyền và nhận giữa bên yêu cầu và thiết bị tương tự như [Cài đặt module](#322-cài-đặt-module).

#### 3.2.4. Cài đặt đồng hồ xung

Các tham số cấu hình đồng hồ xung được định nghĩa bởi kiểu liệt kê `Config_Pulse_Meter_Id_t`.

| Parameter ID (Hex) | Tên | Kiểu dữ liệu | Độ dài dữ liệu | Chú thích |
|---:|---|---|---:|---|
| `0x00` | `CONFIG_PULSE_METER_ENABLE` | `uint8_t` | 1 byte | Bật/tắt chức năng đọc đồng hồ xung (`0x00`: Không, `0x01`: Có) |
| `0x01` | `CONFIG_PULSE_METER_SERIAL_NUMBER` | `uint8_t[]` | 20 bytes (ASCII) | Số serial đồng hồ xung; chuỗi ngắn hơn 20 bytes được padding `0x00` ở cuối |
| `0x02` | `CONFIG_PULSE_METER_PULSE_FACTOR` | `uint16_t` | 2 bytes (LE) | Hệ số xung (xung/m³) |
| `0x03` | `CONFIG_PULSE_METER_PULSE_TYPE` | `uint8_t` | 1 byte | [Kiểu đồng hồ xung](#pulse_meter-type) |
| `0x04` | `CONFIG_PULSE_METER_PIN1` | `uint8_t` | 1 byte | Chân đọc tín hiệu xung 1 (`0x01`–`0x04`: Chân 1–4) |
| `0x05` | `CONFIG_PULSE_METER_PIN2` | `uint8_t` | 1 byte | Chân đọc tín hiệu xung 2 (`0x00`: Không dùng, `0x01`–`0x04`: Chân 1–4) |
| `0x06` | `CONFIG_PULSE_METER_EDGE_TYPE` | `uint8_t` | 1 byte | Sườn tín hiệu dùng để đếm xung 1 (`0x01`: Cạnh lên, `0x02`: Cạnh xuống) |
| `0x07` | `CONFIG_PULSE_METER_DATA` | `double` + `double` | 16 bytes tổng cộng | Tổng lưu lượng xuôi (m³) và tổng lưu lượng ngược (m³), mỗi giá trị 8 bytes (LE) |

Các kiểu đồng hồ xung được hỗ trợ: <a id="pulse_meter-type"></a>

| Hex | Pulse Meter Type | Chú thích |
|---:|---|---|
| `0x00` | `PULSE_TYPE_SINGLE` | Đồng hồ có một tín hiệu đếm xung |
| `0x01` | `PULSE_TYPE_ENCODER_AB` | Đồng hồ có hai tín hiệu encoder A/B |
| `0x02` | `PULSE_TYPE_DIRECTION` | Đồng hồ có tín hiệu đếm xung và tín hiệu xác định chiều dòng chảy |

**Chú ý:**

- Khi sử dụng `PULSE_TYPE_SINGLE`, đặt `CONFIG_PULSE_METER_PIN2` bằng `0x00` (không dùng).
- Số lượng đồng hồ xung tối đa là 4. Số lượng thực tế có thể giảm nếu sử dụng các loại đồng hồ cần hai dây tín hiệu.

**Ví dụ:** Bên phía yêu cầu muốn biết thông số của đồng hồ xung thứ ba đang cài đặt trong thiết bị, đồng hồ xung thứ ba đang có các thông số cài đặt:
- Trạng thái sử dụng: Có.
- Số serial là `7194028561930482`.
- Hệ số xung là 100 xung/m³.
- Kiểu đồng hồ xung là đồng hồ có một tín hiệu đếm xung.
- Chân đọc xung 1 của đồng hồ là chân 2 của thiết bị.
- Chân đọc xung 2 của đồng hồ không dùng.
- Sườn tín hiệu là cạnh xuống.
- Tổng lưu lượng xuôi là 45.71 m³.
- Tổng lưu lượng ngược là 0 m³.

Từ những thông tin trên, hai bên có cùng mã lệnh `CMD` là `SET` và `ID` là `CONFIG_PULSE_METER`, payload truyền và nhận giữa hai bên như sau:
```text
[Host -> Device]: 02 00 01 02 03 04 05 06 07
[Device -> Host]: 02 00 01 01 37 31 39 34 30 32 38 35 36 31 39 33 30 34 38 32 00 00 00 00 02 64 00 03 00 04 02 05 00 06 02 07 7B 14 AE 47 E1 DA 46 40 00 00 00 00 00 00 00 00
```

**Ví dụ:** Khi phía yêu cầu muốn cài đặt thông số cho đồng hồ xung thứ nhất cho thiết bị có thông số như sau:
- Trạng thái sử dụng: Có.
- Số serial là `849201739485`.
- Hệ số xung là 10 xung/m³.
- Kiểu đồng hồ xung là đồng hồ có một tín hiệu đếm xung.
- Chân đọc xung 1 của đồng hồ là chân 4 của thiết bị.
- Chân đọc xung 2 của đồng hồ không dùng.
- Sườn tín hiệu là cạnh xuống.
- Tổng lưu lượng xuôi là 4.15 m³.
- Tổng lưu lượng ngược là 0 m³.

Phía thiết bị phản hồi thành công vào lúc 11:30:55 31-10-2026, khi đó hai bên có cùng mã lệnh `CMD` là `SET` và `ID` là `CONFIG_PULSE_METER`, payload truyền và nhận giữa hai bên như sau:
```text
[Host -> Device]: 00 00 01 01 38 34 39 32 30 31 37 33 39 34 38 35 00 00 00 00 00 00 00 00 02 0A 00 03 00 04 04 05 00 06 02 07 9A 99 99 99 99 99 10 40 00 00 00 00 00 00 00 00
[Device -> Host]: 1A 0A 1F 0B 1E 37 00
```

#### 3.2.5. Cài đặt đồng hồ Modbus

Các tham số cấu hình đồng hồ Modbus được định nghĩa bởi kiểu liệt kê `Config_Modbus_Meter_Id_t`.

| Parameter ID (Hex) | Tên | Kiểu dữ liệu | Độ dài dữ liệu | Chú thích |
|---:|---|---|---:|---|
| `0x00` | `CONFIG_MODBUS_METER_ENABLE` | `uint8_t` | 1 byte | Bật/tắt chức năng đọc đồng hồ Modbus (`0x00`: Không, `0x01`: Có) |
| `0x01` | `CONFIG_MODBUS_METER_SERIAL_NUMBER` | `uint8_t[]` | 20 bytes (ASCII) | Số serial đồng hồ Modbus; chuỗi ngắn hơn 20 bytes được padding `0x00` ở cuối |
| `0x02` | `CONFIG_MODBUS_METER_SLAVE_ADDRESS` | `uint8_t` | 1 byte | Địa chỉ slave Modbus (`1`–`247`) |
| `0x03` | `CONFIG_MODBUS_METER_BAUDRATE` | `uint32_t` | 4 bytes (LE) | Tốc độ truyền dữ liệu |
| `0x04` | `CONFIG_MODBUS_METER_SERIAL_CONFIG` | `uint8_t` | 1 byte | Định dạng khung truyền (`0x00`: 8N1, `0x01`: 8O1, `0x02`: 8E1) |
| `0x05` | `CONFIG_MODBUS_METER_READ_FUNC_CODE` | `uint8_t` | 1 byte | Mã hàm Modbus dùng để đọc thanh ghi (`0x03` hoặc `0x04`) |
| `0x06` | `CONFIG_MODBUS_METER_FORWARD_TOTAL_ENABLE` | `uint8_t` | 1 byte | Bật/tắt đọc tổng lưu lượng xuôi (`0x00`: Không, `0x01`: Có) |
| `0x07` | `CONFIG_MODBUS_METER_FORWARD_TOTAL_REG_ADDR` | `uint16_t` | 2 bytes (LE) | Địa chỉ thanh ghi tổng lưu lượng xuôi |
| `0x08` | `CONFIG_MODBUS_METER_FORWARD_TOTAL_DATA_TYPE` | `uint8_t` | 1 byte | Kiểu dữ liệu tổng lưu lượng xuôi |
| `0x09` | `CONFIG_MODBUS_METER_FORWARD_TOTAL_WORD_SWAP` | `uint8_t` | 1 byte | Đảo thứ tự word 16-bit với dữ liệu từ 32-bit trở lên (`0x00`: Không, `0x01`: Có) |
| `0x0A` | `CONFIG_MODBUS_METER_FORWARD_TOTAL_EXPONENT` | `int8_t` | 1 byte | Số mũ thập phân áp dụng cho tổng lưu lượng xuôi |
| `0x0B` | `CONFIG_MODBUS_METER_REVERSE_TOTAL_ENABLE` | `uint8_t` | 1 byte | Bật/tắt đọc tổng lưu lượng ngược (`0x00`: Không, `0x01`: Có) |
| `0x0C` | `CONFIG_MODBUS_METER_REVERSE_TOTAL_REG_ADDR` | `uint16_t` | 2 bytes (LE) | Địa chỉ thanh ghi tổng lưu lượng ngược |
| `0x0D` | `CONFIG_MODBUS_METER_REVERSE_TOTAL_DATA_TYPE` | `uint8_t` | 1 byte | Kiểu dữ liệu tổng lưu lượng ngược |
| `0x0E` | `CONFIG_MODBUS_METER_REVERSE_TOTAL_WORD_SWAP` | `uint8_t` | 1 byte | Đảo thứ tự word 16-bit với dữ liệu từ 32-bit trở lên (`0x00`: Không, `0x01`: Có) |
| `0x0F` | `CONFIG_MODBUS_METER_REVERSE_TOTAL_EXPONENT` | `int8_t` | 1 byte | Số mũ thập phân áp dụng cho tổng lưu lượng ngược |
| `0x10` | `CONFIG_MODBUS_METER_FLOW_RATE_ENABLE` | `uint8_t` | 1 byte | Bật/tắt đọc lưu lượng tức thời (`0x00`: Không, `0x01`: Có) |
| `0x11` | `CONFIG_MODBUS_METER_FLOW_RATE_REG_ADDR` | `uint16_t` | 2 bytes (LE) | Địa chỉ thanh ghi lưu lượng tức thời |
| `0x12` | `CONFIG_MODBUS_METER_FLOW_RATE_DATA_TYPE` | `uint8_t` | 1 byte | Kiểu dữ liệu lưu lượng tức thời |
| `0x13` | `CONFIG_MODBUS_METER_FLOW_RATE_WORD_SWAP` | `uint8_t` | 1 byte | Đảo thứ tự word 16-bit với dữ liệu từ 32-bit trở lên (`0x00`: Không, `0x01`: Có) |
| `0x14` | `CONFIG_MODBUS_METER_FLOW_RATE_EXPONENT` | `int8_t` | 1 byte | Số mũ thập phân áp dụng cho lưu lượng tức thời |

Payload truyền và nhận giữa bên yêu cầu và thiết bị tương tự như [Cài đặt đồng hồ xung](#324-cài-đặt-đồng-hồ-xung).

#### 3.2.6. Cài đặt cảm biến áp suất

Các tham số cấu hình cảm biến áp suất được định nghĩa bởi kiểu liệt kê `Config_Pressure_Sensor_Id_t`.

| Parameter ID (Hex) | Tên | Kiểu dữ liệu | Độ dài dữ liệu | Chú thích |
|---:|---|---|---:|---|
| `0x00` | `CONFIG_PRESSURE_SENSOR_ENABLE` | `uint8_t` | 1 byte | Bật/tắt chức năng đọc cảm biến áp suất (`0x00`: Không, `0x01`: Có) |
| `0x01` | `CONFIG_PRESSURE_SENSOR_SERIAL_NUMBER` | `uint8_t[]` | 20 bytes (ASCII) | Số serial cảm biến áp suất; chuỗi ngắn hơn 20 bytes được padding `0x00` ở cuối |
| `0x02` | `CONFIG_PRESSURE_SENSOR_MIN_CURRENT` | `float` | 4 bytes (LE) | Dòng điện đầu ra nhỏ nhất của cảm biến áp suất (mA) |
| `0x03` | `CONFIG_PRESSURE_SENSOR_MAX_CURRENT` | `float` | 4 bytes (LE) | Dòng điện đầu ra lớn nhất của cảm biến áp suất (mA) |
| `0x04` | `CONFIG_PRESSURE_SENSOR_MIN_PRESSURE` | `float` | 4 bytes (LE) | Áp suất tương ứng với dòng điện đầu ra nhỏ nhất (bar) |
| `0x05` | `CONFIG_PRESSURE_SENSOR_MAX_PRESSURE` | `float` | 4 bytes (LE) | Áp suất tương ứng với dòng điện đầu ra lớn nhất (bar) |

Payload truyền và nhận giữa bên yêu cầu và thiết bị tương tự như [Cài đặt đồng hồ xung](#324-cài-đặt-đồng-hồ-xung).

#### 3.2.7. Đổi mật khẩu

Chức năng đổi mật khẩu có thể đổi mật khẩu cho những level thấp hơn level đang truy cập hoặc tự đổi cho chính level đang truy cập:
- Đăng nhập ở level 1 chỉ có thể thay đổi mật khẩu cho level 1.
- Đăng nhập ở level 2 có thể thay đổi mật khẩu cho level 1 và level 2, không thay đổi mật khẩu cho level 3 được.
- Đăng nhập ở level 3 có thể thay đổi mật khẩu cho cả ba mức level 1, level 2 và level 3.

Sau khi nhận được yêu cầu đổi mật khẩu, thiết bị sẽ phản hồi lại frame ACK với một số khả năng có thể xảy ra như sau:
- Nếu mật khẩu hiện tại nhập không đúng, thiết bị trả về mã lỗi `0x10` (`PROTOCOL_ERR_PASSWORD_INCORRECT`).
- Nếu level yêu cầu đổi cao hơn level hiện tại, thiết bị trả về mã lỗi `0x11` (`PROTOCOL_ERR_ACCESS_DENIED`).
- Nếu mật khẩu mới có độ dài nằm ngoài khoảng cho phép, thiết bị trả về mã lỗi `0x03` (`PROTOCOL_ERR_FRAME_INVALID`).
- Các mã lỗi khác trả về được định nghĩa ở [Bảng mã lỗi](#table_error_code).

| Offset | Trường | Kích thước | Định dạng | Mô tả |
|---:|---|---:|---|---|
| 0 | Độ dài mật khẩu hiện tại | 1 byte | `uint8_t` | Độ dài mật khẩu tại level đang truy cập |
| 1 | Mật khẩu hiện tại | 4–32 bytes | ASCII | Mật khẩu của level đang truy cập |
| 1 + n | Level cần đổi | 1 byte | `0x01`–`0x03` | Level cần đổi mật khẩu (chọn nhỏ hơn hoặc bằng level hiện tại) |
| 2 + n | Độ dài mật khẩu mới | 1 byte | `uint8_t ` | Độ dài mật khẩu mới của level cần đổi |
| 3 + n | Mật khẩu mới | 4–32 bytes | ASCII | Mật khẩu mới của level cần đổi |

**Ví dụ:** Level đang truy cập là level 2, mật khẩu tại level 2 là `1234`. Muốn đổi mật khẩu ở level 1 thành `123456789` thì phần payload data như sau:
```text
04 31 32 33 34 01 09 31 32 33 34 35 36 37 38 39
```

#### 3.2.8. Tạo sự kiện giả

Chức năng tạo sự kiện giả được dùng để phục vụ cho mục đích test.

| Offset | Trường | Kích thước | Định dạng | Mô tả |
|---:|---|---:|---|---|
| 0 | Loại thiết bị phát hiện sự kiện | 1 byte | `uint8_t` | `0x00`: module / `0x01`: đồng hồ xung / `0x02`: đồng hồ modbus / `0x03`: cảm biến áp suất |
| 1 | Số thứ tự thiết bị | 1 byte | `uint8_t` | Module: `0x00` / Đồng hồ xung: `0x00`–`0x03` / Đồng hồ modbus: `0x00`–`0x03` / Cảm biến áp suất: `0x00`–`0x01` |
| 2 | Sự kiện | 1 byte | `uint8_t` | Tra [Bảng danh sách sự kiện](#table_event_code)  |

**Danh sách sự kiện**<a id="table_event_code"></a>
| Hex | Event | Mô tả |
|---|---|---|
| `0x00` | `EVENT_MCU_RESET` | Phát hiện reset thiết bị |
| `0x01` | `EVENT_MAGNETIC_DETECTED` | Phát hiện từ trường |
| `0x02` | `EVENT_MAGNETIC_ENDED` | Kết thúc phát hiện từ trường |
| `0x03` | `EVENT_POWER_CONNECTED` | Phát hiện nguồn chính |
| `0x04` | `EVENT_POWER_LOST` | Mất nguồn chính |
| `0x05` | `EVENT_LOW_BATTERY` | Pin yếu |
| `0x06` | `EVENT_LOW_BATTERY_ENDED` | Kết thúc pin yếu |
| `0x07` | `EVENT_CONFIG_CHANGED` | Phát hiện thay đổi cài đặt |
| `0x08` | `EVENT_PASSWORD_CHANGED` | Phát hiện thay đổi mật khẩu |
| `0x09` | `EVENT_FIRMWARE_UPDATED` | Update firmware thành công |

**Ví dụ:** Tạo sự kiện *pin yếu* do *module* phát hiện sẽ có payload như sau:
```text
00 00 05
```

### 3.3. QUERY (`0x03`) — Yêu cầu thông tin từ thiết bị

Lệnh `QUERY` được sử dụng để yêu cầu thiết bị cung cấp thông tin cấu hình, trạng thái hoạt động, dữ liệu đo lường và các bản ghi lưu trữ.

| Query ID | Tên định danh | Yêu cầu | Payload (Yêu cầu) | Payload (Phản hồi) | Chú thích |
|---|---|---|---|---|---|
| `0x00` | `QUERY_BOOTLOADER_VERSION` | Lv1 | `NULL` | `uint8_t[]` — 10 bytes ASCII | Truy vấn phiên bản Bootloader |
| `0x01` | `QUERY_FIRMWARE_VERSION` | Lv1 | `NULL` | `uint8_t[]` — 10 bytes ASCII | Truy vấn phiên bản Firmware |
| `0x02` | `QUERY_POWER_INFO` | Lv1 | `NULL` | [Thông tin nguồn điện](#331-yêu-cầu-thông-tin-về-nguồn-điện) — 4 bytes | Truy vấn thông tin nguồn điện |
| `0x03` | `QUERY_OPERATING_TIME` | Lv1 | `NULL` | [Thông tin hoạt động](#332-yêu-cầu-thời-gian-hoạt-động-của-thiết-bị) — 8 bytes | Truy vấn thời gian hoạt động của thiết bị |
| `0x04` | `QUERY_SIM_NETWORK_INFO` | Lv1 | `NULL` | [Thông tin SIM và mạng](#333-yêu-cầu-thông-tin-sim-và-mạng) — 29 bytes | Truy vấn thông tin SIM và mạng di động |
| `0x05` | `QUERY_PULSE_METER_DATA` | Lv1 | `uint8_t` | [Dữ liệu đồng hồ xung / Modbus](#334-yêu-cầu-dữ-liệu-đồng-hồ-xung--đồng-hồ-modbus) — 45 bytes | Truy vấn đồng hồ xung |
| `0x06` | `QUERY_MODBUS_METER_DATA` | Lv1 | `uint8_t` | [Dữ liệu đồng hồ xung / Modbus](#334-yêu-cầu-dữ-liệu-đồng-hồ-xung--đồng-hồ-modbus) — 45 bytes | Truy vấn đồng hồ modbus |
| `0x07` | `QUERY_PRESSURE_SENSOR_DATA` | Lv1 | `uint8_t` | [Dữ liệu cảm biến áp suất](#335-yêu-cầu-dữ-liệu-cảm-biến-áp-suất) — 25 bytes | Truy vấn cảm biến áp suất |
| `0x08` | `QUERY_METADATA` | Lv3 | `NULL` | [Metadata và Runtime Data](#336-yêu-cầu-thông-tin-metadata) — 51 bytes | Truy vấn metadata và dữ liệu runtime |
| `0x09` | `QUERY_LATCH` | Lv1 | `uint16_t` — 2 bytes LE | [Latch Data](#337-yêu-cầu-dữ-liệu-latch) | Chỉ số bản ghi latch cần đọc |
| `0x0A` | `QUERY_EVENT` | Lv1 | `uint16_t` — 2 bytes LE | [Event Data](#338-yêu-cầu-dữ-liệu-event) | Chỉ số bản ghi event cần đọc |
| `0x0B` | `QUERY_PUSH_STATUS` | Lv1 | `uint16_t` — 2 bytes LE | [Push Status Data](#339-yêu-cầu-dữ-liệu-push-status) | Chỉ số bản ghi trạng thái gửi dữ liệu cần đọc |
| `0x0C` | `QUERY_CHANGED_INFO_LOG` | Lv3 | `uint16_t` — 2 bytes LE | [Changed Info Data](#3310-yêu-cầu-nhật-ký-thay-đổi-thông-tin) | Chỉ số bản ghi thay đổi thông tin cần đọc |
| `0x0D` | `QUERY_MCU_RESET_LOG` | Lv3 |  |  | Truy vấn nhật ký khởi động lại MCU |
| `0x0E` | `QUERY_LOG` | Lv3 |  |  | Truy vấn nhật ký hệ thống |

#### 3.3.1. Yêu cầu thông tin về nguồn điện

| Offset | Trường | Kích thước | Định dạng | Mô tả |
|---:|---|---:|---|---|
| 0 | Trạng thái nguồn ngoài | 1 byte | `uint8_t` | `0x00`: Mất nguồn / `0x01`: Có nguồn |
| 1 | Trạng thái sạc pin | 1 byte | `uint8_t` | `0x00`: Không sạc / `0x01`: Đang sạc |
| 2 | Điện áp pin (mV) | 2 bytes | `uint16_t` LE | Điện áp pin tính bằng mV |

**Ví dụ:** Nguồn ngoài đang có, pin đang sạc và điện áp pin là 3.924 V:

```text
01 01 54 0F
```

#### 3.3.2. Yêu cầu thời gian hoạt động của thiết bị

| Offset | Trường | Kích thước | Định dạng |
|---:|---|---:|---|
| 0 | Thời gian hoạt động kể từ lúc có nguồn (giây) | 4 bytes | `uint32_t` LE |
| 4 | Thời gian hoạt động kể từ lần MCU reset gần nhất (giây) | 4 bytes | `uint32_t` LE |

**Ví dụ:** Thời gian hoạt động kể từ lúc có nguồn là **791 giờ 25 phút 03 giây**, kể từ lần MCU reset gần nhất là **34 giờ 58 phút 10 giây**:

```text
4F 79 2B 00 C2 EB 01 00
```

#### 3.3.3. Yêu cầu thông tin SIM và mạng

| Offset | Trường | Kích thước | Định dạng |
|---:|---|---:|---|
| 0 | QCCID | 25 bytes | ASCII |
| 25 | RSSI | 1 byte | `int8_t` |
| 26 | RSRP | 1 byte | `int8_t` |
| 27 | RSRQ | 1 byte | `int8_t` |
| 28 | RSSNR | 1 byte | `int8_t` |

**Chú ý:**

- Nếu không đọc được QCCID, toàn bộ 25 bytes của trường này được đặt bằng `0x00`.
- Nếu không đọc được thông số mạng, cả bốn trường RSSI, RSRP, RSRQ và RSSNR đều được đặt bằng `0x00`. Nếu có ít nhất một trường khác `0`, dữ liệu thông số mạng được xem là hợp lệ theo quy ước này.

**Ví dụ:** QCCID là `8984012601508034517`, RSSI là `-75` dBm, RSRP là `-102` dBm, RSRQ là `-12` dB và RSSNR là `18` dB:

```text
38 39 38 34 30 31 32 36 30 31 35 30 38 30 33 34 35 31 37 00 00 00 00 00 00 B5 9A F4 12
```

#### 3.3.4. Yêu cầu dữ liệu đồng hồ xung / đồng hồ Modbus
Payload yêu cầu là chỉ số đồng hồ `MeterIndex`, giá trị hợp lệ trong khoảng `0x00` — `0x04`. Payload phản hồi được đóng gói theo thứ tự: chỉ số đồng hồ, số serial, tổng lưu lượng xuôi, tổng lưu lượng ngược, lưu lượng tức thời.
| Offset | Trường | Kích thước | Định dạng | Ý nghĩa |
|---:|---|---:|---|---|
| 0 | `u8MeterIndex` | 1 byte | `uint8_t` | Chỉ số đồng hồ |
| 1 | `au8MeterSerial` | 20 bytes | ASCII | Số serial đồng hồ |
| 21 | `dTotalForward` | 8 bytes | `double` | Tổng lưu lượng xuôi (m³) |
| 29 | `dTotalReverse` | 8 bytes | `double` | Tổng lưu lượng ngược (m³) |
| 37 | `dFlowRate` | 8 bytes | `double` | Lưu lượng tức thời (m³/s) |

**Chú ý:**

- Khi giá trị đọc bị lỗi hoặc không xác định, thiết bị trả về giá trị `double` bằng `-1` (`0xBFF0000000000000`).
- Khi truy vấn đồng hồ Modbus, thiết bị gửi frame Modbus RTU qua cổng RS485. Nếu PC kết nối với thiết bị qua RS485, PC Tool cần lọc các frame Modbus RTU trung gian để xác định đúng frame phản hồi của giao thức thiết bị.

**Ví dụ:** Truy vấn đồng hồ thứ ba, có số serial `8492017394856201`, tổng lưu lượng xuôi `31.84` m³, tổng lưu lượng ngược `0.42` m³ và lưu lượng tức thời `4.15` m³/s:

```text
[Host -> Device]: 02
[Device -> Host]: 02 38 34 39 32 30 31 37 33 39 34 38 35 36 32 30 31 00 00 00 00 D7 A3 70 3D 0A D7 3F 40 E1 7A 14 AE 47 E1 DA 3F 9A 99 99 99 99 99 10 40
```

#### 3.3.5. Yêu cầu dữ liệu cảm biến áp suất
Payload yêu cầu là chỉ số đồng hồ `MeterIndex`, giá trị hợp lệ trong khoảng `0x00` — `0x01`. Payload phản hồi gồm được đóng gói theo thứ tự: Chỉ số cảm biến, số serial cảm biến, giá trị áp suất.
| Offset | Trường | Kích thước | Định dạng | Ý nghĩa |
|---:|---|---:|---|---|
| 0 | `u8MeterIndex` | 1 byte | `uint8_t` | Chỉ số cảm biến |
| 1 | `au8MeterSerial` | 20 bytes | ASCII | Số serial cảm biến |
| 21 | `fPressure` | 4 bytes | `float` | Áp suất (bar) |

**Chú ý:**

- Khi giá trị đọc bị lỗi hoặc không xác định, thiết bị trả về giá trị `float` bằng `-1` (`0xBF800000`).
- Nếu cần hiển thị đơn vị áp suất khác, PC Tool hoặc Server tự chuyển đổi từ bar sang đơn vị mong muốn.

**Ví dụ:** Truy vấn cảm biến thứ nhất, có số serial `3049582710493827` và áp suất `3.91` bar:

```text
[Host -> Device]: 01
[Device -> Host]: 01 33 30 34 39 35 38 32 37 31 30 34 39 33 38 32 37 00 00 00 00 71 3D 7A 40
```

#### 3.3.6. Yêu cầu thông tin Metadata

Lệnh `QUERY_METADATA` không yêu cầu Payload Data. Thiết bị trả về dữ liệu metadata và runtime được nối liên tiếp theo thứ tự `Eeprom_Metadata_t`, sau đó là `Eeprom_Runtime_Data_t`.

**Cấu trúc Metadata**

| Offset | Trường | Kích thước | Định dạng | Mô tả |
|---:|---|---:|---|---|
| 0 | `u32Sequence` | 4 bytes | `uint32_t` LE | Số thứ tự metadata |
| 4 | `u16NextLatchSaveIndex` | 2 bytes | `uint16_t` LE | Chỉ số lưu latch tiếp theo |
| 6 | `u16NextLatchLoadIndex` | 2 bytes | `uint16_t` LE | Chỉ số đọc latch tiếp theo |
| 8 | `u16LatchCount` | 2 bytes | `uint16_t` LE | Số bản ghi latch |
| 10 | `u16NextEventSaveIndex` | 2 bytes | `uint16_t` LE | Chỉ số lưu event tiếp theo |
| 12 | `u16NextEventLoadIndex` | 2 bytes | `uint16_t` LE | Chỉ số đọc event tiếp theo |
| 14 | `u16EventCount` | 2 bytes | `uint16_t` LE | Số bản ghi event |
| 16 | `u16NextPushStatusSaveIndex` | 2 bytes | `uint16_t` LE | Chỉ số lưu push status tiếp theo |
| 18 | `u16PushStatusCount` | 2 bytes | `uint16_t` LE | Số bản ghi push status |
| 20 | `u16NextChangedInfoLogSaveIndex` | 2 bytes | `uint16_t` LE | Chỉ số lưu changed info log tiếp theo |
| 22 | `u16ChangedInfoLogCount` | 2 bytes | `uint16_t` LE | Số bản ghi changed info log |

**Cấu trúc Runtime Data**

| Offset | Trường | Kích thước | Định dạng | Mô tả |
|---:|---|---:|---|---|
| 0 | `u32Sequence` | 4 bytes | `uint32_t` LE | Số thứ tự runtime data |
| 4 | `u8ResetCount` | 1 byte | `uint8_t` | Số lần MCU reset |
| 5 | `sPulseCount[0].u64ForwardPulseCount` | 8 bytes | `uint64_t` LE | Tổng số xung xuôi của đồng hồ xung thứ nhất |
| 13 | `sPulseCount[0].u64ReversePulseCount` | 8 bytes | `uint64_t` LE | Tổng số xung ngược của đồng hồ xung thứ nhất |
| 21 | `sPulseCount[1].u64ForwardPulseCount` | 8 bytes | `uint64_t` LE | Tổng số xung xuôi của đồng hồ xung thứ hai |
| 29 | `sPulseCount[1].u64ReversePulseCount` | 8 bytes | `uint64_t` LE | Tổng số xung ngược của đồng hồ xung thứ hai |
| 21 | `sPulseCount[2].u64ForwardPulseCount` | 8 bytes | `uint64_t` LE | Tổng số xung xuôi của đồng hồ xung thứ ba |
| 29 | `sPulseCount[2].u64ReversePulseCount` | 8 bytes | `uint64_t` LE | Tổng số xung ngược của đồng hồ xung thứ ba |
| 21 | `sPulseCount[3].u64ForwardPulseCount` | 8 bytes | `uint64_t` LE | Tổng số xung xuôi của đồng hồ xung thứ tư |
| 29 | `sPulseCount[3].u64ReversePulseCount` | 8 bytes | `uint64_t` LE | Tổng số xung ngược của đồng hồ xung thứ tư |

#### 3.3.7. Yêu cầu dữ liệu Latch

Payload yêu cầu gồm chỉ số bản ghi latch cần đọc, kiểu `uint16_t`, 2 bytes little-endian. Payload phản hồi chứa dữ liệu của một bản ghi `Latch_Data_t`, được đóng gói theo thứ tự: thời gian, dữ liệu đồng hồ xung, dữ liệu đồng hồ Modbus và dữ liệu cảm biến áp suất.

**Cấu trúc Payload phản hồi**

| Thứ tự | Thiết bị | Trường dữ liệu | Kích thước | Định dạng |
|---:|---|---|---:|---|
| 1 | Chung | Thời gian hiện tại | 6 bytes | `Date_Time_t` |
| 2 | Chung | Thời gian latch | 6 bytes | `Date_Time_t` |
| 3 | Đồng hồ xung thứ 1–4 | `Meter Type` | 1 byte | `uint8_t` |
| 4 | Đồng hồ xung thứ 1–4 | `Meter Serial Length` | 1 byte | `uint8_t` |
| 5 | Đồng hồ xung thứ 1–4 | `Meter Serial` | Theo độ dài serial | ASCII |
| 6 | Đồng hồ xung thứ 1–4 | `TOTAL_FORWARD` | 8 bytes | `double` |
| 7 | Đồng hồ xung thứ 1–4 | `TOTAL_REVERSE` | 8 bytes | `double` |
| 8 | Đồng hồ xung thứ 1–4 | `FLOW_RATE` | 8 bytes | `double` |
| 9 | Đồng hồ Modbus thứ 1–4 | `Meter Type` | 1 byte | `uint8_t` |
| 10 | Đồng hồ Modbus thứ 1–4 | `Meter Serial Length` | 1 byte | `uint8_t` |
| 11 | Đồng hồ Modbus thứ 1–4 | `Meter Serial` | Theo độ dài serial | ASCII |
| 12 | Đồng hồ Modbus thứ 1–4 | `TOTAL_FORWARD` | 8 bytes | `double` |
| 13 | Đồng hồ Modbus thứ 1–4 | `TOTAL_REVERSE` | 8 bytes | `double` |
| 14 | Đồng hồ Modbus thứ 1–4 | `FLOW_RATE` | 8 bytes | `double` |
| 15 | Cảm biến áp suất thứ 1–2 | `Meter Type` | 1 byte | `uint8_t` |
| 16 | Cảm biến áp suất thứ 1–2 | `Meter Serial Length` | 1 byte | `uint8_t` |
| 17 | Cảm biến áp suất thứ 1–2 | `Meter Serial` | Theo độ dài serial | ASCII |
| 18 | Cảm biến áp suất thứ 1–2 | `PRESSURE` | 4 bytes | `float` |

**Ghi chú:**

- `Date_Time_t` gồm 6 bytes theo thứ tự `Year`, `Month`, `Date`, `Hours`, `Minutes`, `Seconds`.
- Thứ tự đóng gói: đồng hồ xung thứ 1–4, đồng hồ Modbus thứ 1–4, cảm biến áp suất thứ 1–2. Bản ghi có serial rỗng sẽ bị bỏ qua.
- Số bản tin chốt tối đa là `720`.

**Chú ý:** Nếu không đọc được bản ghi latch, thiết bị trả về lỗi `PROTOCOL_ERR_UNKNOW`.

#### 3.3.8. Yêu cầu dữ liệu Event

Payload yêu cầu gồm chỉ số bản ghi event cần đọc, kiểu `uint16_t`, 2 bytes little-endian. Payload phản hồi chứa một bản ghi `Event_Data_t`.

**Cấu trúc Payload phản hồi**

| Thứ tự | Trường dữ liệu | Kích thước | Định dạng |
|---:|---|---:|---|
| 1 | Thời gian hiện tại | 6 bytes | `Date_Time_t` |
| 2 | Thời gian Event | 6 bytes | `Date_Time_t` |
| 3 | `Meter Type` | 1 byte | `uint8_t` |
| 4 | `Meter Serial Length` | 1 byte | `uint8_t` |
| 5 | `Meter Serial` | Theo độ dài serial | ASCII |
| 6 | `Event Code` | 1 byte | `uint8_t` |

**Ghi chú:**
- `Date_Time_t` gồm 6 bytes theo thứ tự `Year`, `Month`, `Date`, `Hours`, `Minutes`, `Seconds`.
- Số bản ghi sự kiện tối đa là `1000`.

Nếu không đọc được bản ghi event, thiết bị trả về lỗi `PROTOCOL_ERR_UNKNOW`.

#### 3.3.9. Yêu cầu dữ liệu Push Status

Payload yêu cầu gồm chỉ số bản ghi push status cần đọc, kiểu `uint16_t`, 2 bytes little-endian. Payload phản hồi chứa một bản ghi `Push_Status_Data_t`.

**Cấu trúc Payload phản hồi**

| Thứ tự | Trường dữ liệu | Kích thước | Định dạng |
|---:|---|---:|---|
| 1 | Thời gian hiện tại | 6 bytes | `Date_Time_t` |
| 2 | Thời gian Push Status | 6 bytes | `Date_Time_t` |
| 3 | `Session Duration` | 2 bytes | `uint16_t` |
| 4 | `RSSI` | 1 byte | `int8_t` |
| 5 | `RSRP` | 1 byte | `int8_t` |
| 6 | `RSRQ` | 1 byte | `int8_t` |
| 7 | `RSSNR` | 1 byte | `int8_t` |
| 8 | `Cellular Error` | 1 byte | `uint8_t` |
| 9 | `Push Error` | 1 byte | `uint8_t` |

**Ghi chú:**
- `Date_Time_t` gồm 6 bytes theo thứ tự `Year`, `Month`, `Date`, `Hours`, `Minutes`, `Seconds`.
- Số bản ghi trạng thái đẩy tối đa là `1000`.

Nếu không đọc được bản ghi push status, thiết bị trả về lỗi `PROTOCOL_ERR_UNKNOW`.

**Bảng mã lỗi module 4G**
| Hex | Mã lỗi | Mô tả |
|---|---|---|
| `0x00` | `CELLULAR_ERROR_NONE` | Không có lỗi |
| `0x01` | `CELLULAR_ERROR_POWER_ON` | Lỗi bật nguồn module cellular |
| `0x02` | `CELLULAR_ERROR_AT_READY` | Lỗi chờ module phản hồi AT |
| `0x03` | `CELLULAR_ERROR_SIM_NOT_READY` | SIM chưa sẵn sàng |
| `0x04` | `CELLULAR_ERROR_NETWORK_NOT_REGISTERED` | Module chưa đăng ký mạng |
| `0x05` | `CELLULAR_ERROR_SOCKET_SERVICE_NOT_READY` | Dịch vụ socket chưa sẵn sàng |
| `0x06` | `CELLULAR_ERROR_DATE_TIME_SYNC` | Lỗi đồng bộ thời gian |
| `0x07` | `CELLULAR_ERROR_GET_DATE_TIME` | Lỗi lấy thời gian |
| `0x08` | `CELLULAR_ERROR_TCP_CONNECT` | Lỗi kết nối TCP |

**Bảng mã lỗi giao tiếp server**
| Hex | Mã lỗi | Mô tả |
|---|---|---|
| `0x00` | `PUSH_ERROR_NONE` | Không có lỗi |
| `0x01` | `PUSH_ERROR_DISCONNECT` | Lỗi ngắt kết nối |
| `0x02` | `PUSH_ERROR_PACK` | Lỗi đóng gói dữ liệu |
| `0x03` | `PUSH_ERROR_INFO` | Lỗi gửi dữ liệu thông tin |
| `0x04` | `PUSH_ERROR_LATCH` | Lỗi gửi dữ liệu Latch |
| `0x05` | `PUSH_ERROR_EVENT` | Lỗi gửi dữ liệu Event |
| `0x06` | `PUSH_ERROR_REQUEST_PROCESS` | Lỗi xử lý yêu cầu từ máy chủ |

#### 3.3.10. Yêu cầu nhật ký thay đổi thông tin

Payload yêu cầu gồm chỉ số bản ghi changed info log cần đọc, kiểu `uint16_t`, 2 bytes little-endian.

| Offset | Trường | Kích thước | Định dạng | Mô tả |
|---:|---|---:|---|---|
| 0 | `Changed Info Index` | 2 bytes | `uint16_t` LE | Chỉ số bản ghi nhật ký thay đổi thông tin cần đọc |

Payload phản hồi chứa một bản ghi `Changed_Info_Data_t`.

**Cấu trúc Payload phản hồi**

| Thứ tự | Trường dữ liệu | Kích thước | Định dạng |
|---:|---|---:|---|
| 1 | Thời gian hiện tại | 6 bytes | `Date_Time_t` |
| 2 | Thời gian thay đổi thông tin | 6 bytes | `Date_Time_t` |
| 3 | `Source` | 1 byte | `uint8_t` |
| 4 | `Level` | 1 byte | `uint8_t` |
| 5 | `Config ID` | 1 byte | `uint8_t` |
| 6 | `New Value` | 8 bytes | `uint64_t` |

`Source` là nguồn thay đổi, với `0x00` (`PROTOCOL_DATA_SOURCE_CELLULAR`) và `0x01` (`PROTOCOL_DATA_SOURCE_RS485`).

Nếu không đọc được bản ghi changed info log, thiết bị trả về lỗi `PROTOCOL_ERR_UNKNOW`.

**Bảng ID trường thay đổi**

| Hex | Tên định danh | Mô tả |
|---|---|---|
| `0x00` | `CHANGED_MODULE_SERIAL` | Thay đổi số serial thiết bị |
| `0x01` | `CHANGED_ACTIVATION_STATUS` | Thay đổi trạng thái kích hoạt |
| `0x02` | `CHANGED_TIME` | Thay đổi thời gian thiết bị |
| `0x03` | `CHANGED_IP_ENDPOINT` | Thay đổi địa chỉ IP/Endpoint |
| `0x04` | `CHANGED_LATCH_PERIOD` | Thay đổi chu kỳ lưu Latch |
| `0x05` | `CHANGED_PUSH_PERIOD` | Thay đổi chu kỳ Push dữ liệu |
| `0x06` | `CHANGED_TIMEZONE` | Thay đổi múi giờ |
| `0x07` | `CHANGED_CHARGE` | Thay đổi cấu hình sạc |
| `0x08` | `CHANGED_PULSE_METER_1` | Thay đổi cấu hình đồng hồ xung thứ 1 |
| `0x09` | `CHANGED_PULSE_METER_2` | Thay đổi cấu hình đồng hồ xung thứ 2 |
| `0x0A` | `CHANGED_PULSE_METER_3` | Thay đổi cấu hình đồng hồ xung thứ 3 |
| `0x0B` | `CHANGED_PULSE_METER_4` | Thay đổi cấu hình đồng hồ xung thứ 4 |
| `0x0C` | `CHANGED_MODBUS_METER_1` | Thay đổi cấu hình đồng hồ Modbus thứ 1 |
| `0x0D` | `CHANGED_MODBUS_METER_2` | Thay đổi cấu hình đồng hồ Modbus thứ 2 |
| `0x0E` | `CHANGED_MODBUS_METER_3` | Thay đổi cấu hình đồng hồ Modbus thứ 3 |
| `0x0F` | `CHANGED_MODBUS_METER_4` | Thay đổi cấu hình đồng hồ Modbus thứ 4 |
| `0x10` | `CHANGED_PRESSURE_SENSOR_1` | Thay đổi cấu hình cảm biến áp suất thứ 1 |
| `0x11` | `CHANGED_PRESSURE_SENSOR_2` | Thay đổi cấu hình cảm biến áp suất thứ 2 |
| `0x12` | `CHANGED_RESET_SETTING` | Thay đổi cài đặt khôi phục |
| `0x13` | `CHANGED_PASSWORD` | Thay đổi mật khẩu |
| `0x14` | `CHANGED_RESET_PASSWORD` | Đặt lại mật khẩu |
| `0xFF` | `CHANGED_UNKNOW` | Không xác định |

**Giải mã tham số `New Value`**

| Config ID | Ý nghĩa `New Value` | Cách giải mã |
|---|---|---|
| `CHANGED_MODULE_SERIAL` | Số serial mới của thiết bị | `uint64_t`, giá trị serial |
| `CHANGED_ACTIVATION_STATUS` | Trạng thái kích hoạt mới | `uint8_t`, giá trị của `Device_Activation_Status_t` |
| `CHANGED_TIME` | Thời gian mới của thiết bị | 6 bytes đầu của `uint64_t` chứa `Date_Time_t`, gồm `Year`, `Month`, `Date`, `Hours`, `Minutes`, `Seconds` |
| `CHANGED_IP_ENDPOINT` | Địa chỉ IPv4 và cổng TCP mới | Byte 0–3 lần lượt chứa 4 octet IPv4; byte 4–5 chứa cổng TCP (`uint16_t`); byte 6–7 bằng 0 |
| `CHANGED_LATCH_PERIOD` | Chu kỳ lưu Latch mới | `uint16_t`, đơn vị theo cấu hình chu kỳ Latch |
| `CHANGED_PUSH_PERIOD` | Chu kỳ Push dữ liệu mới | `uint16_t`, đơn vị theo cấu hình chu kỳ Push |
| `CHANGED_TIMEZONE` | Múi giờ mới | Giá trị `float` được sao chép nguyên dạng bit vào `uint64_t`; cần đọc lại 4 bytes đầu dưới dạng `float` |

**Lưu ý:**
- `CHANGED_TIME`: chỉ 6 bytes đầu của `New Value` có ý nghĩa; 2 bytes còn lại không được khởi tạo bởi đoạn code này.
- `CHANGED_IP_ENDPOINT`: cách đóng gói dùng các bit 0–31 cho IPv4 và bit 32–47 cho cổng TCP.
- `CHANGED_TIMEZONE`: đây là cách lưu biểu diễn nhị phân IEEE 754 của `float`, không phải chuyển giá trị múi giờ sang số nguyên.
- Các `Config ID` khác không cần quan tâm đến `New Value`.


### 3.4. PUSH (`0x04`) — Thiết bị chủ động đẩy dữ liệu lên server

| Push ID | Tên | Ý nghĩa |
|---|---|---|
| `0x00` | `PUSH_INFO` | Thông tin thiết bị, phiên bản firmware, SIM, tín hiệu mạng và trạng thái nguồn |
| `0x01` | `PUSH_LATCH` | Dữ liệu Latch |
| `0x02` | `PUSH_EVENT` | Danh sách sự kiện |
| `0x03` | `PUSH_MODULE_CONFIG` | Cấu hình module |
| `0x04` | `PUSH_PULSE_METER_CONFIG` | Cấu hình công tơ xung |
| `0x05` | `PUSH_MODBUS_METER_CONFIG` | Cấu hình công tơ Modbus |
| `0x06` | `PUSH_PRESSURE_SENSOR_CONFIG` | Cấu hình cảm biến áp suất |

**Bảng các trường trong Payload PUSH_INFO**
| Thứ tự | Trường dữ liệu | Kích thước | Định dạng / Ý nghĩa |
|---:|---|---:|---|
| 1 | Current Time | 6 bytes | `YY MM DD HH mm SS` |
| 2 | Firmware Version | 10 bytes | Phiên bản firmware |
| 3 | SIM CCID | 25 bytes | Mã định danh SIM; nếu không lấy được thì toàn bộ trường được điền `0x00` |
| 4 | Signal Info | 4 bytes | Thông tin chất lượng tín hiệu theo thứ tự gồm: RSSI RSRP, RSRQ, RSSNR; nếu không lấy được thì 4 bytes được điền `0x00` |
| 5 | VBUS Status | 1 byte | `0x00`: không có VBUS; `0x01`: có VBUS |
| 6 | Charging Status | 1 byte | `0x00`: không sạc; `0x01`: đang sạc |
| 7 | Battery Voltage | 2 bytes | Điện áp pin, đơn vị mV, kiểu `uint16_t` LE |
| 8 | Reset Count | 1 byte | Số lần reset, kiểu `uint8_t` |

**Chú ý:**
- Payload của `PUSH_LATCH` giống như [Payload Latch](#337-yêu-cầu-dữ-liệu-latch).
- Payload của `PUSH_EVENT` giống như [Payload Event](#338-yêu-cầu-dữ-liệu-event).

### 3.5. OTA (`0x05`) — Cập nhật firmware

| Hex | Tên | Ý nghĩa |
|---|---|---|
| `0x00` | `OTA_UPDATE_REQUEST` | Yêu cầu cập nhật firmware, xác định phiên bản, kích thước, CRC32 và slot firmware đích |
| `0x01` | `OTA_SEND_PACKET` | Gửi packet firmware để ghi vào Flash và kiểm tra tính toàn vẹn của firmware |

#### 3.5.1. OTA_UPDATE_REQUEST (`0x00`)

**Request Payload**

| Thứ tự | Trường dữ liệu | Kích thước | Định dạng / Ý nghĩa |
|---:|---|---:|---|
| 1 | Firmware Version | `VERSION_SIZE` bytes | Phiên bản firmware cần cập nhật |
| 2 | Slot A Size | 4 bytes | `uint32_t`, kích thước firmware Slot A |
| 3 | Slot A CRC32 | 4 bytes | `uint32_t`, giá trị CRC32 của firmware Slot A |
| 4 | Slot B Size | 4 bytes | `uint32_t`, kích thước firmware Slot B |
| 5 | Slot B CRC32 | 4 bytes | `uint32_t`, giá trị CRC32 của firmware Slot B |

**Tổng kích thước payload:** `VERSION_SIZE + 16` bytes.

**Response Payload**

| Thứ tự | Trường dữ liệu | Kích thước | Định dạng / Ý nghĩa |
|---:|---|---:|---|
| 1 | Requested Slot | 1 byte | Slot firmware được yêu cầu tải |
| 2 | Packet Index | 2 bytes | `uint16_t`, chỉ số packet bắt đầu tải hoặc tiếp tục tải |

**Giá trị Requested Slot**

| Giá trị | Tên | Ý nghĩa |
|---|---|---|
| Theo định nghĩa enum | `OTA_REQUEST_SLOT_A` | Yêu cầu tải firmware vào Slot A |
| Theo định nghĩa enum | `OTA_REQUEST_SLOT_B` | Yêu cầu tải firmware vào Slot B |
| Theo định nghĩa enum | `OTA_REQUEST_SLOT_NONE` | Không yêu cầu tải firmware do kích thước firmware vượt quá `FIRMWARE_MAX_SIZE` |

**Nguyên tắc xử lý**
- Firmware mới được tải vào slot không chứa firmware đang chạy.
- Nếu firmware cần tải khác với metadata OTA hiện tại, hệ thống khởi tạo trạng thái tải mới và xóa vùng Flash tương ứng.
- Nếu tiếp tục tải firmware trước đó, `Packet Index` được căn về đầu trang Flash tương ứng để tải lại dữ liệu từ ranh giới trang.
- Nếu kích thước firmware vượt quá `FIRMWARE_MAX_SIZE`, hệ thống trả về `OTA_REQUEST_SLOT_NONE`.

### 2. OTA_SEND_PACKET (`0x01`)

**Request Payload**

| Thứ tự | Trường dữ liệu | Kích thước | Định dạng / Ý nghĩa |
|---:|---|---:|---|
| 1 | Packet Index | 2 bytes | `uint16_t`, chỉ số packet firmware |
| 2 | Packet Size | 2 bytes | `uint16_t`, số byte dữ liệu firmware trong packet |
| 3 | Firmware Data | Thay đổi | Dữ liệu firmware cần ghi vào Flash |

**Tổng kích thước payload:** `4 + Packet Size` bytes.

**Địa chỉ ghi firmware**

```c
u32OtaStartAddress + (PacketIndex * PROTOCOL_MAX_OTA_PACKET_SIZE)