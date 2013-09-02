/* --COPYRIGHT--,BSD
 * Copyright (c) 2012, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * --/COPYRIGHT--*/
#include "tlv.h"
#include "../inc/hw_types.h"

#ifdef  __IAR_SYSTEMS_ICC__
#include "../deprecated/IAR/msp430xgeneric.h"
#else
#include "../deprecated/CCS/msp430xgeneric.h"
#endif

#include "../inc/sfr_sys_baseAddress.h"
#include "debug.h"

//******************************************************************************
//
//! The TLV structure uses a tag or base address to identify segments of the 
//! table where information is stored. Some examples of TLV tags are Peripheral 
//! Descriptor, Interrupts, Info Block and Die Record. This function retrieves 
//! the value of a tag and the length of the tag.
//!
//! \param tag represents the tag for which the information needs to be 
//! retrieved. Various tags such as ADC (TLV_TAG_ADCCAL), Peripheral Descriptor 
//! (TLV_TAG_PDTAG) are defined in the tlv.h file.
//! \param instance - In some cases a specific tag may have more than one 
//! instance. For example there may be multiple instances of timer calibration 
//! data present under a single Timer Cal tag. This variable specifies the 
//! instance for which information is to be retrieved (0, 1, etc.). When 
//! only one instance exists; 0 is passed.
//! \param *length acts as a return through indirect reference. The function 
//! retrieves the value of the TLV tag length. This value is pointed to by 
//! *length and can be used by the application level once the function is 
//! called. If the specified tag is not found then the pointer is null 0.
//! \param **data_address acts as a return through indirect reference. Once the 
//! function is called data_address points to the pointer that holds the values
//! retrieved from the specified TLV tag. If the specified tag is not found 
//! then the pointer is null 0.
//!
//! \returns None
//
//******************************************************************************
void TLV_getInfo(unsigned char tag, 
                 unsigned char instance, 
                 unsigned char *length, 
                 unsigned int **data_address
                 )
{
  // TLV Structure Start Address
  char *TLV_address = (char *)TLV_START;         

  while((TLV_address < (char *)TLV_END)
        && ((*TLV_address != tag) || instance)   // check for tag and instance
        && (*TLV_address != TLV_TAGEND))         // do range check first
  {
    if (*TLV_address == tag) 
    {
      // repeat till requested instance is reached
      instance--;         
    }
    // add (Current TAG address + LENGTH) + 2
    TLV_address += *(TLV_address + 1) + 2;       
  }
  
  // Check if Tag match happened..
  if (*TLV_address == tag)                       
  {
    // Return length = Address + 1
    *length = *(TLV_address + 1);                  
    // Return address of first data/value info = Address + 2
    *data_address = (unsigned int *)(TLV_address + 2); 
  }
  // If there was no tag match and the end of TLV structure was reached..
  else                                           
  {
    // Return 0 for TAG not found
    *length = 0;             
    // Return 0 for TAG not found
    *data_address = 0;                           
  }
}


//******************************************************************************
//
//! Retrieves the unique device ID from the TLV structure.
//!
//! \param None
//!
//! \returns The device ID is returned as type unsigned int.
//
//******************************************************************************
unsigned int TLV_getDeviceType()
{
  unsigned int *pDeviceType = (unsigned int *)DEVICE_ID_0;
  // Return Value from TLV Table
  return pDeviceType[0];                         
}

//******************************************************************************
//
//! The Peripheral Descriptor tag is split into two portions � a list of the 
//! available flash memory blocks followed by a list of available peripherals. 
//! This function is used to parse through the first portion and calculate the 
//! total flash memory available in a device. The typical usage is to call the 
//! TLV_getMemory which returns a non-zero value until the entire memory list 
//! has been parsed. When a zero is returned, it indicates that all the memory 
//! blocks have been counted and the next address holds the beginning of the
//! device peripheral list.
//!
//! \param instance In some cases a specific tag may have more than one 
//! instance. This variable specifies the instance for which information is to 
//! be retrieved (0, 1 etc). When only one instance exists; 0 is passed.
//!
//! \returns The returned value is zero if the end of the memory list is reached.
//
//******************************************************************************
unsigned int TLV_getMemory(unsigned char instance)
{
    unsigned char *pPDTAG;
    unsigned char bPDTAG_bytes;
    unsigned int count;

    // set tag for word access comparison
    instance *= 2;                               
    
    // TLV access Function Call
    // Get Peripheral data pointer
    TLV_getInfo(TLV_PDTAG, 
                0, 
                &bPDTAG_bytes, 
                (unsigned int **)&pPDTAG
                ); 
    
    for (count = 0;count <= instance; count += 2)
    {
      if (pPDTAG[count] == 0) 
      {
         // Return 0 if end reached
        return 0;         
      }
      if (count == instance) 
        return (pPDTAG[count] | pPDTAG[count+1]<<8);
    }
    
    // Return 0: not found
    return 0;                                    
}


