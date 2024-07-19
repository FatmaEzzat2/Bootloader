/*
 * BL.c
 *
 *  Created on: Jun 22, 2024
 *      Author: KimoStore
 */

#include "../Inc/BL.h"
#include "../Inc/BL_Private.h"
#include "main.h"



extern CRC_HandleTypeDef hcrc;
extern UART_HandleTypeDef huart2;




/***************************************************/
/*************Static Functions**********************/
/***************************************************/
/**
 * @brief  Verifies CRC of a data array against a provided CRC value.
 * @param  DataArr: Pointer to the array of data bytes.
 * @param  size: Size of the data array.
 * @param  HostCRC: CRC value to verify against.
 * @retval 1 if CRC matches, 0 if CRC does not match.
 */
static uint8_t u8VerifyCRC(uint8_t DataArr[],uint8_t size, uint32_t HostCRC)
{
	uint8_t CRCStatus=CRC_FAIL;
	uint32_t CRCAcculate=0,temp;

	for(int iteration=0;iteration<size;iteration++){
		temp= DataArr[iteration];
		CRCAcculate=HAL_CRC_Accumulate(&hcrc,&temp,1);
	}

	/*Reset CRC Calculation unit*/
	__HAL_CRC_DR_RESET(&hcrc);

	if(CRCAcculate==HostCRC){
		CRCStatus=CRC_SUCCESS;
	}
	return CRCStatus;
}
/****************************************************************************************************************/
/**
 * @brief  Sends an acknowledgment response with specified length to follow.
 * @param  lengthToFollow: Length of data or additional information to follow in the response.
 * @retval None
 */
static void voidSendAck(uint8_t lengthToFollow){
	//First Byte is fixed and the 2nd byte represents length of the reply
	uint8_t responsePacket[2]={ACKBYTE,lengthToFollow};

	//then, send a response packet
	 HAL_UART_Transmit(&huart2,(uint8_t*)responsePacket,2,HAL_MAX_DELAY);
}
/****************************************************************************************************************/
/*@brief  Sends a negative acknowledgment (NAck) response.
* @param  None
* @retval None
*/
static void voidSendNAck(void){

	 uint8_t NACKValue= NACKBYTE;
	 HAL_UART_Transmit(&huart2,&NACKValue,1,HAL_MAX_DELAY);
}
/****************************************************************************************************************/
static uint8_t u8ValidateAddress(uint32_t address){

	uint8_t validAdd= INVALID_ADD;

		if((address>=FLASH_BASE)&&(address<=FLASH_END)){
			validAdd=VALID_ADD;
		}else if((address>=SRAM1_BASE)&&(address<=(SRAM1_BASE+(1024*128)))){
			validAdd=VALID_ADD;
		}
	return validAdd;
}
/****************************************************************************************************************/
static uint8_t u8ExecuteFlashErase(uint8_t sectorNumber,uint8_t NumberOfSectors){
	HAL_StatusTypeDef ErrorStatus=HAL_OK;

	/*check on the inputs validation*/
	if((sectorNumber>=8)&&(sectorNumber!=MASS_ERASE)){

		ErrorStatus =HAL_ERROR;

	}else if((NumberOfSectors>8)&&(sectorNumber!=MASS_ERASE)){
		ErrorStatus =HAL_ERROR;

	}else{
		/*Implementation*/

		FLASH_EraseInitTypeDef flash;
		uint32_t sectorError ;

		if(sectorNumber==MASS_ERASE){
			flash.TypeErase= FLASH_TYPEERASE_MASSERASE;
		}else{
			 uint8_t remainSectors=8-sectorNumber;

			 if(remainSectors<NumberOfSectors){
				 NumberOfSectors = remainSectors;
			 }else{
				 /*Do nothing*/
			 }
			flash.TypeErase = FLASH_TYPEERASE_SECTORS ;
			flash.NbSectors = NumberOfSectors;
			flash.Sector    = sectorNumber;
		}

		flash.VoltageRange =FLASH_VOLTAGE_RANGE_3;
		flash.Banks =FLASH_BANK_1;

		//unlock Registers
		HAL_FLASH_Unlock();

		ErrorStatus=HAL_FLASHEx_Erase(&flash,&sectorError);

		//Lock the flash again
		HAL_FLASH_Lock();

	}
	return ErrorStatus;
}
/****************************************************************************************************************/
static uint8_t u8ExecuteMemoryWrite(uint8_t data[],uint8_t dataLength,uint32_t baseAddress){
	uint8_t ErrorStatus;
	//Step1: Check if this address in the SRAM range or the flash range
	//In the flash range
	if((baseAddress>=FLASH_BASE)&&(baseAddress<=FLASH_END)){

		for(uint8_t iteration=0;iteration<dataLength;iteration++){

			//Step2: Unlock the flash register
			HAL_FLASH_Unlock();
			ErrorStatus=HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, baseAddress+iteration, data[iteration]);

			//Step3: Lock the flash again
			HAL_FLASH_Lock();

		}
	//In the SRAM range
	}else{
		uint8_t* ptrDest= (uint8_t*)baseAddress;
		for(uint8_t iteration=0;iteration<dataLength;iteration++){
			ptrDest[iteration]= data[iteration];

		}
	}

	return ErrorStatus;
}
/****************************************************************************************************************/
/***********************************************************/
/****************Function Implementation********************/
/***********************************************************/


