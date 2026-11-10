#include "dragon_init.h"


//n để chọn mode: n=0 là 128 bit, n=1 là 256 bit
int init_key(unsigned long long *key, unsigned long long *iv, element *W, int n)
{
    if (n==1){
        W[0].lb = key[0];
        W[0].rb = key[1];
        W[1].lb = key[1] ^ iv[1];
		W[1].rb = key[0] ^ iv[0];
		W[2].lb = iv[0];
		W[2].rb = iv[1];
		W[3].lb = key[0] ^ iv[1];
		W[3].rb = key[1] ^ iv[0];
		W[4].lb = key[1];
		W[4].rb = key[0];
		W[5].lb = key[0] ^ iv[0];
		W[5].rb = key[1] ^ iv[1];
		W[6].lb = iv[1];
		W[6].rb = iv[0];
		W[7].lb = key[1] ^ iv[0];
		W[7].rb	= key[0] ^ iv[1];
    } else {
        W[0].lb = key[0];
        W[0].rb = key[1];
        W[1].lb = key[2];
		W[1].rb = key[3];
		W[2].lb = key[0] ^ iv[0];
		W[2].rb = key[1] ^ iv[1];
		W[3].lb = key[2] ^ iv[2];
		W[3].rb = key[3] ^ iv[3];
		W[4].lb = ~(key[0] ^ iv[0]);
		W[4].rb = ~(key[1] ^ iv[1]);
		W[5].lb = ~(key[2] ^ iv[2]);
		W[5].rb = ~(key[3] ^ iv[3]);	
		W[6].lb = iv[0];
		W[6].rb = iv[1];
		W[7].lb = iv[2];
		W[7].rb	= iv[3];
    }
}
