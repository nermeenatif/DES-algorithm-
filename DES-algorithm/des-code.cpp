// Including dependancies
#include <iostream>
#include <assert.h> 
#include <limits.h>
#include <string>
#include <cmath>
#include <cstring>
#ifdef __GNUC__
# define __rdtsc __builtin_ia32_rdtsc
#else
# include<intrin.h>
#endif
typedef unsigned long long u64;

using namespace std;
// Array to hold 16 keys
u64 round_keys[16];
// u64 to hold the plain text
u64 pt;
void split_int(unsigned long long a, unsigned long long& part1, unsigned long long& part2)
{
	part1 = a;
	part2 = 0;
	unsigned long long power = 1;

	while (part1 >= power) {
		unsigned long long r = part1 % 10;

		part1 /= 10;
		part2 += r * power;

		power *= 10;
	}
}
u64 leftRotate(u64 n, u64 d)
{
	u64 num = (n << d) | (n >> (28 - d));
	num &= (0xFFFFFFF);
	return num;

}
// Function to generate the 16 keys.

void generate_keys(u64 key) {
	// The PC1 table
	int pc1[56] = {
	57,49,41,33,25,17,9,
	1,58,50,42,34,26,18,
	10,2,59,51,43,35,27,
	19,11,3,60,52,44,36,
	63,55,47,39,31,23,15,
	7,62,54,46,38,30,22,
	14,6,61,53,45,37,29,
	21,13,5,28,20,12,4
	};
	// The PC2 table
	int pc2[48] = {
	14,17,11,24,1,5,
	3,28,15,6,21,10,
	23,19,12,4,26,8,
	16,7,27,20,13,2,
	41,52,31,37,47,55,
	30,40,51,45,33,48,
	44,49,39,56,34,53,
	46,42,50,36,29,32
	};
	// 1. Compressing the key using the PC1 table
	u64 perm_key=0 ;
	for (int i = 0; i < 56; ++i)
		perm_key |= (key >> (64 - pc1[56 - 1 - i]) & 1) << i;
	//printf("perm_key: %016llX\n", perm_key);
	// 2. Dividing the key into two equal halves
	u64 left=0;
	u64 right=0;
	right = perm_key & 0XFFFFFFF;
	left= perm_key >> 28 & 0XFFFFFFF;
	//printf("left: %016llX\n", left);
	//printf("right: %016llX\n", right);
	for (int i = 0; i < 16; i++) {
		// 3.1. For rounds 1, 2, 9, 16 the key_chunks
		// are shifted by one.
		if (i == 0 || i == 1 || i == 8 || i == 15) {
			left = leftRotate(left, 1);
			//printf("left: %016llX\n", left);
			right = leftRotate(right, 1);
			//printf("right: %016llX\n", right);
		}
		// 3.2. For other rounds, the key_chunks
		// are shifted by two
		else {
			left = leftRotate(left, 2);
			right = leftRotate(right, 2);
		}
		// Combining the two chunks
		
		left= left << 28 & 0XFFFFFFF0000000;
		
		u64 combined_key = left | right;
		
		left = left >> 28 & 0XFFFFFFF;
		u64 round_key=0 ;
		// Finally, using the PC2 table to transpose the key bits
		for (int i = 0; i < 48; ++i)
			round_key |= (combined_key >> (56 - pc2[48 - 1 - i]) & 1) << i;
		round_keys[i] = round_key;
		
	}

}
// Implementing the algorithm
u64 DES() {
	// The initial permutation table 
	int initial_permutation[64] = {
	58,50,42,34,26,18,10,2,
	60,52,44,36,28,20,12,4,
	62,54,46,38,30,22,14,6,
	64,56,48,40,32,24,16,8,
	57,49,41,33,25,17,9,1,
	59,51,43,35,27,19,11,3,
	61,53,45,37,29,21,13,5,
	63,55,47,39,31,23,15,7
	};
	// The expansion table
	int expansion_table[48] = {
	32,1,2,3,4,5,4,5,
	6,7,8,9,8,9,10,11,
	12,13,12,13,14,15,16,17,
	16,17,18,19,20,21,20,21,
	22,23,24,25,24,25,26,27,
	28,29,28,29,30,31,32,1
	};
	// The substitution boxes. The should contain values
	// from 0 to 15 in any order.
	int substition_boxes[8][64] =
	{ {
		14,4,13,1,2,15,11,8,3,10,6,12,5,9,0,7,
		0,15,7,4,14,2,13,1,10,6,12,11,9,5,3,8,
		4,1,14,8,13,6,2,11,15,12,9,7,3,10,5,0,
		15,12,8,2,4,9,1,7,5,11,3,14,10,0,6,13
	},
	{
		15,1,8,14,6,11,3,4,9,7,2,13,12,0,5,10,
		3,13,4,7,15,2,8,14,12,0,1,10,6,9,11,5,
		0,14,7,11,10,4,13,1,5,8,12,6,9,3,2,15,
		13,8,10,1,3,15,4,2,11,6,7,12,0,5,14,9
	},
	{
		10,0,9,14,6,3,15,5,1,13,12,7,11,4,2,8,
		13,7,0,9,3,4,6,10,2,8,5,14,12,11,15,1,
		13,6,4,9,8,15,3,0,11,1,2,12,5,10,14,7,
		1,10,13,0,6,9,8,7,4,15,14,3,11,5,2,12
	},
	{
		7,13,14,3,0,6,9,10,1,2,8,5,11,12,4,15,
		13,8,11,5,6,15,0,3,4,7,2,12,1,10,14,9,
		10,6,9,0,12,11,7,13,15,1,3,14,5,2,8,4,
		3,15,0,6,10,1,13,8,9,4,5,11,12,7,2,14
	},
	{
		2,12,4,1,7,10,11,6,8,5,3,15,13,0,14,9,
		14,11,2,12,4,7,13,1,5,0,15,10,3,9,8,6,
		4,2,1,11,10,13,7,8,15,9,12,5,6,3,0,14,
		11,8,12,7,1,14,2,13,6,15,0,9,10,4,5,3
	},
	{
		12,1,10,15,9,2,6,8,0,13,3,4,14,7,5,11,
		10,15,4,2,7,12,9,5,6,1,13,14,0,11,3,8,
		9,14,15,5,2,8,12,3,7,0,4,10,1,13,11,6,
		4,3,2,12,9,5,15,10,11,14,1,7,6,0,8,13
	},
	{
		4,11,2,14,15,0,8,13,3,12,9,7,5,10,6,1,
		13,0,11,7,4,9,1,10,14,3,5,12,2,15,8,6,
		1,4,11,13,12,3,7,14,10,15,6,8,0,5,9,2,
		6,11,13,8,1,4,10,7,9,5,0,15,14,2,3,12
	},
	{
		13,2,8,4,6,15,11,1,10,9,3,14,5,0,12,7,
		1,15,13,8,10,3,7,4,12,5,6,11,0,14,9,2,
		7,11,4,1,9,12,14,2,0,6,10,13,15,3,5,8,
		2,1,14,7,4,10,8,13,15,12,9,0,3,5,6,11
	} };
	// The permutation table
	int permutation_tab[32] = {
	16,7,20,21,29,12,28,17,
	1,15,23,26,5,18,31,10,
	2,8,24,14,32,27,3,9,
	19,13,30,6,22,11,4,25
	};
	// The inverse permutation table
	int inverse_permutation[64] = {
	40,8,48,16,56,24,64,32,
	39,7,47,15,55,23,63,31,
	38,6,46,14,54,22,62,30,
	37,5,45,13,53,21,61,29,
	36,4,44,12,52,20,60,28,
	35,3,43,11,51,19,59,27,
	34,2,42,10,50,18,58,26,
	33,1,41,9,49,17,57,25
	};
	//1. Applying the initial permutation
	u64 perm=0;
	for (int i = 0; i < 64; ++i)
		perm |= (pt >> (64 - initial_permutation[64 - 1 - i]) & 1) << i;
	
	
	// 2. Dividing the result into two equal halves 
	u64 left=0 ;
	u64 right=0;
	right = perm & 0XFFFFFFFF;
	left = perm >> 32 & 0XFFFFFFFF;
	

	// The plain text is encrypted 16 times  
	for (int i = 0; i < 16; i++) {
		u64 right_expanded=0 ;
		// 3.1. The right half of the plain text is expanded 
		for (int i = 0; i < 48; ++i)
			right_expanded |= (right >> (32 - expansion_table[48 - 1 - i]) & 1) << i;
		//printf("right_expanded: %016llX\n", right_expanded);
		// 3.3. The result is xored with a key
		u64 xored = round_keys[i]^right_expanded;
		//printf("xored: %016llX\n", xored);
		u64 res=0 ;
		// 3.4. The result is divided into 8 equal parts and passed 
		// through 8 substitution boxes. After passing through a 
		// substituion box, each box is reduces from 6 to 4 bits.
		for (int i = 0; i < 8; i++) {
			
			int idx;
			idx = xored >> (7 - i) * 6 & 0x3F; 
			idx = idx >> 1 & 15 | (idx & 1) << 4 | idx & 0x20;
			res |= substition_boxes[i][idx] << (7 - i) * 4;
		}
		
		// 3.5. Another permutation is applied
		u64 perm2=0 ;
		for (int i = 0; i < 32; ++i)
			perm2 |= (res >> (32 - permutation_tab[32 - 1 - i]) & 1) << i;
		
		// 3.6. The result is xored with the left half
		xored = perm2^left;
		
		// 3.7. The left and the right parts of the plain text are swapped 
		left = xored;
		if (i < 15) {
			u64 temp = right;
			right = xored;
			left = temp;
		}
	}
	
	// 4. The halves of the plain text are applied
	left = left << 32 & 0XFFFFFFFF00000000;
	
	u64 combined_text = left | right;
	
	left = left >> 28 & 0XFFFFFFF;
	u64 ciphertext=0;
	// The inverse of the initial permuttaion is applied
	for (int i = 0; i < 64; ++i)
		ciphertext |= (combined_text >> (64 - inverse_permutation[64 - 1 - i]) & 1) << i;
	//And we finally get the cipher text
	return ciphertext;
}