void BL_voidHandleGetVersionCmd(uint8_t commandPacket[]){
	uint8_t packetLength =commandPacket[0]+1 ;
	uint8_t CRCStatus;
	uint8_t BLVersion=BL_VERSION;

	uint32_t HostCRC = *((uint32_t*)(commandPacket+packetLength-4));

	//Step1: Verify CRC
	CRCStatus = u8VerifyCRC(commandPacket,packetLength-4,HostCRC);

	if(CRCStatus==CRC_SUCCESS){
		//Firstly, Send the response Packet
		voidSendAck(1u); //Bootloader version has size of 1byte

		//Secondly, Send reply packet in which there is the bootloader version
		 HAL_UART_Transmit(&huart2,&BLVersion,1,HAL_MAX_DELAY);

	}else{
		voidSendNAck();
	}

}
/****************************************************************************************************************/
void BL_voidHandleGetHelpCmd(uint8_t commandPacket[]){
	uint8_t packetLength =commandPacket[0]+1 ;
	uint8_t CRCStatus;
	uint8_t arrOfCommands[]={
						 BL_GET_VERSION,
						 BL_GET_HELP,
						 BL_GET_CHIP_ID,
						 BL_GET_RDP_STATUS,
						 BL_GOTO_ADDRESS,
						 BL_FLASH_ERASE,
						 BL_MEMEORY_WRITE,
						 BL_EN_RW_PROTECT,
						 BL_MEMEORY_READ,
						 BL_READ_SECTOR_STATUS,
						 BL_OTP_READ,
						 BL_DIS_RW_PROTECT
	  };
	uint32_t HostCRC = *((uint32_t*)(commandPacket+packetLength-4));

	//Step1: Verify CRC
	CRCStatus = u8VerifyCRC(commandPacket,packetLength-4,HostCRC);

	if(CRCStatus==CRC_SUCCESS){
		//Firstly, Send the response Packet
		//The supported commands have size of 10 bytes

		voidSendAck(sizeof(arrOfCommands));
		//Secondly, Send reply packet in which there are the supported commands
		HAL_UART_Transmit(&huart2,arrOfCommands,sizeof(arrOfCommands),HAL_MAX_DELAY);

	}else{
			voidSendNAck();
	}
}
/****************************************************************************************************************/

void BL_voidHandleGetChipIdCmd(uint8_t commandPacket[]){
	uint8_t packetLength =commandPacket[0]+1 ;
    uint8_t CRCStatus;
    uint16_t ChipID =(CHIP_ID_REG_ADD & 0x0fff);
    uint32_t HostCRC = *((uint32_t*)(commandPacket+packetLength-4));

    //Step1: Verify CRC
    CRCStatus = u8VerifyCRC(commandPacket,packetLength-4,HostCRC);

    if(CRCStatus==CRC_SUCCESS){
    	//Firstly, Send the response Packet
    	voidSendAck(2u); //The chip id has size of 1byte

    	HAL_UART_Transmit(&huart2,(uint8_t*)&ChipID,2,HAL_MAX_DELAY);


    }else{
    		voidSendNAck();
    }

}
/****************************************************************************************************************/

