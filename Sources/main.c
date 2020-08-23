#include <hidef.h> /* for EnableInterrupts macro */
#include "derivative.h" /* include peripheral declarations */

#define VNkeyboard 22 /* Interrupt vector for Keyboard */ 
#define VNTimer 11

typedef	unsigned char	uint8;
typedef	unsigned short	uint16;
typedef	unsigned long	uint32;

typedef	char			int8;
typedef	short			int16;
typedef	long			int32;

// To Clear or Set single bits in a byte variable.
#define	b_SetBit(bit_ID, varID)		(varID |= (uint8)(1<<bit_ID))
#define	b_ClearBit(bit_ID, varID)	(varID &= ~(uint8)(1<<bit_ID))
#define	b_XorBit(bit_ID, varID)		(varID ^= (uint8)(1<<bit_ID))

// To Clear or Set single bits in a word(16-bit) variable.
#define	w_SetBit(bit_ID, varID)		(varID |= (uint16)(1<<bit_ID))
#define	w_ClearBit(bit_ID, varID)	(varID &= ~(uint16)(1<<bit_ID))
#define	w_XorBit(bit_ID, varID)		(varID ^= (uint16)(1<<bit_ID))

// To check single bit status in a given variable in ZERO PAGE
#define	b_CheckBit(varID, bit_ID)	((varID & (uint8)(1<<bit_ID))?1:0)
//#define	b_CheckBit(varID, bit_ID)	(varID & (muint8)(1<<bit_ID))
#define	w_CheckBit(varID, bit_ID) ((varID & (uint16)(1<<bit_ID))?1:0)

// To check single bit status in a given variable in ZERO PAGE
#define		DummyRead(varID)			{__asm lda varID;}

int led_no = 0;
int delay_step = 1;
int delay_current = 1;
void init(void);






//
//
//NOTE: This is a workaround i have done to simulate the doubling of timer.
//I use delay_step to know how many steps(seconds) the timer will take after we push and let the buttons.
//
//Delay_current counts which cycle we're at: if delay_current = 3 and delay_step = 16, we are at the 3rd second of the 16 seconds we will have the leds in the current state
//
//
//Initial plan was to either change prescaler or division in registers, or lower the bus speed, but no matter what I set the registers. bus speed does not change at all
//
//
//
//
//
//
//







void main(void) {
  EnableInterrupts;			//enable global interrupts
  /* include your code here */
  init();					//calling the method that configures the ports and registers
  

  while(1)
  	{
  		__RESET_WATCHDOG(); 
  	}
  /* please make sure that you never leave main */
}

interrupt VNTimer void TPM1_overflow()
{ 
	byte varTOF; 
	varTOF = TPM1SC_TOF; // clear TOF; first read and then write 0 to the flag
	TPM1SC_TOF = 0;
	
	if(delay_current < delay_step){
		delay_current += 1;
	}
	else{
		delay_current = 1;
		if( led_no == 7){
			led_no = 0;
		}
		else{
			led_no +=1;
		}
			
	}
	if(led_no == 0){
				PTFD = 0x0;
				PTFD_PTFD0=1;
			}
			if(led_no == 1){
				PTFD = 0x0;
				PTFD_PTFD1=1;
				}
			if(led_no == 2){
				PTFD = 0x0;
				PTFD_PTFD2=1;
				}
			if(led_no == 3){
				PTFD = 0x0;
				PTFD_PTFD3=1;
				}
			if(led_no == 4){
				PTFD = 0x0;
				PTFD_PTFD4=1;
				}
			if(led_no == 5){
				PTFD = 0x0;
				PTFD_PTFD5=1;
				}
			if(led_no == 6){
				PTFD = 0x0;
				PTFD_PTFD6=1;
				}
			if(led_no == 7){
				PTFD = 0x0;
				PTFD_PTFD7=1;
				}
}



interrupt VNkeyboard void intKBI_SW()
{
	
	
	if(delay_step == 128){
		delay_step = 1;
	}
	else{
		delay_step = delay_step*2;
	}
	
	KBI1SC_KBACK = 1; /*acknowledge interrupt*/
}




void init(){
	SOPT = 0x00;   			//disable watchdog
	
	
	
	ICGC2 = 0b00000000; // Set up ICG control register 2
	ICGC1 = 0X78;
	//THESE 2 DO NOT WORK FOR SOME REASON?(in the simulation)
	
	
	
	
	PTFDD = 0xFF;  			//set all port F pins as output
	PTFD = 0x0; 			//turn off all LEDs
	
							//For pushbutton switches we can use either PTC2 PTC6 PTD3 or PTD2
							// However only PTD3 and PTD2 can trigger interrupts.
							//we will set KBIP5 *port D2 pin*
	
	
	PTDDD = 0xA9; 			//set port D pin 2 as input
						// note: we could have set it as 0b11111011 as well
	
	PTDPE_PTDPE2 = 1; 		//enabled port D pin 2 pull ups

	KBIPE_KBIPE5 = 1; 		//enable KBPIE 5 pin
	
	KBI1SC_KBEDG5 = 0; 		//falling edge trigger  for the pin (pull up pin)
	
	KBI1SC = 0b11110010;	//bit7:4 = 0 => trigger on rising edge for all KBI ports
							//bit3 = 0   => no KBI interrupt pending
							//bit2 = 0   => write-only
							//bit1 = 1   => KBI generates hardware interrupt
							//bit0 = 0   => edge-only detection
						//note: I set bit0 as 0  -KBIMOD-  (in course we have set it as 1) to trigger interrupts only the moment 
						//i "click" and let go the button, not while having it not pressed at all.
	
	KBI1PE_KBIPE5 = 1;  //enable keyboard interrupt on the pin
	
	TPM1SC = 0b01001011;	//b7- 0 
								//b6 - 1
								//b5 - 0
								//b4:3 = 01 (select bus rate clock)
								//b2:0 = 011 (in the course example we used 111 to simulate 1 second(62500 counter overflow) 
								//            but in the simulator, same config results in aprox 16seconds simulated
								//			 So i decided to use a smaller divisor.(128/16 = 8 ->b2:0= 011) 
				
		
		//set overflow value to 62500
		TPM1MODH = 0xF4;
		TPM1MODL = 0x24;
	
}
