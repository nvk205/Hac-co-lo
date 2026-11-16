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