//******************************************************************************
//
//! The Peripheral Descriptor tag is split into two portions � a list of the 
//! available flash memory blocks followed by a list of available peripherals. 
//! This function is used to parse through the second portion and can be used to 
//! check if a specific peripheral is present in a device. The function calls 
//! TLV_getPeripheral() recursively until the end of the memory list and 
//! consequently the beginning of the peripheral list is reached.
//!
//! \param tag represents represents the tag for a specific peripheral for which 
//! the information needs to be retrieved. In the header file tlv.h 
//! specific peripheral tags are pre-defined, for example USCIA_B and TA0 are 
//! defined as TLV_PID_USCI_AB and TLV_PID_TA2 respectively.
//! \param instance - In some cases a specific tag may have more than one 
//! instance. For example a device may have more than a single USCI module, 
//! each of which is defined by an instance number 0, 1, 2, etc. When only one 
//! instance exists; 0 is passed.
//!
//! \returns The returned value is zero if the specified tag value (peripheral) 
//! is not available in the device.
//
//******************************************************************************
unsigned int TLV_getPeripheral(unsigned char tag, 
                               unsigned char instance
                              )
{
    unsigned char *pPDTAG;
    unsigned char bPDTAG_bytes;
    unsigned int count = 0;
    unsigned int pcount = 0;

    // Get Peripheral data pointer
    TLV_getInfo(TLV_PDTAG, 
                0, 
                &bPDTAG_bytes, 
                (unsigned int **)&pPDTAG
                ); 

    // read memory configuration from TLV to get offset for Peripherals
    while (TLV_getMemory(count))
    {
      count++;
    }
    // get number of Peripheral entries
    pcount = pPDTAG[count * 2 + 1];              
    // inc count to first Periperal
    count++;                                     
    // adjust point to first address of Peripheral
    pPDTAG += count*2;                           
    // set counter back to 0
    count = 0;                                   
    // align pcount for work comparision
    pcount *= 2;                                 

    // TLV access Function Call
    for (count = 0; count <= pcount; count += 2)
    {
      if (pPDTAG[count+1] == tag) 
      { 
        // test if required Peripheral is found
        if (instance > 0) 
        {
          // test if required instance is found
          instance--;
        }
        else
        {
          // Return found data
          return (pPDTAG[count] | pPDTAG[count + 1] << 8); 
        }
      }
    }
    
    // Return 0: not found
    return 0;                                    
}

//******************************************************************************
//
//! This function is used to retrieve information on available interrupt 
//! vectors. It allows the user to check if a specific interrupt vector is 
//! defined in a given device.
//!
//! \param tag represents the tag for the interrupt vector. Interrupt vector 
//! tags number from 0 to N depending on the number of available interrupts. 
//! Refer to the device datasheet for a list of available interrupts.
//!
//! \returns The returned value is zero is the specified interrupt vector is 
//! not defined.
//
//******************************************************************************
unsigned char TLV_getInterrupt(unsigned char tag)
{
    unsigned char *pPDTAG;
    unsigned char bPDTAG_bytes;
    unsigned int count = 0;
    unsigned int pcount = 0;

    // Get Peripheral data pointer
    TLV_getInfo(TLV_PDTAG, 
                0, 
                &bPDTAG_bytes, 
                (unsigned int **)&pPDTAG
                ); 
    
    // read memory configuration from TLV to get offset for Peripherals
    while (TLV_getMemory(count))
    {
      count++;
    }

    pcount = pPDTAG[count * 2 + 1];
    // inc count to first Periperal
    count++;                                     
    // adjust point to first address of Peripheral
    pPDTAG += (pcount + count) * 2;              
    // set counter back to 0
    count = 0;                                   

    // TLV access Function Call
    for (count = 0; count <= tag; count += 2)
    {
      if (pPDTAG[count] == 0) 
      {
        // Return 0: not found/end of table
        return 0;          
      }
      if (count == tag) 
      {
        // Return found data
        return (pPDTAG[count]);  
      }
    }
    
    // Return 0: not found
    return 0;                                    
}
//******************************************************************************
//
//Close the Doxygen group.
//! @}
//
//******************************************************************************
