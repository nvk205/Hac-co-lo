# Dự Án Mật Mã Dòng Dragon (Dragon Stream Cipher)

Dự án cài đặt, kiểm thử và minh họa hoạt động của thuật toán mã hóa dòng **Dragon** bằng ngôn ngữ C. Mã nguồn hỗ trợ cả hai chế độ khóa $128$-bit và $256$-bit, tích hợp bộ tự kiểm tra (self-test), kiểm thử tự động qua Test Vectors chuẩn của eSTREAM và chương trình mã hóa/giải mã văn bản trực tiếp.

---

## 1. Giới thiệu mã Dragon

**Dragon** là một thuật toán mã hóa dòng hoạt động trên mức từ (word-based stream cipher), được thiết kế bởi các nhà nghiên cứu tại Viện An toàn Thông tin thuộc Đại học Công nghệ Queensland (QUT), Úc (bao gồm Ed Dawson, K. Chen, M. Henricksen, W. Millan, L. Simpson, H. Wu, v.v.).

* **Bối cảnh ra đời:** Thuật toán được đệ trình vào năm 2005 cho dự án **eSTREAM** (thuộc mạng lưới ECRYPT Châu Âu) và đã xuất sắc vượt qua nhiều vòng đánh giá khắt khe để lọt vào **Giai đoạn 3 (Phase 3)** thuộc **Profile 1** (nhóm các thuật toán tối ưu hóa tốc độ cao trên phần mềm).
* **Đặc điểm kỹ thuật:**
  * Thay vì xử lý từng bit rời rạc như các hệ mật mã dòng LFSR truyền thống, Dragon xử lý dữ liệu theo các từ máy tính $32$-bit và $64$-bit, giúp tận dụng tối đa năng lực của các bộ vi xử lý hiện đại.
  * Hỗ trợ hai độ dài khóa bí mật ($K$) và Vector khởi tạo ($IV$): **$128$-bit** và **$256$-bit**.
  * Mỗi chu kỳ sinh khóa tạo ra một từ khóa (keystream word) có kích thước **$64$-bit**.

---

## 2. Cơ sở lý thuyết

Cấu trúc toán học của thuật toán Dragon dựa trên một Thanh ghi dịch phản hồi phi tuyến (**NLFSR** - *Non-Linear Feedback Shift Register*) kết hợp với một bộ đếm trạng thái và hàm cập nhật phi tuyến $F$.

### 2.1. Trạng thái nội bộ (Internal State) và Bộ đếm (Counter)
* **Thanh ghi trạng thái:** Có tổng kích thước $1024$ bit.
  * Trong giai đoạn khởi tạo, trạng thái được biểu diễn thành $8$ phần tử $128$-bit ký hiệu là $W_0, W_1, \dots, W_7$ (mỗi phần tử gồm hai nửa $64$-bit là `lb` - *left block* và `rb` - *right block*).
  * Trong giai đoạn sinh dòng khóa, trạng thái được chia nhỏ thành mảng $32$ từ $32$-bit ký hiệu là $B_0, B_1, \dots, B_{31}$.
* **Bộ nhớ / Bộ đếm $M$:** Là một thanh ghi $64$-bit (chia thành hai nửa $32$-bit là $M_0$ và $M_1$), đóng vai trò ngăn chặn các cuộc tấn công trượt (slide attacks) và đảm bảo chu kỳ dòng khóa tối thiểu là $2^{64}$. Giá trị khởi tạo mặc định của $M$ là hằng số:
  $$M = \text{0x0000447261676F6E}$$
  *(Tương ứng với chuỗi ký tự ASCII `"Dragon"`).*

