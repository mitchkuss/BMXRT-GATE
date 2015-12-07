#include <msp430.h> 

/*			USE THIS CODE
 * led driver code cleaned up using loops
 *
 *					 _______
 *				 GND|		|VCC 	(3 TO 5V)
 * (P1.2 MOSI)	 SIN|		|IREF 	(2.52K OHM TO GND)
 *	(P1.4 CLK)	SCLK|		|SOUT 	(P1.1 MISO)
 *	 (P2.0 CS)	 LAT|		|BLANK	(GND)
 *				OUT0|		|OUT15
 *				  |	|		|  |
 *				  |	|		|  |
 *				OUT7|_______|OUT8
 *
 * main.c
 */
void lights();
void sound();
volatile    char    received_ch =   0;
int j=0,k=0;
// A crude delay function.
void __delay_cycles( unsigned long n ) {
    volatile unsigned int i = n/6;
    while( i-- ) ;
}

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

    P2DIR  |=  BIT0 + BIT1 + BIT2;
    P2OUT  |=  BIT0 + BIT2;
    P1DIR  |=  BIT0;
    P1SEL  =   BIT1    |   BIT2    |   BIT4;
    P1SEL2 =   BIT1    |   BIT2    |   BIT4;

    UCA0CTL1   =   UCSWRST;
    UCA0CTL0   |=  UCCKPH  +   UCMSB   +   UCMST   +   UCSYNC; //  3-pin,  8-bit   SPI master
    UCA0CTL1   |=  UCSSEL_2;   			//  SMCLK
    UCA0BR0    |=  0x02;   				//  /2
    UCA0BR1    =   0;  					//
    UCA0MCTL   =   0;  					//  No  modulation
    UCA0CTL1   &=  ~UCSWRST;   			//  **Initialize    USCI    state   machine**
    //  Set all pin outputs to zero to turn off lights

    for(j=1;j>=0;j--){
       	P2OUT  &=  (~BIT0);    				//  Select Device

       	while  (!(IFG2 &   UCA0TXIFG));    	//  USCI_A0 TX buffer ready?
       	UCA0TXBUF  =   0x00;   				//  Send 0xAA over SPI to Slave
       	while  (!(IFG2 &   UCA0RXIFG));    	//  USCI_A0 RX Received?
       	received_ch    =   UCA0RXBUF;  		//  Store received data

    	P2OUT  |=  (BIT0); 					//  Unselect Device
    }
    //__delay_cycles(3200000);			//  (Hold values) Delay 2 Seconds
/*
*	THIS WILL BE WHERE THE SOUND GETS PLAYED
*
*
*/
    P1OUT |= BIT3+BIT0;
    P1REN |= BIT3 + BIT6;
    P1IE |= BIT3+BIT6;
    P1IES &= ~(BIT6);
    P1IFG &= ~(BIT3+BIT6);
    //IE2 |= UCA0RXIE;                          // Enable USCI0 RX interrupt
    __enable_interrupt();
    //lights();
    while(1){
    	P2OUT &= ~BIT1;
    	P1OUT |= BIT0;
    }
    //return 0;
}

#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void){
	if(P1IFG&BIT3){
	  P1OUT ^= BIT0;
	  sound();
	  P1IFG &= ~BIT3;
	}
	if(P1IFG&BIT6){
	  P1OUT ^= BIT0;
	  sound();
	  P1IFG &= ~BIT6;
	  P1IES &= ~(BIT6);
	}

}

void sound(void){
		P2OUT |= BIT1;                      //  Take sound MCU out of RST

		__delay_cycles(3200000);			//  (Hold values) Delay 2 Seconds
		__delay_cycles(3200000);			//  (Hold values) Delay 2 Seconds
		//__delay_cycles(3200000);			//  (Hold values) Delay 2 Seconds

		__delay_cycles(1100000);				//  .5 seconds
		P2OUT &= ~BIT2;					//  Release Gate Before Last Light turns on
		__delay_cycles(100000);				//  .5 seconds
		lights();							//  Shine Lights
		P2OUT |= BIT2;						//  Turn power to gate back on
		P2OUT &= ~BIT1;						//	Put Sound MCU back in RST
}
void lights(void){
	// Set the first light value
	    P2OUT  &=  (~BIT0);    				//  Select  Device

	    while  (!(IFG2 &   UCA0TXIFG));    	//  USCI_A0 TX  buffer  ready?
	    UCA0TXBUF  =   0x0f;   				//  Send 0x0F over SPI to  Slave
	    while  (!(IFG2 &   UCA0RXIFG));    	//  USCI_A0 RX  Received?
	    received_ch    =   UCA0RXBUF;  		//  Store received data

	    P2OUT  |=  (BIT0); 					//  Unselect Device

	    __delay_cycles(160000);				//  Delay 0.1 seconds

	    // Push in next three light values
	    for(j=2;j>=0;j--){
	    	/*if(j==0){
	    		P2OUT &= ~BIT2;					//  Release Gate Before Last Light turns on
	    	}*/
	    	P2OUT  &=  (~BIT0);    				//  Select Device

	    	while  (!(IFG2 &   UCA0TXIFG));    	//  USCI_A0 TX buffer ready?
	    	UCA0TXBUF  =   0xff;   				//  Send 0xFF over SPI to Slave
	    	while  (!(IFG2 &   UCA0RXIFG));    	//  USCI_A0 RX Received?
	    	received_ch    =   UCA0RXBUF;  		//  Store received data

	    	P2OUT  |=  (BIT0); 					//  Unselect Device

	    	__delay_cycles(160000);				//  Delay 0.1 seconds
	    }

	    __delay_cycles(3200000);			//  (Hold values) Delay 2 Seconds

	    //  Set all pin outputs to zero to turn off lights
	    for(j=1;j>=0;j--){
	    	P2OUT  &=  (~BIT0);    				//  Select Device

	    	while  (!(IFG2 &   UCA0TXIFG));    	//  USCI_A0 TX buffer ready?
	    	UCA0TXBUF  =   0x00;   				//  Send 0xAA over SPI to Slave
	    	while  (!(IFG2 &   UCA0RXIFG));    	//  USCI_A0 RX Received?
	    	received_ch    =   UCA0RXBUF;  		//  Store received data

	    	P2OUT  |=  (BIT0); 					//  Unselect Device
	    }
	    return;
}
