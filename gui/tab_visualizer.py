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



# 5. GIAO DIỆN 

class VisualizerTab(tk.Frame):
    BG_MAIN = "#0F172A"
    BG_PANEL = "#1E293B"
    BG_CELL_NORMAL = "#334155"
    BG_CELL_TAP = "#D97706"      # Tô nổi bật 6 ô đưa vào hàm F: B[0], B[9], B[16], B[19], B[30], B[31]
    BG_CELL_UPDATED = "#059669"  # Tô nổi bật các ô nhận dữ liệu phản hồi mới
    FG_TEXT = "#F8FAFC"
    FG_SUB = "#94A3B8"
    FG_ACCENT = "#38BDF8"

    def __init__(self, parent=None, *args, **kwargs):
        super().__init__(parent, bg=self.BG_MAIN, *args, **kwargs)
        self.sim = DragonStateSimulator()
        self.auto_playing = False
        self.cell_frames = []
        self.cell_val_labels = []
        self.cell_tag_labels = []
        self.f_table_labels = {}

        self._build_ui()
        self.on_load_key_iv()

    def _build_ui(self):
        # 1. Thanh tiêu đề
        header_frame = tk.Frame(self, bg=self.BG_MAIN, padx=14, pady=8)
        header_frame.pack(fill=tk.X)

        tk.Label(
            header_frame,
            text="MÔ PHỎNG TRẠNG THÁI DRAGON",
            font=("Consolas", 14, "bold"),
            bg=self.BG_MAIN,
            fg=self.FG_ACCENT,
        ).pack(side=tk.LEFT)

        self.lbl_phase = tk.Label(
            header_frame,
            text="Trạng thái: Chưa khởi tạo",
            font=("Segoe UI", 10, "bold"),
            bg="#1D4ED8",
            fg=self.FG_TEXT,
            padx=10,
            pady=4,
        )
        self.lbl_phase.pack(side=tk.RIGHT)

        # 2. Bảng điều khiển
        ctrl_frame = tk.LabelFrame(
            self,
            text=" Bảng Điều Khiển (Load Key/IV → Vòng Init 1..16 → Next Tick) ",
            font=("Segoe UI", 10, "bold"),
            bg=self.BG_PANEL,
            fg=self.FG_ACCENT,
            padx=12,
            pady=6,
        )
        ctrl_frame.pack(fill=tk.X, padx=14, pady=4)

        input_row = tk.Frame(ctrl_frame, bg=self.BG_PANEL)
        input_row.pack(fill=tk.X, pady=(0, 6))

        tk.Label(input_row, text="Key (Hex):", font=("Consolas", 10, "bold"), bg=self.BG_PANEL, fg=self.FG_TEXT).pack(side=tk.LEFT)
        self.ent_key = tk.Entry(input_row, font=("Consolas", 10), width=34, bg="#0F172A", fg="#38BDF8", insertbackground="white")
        self.ent_key.insert(0, "00000000000000000000000000000000")
        self.ent_key.pack(side=tk.LEFT, padx=(6, 16))

        tk.Label(input_row, text="IV (Hex):", font=("Consolas", 10, "bold"), bg=self.BG_PANEL, fg=self.FG_TEXT).pack(side=tk.LEFT)
        self.ent_iv = tk.Entry(input_row, font=("Consolas", 10), width=34, bg="#0F172A", fg="#38BDF8", insertbackground="white")
        self.ent_iv.insert(0, "00000000000000000000000000000000")
        self.ent_iv.pack(side=tk.LEFT, padx=(6, 12))

        tk.Button(
            input_row,
            text="Nạp mẫu từ tests/vectors.txt",
            font=("Segoe UI", 9),
            bg="#334155",
            fg="#F8FAFC",
            padx=8,
            pady=2,
            command=self.on_load_from_vectors_file,
        ).pack(side=tk.LEFT)

        btn_row = tk.Frame(ctrl_frame, bg=self.BG_PANEL)
        btn_row.pack(fill=tk.X)

        self.btn_init = tk.Button(
            btn_row,
            text="1. Khởi tạo (Load Key/IV)",
            font=("Segoe UI", 9, "bold"),
            bg="#2563EB",
            fg="white",
            padx=10,
            pady=4,
            command=self.on_load_key_iv,
        )
        self.btn_init.pack(side=tk.LEFT, padx=(0, 8))

        self.btn_step_init = tk.Button(
            btn_row,
            text="2. Chạy từng vòng Init (Vòng 1..16)",
            font=("Segoe UI", 9, "bold"),
            bg="#D97706",
            fg="white",
            padx=10,
            pady=4,
            command=self.on_step_init,
        )
        self.btn_step_init.pack(side=tk.LEFT, padx=(0, 8))

        self.btn_run_all_init = tk.Button(
            btn_row,
            text="Chạy hết 16 vòng Init",
            font=("Segoe UI", 9),
            bg="#475569",
            fg="white",
            padx=8,
            pady=4,
            command=self.on_finish_init,
        )
        self.btn_run_all_init.pack(side=tk.LEFT, padx=(0, 8))

        self.btn_next_tick = tk.Button(
            btn_row,
            text="3. Sinh 1 nhịp Keystream (Next Tick)",
            font=("Segoe UI", 9, "bold"),
            bg="#059669",
            fg="white",
            padx=10,
            pady=4,
            command=self.on_next_tick,
        )
        self.btn_next_tick.pack(side=tk.LEFT, padx=(0, 8))

        self.btn_auto = tk.Button(
            btn_row,
            text="▶ Trình chiếu tự động",
            font=("Segoe UI", 9, "bold"),
            bg="#7C3AED",
            fg="white",
            padx=10,
            pady=4,
            command=self.toggle_auto_play,
        )
        self.btn_auto.pack(side=tk.RIGHT)

        # 3. Lưới 4 x 8 hiển thị 32 ô thanh ghi B[0]...B[31] (mỗi ô 8 ký tự Hex)
        grid_container = tk.LabelFrame(
            self,
            text=" 32 Ô Thanh Ghi B[0]...B[31] (Lưới 4 × 8 ô, mỗi ô hiện 8 ký tự Hex) ",
            font=("Segoe UI", 10, "bold"),
            bg=self.BG_PANEL,
            fg=self.FG_ACCENT,
            padx=8,
            pady=6,
        )
        grid_container.pack(fill=tk.BOTH, expand=True, padx=14, pady=4)

        legend_frame = tk.Frame(grid_container, bg=self.BG_PANEL)
        legend_frame.pack(fill=tk.X, pady=(0, 4))
        self._add_legend(legend_frame, self.BG_CELL_TAP, "Ô đưa dữ liệu vào hàm F: B[0], B[9], B[16], B[19], B[30], B[31]")
        self._add_legend(legend_frame, self.BG_CELL_UPDATED, "Ô vừa nhận dữ liệu phản hồi mới (B[0], B[1], B[10], B[11])")
        self._add_legend(legend_frame, self.BG_CELL_NORMAL, "Ô dịch chuyển trạng thái")

        cells_frame = tk.Frame(grid_container, bg=self.BG_PANEL)
        cells_frame.pack(fill=tk.BOTH, expand=True)

        for r in range(4):
            cells_frame.rowconfigure(r, weight=1)
            for c in range(8):
                cells_frame.columnconfigure(c, weight=1)
                idx = r * 8 + c
                cell = tk.Frame(cells_frame, bg=self.BG_CELL_NORMAL, bd=2, relief=tk.RIDGE, padx=4, pady=3)
                cell.grid(row=r, column=c, sticky="nsew", padx=3, pady=3)

                top_lbl = tk.Label(cell, text=f"B[{idx}]", font=("Consolas", 9, "bold"), bg=self.BG_CELL_NORMAL, fg="#E2E8F0", anchor="w")
                top_lbl.pack(fill=tk.X)

                val_lbl = tk.Label(cell, text="00000000", font=("Consolas", 12, "bold"), bg=self.BG_CELL_NORMAL, fg="#FFFFFF")
                val_lbl.pack(expand=True)

                tag_lbl = tk.Label(cell, text="", font=("Segoe UI", 8, "bold"), bg=self.BG_CELL_NORMAL, fg="#FEF08A")
                tag_lbl.pack(fill=tk.X)

                self.cell_frames.append(cell)
                self.cell_val_labels.append((top_lbl, val_lbl))
                self.cell_tag_labels.append(tag_lbl)

        # 4. Bộ đếm M (64-bit) & 6 biến (a, b, c, d, e, f) qua 3 tầng của hàm F
        bottom_frame = tk.Frame(self, bg=self.BG_MAIN)
        bottom_frame.pack(fill=tk.BOTH, expand=True, padx=14, pady=(4, 10))

        m_frame = tk.LabelFrame(
            bottom_frame,
            text=" Ô Bộ Đếm M (64-bit) & Keystream ",
            font=("Segoe UI", 10, "bold"),
            bg=self.BG_PANEL,
            fg=self.FG_ACCENT,
            padx=12,
            pady=8,
        )
        m_frame.pack(side=tk.LEFT, fill=tk.BOTH, padx=(0, 8))

        tk.Label(m_frame, text="Ô BỘ ĐẾM M (64-bit / 16 ký tự Hex):", font=("Segoe UI", 9, "bold"), bg=self.BG_PANEL, fg=self.FG_SUB).pack(anchor="w")
        self.lbl_m_64 = tk.Label(
            m_frame,
            text="0x0000000000000000",
            font=("Consolas", 14, "bold"),
            bg="#0F172A",
            fg="#FBBF24",
            padx=10,
            pady=6,
            relief=tk.SUNKEN,
        )
        self.lbl_m_64.pack(fill=tk.X, pady=(2, 6))

        self.lbl_m_split = tk.Label(
            m_frame,
            text="M_L (32-bit): 00000000 | M_R (32-bit): 00000000",
            font=("Consolas", 9),
            bg=self.BG_PANEL,
            fg=self.FG_TEXT,
        )
        self.lbl_m_split.pack(anchor="w", pady=(0, 8))

        tk.Label(m_frame, text="KEYSTREAM SINH RA Ở NHỊP HIỆN TẠI (z_t):", font=("Segoe UI", 9, "bold"), bg=self.BG_PANEL, fg=self.FG_SUB).pack(anchor="w")
        self.lbl_keystream = tk.Label(
            m_frame,
            text="Chưa sinh Keystream",
            font=("Consolas", 11, "bold"),
            bg="#0F172A",
            fg="#34D399",
            padx=10,
            pady=6,
            relief=tk.SUNKEN,
        )
        self.lbl_keystream.pack(fill=tk.X, pady=2)

        f_frame = tk.LabelFrame(
            bottom_frame,
            text=" 6 Biến (a, b, c, d, e, f) Chạy Qua 3 Tầng Của Hàm F (dragon_F_trace) ",
            font=("Segoe UI", 10, "bold"),
            bg=self.BG_PANEL,
            fg=self.FG_ACCENT,
            padx=10,
            pady=6,
        )
        f_frame.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)

        headers = ["Tầng hàm F", "a (B[0])", "b (B[9])", "c (B[16])", "d (B[19])", "e (B[30]⊕M_L)", "f (B[31]⊕M_R)"]
        for col, h_text in enumerate(headers):
            f_frame.columnconfigure(col, weight=1)
            tk.Label(
                f_frame,
                text=h_text,
                font=("Consolas", 9, "bold"),
                bg="#0F172A",
                fg="#38BDF8",
                pady=4,
                padx=4,
            ).grid(row=0, column=col, sticky="nsew", padx=2, pady=2)

        stages = [
            ("input", "Đầu vào hàm F", "#E2E8F0"),
            ("layer1", "Tầng 1 (Pre-mixing)", "#FDE047"),
            ("layer2", "Tầng 2 (S-box G, H)", "#FB923C"),
            ("layer3", "Tầng 3 (Post-mixing)", "#4ADE80"),
        ]

        for row_idx, (stage_key, stage_title, color) in enumerate(stages, start=1):
            tk.Label(
                f_frame,
                text=stage_title,
                font=("Segoe UI", 9, "bold"),
                bg="#1E293B",
                fg=color,
                anchor="w",
                padx=6,
            ).grid(row=row_idx, column=0, sticky="nsew", padx=2, pady=2)

            self.f_table_labels[stage_key] = []
            for var_idx in range(6):
                lbl = tk.Label(
                    f_frame,
                    text="00000000",
                    font=("Consolas", 10, "bold"),
                    bg="#0F172A",
                    fg=color,
                    pady=4,
                )
                lbl.grid(row=row_idx, column=var_idx + 1, sticky="nsew", padx=2, pady=2)
                self.f_table_labels[stage_key].append(lbl)

    def _add_legend(self, parent, color, text):
        box = tk.Frame(parent, bg=color, width=14, height=14, bd=1, relief=tk.SOLID)
        box.pack(side=tk.LEFT, padx=(0, 5))
        tk.Label(parent, text=text, font=("Segoe UI", 8), bg=self.BG_PANEL, fg=self.FG_TEXT).pack(side=tk.LEFT, padx=(0, 16))


    def refresh_display(self):
        self.lbl_phase.config(text=self.sim.phase)
        tap_names = {
            0: "→ biến a (Hàm F)",
            9: "→ biến b (Hàm F)",
            16: "→ biến c (Hàm F)",
            19: "→ biến d (Hàm F)",
            30: "→ biến e (⊕ M_L)",
            31: "→ biến f (⊕ M_R)",
        }

        for idx in range(32):
            val_hex = f"{self.sim.B[idx]:08X}"
            top_lbl, val_lbl = self.cell_val_labels[idx]
            tag_lbl = self.cell_tag_labels[idx]
            cell = self.cell_frames[idx]

            if idx in self.sim.TAP_INDICES:
                bg_color = self.BG_CELL_TAP
                tag_text = tap_names[idx]
            elif idx in self.sim.updated_indices and (self.sim.init_round + self.sim.tick_count > 0):
                bg_color = self.BG_CELL_UPDATED
                tag_text = "★ Mới cập nhật"
            else:
                bg_color = self.BG_CELL_NORMAL
                tag_text = ""

            cell.config(bg=bg_color)
            top_lbl.config(bg=bg_color)
            val_lbl.config(text=val_hex, bg=bg_color)
            tag_lbl.config(text=tag_text, bg=bg_color)

        m_hi = (self.sim.M >> 32) & MASK32
        m_lo = self.sim.M & MASK32
        self.lbl_m_64.config(text=f"0x{self.sim.M:016X}")
        self.lbl_m_split.config(text=f"M_L (32-bit cao): {m_hi:08X} | M_R (32-bit thấp): {m_lo:08X}")

        if self.sim.last_keystream is None:
            self.lbl_keystream.config(text=f"Đang ở giai đoạn Init (Vòng {self.sim.init_round}/16)")
        else:
            z_hi, z_lo = self.sim.last_keystream
            self.lbl_keystream.config(text=f"z_{self.sim.tick_count} = {z_hi:08X} {z_lo:08X} (64-bit)")

        if self.sim.last_trace:
            for stage_key in ("input", "layer1", "layer2", "layer3"):
                vals = self.sim.last_trace.get(stage_key, (0, 0, 0, 0, 0, 0))
                for i in range(6):
                    self.f_table_labels[stage_key][i].config(text=f"{vals[i]:08X}")

        if self.sim.init_round < 16:
            self.btn_step_init.config(state=tk.NORMAL, text=f"2. Chạy vòng Init ({self.sim.init_round + 1}/16)")
            self.btn_run_all_init.config(state=tk.NORMAL)
        else:
            self.btn_step_init.config(state=tk.DISABLED, text="2. Đã xong 16 vòng Init")
            self.btn_run_all_init.config(state=tk.DISABLED)

    def on_load_from_vectors_file(self):
        vec_path = os.path.join(ROOT_DIR, "tests", "vectors.txt")
        if os.path.exists(vec_path):
            try:
                with open(vec_path, "r", encoding="utf-8", errors="ignore") as f:
                    text = f.read()
                hex_strings = re.findall(r"[0-9A-Fa-f]{32}", text)
                if len(hex_strings) >= 2:
                    self.ent_key.delete(0, tk.END)
                    self.ent_key.insert(0, hex_strings[0].upper())
                    self.ent_iv.delete(0, tk.END)
                    self.ent_iv.insert(0, hex_strings[1].upper())
                    self.on_load_key_iv()
                    return
            except Exception:
                pass
        # Mẫu Test Vector tiêu chuẩn nếu file vectors.txt dùng định dạng khác
        self.ent_key.delete(0, tk.END)
        self.ent_key.insert(0, "00112233445566778899AABBCCDDEEFF")
        self.ent_iv.delete(0, tk.END)
        self.ent_iv.insert(0, "FFEEDDCCBBAA99887766554433221100")
        self.on_load_key_iv()

    def on_load_key_iv(self):
        self.stop_auto_play()
        try:
            self.sim.load_key_iv(self.ent_key.get(), self.ent_iv.get())
            self.refresh_display()
        except ValueError as err:
            messagebox.showerror("Lỗi dữ liệu Key/IV", str(err))

    def on_step_init(self):
        self.sim.step_init_round()
        self.refresh_display()

    def on_finish_init(self):
        while self.sim.init_round < 16:
            self.sim.step_init_round()
        self.refresh_display()

    def on_next_tick(self):
        while self.sim.init_round < 16:
            self.sim.step_init_round()
        self.sim.step_keystream_tick()
        self.refresh_display()

    def toggle_auto_play(self):
        if self.auto_playing:
            self.stop_auto_play()
        else:
            self.auto_playing = True
            self.btn_auto.config(text="⏸ Tạm dừng", bg="#DC2626")
            self._auto_step()

    def stop_auto_play(self):
        self.auto_playing = False
        self.btn_auto.config(text="▶ Trình chiếu tự động", bg="#7C3AED")

    def _auto_step(self):
        if not self.auto_playing:
            return
        if self.sim.init_round < 16:
            self.sim.step_init_round()
        else:
            self.sim.step_keystream_tick()
        self.refresh_display()
        self.after(650, self._auto_step)


TabVisualizer = VisualizerTab
DragonVisualizerTab = VisualizerTab

def create_visualizer_tab(parent):
    return VisualizerTab(parent)


if __name__ == "__main__":
    root = tk.Tk()
    root.title("Dragon Stream Cipher — White-box Step-by-Step Visualizer")
    root.geometry("1280x820")
    tab = VisualizerTab(root)
    tab.pack(fill=tk.BOTH, expand=True)
    root.mainloop()