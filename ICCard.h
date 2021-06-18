#ifndef __IC_CARD_H__
#define __IC_CARD_H__

// return block_num>0, or error. (block_num+1 used for block backup)
char iccard_get_block_num();

// key_type: 0x1 A, 0x2 B. fill 6 bytes key, return key_type, or 0 for invalid.
char iccard_get_key(char key_type, char * key6bytes);

struct ICCARD_DATA {
	char version; // 0x0 invalid, 0x1 first version.
	char type; //0x0 invalid, 0x1 normal, 0x2 company, 0x3 VIP, bit7 1 is for locked card.
	char vyear; //year - 2016
	char vday; // month:(vday>>4), day/2:(vday&0xf)
	unsigned int region;
	unsigned int value;
	unsigned int crc;
} ;

// return null for error, or data16bytes, snr is card serial number.
ICCARD_DATA * iccard_decode(char * data16bytes, char * snr4bytes);
char * iccard_encode(ICCARD_DATA * data16bytes, char * snr4bytes);


//The following functions are used by setting from cloud.
char iccard_set_key(char key_type, char * key6bytes);
char iccard_set_block_num(char n);
void iccard_set_seed(char * seed4bytes);

#endif
