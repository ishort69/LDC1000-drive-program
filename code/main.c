
#include <msp430f5529.h>
#include "LDC1000_cmd.h"
   

void SetVCoreUp(unsigned int level);

char spi_readByte( char addr, char * data);
char spi_readWord(char addr, unsigned int * data);  // Big Endian
char spi_readBytes( char addr, char * buffer, unsigned char len);
char spi_writeByte(char addr, char data);
char spi_writeWord(char addr, unsigned int data);  // Big Endian
char spi_writeBytes( char addr, char * buffer, unsigned char len);
char orgVal[20]; 
static unsigned char txlen;
static unsigned char rxlen;
static char *txbuf;
static char *rxbuf;
static char txaddr;
static char wordbuf[2];
  
long int L,L1;
int proximtyData1[52];
int  frequencyData1[52];
float Sum_pro,Sum_pro1;
long int Sum_fre,Sum_fre1;
float FENZI;
float FENMU;
long int RP;
float  proximtyDataMIX;
long int  frequencyDataMIX;
float Y1,Y;
char statusData[1];
char proximtyData[2];
char frequencyData[3];

 unsigned int proximtyDataMIN;
long int frequencyDataMIN;

 unsigned int proximtyDataMAX;
long  int frequencyDataMAX;

 unsigned int proximtyDataTEMP;
long int frequencyDataTEMP;

#define RPMIN 0x34
#define RPMAX 0x0B

void UCS_INIT()
{
SetVCoreUp(1);
	SetVCoreUp(2);
	SetVCoreUp(3);

//	//UCS SETTING
    UCSCTL3 |= SELREF__REFOCLK;

    __bis_SR_register(SCG0);                  // Disable the FLL control loop
    UCSCTL0 = 0x0000;                         // Set lowest possible DCOx, MODx
    UCSCTL1 = DCORSEL_6;                      // Select DCO range 24MHz operation
	UCSCTL2 = FLLD_0 + 731;                   // Set DCO Multiplier for 24MHz
												// (N + 1) * FLLRef = Fdco
											   // (731 + 1) * 32768 = 24MHz
											   // Set FLL Div = fDCOCLK/2
    __bic_SR_register(SCG0);                  // Enable the FLL control loop
	UCSCTL4 |= SELA__DCOCLK + SELS__XT1CLK +SELM__DCOCLK; //ACLK,SMCLK,MCLK Source select
    UCSCTL5 |= DIVPA_2;                                   //ACLK output divide
    UCSCTL6 |= XT1DRIVE_3 + XCAP_0;                       //XT1 cap
}

void PORT_INIT()
{
 //PORT INIT

    P1DIR |= BIT0;                        // LDC CLK for Freq counter (set to output selected clock)
    P1SEL |=BIT0;

	// LEDs
	P7DIR |= BIT0;
	P4DIR |= BIT7;

	

	// initialize SPI
	P4DIR |= BIT0;  // Output
	P4SEL &= ~BIT0;
}

void SPI_INIT()
{
//SPI SETUP
	P4SEL |=BIT1 + BIT2 + BIT3;
	UCB1CTL1 |=UCSWRST;
	UCB1CTL0 |= UCMST+UCMSB+UCSYNC+UCCKPL;   // 3-pin, 8-bit SPI master,Clock polarity high, MSB
                  UCB1CTL1 |= UCSSEL_1;                 // CLOCK ACLK
	UCB1BR0 = 0x06;
                  UCB1BR1 = 0;
                  UCB1CTL1 &= ~UCSWRST;
}
/** @} */
/*
 * main.c
 */
