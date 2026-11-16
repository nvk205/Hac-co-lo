import ctypes
import os
import re
import sys
import tkinter as tk
from tkinter import messagebox

ROOT_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
if ROOT_DIR not in sys.path:
    sys.path.insert(0, ROOT_DIR)

MASK32 = 0xFFFFFFFF
MASK64 = 0xFFFFFFFFFFFFFFFF

# 1. ĐỌC BẢNG S-BOX 

def _load_sboxes_from_c():
    """Đọc trực tiếp mảng S-box từ file src/dragon/dragon_sbox.c để đồng bộ 100% với code C."""
    sbox_c_path = os.path.join(ROOT_DIR, "src", "dragon", "dragon_sbox.c")
    if os.path.exists(sbox_c_path):
        try:
            with open(sbox_c_path, "r", encoding="utf-8", errors="ignore") as f:
                content = f.read()
            # Tìm các khối mảng { ... } có chứa 256 phần tử hex/số nguyên
            blocks = re.findall(r"\{([^{}]+)\}", content)
            tables = []
            for blk in blocks:
                nums = [int(x, 0) for x in re.findall(r"0x[0-9A-Fa-f]+|\b\d+\b", blk)]
                if len(nums) == 256:
                    tables.append(nums)
            if len(tables) >= 2:
                return tables[0], tables[1]
        except Exception:
            pass
    # Nếu không tìm thấy file hoặc lỗi, trả về S-box mặc định
    s1 = [((i * 167 + 13) & 0xFF) for i in range(256)]
    s2 = [((i * 251 + 89) & 0xFF) for i in range(256)]
    return s1, s2

SBOX1, SBOX2 = _load_sboxes_from_c()


# 2. HÀM F

EXTERNAL_PY_TRACE = None
EXTERNAL_C_LIB = None

# Kiểm tra nếu nhóm có biên dịch file C ra thư viện động (.dll / .so)
for lib_name in ["dragon.dll", "libdragon.so", "libdragon.dylib", "src/dragon/dragon.dll", "src/dragon/libdragon.so"]:
    lib_path = os.path.join(ROOT_DIR, lib_name)
    if os.path.exists(lib_path):
        try:
            lib = ctypes.CDLL(lib_path)
            if hasattr(lib, "dragon_F_trace"):
                EXTERNAL_C_LIB = lib
                break
        except Exception:
            continue

# Kiểm tra wrapper Python cho dragon_F_trace
for mod_name in ["src.dragon", "src.dragon.dragon_f", "dragon_f"]:
    try:
        mod = __import__(mod_name, fromlist=["dragon_F_trace"])
        if hasattr(mod, "dragon_F_trace"):
            EXTERNAL_PY_TRACE = getattr(mod, "dragon_F_trace")
            break
    except Exception:
        continue



# 3. CÁC HÀM G, H VÀ HÀM F (3 TẦNG)

def _g1(x):
    b0, b1, b2, b3 = (x >> 24) & 0xFF, (x >> 16) & 0xFF, (x >> 8) & 0xFF, x & 0xFF
    return ((SBOX1[b0] << 24) | (SBOX1[b1] << 16) | (SBOX2[b2] << 8) | SBOX2[b3]) & MASK32

def _g2(x):
    b0, b1, b2, b3 = (x >> 24) & 0xFF, (x >> 16) & 0xFF, (x >> 8) & 0xFF, x & 0xFF
    return ((SBOX1[b0] << 24) | (SBOX2[b1] << 16) | (SBOX1[b2] << 8) | SBOX2[b3]) & MASK32

def _g3(x):
    b0, b1, b2, b3 = (x >> 24) & 0xFF, (x >> 16) & 0xFF, (x >> 8) & 0xFF, x & 0xFF
    return ((SBOX2[b0] << 24) | (SBOX1[b1] << 16) | (SBOX2[b2] << 8) | SBOX1[b3]) & MASK32

def _h1(x):
    b0, b1, b2, b3 = (x >> 24) & 0xFF, (x >> 16) & 0xFF, (x >> 8) & 0xFF, x & 0xFF
    return ((SBOX2[b0] << 24) | (SBOX2[b1] << 16) | (SBOX1[b2] << 8) | SBOX1[b3]) & MASK32

def _h2(x):
    b0, b1, b2, b3 = (x >> 24) & 0xFF, (x >> 16) & 0xFF, (x >> 8) & 0xFF, x & 0xFF
    return ((SBOX2[b0] << 24) | (SBOX1[b1] << 16) | (SBOX1[b2] << 8) | SBOX2[b3]) & MASK32

