//
// Copyright (c) 2017 The nanoFramework project contributors
// Portions Copyright (c) 2016 STMicroelectronics.  All rights reserved.
// See LICENSE file in the project root for full license information.
//

///////////////////////////////////////////////////////////////////////////////
// This driver has been tested with the following STM32 series: F4
///////////////////////////////////////////////////////////////////////////////

#include "hal.h"

#if (HAL_USE_STM32_FLASH == TRUE)

///////////////////////////////////////////////////////////////////////////////
// Driver local definitions.                                                 //
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Driver exported variables.                                                //
///////////////////////////////////////////////////////////////////////////////
SMT32FlashDriver STM32FLASH;

///////////////////////////////////////////////////////////////////////////////
// Driver local variables and types.                                         //
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Driver local functions.                                                   //
///////////////////////////////////////////////////////////////////////////////

// Unlock the FLASH control register access
bool HAL_FLASH_Unlock(void)
{
  if((FLASH->CR & FLASH_CR_LOCK) != RESET)
  {
    // Authorize the FLASH Registers access
    FLASH->KEYR = FLASH_KEY1;
    FLASH->KEYR = FLASH_KEY2;
  }
  else
  {
    return false;
  }

  return true; 
}

// Locks the FLASH control register access
void HAL_FLASH_Lock(void)
{
  // Set the LOCK Bit to lock the FLASH Registers access
  FLASH->CR |= FLASH_CR_LOCK;
}

bool FLASH_WaitForLastOperation()
{ 
    // Wait for the FLASH operation to complete by polling on BUSY flag to be reset.
    //  Even if the FLASH operation fails, the BUSY flag will be reset and an error
    //  flag will be set

    while(__HAL_FLASH_GET_FLAG(FLASH_FLAG_BSY) != RESET) 
    { 
        // TODO: add a timeout here using an OS function
        __NOP();
    }

    // If there is no error flag set
    return true;
}  

#if defined(STM32F427xx) || defined(STM32F437xx) || defined(STM32F429xx)|| defined(STM32F439xx) ||\
    defined(STM32F469xx) || defined(STM32F479xx) ||\
    defined(STM32F405xx) || defined(STM32F415xx) || defined(STM32F407xx) || defined(STM32F417xx) || defined(STM32F412Zx) ||\
    defined(STM32F412Vx) || defined(STM32F412Rx) || defined(STM32F412Cx) ||\
    defined(STM32F401xC) ||\
    defined(STM32F410Tx) || defined(STM32F410Cx) || defined(STM32F410Rx) ||\
    defined(STM32F401xE) || defined(STM32F411xE) || defined(STM32F446xx)

void FLASH_FlushCaches(void)
{
  // Flush instruction cache 
  if(READ_BIT(FLASH->ACR, FLASH_ACR_ICEN)!= RESET)
  {
    // Disable instruction cache
    __HAL_FLASH_INSTRUCTION_CACHE_DISABLE();
    // Reset instruction cache
    __HAL_FLASH_INSTRUCTION_CACHE_RESET();
    // Enable instruction cache
    __HAL_FLASH_INSTRUCTION_CACHE_ENABLE();
  }
  
  // Flush data cache
  if(READ_BIT(FLASH->ACR, FLASH_ACR_DCEN) != RESET)
  {
    // Disable data cache 
    __HAL_FLASH_DATA_CACHE_DISABLE();
    // Reset data cache
    __HAL_FLASH_DATA_CACHE_RESET();
    // Enable data cache
    __HAL_FLASH_DATA_CACHE_ENABLE();
  }
}
#endif // define STM32F4xxxx

///////////////////////////////////////////////////////////////////////////////
// Driver exported functions.                                                //
///////////////////////////////////////////////////////////////////////////////

void flash_lld_init() {
    stm32FlashObjectInit(&STM32FLASH);
}

void flash_lld_readBytes(uint32_t startAddress, uint32_t length, uint8_t* buffer) {

    __IO uint8_t* cursor = (__IO uint8_t*)startAddress;
    __IO uint8_t* endAddress = (__IO uint8_t*)(startAddress + length);

    // copy contents from flash to buffer starting from the start address 
    while(cursor < endAddress)
    {
        *buffer++ = *cursor++;
    }
}

