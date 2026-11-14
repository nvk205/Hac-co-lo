# Hệ Mật Mã Dòng Dragon (eSTREAM) — Cài Đặt, Mô Phỏng Trực Quan & Phân Tích Mật Mã

Dự án Bài tập lớn môn **Lý thuyết Mật mã** thực hiện bởi nhóm 10 sinh viên. Dự án tập trung vào việc cài đặt từ gốc bằng ngôn ngữ C (không phụ thuộc thư viện ngoài), mô phỏng trực quan hóa trạng thái bên trong (White-box Step-by-Step Visualizer) và đánh giá thống kê mật mã cho hệ mật dòng **Dragon** (hỗ trợ cả hai chế độ khóa 128-bit và 256-bit).

---

## 1. Tổng quan Kiến trúc Toán học của Hệ mật Dragon

Dragon là hệ mật dòng hướng từ (word-oriented stream cipher) thuộc dự án **eSTREAM**, vận hành trên các từ dữ liệu 32-bit với hai thành phần bộ nhớ trạng thái cốt lõi:
* **Thanh ghi dịch phản hồi phi tuyến (NLFSR) kích thước 1024-bit:** Được chia thành 32 từ trạng thái 32-bit ký hiệu là $B[0], B[1], \dots, B[31]$.
* **Bộ nhớ trạng thái kiêm bộ đếm 64-bit ($M$):** Được tách thành hai nửa 32-bit $(M_H, M_L)$. Khi khởi tạo, $M$ nhận hằng số `0x0000447261676F6E` (mã ASCII của chuỗi `"Dragon"`). Trong giai đoạn sinh khóa dòng, $M$ tăng thêm 1 sau mỗi nhịp để đảm bảo chu kỳ tối thiểu đạt ít nhất $2^{64}$.

### Luồng dữ liệu qua các Module trong dự án

```text
                  [ Khóa Key & Vector khởi tạo IV (128-bit / 256-bit) ]
                                            │
                                            ▼
┌───────────────────────────────────────────────────────────────────────────────────────┐
│ PHA 1: KHỞI TẠO TRẠNG THÁI (src/dragon/dragon_init.c)                                 │
│  • Ghép Key, IV và phần bù (~Key, ~IV) lấp đầy 32 từ trạng thái B[0..31].             │
│  • Gán bộ nhớ M = 0x0000447261676F6E ("Dragon").                                      │
│  • Thực hiện 16 vòng lặp nhào trộn có phản hồi thông qua hàm cập nhật F.              │
└───────────────────────────────────────────┬───────────────────────────────────────────┘
                                            │
                                            ▼
┌───────────────────────────────────────────────────────────────────────────────────────┐
│ PHA 2: SINH DÒNG KHÓA - KEYSTREAM GENERATION (src/dragon/dragon.c)                    │
│  • Tăng bộ đếm: M = M + 1 (tách thành M_H và M_L).                                    │
│  • Trích xuất 6 từ từ NLFSR: a=B[0], b=B[9], c=B[16], d=B[19], e=B[30]^M_H, f=B[31]^M_L│
│  • Đưa (a, b, c, d, e, f) qua HÀM CẬP NHẬT F (src/dragon/dragon_f.c):                 │
│       ├── Tầng 1 (Pre-mixing):  Trộn tuyến tính bằng phép XOR (^) và cộng mod 2^32 (+)│
│       ├── Tầng 2 (S-box Layer): Đi qua các hàm phi tuyến G1, G2, H                    │
│       │                         tra cứu trên 2 hộp S-box 8x32-bit S1, S2              │
│       │                         (src/dragon/dragon_sbox.c)                            │
│       └── Tầng 3 (Post-mixing): Khuếch tán hoàn toàn bằng phép cộng mod 2^32 và XOR   │
│  • Dịch thanh ghi B[0..31] đi 2 từ, phản hồi (a', d') vào đầu thanh ghi.              │
│  • Xuất 64-bit (8 byte) khóa dòng mỗi nhịp: z_t = (a' || e').                         │
└───────────────────────────────────────────┬───────────────────────────────────────────┘
                                            │
                                            ▼
┌───────────────────────────────────────────────────────────────────────────────────────┐
│ PHA 3: MÃ HÓA / GIẢI MÃ DỮ LIỆU & ĐÁNH GIÁ (dragon_cipher.c & analysis/crypto_eval.c) │
│  • Mã hóa / Giải mã đối xứng: Ciphertext = Plaintext XOR Keystream.                   │
│  • Kiểm định Hiệu ứng thác đổ (Avalanche Effect) và phân bố thống kê chuỗi khóa.      │
└───────────────────────────────────────────────────────────────────────────────────────┘
```