void main() {

	WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
	UCS_INIT();
	PORT_INIT();
                  SPI_INIT();
	
    /*****************TEST*********TEST***************TEST*********/

    //read all REG value using default setting
   // char orgVal[20];

    //write to register
    spi_writeByte(LDC1000_CMD_RPMAX,       RPMAX);
    spi_writeByte(LDC1000_CMD_RPMIN,       RPMIN);
	spi_writeByte(LDC1000_CMD_SENSORFREQ,  233);
	spi_writeByte(LDC1000_CMD_LDCCONFIG,   0x17);
	spi_writeByte(LDC1000_CMD_CLKCONFIG,   0x00);
	spi_writeByte(LDC1000_CMD_INTCONFIG,   0x02);//比较输出模式

	spi_writeByte(LDC1000_CMD_THRESHILSB,  0x50);
	spi_writeByte(LDC1000_CMD_THRESHIMSB,  0x14);
	spi_writeByte(LDC1000_CMD_THRESLOLSB,  0xC0);
	spi_writeByte(LDC1000_CMD_THRESLOMSB,  0x12);

	spi_writeByte(LDC1000_CMD_PWRCONFIG,   0x01);

	//read all registers

    spi_readBytes(LDC1000_CMD_REVID, &orgVal[0],12);
   
    int i;

    //read all registers using extended（扩展的） SPI
    while (1)
    {
      
     // fenmu=
    //  RP=()
     spi_readByte (LDC1000_CMD_STATUS,&statusData[0]);
    // spi_readByte (LDC1000_CMD_STATUS,&statusData[0]);
     
    // while(statusData[0] & 0x10 ==0x10)
  // {
        //spi_readBytes(LDC1000_CMD_PROXLSB,&proximtyData[0],1);
         spi_readBytes(LDC1000_CMD_PROXLSB,&proximtyData[0],2);
        spi_readBytes(LDC1000_CMD_FREQCTRLSB,&frequencyData[0],3);

        proximtyDataMAX = ((unsigned char) proximtyData[1]<<8) + proximtyData [0];
        frequencyDataMAX = ((unsigned char)frequencyData[1]<<8) + frequencyData[0];

        proximtyDataMIN = proximtyDataMAX;
        frequencyDataMIN = frequencyDataMAX;
         

    	for (i=0;i<52;i++)
    	{
                    
                   
      spi_readBytes(LDC1000_CMD_PROXLSB,&proximtyData[0],2);
      spi_readBytes(LDC1000_CMD_FREQCTRLSB,&frequencyData[0],3);

      proximtyDataTEMP = ((unsigned char)proximtyData[1]<<8) + proximtyData [0];
      frequencyDataTEMP = ((unsigned char)frequencyData[1]<<8) + frequencyData[0];
       
      
      
      if (proximtyDataTEMP < proximtyDataMIN)
    	  proximtyDataMIN = proximtyDataTEMP;
      if (frequencyDataTEMP < frequencyDataMIN)
    	  frequencyDataMIN = frequencyDataTEMP;

      if (proximtyDataTEMP > proximtyDataMAX)
       	  proximtyDataMAX = proximtyDataTEMP;
        if (frequencyDataTEMP > frequencyDataMAX)
       	  frequencyDataMAX = frequencyDataTEMP;
        
        
         frequencyData1[i]= frequencyDataTEMP;
                  proximtyData1[i]=proximtyDataTEMP;
                  Sum_pro+= proximtyData1[i];
                  Sum_fre+=frequencyData1[i];
                  if(i==51)
                  {
               Sum_pro1=(Sum_pro-proximtyDataMIN-proximtyDataMAX)/50;
               Sum_fre1=(Sum_fre-frequencyDataMIN -frequencyDataMAX )/50;
              // proximtyDataMIX=Sum_pro1;
               //frequencyDataMIX=Sum_fre1;
               Y1=Sum_pro1/4096;//Y1=16Y(float不能显示小于1的小数)
               FENMU=3.078*(8-Y1)+(748*Y1);
               FENZI= (float) 748*3078*8;
               RP=(long int)FENZI/FENMU;
               L1=( long int)(Sum_fre1*Sum_fre1)/60298; //算出来的是单位：uH
               L=L1/2;
               Sum_fre1=0;
               Sum_fre=0;
                 Sum_pro=0;
               Sum_pro1=0;
               proximtyDataMIN=0;
               proximtyDataMAX=0;
               frequencyDataMIN=0;
               frequencyDataMAX=0;
               
                  }  
      
                  
    	}
         
      __no_operation();
   // }
    }
//    while(1)
//      {
//        __bis_SR_register(LPM4_bits + GIE);     // Enter LPM4 w/interrupt
//        __no_operation();						// For debugger
//
//      }


	//return 0;
}


/**sub functions**/

char spi_readByte( char addr, char * data)
{
	    rxlen = 1;
		rxbuf = data;
		txaddr = addr | 0x80;

		P4OUT &= ~BIT0;
		while (!(UCB1IFG&UCTXIFG));
		UCB1TXBUF = txaddr;
		while (!(UCB1IFG&UCTXIFG));
		UCB1TXBUF = 0;
		while (UCB1STAT & UCBUSY);
		* rxbuf = UCB1RXBUF;

		while (UCB1STAT & UCBUSY);
		P4OUT |= BIT0;

		return 0;
}

