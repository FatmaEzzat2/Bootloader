/*
 * BL.h
 *
 *  Created on: Jun 22, 2024
 *      Author: KimoStore
 */

#ifndef INC_BL_H_
#define INC_BL_H_
#include<stdint.h>

#define BL_GET_VERSION     		 0X51
#define BL_GET_HELP	        	 0X52
#define BL_GET_CHIP_ID      	 0X53
#define BL_GET_RDP_STATUS    	 0X54
#define BL_GOTO_ADDRESS      	 0X55
#define BL_FLASH_ERASE       	 0X56
#define BL_MEMEORY_WRITE         0X57
#define BL_EN_RW_PROTECT      	 0X58
#define BL_MEMEORY_READ      	 0X59
#define BL_READ_SECTOR_STATUS    0X5A
#define BL_OTP_READ				 0X5B
#define BL_DIS_RW_PROTECT        0X5C

/* Bootloader reply*/
#define NACKBYTE    0x7F
#define ACKBYTE		0xA5
/**
 * @brief  Handles the Get Version command received via UART.
 * @param  commandPacket: Array of bytes representing the command packet received.
 * @retval None
 */
void BL_voidHandleGetVersionCmd(uint8_t commandPacket[]);
/**
 * @brief  Handles the Get Help command received via UART.
 * @param  commandPacket: Array of bytes representing the command packet received.
 * @retval None
 */
void BL_voidHandleGetHelpCmd(uint8_t commandPacket[]);
/**
 * @brief  Handles the Get Chip ID command received via UART.
 * @param  commandPacket: Array of bytes representing the command packet received.
 * @retval None
 */
void BL_voidHandleGetChipIdCmd(uint8_t commandPacket[]);
/**
 * @brief  Handles the Read Protection (RDP) Status command received via UART.
 * @param  commandPacket: Array of bytes representing the command packet received.
 * @retval None
 */
void BL_voidHandleRDPStatusCmd(uint8_t commandPacket[]);
/**
 * @brief  Handles the Go To Address command received via UART.
 * @param  commandPacket: Array of bytes representing the command packet received.
 * @retval None
 */
void BL_voidHandleGoToAddressCmd(uint8_t commandPacket[]);
/**
 * @brief  Handles the Flash Erase command received via UART.
 *         This function processes the command packet to extract the target
 *         sector(s) to erase, performs the flash erase operation, and sends
 *         a response back via UART.
 * @param  commandPacket: Array of bytes representing the command packet received.
 *         The command packet should include information about which sector(s) to erase.
 * @retval None
 */
void BL_voidHandleFlashEraseCmd(uint8_t commandPacket[]);
/**
 * @brief  Handles the Memory Write command received via UART.
 *         This function processes the command packet to extract the target
 *         address and data to write to flash memory, performs the flash write
 *         operation, and sends a response back via UART.
 * @param  commandPacket: Array of bytes representing the command packet received.
 *         The command packet should include the target address and data to write.
 * @retval None
 */
void BL_voidHandleMemeoryWriteCmd(uint8_t commandPacket[]);





#endif /* INC_BL_H_ */