---

## 2. Cấu trúc Cây thư mục Dự án

```text
Hac-co-lo/
├── .gitignore                              # Cấu hình loại trừ file rác biên dịch (.exe, .o, __pycache__)
├── Makefile                                # Kịch bản biên dịch tự động đa nền tảng (Windows / Linux)
├── README.md                               # Tài liệu kiến trúc hệ thống và hướng dẫn sử dụng
│
├── docs/
│   └── so_do_ham_f.drawio                  # Sơ đồ khối chi tiết cấu trúc 3 tầng của hàm cập nhật F
│
├── src/
│   └── dragon/
│       ├── dragon_sbox.h / dragon_sbox.c   # Bảng hằng số S-box (sbox1, sbox2) và hàm biến đổi G_box, H_box
│       ├── dragon_f.h / dragon_f.c         # Hàm cập nhật trạng thái F và cấu trúc lưu vết trung gian
│       ├── dragon_init.h / dragon_init.c   # Khởi tạo Key/IV (128-bit & 256-bit) và 16 vòng trộn trạng thái
│       ├── dragon.h / dragon.c             # Cấu trúc Dragon_Ctx, dịch thanh ghi NLFSR & sinh Keystream
│       ├── dragon_cipher.h / dragon_cipher.c # Mã hóa và giải mã bộ đệm văn bản / tệp tin bất kỳ
│       ├── main.c                          # Giao diện dòng lệnh (CLI) và xuất dữ liệu cho GUI
│
├── tests/
│   ├── test_dragon.c                       # Kiểm thử tích hợp toàn hệ thống theo bộ Vector chuẩn
│   └── vectors.txt                         # Bộ Test Vectors chuẩn của dự án eSTREAM cho hệ mật Dragon
│
├── analysis/
│   └── crypto_eval.c                       # Đánh giá khoảng cách Hamming, Hiệu ứng thác đổ & tính ngẫu nhiên
│
└── gui/
    ├── app_main.py                         # Cửa sổ giao diện chính tích hợp các tab chức năng
    ├── tab_visualizer.py                   # Tab 1: Mô phỏng trực quan từng bước thanh ghi B[32], M và hàm F
    └── tab_cipher_charts.py                # Tab 2: Mã hóa trực quan và biểu đồ phân tích mật mã
```

---

## 3. Bảng Phân công Nhiệm vụ & Tiến độ (10 Thành viên)

| Thành viên | Tài khoản / Họ tên | Module phụ trách | Nhiệm vụ chi tiết | Tiến độ |
| :---: | :--- | :--- | :--- | :---: |
| **TV1** | `HuanPham1208` | `dragon_sbox.h/.c`<br>`test_G-H_func.c` | Cài đặt 2 bảng hằng số `sbox1[256]`, `sbox2[256]`, hàm phi tuyến `G_box`, `H_box` và viết bài test đơn vị cho hàm G, H. | ✅ Đã hoàn thành |
| **TV2** | Thành viên 2 | `dragon_f.h/.c`<br>`so_do_ham_f.drawio` | Cài đặt 3 tầng hàm cập nhật trạng thái `F` (Pre-mixing, S-box layer, Post-mixing), lưu vết trung gian và vẽ sơ đồ khối hàm F. | ✅ Đã hoàn thành |
| **TV3** | `nguyentuanphong257` | `dragon_init.h/.c` | Cài đặt thủ tục nạp khóa và IV cho cả hai chế độ 128-bit và 256-bit vào `Dragon_Ctx`, thực hiện 16 vòng khởi tạo trạng thái. | ✅ Đã hoàn thành |
| **TV4** | `haxuanchinh05` | `dragon.h/.c` | Định nghĩa cấu trúc `Dragon_Ctx` (`B[32]`, `M_L`, `M_H`), cài đặt cơ chế dịch thanh ghi NLFSR 1024-bit và trích xuất 64-bit khóa dòng. | ✅ Đã hoàn thành |
| **TV5** | Thành viên 5 | `dragon_cipher.h/.c`<br>`src/dragon/main.c` | Cài đặt hàm XOR mã hóa/giải mã dữ liệu độ dài bất kỳ, mã hóa file và xây dựng chương trình dòng lệnh `main.c`. | 🔄 Đang hoàn thiện `main.c` |
| **TV6** | Thành viên 6 | `tests/test_dragon.c`<br>`tests/vectors.txt` | Kiểm thử tích hợp toàn bộ chu trình khởi tạo và sinh khóa đối chiếu với bộ Test Vectors chuẩn của eSTREAM. | ⏳ Đang ghép nối |
| **TV7** | `HoangAn1603` | `analysis/crypto_eval.c` | Cài đặt hàm tính khoảng cách Hamming (`dem_bit_1`, `dem_bit_khac_nhau`), thực nghiệm Hiệu ứng thác đổ (Avalanche) và kiểm định Monobit. | 🔄 Đang viết hàm thực nghiệm |
| **TV8** | Thành viên 8 | `gui/tab_visualizer.py` | Xây dựng giao diện mô phỏng từng bước (Step-by-step) hiển thị 32 ô thanh ghi `B[0..31]`, bộ đếm `M` và luồng biến đổi trong hàm `F`. | ⏳ Đang thực hiện |
| **TV9** | Thành viên 9 | `gui/tab_cipher_charts.py`<br>`gui/app_main.py` | Xây dựng giao diện mã hóa văn bản/file trực tiếp (hiển thị phép XOR nhị phân/Hex) và vẽ biểu đồ thống kê Avalanche. | ⏳ Đang thực hiện |
| **TV10** | `nvk205`<br>(Nguyễn Văn Khánh) | `Makefile`, `.gitignore`,<br>`README.md` | Thiết kế cấu trúc phân rã module, chuẩn hóa giao diện `.h`, viết kịch bản build đa nền tảng, kiểm duyệt Pull Request và quản lý nhánh Git. | ✅ Đang điều phối |

