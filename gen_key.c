int gen_key(u_int *B, u64 *z) {
	u_int A[6];
	A[0] = B[0];
	A[1] = B[9];
	A[2] = B[16];
	A[3] = B[19];
	A[4] = B[30] ^ M[0];
	A[5] = B[31] ^ M[1];

	F_func(A);

	for (int i = 31; i >= 2; i--)
	       B[i] = B[i-2];
	
	B[0] = A[1];
	B[1] = A[2];
	
	u64 new_M = (((u64)M[0])<<32) | ((u64)(M[1]));
	new_M += 1;
	M[0] = (u_int)(new_M >> 32);
	M[1] = (u_int)(new_M & 0xFFFFFFFFULL);

	*z = (((u64)(A[0]))<<32) ^ ((u64)(A[4]));
	return 0;
}