### 2.2. Hộp thay thế (S-boxes) và các hàm $G, H$
Tính phi tuyến của thuật toán được cung cấp bởi hai bảng tra cứu tĩnh `sbox1` ($S_1$) và `sbox2` ($S_2$). Mỗi bảng gồm $256$ phần tử $32$-bit ($8 \times 32$-bit S-box):
- **Tính chất cân bằng:** Trên toàn bộ $256$ phần tử của mỗi S-box, tại bất kỳ vị trí bit đầu ra nào (từ bit $0$ đến bit $31$), số lượng bit $1$ và bit $0$ luôn xuất hiện chính xác $128$ lần.
- **Các hàm thành phần $G_1, G_2, G_3$ và $H_1, H_2, H_3$:** Nhận đầu vào là một từ $32$-bit $x$ được tách thành $4$ byte riêng biệt $x = x_0 \parallel x_1 \parallel x_2 \parallel x_3$ (với $x_0$ là byte cao nhất):

  $\begin{aligned} G_1(x) &= S_1[x_0] \oplus S_1[x_1] \oplus S_1[x_2] \oplus S_2[x_3] \\ G_2(x) &= S_1[x_0] \oplus S_1[x_1] \oplus S_2[x_2] \oplus S_1[x_3] \\ G_3(x) &= S_1[x_0] \oplus S_2[x_1] \oplus S_1[x_2] \oplus S_1[x_3] \end{aligned}$

  $\begin{aligned} H_1(x) &= S_2[x_0] \oplus S_2[x_1] \oplus S_2[x_2] \oplus S_1[x_3] \\ H_2(x) &= S_2[x_0] \oplus S_2[x_1] \oplus S_1[x_2] \oplus S_2[x_3] \\ H_3(x) &= S_2[x_0] \oplus S_1[x_1] \oplus S_2[x_2] \oplus S_2[x_3] \end{aligned}$

### 2.3. Hàm cập nhật phi tuyến $F$ (`F_func`)
Hàm $F$ nhận vào một mảng $6$ phần tử $32$-bit: $A = (A_0, A_1, A_2, A_3, A_4, A_5)$ và biến đổi trực tiếp qua $6$ lớp phép toán liên tiếp (kết hợp giữa phép cộng modulo $2^{32}$ và phép XOR $\oplus$):
1. **Lớp tiền xáo trộn XOR:** $A_1 \leftarrow A_1 \oplus A_0$; $A_3 \leftarrow A_3 \oplus A_2$; $A_5 \leftarrow A_5 \oplus A_4$.
2. **Lớp tiền xáo trộn Cộng:** $A_2 \leftarrow (A_2 + A_1) \pmod{2^{32}}$; $A_4 \leftarrow (A_4 + A_3) \pmod{2^{32}}$; $A_0 \leftarrow (A_0 + A_5) \pmod{2^{32}}$.
3. **Lớp S-box $G$:** $A_3 \leftarrow A_3 \oplus G_1(A_0)$; $A_5 \leftarrow A_5 \oplus G_2(A_2)$; $A_1 \leftarrow A_1 \oplus G_3(A_4)$.
4. **Lớp S-box $H$:** $A_0 \leftarrow A_0 \oplus H_1(A_1)$; $A_2 \leftarrow A_2 \oplus H_2(A_3)$; $A_4 \leftarrow A_4 \oplus H_3(A_5)$.
5. **Lớp hậu xáo trộn Cộng:** $A_3 \leftarrow (A_3 + A_0) \pmod{2^{32}}$; $A_5 \leftarrow (A_5 + A_2) \pmod{2^{32}}$; $A_1 \leftarrow (A_1 + A_4) \pmod{2^{32}}$.
6. **Lớp hậu xáo trộn XOR:** $A_2 \leftarrow A_2 \oplus A_1$; $A_4 \leftarrow A_4 \oplus A_3$; $A_0 \leftarrow A_0 \oplus A_5$.

### 2.4. Hai giai đoạn hoạt động chính
1. **Khởi tạo khóa và IV (`init_key`):**
   * Khóa $K$ và $IV$ được nạp vào $8$ phần tử $W_0 \dots W_7$ thông qua các tổ hợp ghép nối, XOR và đảo bit (`~`).
   * Thuật toán thực hiện **$16$ vòng lặp xáo trộn** không sinh khóa. Ở mỗi vòng, giá trị $W_0 \oplus W_6 \oplus W_7$ cùng bộ đếm $M$ được đưa vào mảng $A$, chạy qua hàm $F$, sau đó kết hợp với $W_4$ để tạo ra phần tử mới đẩy vào $W_0$ (các phần tử cũ dịch phải $1$ vị trí), đồng thời cập nhật lại $M_0, M_1$.