def _h3(x):
    b0, b1, b2, b3 = (x >> 24) & 0xFF, (x >> 16) & 0xFF, (x >> 8) & 0xFF, x & 0xFF
    return ((SBOX1[b0] << 24) | (SBOX2[b1] << 16) | (SBOX2[b2] << 8) | SBOX1[b3]) & MASK32

def dragon_F_trace_internal(a, b, c, d, e, f):
    """Tính giá trị 6 biến (a, b, c, d, e, f) qua 3 tầng biến đổi của hàm F."""
    a, b, c, d, e, f = a & MASK32, b & MASK32, c & MASK32, d & MASK32, e & MASK32, f & MASK32
    stage_in = (a, b, c, d, e, f)

    # Tầng 1: Pre-mixing (XOR và Cộng modulo 2^32)
    b1 = (b ^ a) & MASK32
    d1 = (d ^ c) & MASK32
    f1 = (f ^ e) & MASK32
    c1 = (c + b1) & MASK32
    e1 = (e + d1) & MASK32
    a1 = (a + f1) & MASK32
    layer1 = (a1, b1, c1, d1, e1, f1)

    # Tầng 2: S-box G1, G2, G3 và H1, H2, H3
    b2 = (b1 ^ _g1(c1)) & MASK32
    d2 = (d1 ^ _g2(e1)) & MASK32
    f2 = (f1 ^ _g3(a1)) & MASK32
    c2 = (c1 ^ _h1(d2)) & MASK32
    e2 = (e1 ^ _h2(f2)) & MASK32
    a2 = (a1 ^ _h3(b2)) & MASK32
    layer2 = (a2, b2, c2, d2, e2, f2)

    # Tầng 3: Post-mixing (Đầu ra a', b', c', d', e', f')
    d3 = (d2 + a2) & MASK32
    f3 = (f2 + c2) & MASK32
    b3 = (b2 + e2) & MASK32
    c3 = (c2 ^ b2) & MASK32
    e3 = (e2 ^ d2) & MASK32
    a3 = (a2 ^ f2) & MASK32
    layer3 = (a3, b3, c3, d3, e3, f3)

    return {
        "input": stage_in,
        "layer1": layer1,
        "layer2": layer2,
        "layer3": layer3,
        "output": layer3,
    }

def call_dragon_F_trace(a, b, c, d, e, f):
    """Ưu tiên gọi hàm dragon_F_trace của TV2 nếu có, nếu chưa có thì dùng hàm nội bộ."""
    if EXTERNAL_PY_TRACE is not None:
        res = EXTERNAL_PY_TRACE(a, b, c, d, e, f)
        if isinstance(res, dict):
            inp = res.get("input", (a, b, c, d, e, f))
            l1 = res.get("layer1", res.get("stage1", inp))
            l2 = res.get("layer2", res.get("stage2", l1))
            l3 = res.get("layer3", res.get("stage3", res.get("output", l2)))
            return {"input": inp, "layer1": l1, "layer2": l2, "layer3": l3, "output": l3}
        elif isinstance(res, (list, tuple)) and len(res) >= 3:
            return {"input": (a, b, c, d, e, f), "layer1": res[0], "layer2": res[1], "layer3": res[2], "output": res[-1]}

    if EXTERNAL_C_LIB is not None:
        try:
            out_buf = (ctypes.c_uint32 * 18)()
            EXTERNAL_C_LIB.dragon_F_trace(
                ctypes.c_uint32(a), ctypes.c_uint32(b), ctypes.c_uint32(c),
                ctypes.c_uint32(d), ctypes.c_uint32(e), ctypes.c_uint32(f),
                out_buf
            )
            vals = list(out_buf)
            return {
                "input": (a, b, c, d, e, f),
                "layer1": tuple(vals[0:6]),
                "layer2": tuple(vals[6:12]),
                "layer3": tuple(vals[12:18]),
                "output": tuple(vals[12:18]),
            }
        except Exception:
            pass

    return dragon_F_trace_internal(a, b, c, d, e, f)



# 4. BỘ MÔ PHỎNG TRẠNG THÁI

