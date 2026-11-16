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
