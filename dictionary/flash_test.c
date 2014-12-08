/*
************************************************************************
 ECE 362 - Mini-Project C Source File - Fall 2014                    
***********************************************************************
	 	   			 		  			 		  		
 Team ID: < ? >

 Project Name: < ? >

 Team Members:

   - Team/Doc Leader: < ? >      Signature: ______________________
   
   - Software Leader: < ? >      Signature: ______________________

   - Interface Leader: < ? >     Signature: ______________________

   - Peripheral Leader: < ? >    Signature: ______________________


 Academic Honesty Statement:  In signing above, we hereby certify that we 
 are the individuals who created this HC(S)12 source file and that we have
 not copied the work of any other student (past or present) while completing 
 it. We understand that if we fail to honor this agreement, we will receive 
 a grade of ZERO and be subject to possible disciplinary action.

***********************************************************************

 The objective of this Mini-Project is to .... < ? >


***********************************************************************

 List of project-specific success criteria (functionality that will be
 demonstrated):

 1.

 2.

 3.

 4.

 5.

***********************************************************************

  Date code started: < ? >

  Update history (add an entry every time a significant change is made):

  Date: < ? >  Name: < ? >   Update: < ? >

  Date: < ? >  Name: < ? >   Update: < ? >

  Date: < ? >  Name: < ? >   Update: < ? >


***********************************************************************
*/

#include <hidef.h>      /* common defines and macros */
#include "derivative.h"      /* derivative-specific definitions */
#include <mc9s12c32.h>

#include "../course_includes/our_termio.h"

union Address{
  long addr;
  char bytes[4];
}; 

/* All functions after main should be initialized here */
void spi_flash_shiftout(char x);
char spi_flash_shiftin(void);
void delay(unsigned int x);
char spi_flash_read_addr(long addr);
void spi_flash_read_word(long addr, char* buffer);
void spi_flash_read_next_word(long addr, char* buffer);


/* Variable declarations */
   	   			 		  			 		       

/* Special ASCII characters */
#define CR 0x0D		// ASCII return 
#define LF 0x0A		// ASCII new line 

/* LCD COMMUNICATION BIT MASKS (note - different than previous labs) */
#define RS 0x10		// RS pin mask (PTT[4])
#define RW 0x20		// R/W pin mask (PTT[5])
#define LCDCLK 0x40	// LCD EN/CLK pin mask (PTT[6])

/* LCD INSTRUCTION CHARACTERS */
#define LCDON 0x0F	// LCD initialization command
#define LCDCLR 0x01	// LCD clear display command
#define TWOLINE 0x38	// LCD 2-line enable command
#define CURMOV 0xFE	// LCD cursor move instruction
#define LINE1 = 0x80	// LCD line 1 cursor position
#define LINE2 = 0xC0	// LCD line 2 cursor position

/*spi flash instruction characters*/
#define spi_flash_read 0x03

#define CS_PIN PTT_PTT3

#define dict_length 653339
	 	   		
/*	 	   		
***********************************************************************
 Initializations
***********************************************************************
*/