bool flash_lld_write(uint32_t startAddress, uint32_t length, const uint8_t* buffer) {

    __IO uint8_t* cursor = (__IO uint8_t*)startAddress;
    __IO uint8_t* endAddress = (__IO uint8_t*)(startAddress + length);

    // unlock the FLASH
    if(HAL_FLASH_Unlock())
    {
        // Clear pending flags (if any)
        __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);

        // clear the program size mask
        CLEAR_BIT(FLASH->CR, CR_PSIZE_MASK);

        // proceed to program the flash by setting the PG Bit
        SET_BIT(FLASH->CR, FLASH_CR_PG);

        while(cursor < endAddress)
        {
            // if buffer has enough data, program half-words (16 bits) in a single operation to speed up things
            // NOTE: assuming that the supply voltage is able to cope with half-word programming
            if((endAddress - cursor) >= 2)
            {
                // set the size of of the programming word to HALF WORD
                SET_BIT(FLASH->CR, FLASH_PSIZE_HALF_WORD);

                *((__IO uint16_t*)cursor++) = *((uint16_t*)buffer++);

                // update flash and buffer pointers by the 'extra' byte that was programmed
                cursor++;
                buffer++;
            }
            else
            {
                // program single byte

                // clear the program size mask
                CLEAR_BIT(FLASH->CR, CR_PSIZE_MASK);
                // set the size of of the programming word to BYTE
                SET_BIT(FLASH->CR, FLASH_PSIZE_BYTE);

                *cursor = *buffer++;
            }
                
            // wait for any flash operation to be completed 
            if(!FLASH_WaitForLastOperation())
            {
                // lock the FLASH
                HAL_FLASH_Lock();

                return false;
            }
        }

        // after the program operation is completed disable the PG Bit
        CLEAR_BIT(FLASH->CR, FLASH_CR_PG);
                
        // lock the FLASH
        HAL_FLASH_Lock();

        // check for errors
        if(__HAL_FLASH_GET_FLAG(FLASH_FLAG_ALL_ERRORS) != RESET)
        {
            return false;
        }

        // done here
        return true;
    }

    // default to false
    return false;
}

bool flash_lld_isErased(uint32_t startAddress, uint32_t length) {

    __IO uint32_t* cursor = (__IO uint32_t*)startAddress;
    __IO uint32_t* endAddress = (__IO uint32_t*)(startAddress + length);

    // an erased flash address has to read FLASH_ERASED_WORD
    // OK to check by word (32 bits) because the erase is performed by 'page' whose size is word multiple
    while(cursor < endAddress)
    {
        if(*cursor++ != FLASH_ERASED_WORD)
        {
            // found an address with something other than FLASH_ERASED_WORD!!
            return false;
        }
    }
    
    // reached here so the segment must be erased
    return true;
}

uint8_t flash_lld_getSector(uint32_t address)
{
  uint8_t sector = 0;
  bool sectorInBank2 = false;

  if ((address - FLASH_BASE) >= 0x100000) {
    sectorInBank2 = true;
  } 

  // clever algorithm to find out the sector number knowing the address
  sector = sectorInBank2 ? ((address - 0x100000 - FLASH_BASE) >> 14) : ((address - FLASH_BASE) >> 14);
  
  if (sector >= 4) {
    sector = (sector >> 3) + 4;
  }

  return sector;
}

bool flash_lld_erase(uint32_t address) {

    // unlock the FLASH
    if(HAL_FLASH_Unlock())
    {
        // Clear pending flags (if any)  
        __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);
        
        // get the sector number to erase
        uint8_t sectorNumber = flash_lld_getSector(address);

        // Need to add offset of 4 when sector higher than FLASH_SECTOR_11
        if(sectorNumber > FLASH_SECTOR_11) 
        {
            sectorNumber += 4U;
        }

        // If the previous operation is completed, proceed to erase the sector
        CLEAR_BIT(FLASH->CR, FLASH_CR_PSIZE);
        SET_BIT(FLASH->CR, FLASH_PSIZE_HALF_WORD);
        CLEAR_BIT(FLASH->CR, FLASH_CR_SNB);
        FLASH->CR |= FLASH_CR_SER | (sectorNumber << POSITION_VAL(FLASH_CR_SNB));
        
        // start erase operation
        FLASH->CR |= FLASH_CR_STRT;
        
        if(FLASH_WaitForLastOperation())
        {
            // lock the FLASH anyways
            HAL_FLASH_Lock();

            return false;
        }

        // after erase operation completed disable the SER and SNB Bits
        CLEAR_BIT(FLASH->CR, (FLASH_CR_SER | FLASH_CR_SNB));

#if defined(STM32F427xx) || defined(STM32F437xx) || defined(STM32F429xx)|| defined(STM32F439xx) ||\
    defined(STM32F469xx) || defined(STM32F479xx) ||\
    defined(STM32F405xx) || defined(STM32F415xx) || defined(STM32F407xx) || defined(STM32F417xx) || defined(STM32F412Zx) ||\
    defined(STM32F412Vx) || defined(STM32F412Rx) || defined(STM32F412Cx) ||\
    defined(STM32F401xC) ||\
    defined(STM32F410Tx) || defined(STM32F410Cx) || defined(STM32F410Rx) ||\
    defined(STM32F401xE) || defined(STM32F411xE) || defined(STM32F446xx)

        // Flush the caches to be sure of the data consistency
        FLASH_FlushCaches();
#endif

        // lock the FLASH
        HAL_FLASH_Lock();

        // check for errors
        if(__HAL_FLASH_GET_FLAG(FLASH_FLAG_ALL_ERRORS) != RESET)
        {
            return false;
        }

        // done here
        return true;
    }

    // default to false
    return false;
}

#endif