class DragonStateSimulator:
    # 6 ô thanh ghi được lấy dữ liệu đưa vào hàm F ở mỗi nhịp
    TAP_INDICES = (0, 9, 16, 19, 30, 31)

    def __init__(self):
        self.B = [0] * 32
        self.M = 0
        self.init_round = 0
        self.tick_count = 0
        self.phase = "CHƯA KHỞI TẠO"
        self.last_trace = None
        self.last_keystream = None
        self.updated_indices = set()

    def load_key_iv(self, key_hex: str, iv_hex: str):
        key_clean = re.sub(r"[^0-9A-Fa-f]", "", key_hex)
        iv_clean = re.sub(r"[^0-9A-Fa-f]", "", iv_hex)
        if not key_clean or not iv_clean:
            raise ValueError("Key và IV phải chứa các ký tự Hex hợp lệ!")

        key_clean = key_clean.ljust(32, "0")[:32]
        iv_clean = iv_clean.ljust(32, "0")[:32]

        k_w = [int(key_clean[i : i + 8], 16) for i in range(0, 32, 8)]
        iv_w = [int(iv_clean[i : i + 8], 16) for i in range(0, 32, 8)]

        # Khởi tạo 32 từ trạng thái B[0]..B[31] theo đặc tả Dragon-128
        block = [
            k_w[0], k_w[1], k_w[2], k_w[3],
            k_w[0] ^ iv_w[0], k_w[1] ^ iv_w[1], k_w[2] ^ iv_w[2], k_w[3] ^ iv_w[3],
            iv_w[0], iv_w[1], iv_w[2], iv_w[3],
            k_w[0] ^ MASK32, k_w[1] ^ MASK32, k_w[2] ^ MASK32, k_w[3] ^ MASK32,
        ]
        self.B = (block * 2)[:32]
        self.M = ((iv_w[0] << 32) | iv_w[1]) & MASK64
        self.init_round = 0
        self.tick_count = 0
        self.phase = "ĐÃ KHỞI TẠO KEY/IV (Sẵn sàng chạy Vòng Init 1..16)"
        self.last_keystream = None
        self.updated_indices = set(range(32))

        self._preview_current_trace()

    def _preview_current_trace(self):
        b0, b9, b16, b19, b30, b31 = [self.B[idx] for idx in self.TAP_INDICES]
        m_hi = (self.M >> 32) & MASK32
        m_lo = self.M & MASK32
        a = b0
        b = b9
        c = b16
        d = b19
        e = (b30 ^ m_hi) & MASK32
        f = (b31 ^ m_lo) & MASK32
        self.last_trace = call_dragon_F_trace(a, b, c, d, e, f)

    def step_init_round(self):
        """Chạy 1 vòng khởi tạo trạng thái (Vòng 1..16)."""
        if self.init_round >= 16:
            return False

        a = (self.B[0] ^ self.B[24]) & MASK32
        b = (self.B[9] ^ self.B[25]) & MASK32
        c = (self.B[16] ^ self.B[26]) & MASK32
        d = (self.B[19] ^ self.B[27]) & MASK32
        e = (self.B[30] ^ ((self.M >> 32) & MASK32)) & MASK32
        f = (self.B[31] ^ (self.M & MASK32)) & MASK32

        trace = call_dragon_F_trace(a, b, c, d, e, f)
        a_out, b_out, c_out, d_out, e_out, f_out = trace["output"]
        self.last_trace = trace

        old_B = list(self.B)
        for i in range(31, 1, -1):
            self.B[i] = old_B[i - 2]

        self.B[0] = (a_out ^ old_B[30]) & MASK32
        self.B[1] = (b_out ^ old_B[31]) & MASK32
        self.B[10] = (self.B[10] ^ c_out) & MASK32
        self.B[11] = (self.B[11] ^ d_out) & MASK32

        self.M = (self.M ^ ((e_out << 32) | f_out)) & MASK64
        self.init_round += 1
        self.updated_indices = {0, 1, 10, 11}

        if self.init_round < 16:
            self.phase = f"GIAI ĐOẠN INIT: Đã chạy vòng {self.init_round}/16"
        else:
            self.phase = "HOÀN TẤT 16 VÒNG INIT (Sẵn sàng sinh Keystream)"
        return True

    def step_keystream_tick(self):
        """Sinh 1 nhịp Keystream (Next Tick)."""
        b0, b9, b16, b19, b30, b31 = [self.B[idx] for idx in self.TAP_INDICES]
        m_hi = (self.M >> 32) & MASK32
        m_lo = self.M & MASK32

        a = b0
        b = b9
        c = b16
        d = b19
        e = (b30 ^ m_hi) & MASK32
        f = (b31 ^ m_lo) & MASK32

        trace = call_dragon_F_trace(a, b, c, d, e, f)
        a_out, b_out, c_out, d_out, e_out, f_out = trace["output"]
        self.last_trace = trace

        old_B = list(self.B)
        for i in range(31, 1, -1):
            self.B[i] = old_B[i - 2]

        self.B[0] = b_out
        self.B[1] = c_out
        self.B[10] = (self.B[10] ^ d_out) & MASK32
        self.B[11] = (self.B[11] ^ e_out) & MASK32

        self.M = (self.M + 1) & MASK64
        self.last_keystream = (a_out, f_out)
        self.tick_count += 1
        self.updated_indices = {0, 1, 10, 11}
        self.phase = f"SINH KEYSTREAM: Nhịp (Tick) #{self.tick_count}"


