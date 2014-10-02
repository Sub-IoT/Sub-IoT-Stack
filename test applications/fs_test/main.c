#include <d7aoss.h>
#include <msp430.h>

//#include "ff.h"

static uint8_t buffer[128];


// Function prototypes
void  write_SegA (char value);
void  copy_A2B (void);

uint8_t  value;                                // 8-bit value to write to segment A

int main(void) {

	system_init(buffer, 128, buffer, 128);
	
	//FCTL2 = FWKEY + FSSEL0 + FN0;             // MCLK/2 for Flash Timing Generator
	value = 0;                                // Initialize value

	while(1)                                  // Repeat forever
	{
		write_SegA(value++);                    // Write segment A, increment value
		copy_A2B();                             // Copy segment A to B
		_NOP();                                 // SET BREAKPOINT HERE
	}

	//return 0;
}


void write_SegA (char value)
{
  char *Flash_ptr;                          // Flash pointer
  unsigned int i;

  Flash_ptr = (char *) 0x1080;              // Initialize Flash pointer
  FCTL1 = FWKEY + ERASE;                    // Set Erase bit
  FCTL3 = FWKEY;                            // Clear Lock bit
  *Flash_ptr = 0;                           // Dummy write to erase Flash segment

  FCTL1 = FWKEY + WRT;                      // Set WRT bit for write operation

  for (i=0; i<128; i++)
  {
    *Flash_ptr++ = value;                   // Write value to flash
  }

  FCTL1 = FWKEY;                            // Clear WRT bit
  FCTL3 = FWKEY + LOCK;                     // Set LOCK bit
}


void copy_A2B (void)
{
  char *Flash_ptrA;                         // Segment A pointer
  char *Flash_ptrB;                         // Segment B pointer
  unsigned int i;

  Flash_ptrA = (char *) 0x1080;             // Initialize Flash segment A pointer
  Flash_ptrB = (char *) 0x1000;             // Initialize Flash segment B pointer
  FCTL1 = FWKEY + ERASE;                    // Set Erase bit
  FCTL3 = FWKEY;                            // Clear Lock bit
  *Flash_ptrB = 0;                          // Dummy write to erase Flash segment B
  FCTL1 = FWKEY + WRT;                      // Set WRT bit for write operation

  for (i=0; i<128; i++)
  {
    *Flash_ptrB++ = *Flash_ptrA++;           // Copy value segment A to segment B
  }

  FCTL1 = FWKEY;                            // Clear WRT bit
  FCTL3 = FWKEY + LOCK;                     // Set LOCK bit
}