2. **Sinh dòng khóa (`gen_key`) và Mã hóa (`dragon_crypt`):**
   * Tại mỗi bước sinh khóa, thuật toán trích xuất $6$ từ $32$-bit từ trạng thái $B$ và bộ đếm $M$:
     $$A_0 = B_0, \quad A_1 = B_9, \quad A_2 = B_{16}, \quad A_3 = B_{19}, \quad A_4 = B_{30} \oplus M_0, \quad A_5 = B_{31} \oplus M_1$$
   * Đưa mảng $A$ qua hàm `F_func(A)`.
   * Dịch chuyển mảng trạng thái $B$ đi $2$ vị trí ($B_i \leftarrow B_{i-2}$), cập nhật $B_0 = A_1, B_1 = A_2$ và tăng bộ đếm $M \leftarrow (M + 1) \pmod{2^{64}}$.
   * Xuất từ khóa $64$-bit:
     $$z = (A_0 \ll 32) \oplus A_4$$
   * **Mã hóa / Giải mã:** Mỗi từ khóa $z$ ($8$ byte) được tách từ byte cao nhất xuống byte thấp nhất để XOR trực tiếp với từng byte của bản rõ ($P_i$) tạo ra bản mã ($C_i$):
     $$C_i = P_i \oplus K_i \quad \text{và} \quad P_i = C_i \oplus K_i$$

---

## 3. Cấu trúc Repository

```text
├── dragon.h         # (dragon_3.h) File tiêu đề khai báo cấu trúc dữ liệu, hằng số và nguyên mẫu hàm
├── dragon.c         # (dragon_3.c) Cài đặt lõi thuật toán Dragon, các S-box, hàm mã hóa và tự kiểm tra
├── main.c           # (main_2.c)   Chương trình chính: in 16 từ khóa đầu tiên và demo mã hóa/giải mã
├── test_dragon.c    #              Module đọc file vectors.txt và kiểm thử tự động các Test Vectors
├── vectors.txt      #              Dữ liệu Test Vectors chuẩn cho chế độ 128-bit và 256-bit
└── README.md        #              Tài liệu hướng dẫn và báo cáo tổng quan dự án
```

> **Lưu ý khi biên dịch:** Đảm bảo các tệp mã nguồn được đổi tên về đúng chuẩn `dragon.h`, `dragon.c` và `main.c` trong cùng một thư mục trước khi biên dịch.

### Hướng dẫn Biên dịch và Chạy chương trình
Sử dụng trình biên dịch `gcc` trên Terminal / Command Prompt:

```bash
# Biên dịch chương trình chính
gcc -O2 main.c dragon.c -o dragon_app

# Chạy chương trình
./dragon_app
```

---

## 4. Hàm và các chức năng trong hàm

Hệ thống hàm trong dự án được phân tách rõ ràng theo từng nhiệm vụ từ toán học cấp thấp đến giao tiếp ứng dụng:

| Tên hàm | Tệp nguồn | Đầu vào (Inputs) | Đầu ra (Outputs) | Chức năng chi tiết |
| :--- | :--- | :--- | :--- | :--- |
| `G_box` | `dragon.c` | `u_int x`, `int n` ($1..3$) | `u_int` ($32$-bit) | Tách từ $32$-bit `x` thành $4$ byte, tra bảng `sbox1`, `sbox2` theo công thức $G_n$ và trả về kết quả XOR. |
| `H_box` | `dragon.c` | `u_int x`, `int n` ($1..3$) | `u_int` ($32$-bit) | Tách từ $32$-bit `x` thành $4$ byte, tra bảng `sbox1`, `sbox2` theo công thức $H_n$ và trả về kết quả XOR. |
| `F_func` | `dragon.c` | `u_int *A` (mảng $6$ từ) | Trả về `0`; biến đổi trực tiếp mảng `A` | Thực hiện hàm cập nhật phi tuyến $F$ gồm $6$ vòng lặp cộng modulo $2^{32}$, XOR và gọi `G_box`, `H_box`. |
| `init_key` | `dragon.c` | `u64 *key`, `u64 *iv`, `element *W`, `int n` | Trả về `0`; cập nhật `W` và `M` | Nạp khóa và IV vào mảng trạng thái $W$ ($n=1$ cho $128$-bit, $n \neq 1$ cho $256$-bit) và chạy $16$ vòng lặp xáo trộn khởi tạo. |
| `gen_key` | `dragon.c` | `u_int *B` ($32$ từ), `u64 *z` | Trả về `0`; ghi từ khóa vào `*z`, cập nhật `B`, `M` | Thực hiện $1$ chu kỳ sinh khóa: rút trích trạng thái, gọi `F_func`, dịch thanh ghi $B$, cộng bộ đếm $M$ thêm $1$ và xuất $64$-bit khóa vào `z`. |
| `dragon_state_init` | `dragon.c` | `u_int *B`, `u64 *key`, `u64 *iv`, `int kind` | `void`; khởi tạo hoàn chỉnh mảng `B` | Đặt lại bộ đếm $M$ về giá trị gốc (`0x0000447261676F6E`), gọi `init_key` và chuyển đổi $8$ phần tử $128$-bit từ $W$ sang $32$ từ $32$-bit của $B$. |
| `dragon_crypt` | `dragon.c` | `u_int *B`, `const uint8_t *in`, `uint8_t *out`, `size_t len` | `void`; ghi dữ liệu đã mã hóa/giải mã vào `out` | Liên tục gọi `gen_key` để sinh từ khóa $64$-bit, cắt thành từng byte từ cao xuống thấp để XOR với mảng `in` tạo ra mảng `out`. |
| `check` | `dragon.c` | `int ok`, `const char *name`, `int verbose` | Trả về `0` (Đạt) hoặc `1` (Lỗi) | Hàm phụ trợ in trạng thái kiểm thử `[PASS]` hoặc `[FAIL]` kèm tên bài kiểm tra ra màn hình. |
| `dragon_selftest` | `dragon.c` | `int verbose` | Số lượng lỗi phát hiện (`int`) | Tự động kiểm tra tính toàn vẹn của `sbox1`, `sbox2` (giá trị đầu/cuối, tính cân bằng $128$ bit $1$), kiểm tra `G_box`, `H_box` và `F_func` với vector mẫu. |
| `print_hex` | `main.c` | `const uint8_t *d`, `size_t n` | `void` (In ra màn hình) | In mảng byte dưới dạng chuỗi thập lục phân (Hex) viết hoa, cứ mỗi $8$ byte cách nhau bởi một khoảng trắng. |
| `encrypt_demo` | `main.c` | `void` (Đọc từ `stdin`) | Trả về `0` nếu khớp, mã lỗi nếu thất bại | Cho phép người dùng nhập văn bản Plaintext từ bàn phím, tiến hành mã hóa sang bản mã Hex, giải mã ngược lại và đối chiếu với bản gốc. |
| `main` | `main.c` | `void` | Trả về `0` | Điểm bắt đầu chương trình: sinh và in $16$ từ khóa $64$-bit đầu tiên của bộ khóa mẫu $128$-bit, sau đó gọi `encrypt_demo()`. |
| `run_dragon_test` | `test_dragon.c` | `void` (Đọc file `vectors.txt`) | Tổng số lượt sai khác (`int`) | Đọc từng bộ `KEY`, `IV` và `KS` kỳ vọng từ file `vectors.txt`, chạy thuật toán và lập bảng đối chiếu từng từ khóa thực tế với lý thuyết. |

---

## 5. Test Vector

Dự án sử dụng hai tầng kiểm thử để đảm bảo thuật toán được cài đặt chính xác tuyệt đối tới từng bit:

### 5.1. Tự kiểm tra nội bộ (`dragon_selftest`)
Kiểm tra trực tiếp các thành phần toán học cơ sở:
* **Kiểm tra biên S-box:** `sbox1[0] == 0x393BCE6B`, `sbox1[255] == 0x0192B555`, `sbox2[0] == 0xA94BC384`, `sbox2[255] == 0x51561E77`.
* **Kiểm tra cân bằng bit:** Đảm bảo cả $32$ cột bit của `sbox1` và `sbox2` đều có tổng số bit $1$ bằng đúng $128$.
* **Kiểm tra $G, H$ và $F$:** Đối chiếu kết quả đầu ra tại các điểm `x = 0x00000000`, `0x01020304`, `0xFFFFFFFF` và $3$ mảng đầu vào chuẩn của hàm `F_func`.

### 5.2. Kiểm thử dòng khóa chuẩn (`vectors.txt` & `test_dragon.c`)
Tệp `vectors.txt` chứa $4$ bộ Test Vectors kiểm tra $16$ từ khóa $64$-bit đầu tiên ($1024$ bit keystream) cho cả hai chế độ `DRAGON_128` và `DRAGON_256`:

