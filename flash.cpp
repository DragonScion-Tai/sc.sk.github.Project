
#include "mbed.h"
#include "string.h"
#include "hal/flash_api.h"

#include "utils.h"

#define BL_CCM 0x10000000
#define BL_CUR_VERSION (((unsigned char *)BL_CCM)[0])
#define BL_REQ_UPDATE (((unsigned char *)BL_CCM)[1])

#define MAIN_FLASH (uint32_t)(0x08000000)
#define CUR_ADDRESS (MAIN_FLASH+0x20000)
#define V1_ADDRESS (MAIN_FLASH+0x60000)
#define V2_ADDRESS (MAIN_FLASH+0xA0000)

#define VERSION_A 0x5b
#define VERSION_B 0x6a
#define UP_VERSION_A 0x3c
#define UP_VERSION_B 0xc3

static flash_t flash;

void upgrade_init(){

	flash_init(&flash);
}

uint32_t upgrade_get_base_addr(){
	if(BL_CUR_VERSION == VERSION_B){
		return V1_ADDRESS;
	} else if(BL_CUR_VERSION == VERSION_A){
		return V2_ADDRESS;
	} else {
		return V2_ADDRESS;
	}
}

void upgrade_erase(uint32_t base_addr){
    core_util_critical_section_enter();

	flash_erase_sector(&flash, base_addr);
	flash_erase_sector(&flash, base_addr + 0x20000);

    core_util_critical_section_exit();
}

int upgrade_flash_copy(uint32_t dest, const uint8_t * src, uint32_t size){
    int result = 0;
    core_util_critical_section_enter();
	result = flash_program_page(&flash, dest, src, size);
    core_util_critical_section_exit();

    return result;
}

void upgrade_commit(){
	if(BL_CUR_VERSION == VERSION_B){
		BL_REQ_UPDATE = UP_VERSION_A;
	} else if(BL_CUR_VERSION == VERSION_A){
		BL_REQ_UPDATE = UP_VERSION_B;
	} else {
		BL_REQ_UPDATE = UP_VERSION_B;
	}

	//Bootloader will upgrade when next reset.
	//NVIC_SystemReset();
}

#if ENABLE_CRC32_CHECKSUM
extern uint32_t make_crc(uint32_t crc, unsigned char *string, uint32_t size);
#endif //ENABLE_CRC32_CHECKSUM
int upgrade_check_hash_code(unsigned int size, unsigned int hashcode)
{
#if ENABLE_CRC32_CHECKSUM
    {
        uint32_t crc32 = 0xFFFFFFFF;
        crc32 = make_crc(crc32, (unsigned char*)upgrade_get_base_addr(), size);
        crc32 ^= 0xFFFFFFFF;
        if(crc32 != (uint32_t)hashcode)
        {
            FTRACE("###upgrade_check_hash_code() hashcode[0x%08X] check failed; [0x%08X]\r\n", hashcode, crc32);
            return -1;
        }
    }
#endif //ENABLE_CRC32_CHECKSUM

    return 0;
}

