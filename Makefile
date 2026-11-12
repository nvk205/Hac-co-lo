CC      = gcc
CFLAGS  = -Wall -Wextra -O2 -std=c99 -Isrc/dragon

# Tự động nhận diện Windows/Linux
ifeq ($(OS),Windows_NT)
    EXT = .exe
    RM  = del /Q /F
else
    EXT =
    RM  = rm -f
endif

# Danh sách các file Dragon core
CORE_SRCS = src/dragon/dragon_sbox.c \
            src/dragon/dragon_f.c \
            src/dragon/dragon_init.c \
            src/dragon/dragon.c \
            src/dragon/dragon_cipher.c