#### Bộ 1: 128-bit Set 1
* **KEY (128-bit):** `00001111222233334444555566667777`
* **IV (128-bit):** `00001111222233334444555566667777`
* **Keystream kỳ vọng (4 từ 64-bit đầu tiên trên tổng 16 từ):**
  * Từ 01: `99b3aa14b63bd02f`
  * Từ 02: `e14358a454950425`
  * Từ 03: `f4b0d3fd8ba69178`
  * Từ 04: `e0392938a718c165`

#### Bộ 2: 128-bit Set 2
* **KEY (128-bit):** `00112233445566778899AABBCCDDEEFF`
* **IV (128-bit):** `00112233445566778899AABBCCDDEEFF`
* **Keystream kỳ vọng (4 từ 64-bit đầu tiên trên tổng 16 từ):**
  * Từ 01: `988215060e87e695`
  * Từ 02: `eb7aef36313ff910`
  * Từ 03: `e6c7312f30357424`
  * Từ 04: `4922043d98146ee2`

#### Bộ 3: 256-bit Set 1
* **KEY (256-bit):** `0000111122223333444455556666777788889999AAAABBBBCCCCDDDDEEEEFFFF`
* **IV (256-bit):** `0000111122223333444455556666777788889999AAAABBBBCCCCDDDDEEEEFFFF`
* **Keystream kỳ vọng (4 từ 64-bit đầu tiên trên tổng 16 từ):**
  * Từ 01: `bc020767dc48dae3`
  * Từ 02: `14778d8c927e8b32`
  * Từ 03: `e086c6cde593c008`
  * Từ 04: `600c9d47a488f622`

#### Bộ 4: 256-bit Set 2
* **KEY (256-bit):** `00112233445566778899AABBCCDDEEFF00112233445566778899AABBCCDDEEFF`
* **IV (256-bit):** `00112233445566778899AABBCCDDEEFF00112233445566778899AABBCCDDEEFF`
* **Keystream kỳ vọng (4 từ 64-bit đầu tiên trên tổng 16 từ):**
  * Từ 01: `8d3ab9ba01daa3eb`
  * Từ 02: `5cbd0f6de3ecfcab`
  * Từ 03: `619af808cf9c4a42`
  * Từ 04: `e28777666d2d7037`

---

## 6. Phân công công việc

Bảng phân chia nhiệm vụ các thành viên thực hiện dự án:

| STT | Họ và tên | MSSV | Công việc phụ trách |
| :---: | :--- | :---: | :--- |
| 1 | Nguyễn Văn Khánh | `20233465` | Nghiên cứu lý thuyết mật mã Dragon, thiết kế cấu trúc `dragon.h` và cài đặt các bảng S-box. |
| 2 | Phạm Văn Huấn | `20233421` |  Cài đặt các hàm `G_box`, `H_box`, `F_func`. |
| 3 | Nguyễn Tuấn Phong   | `20233575` | Bổ sung hàm init_key trong file dragon.c. |
| 4 | Hạp Tiến Duy Anh   | `20233245` | Thêm dragon_state_init và dragon_crypt (xử lý nhập bản rõ Plain text và giải mã). |
| 5 | Hoàng Đức An   | `20233234` | Thêm hàm dragon_selftest để kiểm thử độc lập các hàm xử lý. |
| 6 | Đoàn Ngọc Đức   | `20233322` | Hoàn thiện file test_dragon.c (thêm hàm test DRAGON hoàn chỉnh, refactor logic xử lý key và IV).Tạo mới và cập nhật file vectors.txt cho các bộ test 128-bit và 256-bit |
| 7 | Hà Xuân Chinh   | `20233288` | Viết chương trình chính (hàm main) dùng để khởi tạo và chạy thử nghiệm toàn bộ thuật toán. |
| 8 | Nguyễn Chí Tâm   | `20233630` | Xây dựng chương trình chính `main.c`, kịch bản tương tác `encrypt_demo`, biên soạn tài liệu `README.md` và chuẩn bị slide thuyết trình. |
| 9 | Hoàng Anh Dũng   | `20233344` | Viết và cập nhật hàm gen_key (sinh chuỗi khóa stream). |
| 10 | Nguyễn Đức Duy   | `20233366` | Cài đặt giai đoạn khởi tạo khóa `init_key`, sinh dòng khóa `gen_key` và các hàm mã hóa/giải mã `dragon_state_init`, `dragon_crypt`. |