char spi_readWord(char addr, unsigned int * data)
{
		rxlen = 2;
		rxbuf = &wordbuf[0];
		txaddr = addr | 0x80;

		P4OUT &= ~BIT0;
		while (!(UCB1IFG&UCTXIFG));
		UCB1TXBUF = txaddr;
		while (!(UCB1IFG&UCTXIFG));
		UCB1TXBUF = 0;
		while (UCB1STAT & UCBUSY);
		* rxbuf = UCB1RXBUF;
		rxbuf++;
		while (!(UCB1IFG&UCTXIFG));
		UCB1TXBUF = 0;
		while (UCB1STAT & UCBUSY);
		* rxbuf = UCB1RXBUF;

		while (UCB1STAT & UCBUSY);
		P4OUT |= BIT0;

		return 0;

}
char spi_readBytes( char addr, char * buffer, unsigned char len)
{
		rxlen = len;
		rxbuf = buffer;
		txaddr = addr | 0x80;
                                    
		P4OUT &= ~BIT0;
		while (!(UCB1IFG&UCTXIFG));
		UCB1TXBUF = txaddr;

		while (rxlen > 0) {
			while (!(UCB1IFG&UCTXIFG));
			UCB1TXBUF = 0;
			while (UCB1STAT & UCBUSY);
			* rxbuf = UCB1RXBUF;
			rxbuf++;
			rxlen--;
			}

		while (UCB1STAT & UCBUSY);
		P4OUT |= BIT0;

		return 0;
	}

char spi_writeByte(char addr, char data)
{
		wordbuf[0] = data;          // copy from stack to memory
		txlen = 1;
		txbuf = &wordbuf[0];
		txaddr = addr & ~0x80;

		P4OUT &= ~BIT0;
		while (!(UCB1IFG&UCTXIFG));
		UCB1TXBUF = txaddr;
		while (!(UCB1IFG&UCTXIFG));
		UCB1TXBUF = *txbuf;

		while (UCB1STAT & UCBUSY);
		P4OUT |= BIT0;

		return 0;
	}

char spi_writeWord(char addr, unsigned int data)
{
		wordbuf[0] = data >> 8;    // Big Endian
		wordbuf[1] = data & 0xFF;
		txlen = 2;
		txbuf = &wordbuf[0];
		txaddr = addr & ~0x80;

		P4OUT &= ~BIT0;
		while (!(UCB1IFG&UCTXIFG));
		UCB1TXBUF = txaddr;
		while (!(UCB1IFG&UCTXIFG));
		UCB1TXBUF = *txbuf;
		txbuf++;
		while (!(UCB1IFG&UCTXIFG));
		UCB1TXBUF = *txbuf;

		while (UCB1STAT & UCBUSY);
		P4OUT |= BIT0;

		return 0;

}

char spi_writeBytes( char addr, char * buffer, unsigned char len)
{
		txlen = len;
		txbuf = buffer;
		txaddr = addr & ~0x80;

		P4OUT &= ~BIT0;
		while (!(UCB1IFG&UCTXIFG));
		UCB1TXBUF = txaddr;

		while (txlen > 0) {
			while (!(UCB1IFG&UCTXIFG));
			UCB1TXBUF = *txbuf;
			txbuf++;
			txlen--;
				}

		while (UCB1STAT & UCBUSY);
		P4OUT |= BIT0;

		return 0;

	}


void SetVCoreUp (unsigned int level)
{
		// Open PMM registers for write access
		PMMCTL0_H = 0xA5;
		// Make sure no flags are set for iterative sequences
//		while ((PMMIFG & SVSMHDLYIFG) == 0);
//		while ((PMMIFG & SVSMLDLYIFG) == 0);
		// Set SVS/SVM high side new level
		SVSMHCTL = SVSHE + SVSHRVL0 * level + SVMHE + SVSMHRRL0 * level;
		// Set SVM low side to new level
		SVSMLCTL = SVSLE + SVMLE + SVSMLRRL0 * level;
		// Wait till SVM is settled
		while ((PMMIFG & SVSMLDLYIFG) == 0);
		// Clear already set flags
		PMMIFG &= ~(SVMLVLRIFG + SVMLIFG);
		// Set VCore to new level
		PMMCTL0_L = PMMCOREV0 * level;
		// Wait till new level reached
		if ((PMMIFG & SVMLIFG))
		while ((PMMIFG & SVMLVLRIFG) == 0);
		// Set SVS/SVM low side to new level
		SVSMLCTL = SVSLE + SVSLRVL0 * level + SVMLE + SVSMLRRL0 * level;
		// Lock PMM registers for write access
		PMMCTL0_H = 0x00;
}