void  initializations(void) {

/* Set the PLL speed (bus clock = 24 MHz) */
  CLKSEL = CLKSEL & 0x80; //; disengage PLL from system
  PLLCTL = PLLCTL | 0x40; //; turn on PLL
  SYNR = 0x02;            //; set PLL multiplier
  REFDV = 0;              //; set PLL divider
  while (!(CRGFLG & 0x08)){  }
  CLKSEL = CLKSEL | 0x80; //; engage PLL

/* Disable watchdog timer (COPCTL register) */
  COPCTL = 0x40   ; //COP off; RTI and COP stopped in BDM-mode

/* Initialize asynchronous serial port (SCI) for 9600 baud, interrupts off initially */
  SCIBDH =  0x00; //set baud rate to 9600
  SCIBDL =  0x9C; //24,000,000 / 16 / 156 = 9600 (approx)  
  SCICR1 =  0x00; //$9C = 156
  SCICR2 =  0x0C; //initialize SCI for program-driven operation
  DDRB   =  0x10; //set PB4 for output mode
  PORTB  =  0x10; //assert DTR pin on COM port

/* Initialize peripherals */
  SPICR1 = 0x50;  //interrupts disabled, power on, master mode, active high clk,msb first
  SPICR2 = 0;     //disable bi-directional mode
  SPIBR = 0;      // No scale - set baud rate to 12 Mbs (sppr=0, spr=0)
             
/* Initialize interrupts */
  DDRT_DDRT3 = 1;//pt3 as output(/cs)	 
  PTT_PTT3 = 1;//initially /cs is negated    
	      
}

	 		  			 		  		
/*	 		  			 		  		
***********************************************************************
Main
***********************************************************************
*/
void main(void) {
	long i;
	char string[100];


	DisableInterrupts
	initializations();
	EnableInterrupts;
	//	delay(100);
	//	for(i=0;i<40;i++) {	  
	//    outchar(spi_flash_read_addr(i));
	//	}

//	spi_flash_read_next_word(0, string);
//
//	outstr(string);
	
	for(i = 0; i < dict_length; i++)
	{
		outchar(spi_flash_read_addr(i));
	}
	
	outstr("Done");
	
  for(;;) {
  
/* < start of your main loop > */ 
      

  
  } /* loop forever */
   
}   /* do not leave main */


/*
***********************************************************************                       
  SPI flash functions	 		  		
***********************************************************************
*/
char spi_flash_read_addr(long addr) 
{
  union Address a;
  char h;//high, middle, and low bytes
  char m;
  char l;
  a.addr = addr;
  h=a.bytes[1];
  m=a.bytes[2];
  l=a.bytes[3];
  CS_PIN = 0;//assert /cs to signal start of instruction
  spi_flash_shiftout(spi_flash_read);
  spi_flash_shiftout(h);//1st addr byte 
  spi_flash_shiftout(m);//2nd addr byte
  spi_flash_shiftout(l);//3rd addr byte
  asm{		// We need at least 5 cycles of delay here for reasons unknown. :(
	  nop
	  nop
	  nop
	  nop
	  nop
  }
  spi_flash_shiftin();//clear spif
  spi_flash_shiftout(0xFF);
  h =  spi_flash_shiftin();
  CS_PIN = 1;//negate /cs to signal end of instruction
  
  return h;
}

/*
 * Read a word terminated with a newline, string starting at addr.
 */
void spi_flash_read_word(long addr, char* buffer)
{
	char read;
	
	read = spi_flash_read_addr(addr);
	while(read != '\n')
	{
		*buffer = read;
		buffer++;
		
		addr++;
		read = spi_flash_read_addr(addr);
	}
	
	*buffer = '\0';
}

/*
 * Look for the next newline character, then read the word after it to buffer.
 */
void spi_flash_read_next_word(long addr, char* buffer)
{
	char read;
	
	read = spi_flash_read_addr(addr);
	while(read != '\n')
	{
		addr++;
		read = spi_flash_read_addr(addr);
	}
	
	spi_flash_read_word(addr + 1, buffer);
}

void spi_flash_shiftout(char x)
{
  while(SPISR_SPTEF != 1);//read the SPTEF bit, continue if bit is 1
  SPIDR = x;                 //write data to SPI data register
//  delay(5);
}

char spi_flash_shiftin() 
{
  while(SPISR_SPIF != 1);//wait for data to shift in
  return SPIDR;
}

void delay(unsigned int x) {
  int i;
  for(i=0;i<x;i++);
}


/*
***********************************************************************                       
 RTI interrupt service routine: RTI_ISR
************************************************************************
*/

interrupt 7 void RTI_ISR(void)
{
  	// clear RTI interrupt flagt 
  	CRGFLG = CRGFLG | 0x80; 
}

/*
***********************************************************************                       
  TIM interrupt service routine	  		
***********************************************************************
*/

interrupt 15 void TIM_ISR(void)
{
  	// clear TIM CH 7 interrupt flag 
 	TFLG1 = TFLG1 | 0x80; 
 

}

/*
***********************************************************************                       
  SCI interrupt service routine		 		  		
***********************************************************************
*/

interrupt 20 void SCI_ISR(void)
{
 


}