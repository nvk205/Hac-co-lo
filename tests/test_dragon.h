#ifndef TEST_DRAGON_H
#define TEST_DRAGON_H

int test_sbox_and_G(const char *filepath);
int test_f_function(void);
int test_encrypt_decrypt_roundtrip(void);
int test_estream_vectors(const char *vector_filepath);

#endif 