u64 read_DES_input(const char* data)
{
	assert(strlen(data) == 16);
	u64 value = 0;
	for (int i = 0; i < 16; ++i)
	{
		char c = data[i];
		if (c >= '0' && c <= '9')
			value |= (u64)(c - '0') << ((15 - i) << 2);
		else if (c >= 'A' && c <= 'F')
			value |= (u64)(c - 'A' + 10) << ((15 - i) << 2);
		else if (c >= 'a' && c <= 'f')
			value |= (u64)(c - 'a' + 10) << ((15 - i) << 2);
	}
	return value;
}



int main(int argc, char** argv) {
	string name = argv[1];
	string data=argv[2];
	string data1=argv[3];
	

	
    char d[64];
    strcpy(d, data.c_str());
    char d1[64];
    strcpy(d1, data1.c_str());
	u64 key=0;
	pt= read_DES_input(d);
	key = read_DES_input(d1);
	generate_keys(key);
	if (name == "encrypt") {
		long long t1 = __rdtsc();
		u64 cipher = DES();
		long long t2 = __rdtsc();
		printf("cipher: %016llX\n", cipher);
		printf("Cycles: %lld\n", t2 - t1);
	}
	else {
		int i = 15;
		int j = 0;
		while (i > j)
		{
			u64 temp = round_keys[i];
			round_keys[i] = round_keys[j];
			round_keys[j] = temp;
			i--;
			j++;
		}
		long long t1 = __rdtsc();
		u64 plain = DES();
		long long t2 = __rdtsc();
		printf("plain: %016llX\n", plain);
		printf("Cycles: %lld\n", t2 - t1);
	}


	
return 0;
}