#include "string.h"
#include "ICCard.h"
#include "utils.h"

static char block_num = 44; // 2C
static char seed[4] = {97, 164, 87, 104};

static bool useKeyA = true;
static bool useKeyB = false;
static char keyA[6] = {117, 183, 147, 12, 88, 238}; // {0x75, 0xB7, 0x93, 0x0C, 0x58, 0xEE}
static char keyB[6] = {117, 183, 147, 12, 88, 238};

// return block_num>0, or error. (block_num+1 used for block backup)
char iccard_get_block_num(){
	return block_num;
}

char iccard_set_block_num( char n){
	block_num = n;
	return n;
}

// key_type: 0x1 A, 0x2 B. fill 6 bytes key, return key_type, or 0 for invalid.
char iccard_get_key(char key_type, char * key6bytes){
	if(key_type == 0x1){
		if(!useKeyA) return 0;
		memcpy(key6bytes, keyA, 6);
	} else if(key_type == 0x2){
		if(!useKeyB) return 0;
		memcpy(key6bytes, keyB, 6);
	} else {
		return 0;
	}

	return key_type;
}

char iccard_set_key(char key_type, char * key6bytes){
	if(key_type == 0x1){
		memcpy(keyA, key6bytes, 6);
		useKeyA = true;
	} else if(key_type == 0x2){
		memcpy(keyB, key6bytes, 6);
		useKeyB = 1;
	} else {
		return 0;
	}

	return key_type;
}


static void calc_key(char * key4, char * snr4){
	key4[0] = snr4[2]^seed[1];
	key4[1] = snr4[1]^seed[3];
	key4[2] = snr4[3]^seed[0];
	key4[3] = snr4[0]^seed[2];
}

// return null for error, or data16bytes.
ICCARD_DATA * iccard_decode(char * data16bytes, char * snr4bytes){
	int key;
	calc_key((char *)&key, snr4bytes);
	int * d = (int *)data16bytes;

    // invalid data
    if((d[0] == 0x30303030 && d[1] == 0x30303030 && d[2] == 0x30303030 && d[3] == 0x30303030) ||
        (d[0] == 0x00 && d[1] == 0x00 && d[2] == 0x00 && d[3] == 0x00))
    {
        FTRACE("###iccard_decode() invalid data: 0x%08X\r\n", d[0]);
        return 0;
    }
	
	d[0]^=key;
	d[1]^=key;
	d[2]^=key;
	d[3]^=key;

	if(d[3] != (d[1]^d[0]^d[2])) return 0;
	return (ICCARD_DATA *) d;
}

char * iccard_encode(ICCARD_DATA * data16bytes, char * snr4bytes){
	int key;
	calc_key((char *)&key, snr4bytes);
	int * d = (int *)data16bytes;

	d[3] = d[1]^d[0]^d[2];
	
	d[0]^=key;
	d[1]^=key;
	d[2]^=key;
	d[3]^=key;

	
	return (char *)d;
}

void iccard_set_seed(char * seed4bytes){
	memcpy(seed, seed4bytes, 4);
}
