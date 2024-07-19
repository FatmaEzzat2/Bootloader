/*
 * BL_Private.h
 *
 *  Created on: Jun 22, 2024
 *      Author: KimoStore
 */

#ifndef INC_BL_PRIVATE_H_
#define INC_BL_PRIVATE_H_


#define CRC_FAIL			1u
#define CRC_SUCCESS 	 	0u

#define INVALID_ADD			1u
#define VALID_ADD			0u

#define WRITING_SUCCESS		0u
#define WRITING_FAIL		1u

#define BL_VERSION			1u

#define CHIP_ID_REG_ADD				*((volatile uint32_t*)0xE0042000)
#define RDP_USER_OPTION_WORD 		*((volatile uint32_t*)0x1FFFC000)

#define MASS_ERASE			0xff

static uint8_t u8VerifyCRC(uint8_t DataArr[],uint8_t size, uint32_t HostCRC);
static void voidSendNAck(void);
static void voidSendAck(uint8_t lengthToFollow);
static uint8_t u8ValidateAddress(uint32_t address);
static uint8_t u8ExecuteFlashErase(uint8_t sectorNumber,uint8_t NumberOfSectors);
static uint8_t u8ExecuteMemoryWrite(uint8_t data[],uint8_t dataLength,uint32_t baseAddress);
#endif /* INC_BL_PRIVATE_H_ */
