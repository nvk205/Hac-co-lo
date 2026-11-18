CC      = gcc
CFLAGS  = -Wall -Wextra -O2 -std=c99 -Isrc/dragon

# Tự động nhận diện Windows/Linux
ifeq ($(OS),Windows_NT)
    EXT = .exe
    RM  = del /Q /F
    RUN_PFX =
    PYTHON = python
else
    EXT =
    RM  = rm -f
    RUN-PFX = ./
    PYTHON = python3
endif

# Danh sách các file Dragon core
CORE_SRCS = src/dragon/dragon_sbox.c \
            src/dragon/dragon_f.c \
            src/dragon/dragon_init.c \
            src/dragon/dragon.c \
            src/dragon/dragon_cipher.c

# Các file thực thi đầu ra
TARGET_CLI  = dragon_cli$(EXT)
TARGET_TEST = test_dragon$(EXT)
TARGET_GH   = test_gh_func$(EXT)
TARGET_EVAL = crypto_eval$(EXT)


.PHONY: all test test-gh analysis gui clean help

# Biên dịch chương trình
all: $(TARGET_CLI) $(TARGET_TEST) $(TARGET_GH) $(TARGET_EVAL)

# Main
$(TARGET_CLI): $(CORE_SRCS) src/dragon/main.c
	$(CC) $(CFLAGS) -o $(TARGET_CLI) $(CORE_SRCS) src/dragon/main.c

# Test vector
$(TARGET_TEST): $(CORE_SRCS) tests/test_dragon.c
	$(CC) $(CFLAGS) -o $(TARGET_TEST) $(CORE_SRCS) tests/test_dragon.c

# Test function
$(TARGET_GH): src/dragon/dragon_sbox.c src/dragon/test_function/test_G-H_func.c
	$(CC) $(CFLAGS) -o $(TARGET_GH) src/dragon/dragon_sbox.c src/dragon/test_function/test_G-H_func.c

# Crypto_Eval
$(TARGET_EVAL): $(CORE_SRCS) analysis/crypto_eval.c
	$(CC) $(CFLAGS) -o $(TARGET_EVAL) $(CORE_SRCS) analysis/crypto_eval.c -lm

# Lệnh chạy nhanh bộ kiểm thử chuẩn
test: $(TARGET_TEST)
	$(RUN_PFX)$(TARGET_TEST)

# Lệnh chạy nhanh kiểm thử hàm G, H
test-gh: $(TARGET_GH)
	$(RUN_PFX)$(TARGET_GH)

# Lệnh chạy thực nghiệm phân tích mật mã (Avalanche & Randomness)
analysis: $(TARGET_EVAL)
	$(RUN_PFX)$(TARGET_EVAL)

# Lệnh khởi chạy Giao diện trực quan (TV8 & TV9)
gui: $(TARGET_CLI) $(TARGET_EVAL)
	$(PYTHON) gui/app_main.py

# Dọn dẹp các tệp thực thi
clean:
	$(RM) $(TARGET_CLI) $(TARGET_TEST) $(TARGET_GH) $(TARGET_EVAL) *.o *.csv