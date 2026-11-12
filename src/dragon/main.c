#include <stdio.h>
#include <string.h>
#include "dragon.h"
#include "dragon_init.h"
#include "dragon_f.h"
#include "dragon_cipher.h"

/* 
Xây dựng hàm main hỗ trợ các tham số dòng lệnh để cả người dùng lẫn Giao diện Python có thể gọi dễ dàng:
1. ./dragon_cli --encrypt-text <key_hex> <iv_hex> "<plaintext>"
2. ./dragon_cli --encrypt-file <key_hex> <iv_hex> <input_file> <output_file>
3. ./dragon_cli --trace-json   <key_hex> <iv_hex> <num_ticks>
(In ra màn hình dạng JSON trạng thái 16 vòng Init và từng bước hàm F để GUI đọc)
*/

int main(int argc, char *argv[]) {
    //Cài đặt menu dòng lệnh hoặc xử lý tham số argv[]

    return 0;
}