void BL_voidHandleRDPStatusCmd(uint8_t commandPacket[]){
	uint8_t packetLength =commandPacket[0]+1 ;
	uint8_t CRCStatus;

	uint32_t HostCRC = *((uint32_t*)(commandPacket+packetLength-4));

	//Step1: Verify CRC
	CRCStatus = u8VerifyCRC(commandPacket,packetLength-4,HostCRC);

	if(CRCStatus==CRC_SUCCESS){
		//Firstly, Send the response Packet
		voidSendAck(1u);

		uint8_t RDPStatus=(uint8_t)((RDP_USER_OPTION_WORD>>8) & 0xff);

		HAL_UART_Transmit(&huart2,&RDPStatus,1,HAL_MAX_DELAY);

	}else{
		voidSendNAck();
	}

}
/****************************************************************************************************************/

void BL_voidHandleGoToAddressCmd(uint8_t commandPacket[]){
	uint8_t packetLength =commandPacket[0]+1 ;
	uint8_t CRCStatus;

	uint32_t HostCRC = *((uint32_t*)(commandPacket+packetLength-4));

	//Step1: Verify CRC
	CRCStatus = u8VerifyCRC(commandPacket,packetLength-4,HostCRC);

	if(CRCStatus==CRC_SUCCESS){
		//Firstly, Send the response Packet
		voidSendAck(1u);

		//Extract The address from the packet
		uint32_t address=*((uint32_t*)&commandPacket[2]);

		//Check on the address
		uint8_t valid_address=u8ValidateAddress(address);

		if(valid_address==VALID_ADD){

			//Send the replay packet
			HAL_UART_Transmit(&huart2,&valid_address,1,HAL_MAX_DELAY);

			//Jump on this address
			void(*ptr)(void)=NULL;

			//Increment the address by 1 to make T bit = 1
			address|=0x1;
			ptr = (void*)address;
			ptr();

		}
		else{
			//Send the replay packet
			HAL_UART_Transmit(&huart2,&valid_address,1,HAL_MAX_DELAY);
		}
	}else{
			voidSendNAck();
	}

}
/****************************************************************************************************************/

void BL_voidHandleFlashEraseCmd(uint8_t commandPacket[]){
	uint8_t packetLength =commandPacket[0]+1 ;
	uint8_t CRCStatus;
	uint8_t eraseStatus;
	uint32_t HostCRC = *((uint32_t*)(commandPacket+packetLength-4));

	//Step1: Verify CRC
	CRCStatus = u8VerifyCRC(commandPacket,packetLength-4,HostCRC);

	if(CRCStatus==CRC_SUCCESS){
		//Firstly, Send the response Packet
		voidSendAck(1u);
		eraseStatus =u8ExecuteFlashErase(commandPacket[2],commandPacket[3]);

		HAL_UART_Transmit(&huart2,&eraseStatus,1,HAL_MAX_DELAY);

	}else{
			voidSendNAck();
	}
}
/****************************************************************************************************************/

void BL_voidHandleMemeoryWriteCmd(uint8_t commandPacket[]){
	uint8_t packetLength =commandPacket[0]+1 ;
	uint8_t CRCStatus;
	uint8_t writtingStatus;
	uint8_t addressStatus;
	uint8_t dataLength;

	uint32_t HostCRC = *((uint32_t*)(commandPacket+packetLength-4));

	//Step1: Verify CRC
	CRCStatus = u8VerifyCRC(commandPacket,packetLength-4,HostCRC);

	if(CRCStatus==CRC_SUCCESS){
		//Firstly, Send the response Packet
		voidSendAck(1u);

		//Step1:Extract the base address
		uint32_t baseAddress=*((uint32_t*)(&commandPacket[2]));

		//Step2:Check on the address
		addressStatus =u8ValidateAddress(baseAddress);

		if(addressStatus==VALID_ADD){

			//Step3:Extract the data length
			dataLength=commandPacket[6];
			writtingStatus= u8ExecuteMemoryWrite(&commandPacket[7],dataLength,baseAddress);

		}else{
			//Wrong address
			writtingStatus = WRITING_FAIL;
		}
		HAL_UART_Transmit(&huart2,&writtingStatus,1,HAL_MAX_DELAY);

	}else{
			voidSendNAck();
	}

}
/****************************************************************************************************************/