---

## 4. Hướng dẫn Biên dịch và Chạy chương trình

### Yêu cầu môi trường
* Trình biên dịch C: `gcc` (MinGW-w64 trên Windows hoặc GCC trên Ubuntu/Linux).
* Công cụ build: `make` (hoặc `mingw32-make` trên Windows).
* Python 3.x (dành cho phần chạy giao diện trực quan trong thư mục `gui/`).

### Cách 1: Sử dụng `Makefile` (Khuyên dùng)

Mở Terminal (Git Bash / PowerShell / Ubuntu Terminal) tại thư mục gốc của dự án:

```bash
# 1. Biên dịch toàn bộ các chương trình thực thi
make all

# 2. Chạy kiểm thử riêng hàm phi tuyến G, H và bảng S-box (của TV1)
make test-gh

# 3. Chạy bộ kiểm thử chuẩn eSTREAM Test Vectors (của TV6)
make test

# 4. Chạy chương trình phân tích mật mã & đánh giá hiệu ứng thác đổ (của TV7)
make analysis

# 5. Khởi chạy giao diện mô phỏng trực quan (của TV8 & TV9)
make gui

# 6. Xóa các file thực thi (.exe) và file tạm sau khi chạy xong
make clean
```

### Cách 2: Biên dịch thủ công bằng lệnh `gcc` (Nếu máy không cài `make`)

```bash
# Biên dịch và chạy kiểm thử hàm G, H:
gcc -Wall -Wextra -O2 -std=c99 -Isrc/dragon src/dragon/dragon_sbox.c src/dragon/test_function/test_G-H_func.c -o test_gh_func
./test_gh_func

# Biên dịch chương trình chính (CLI):
gcc -Wall -Wextra -O2 -std=c99 -Isrc/dragon src/dragon/dragon_sbox.c src/dragon/dragon_f.c src/dragon/dragon_init.c src/dragon/dragon.c src/dragon/dragon_cipher.c src/dragon/main.c -o dragon_cli

# Biên dịch bộ kiểm thử eSTREAM Vectors:
gcc -Wall -Wextra -O2 -std=c99 -Isrc/dragon src/dragon/dragon_sbox.c src/dragon/dragon_f.c src/dragon/dragon_init.c src/dragon/dragon.c src/dragon/dragon_cipher.c tests/test_dragon.c -o test_dragon

# Biên dịch chương trình phân tích mật mã:
gcc -Wall -Wextra -O2 -std=c99 -Isrc/dragon src/dragon/dragon_sbox.c src/dragon/dragon_f.c src/dragon/dragon_init.c src/dragon/dragon.c src/dragon/dragon_cipher.c analysis/crypto_eval.c -o crypto_eval -lm
```

---

## 5. Quy tắc Làm việc Nhóm trên GitHub

1. **Cập nhật nhánh `main` trước khi code:**
   ```bash
   git pull origin main
   ```
2. **Chỉ chỉnh sửa trên nhánh (branch) và tệp được phân công:** Không tự ý thay đổi nguyên mẫu hàm trong các tệp `.h` dùng chung nếu chưa thống nhất với nhóm trưởng (TV10) để tránh lỗi liên kết (`undefined reference`).
3. **Không đẩy file thực thi lên kho lưu trữ:** Tuyệt đối không commit các file `.exe`, `.o`, `.out` lên GitHub. Chạy `make clean` trước khi thực hiện `git add` và `git commit`.