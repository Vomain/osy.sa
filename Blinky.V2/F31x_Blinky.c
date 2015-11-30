//-----------------------------------------------------------------------------
// F31x_Blinky.c
//-----------------------------------------------------------------------------
// Copyright (C) 2007 Silicon Laboratories, Inc.
//
// AUTH: JS
// DATE: 03 JUL 02
//
// This program flashes the green LED on the C8051F31x target board about
// five times a second using the interrupt handler for Timer2.
//
// Target: C8051F31x
//
// Tool chain: KEIL Eval 'c'
//

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <c8051f310.h>                    // SFR declarations
#include "stdio.h"

//-----------------------------------------------------------------------------
// 16-bit SFR Definitions for 'F30x
//-----------------------------------------------------------------------------

sfr16 TMR2RL   = 0xca;                    // Timer2 reload value
sfr16 TMR2     = 0xcc;                    // Timer2 counter

sfr16 ADC0     = 0xbd;                    // ADC  

//-----------------------------------------------------------------------------
// Global CONSTANTS
//-----------------------------------------------------------------------------

#define SYSCLK       24500000 / 8         // SYSCLK frequency in Hz

sbit LED = P3^3;                          // LED='1' means ON
sbit SW2 = P0^7;                          // SW2='0' means switch pressed
unsigned int AnBiPot;                     // Analog Binary Potentiometer (binary)
unsigned int AnVoPot;                     // Analog Voltage Potentiometer (mV)
unsigned int AnBiTemp;                    // Analog Binary Temperature (binary)
unsigned int AnVoTemp;                    // Analog Voltage Temperature (�C)
unsigned long L;
unsigned long M;

//-----------------------------------------------------------------------------
// event def
//-----------------------------------------------------------------------------

typedef unsigned char event;

void event_trigger (event *e)
{
    *e = 1;
}

event top_second;

void event_init (event *e) 
{
   *e =0;
}

int event_check (event *e)
{
   if (*e) 
   {
      *e = 0;
	  return 1;
   }
   return 0;
}

//-----------------------------------------------------------------------------
// Time manager
//-----------------------------------------------------------------------------
unsigned int time_special = 0, time_S = 0, time_M = 0, time_H = 0;
void maj_horloge()
{
   time_special++;
   if (time_special == 1) {
	   time_special = 0;
	   time_S++;
	   if (time_S == 60) 
	   {
	      time_S = 0;
		  time_M++;
		  if (time_M == 60) 
		  {
		     time_H++;
			 time_M = 0;
		  }
	   }
   }
}


//-----------------------------------------------------------------------------
// Mesure P1.1
//-----------------------------------------------------------------------------
void ADC_Init(void)
{
   AMX0N     = 0x1f;      // GND en entr�e
   AMX0P     = 0x01;      // entr�e en P1.1

   ADC0CN   |= 0x82;      // tracking mode, start sur timer2
   ADC0CF    = 0x60;      // ajust� � droite, oscillateur � 1,88MHz
   REF0CN    = 0x0e;      // r�f�rence Vdd, capteur temp�rature valide

   EIE1     |= 0x08;      // d�masque l'it ADC
}


//-----------------------------------------------------------------------------
// Function PROTOTYPES
//-----------------------------------------------------------------------------
void SYSCLK_Init (void);
void PORT_Init (void);
void Timer2_Init (int counts);
void Timer2_ISR (void);
void UART0_Init (void);

//-----------------------------------------------------------------------------
// MAIN Routine
//-----------------------------------------------------------------------------
void main (void) {

   // disable watchdog timer
   PCA0MD &= ~0x40;                       // WDTE = 0 (clear watchdog timer
                                          // enable)

   SYSCLK_Init ();                        // Initialize system clock to
                                          // 24.5MHz
   PORT_Init ();                          // Initialize crossbar and GPIO
   Timer2_Init (0x9F81);                  // Init Timer2
   UART0_Init ();
   event_init (&top_second);

   ADC_Init();

   EA = 1;                                // enable global interrupts

   

   while (1) {                            // spin forever
      if (event_check(&top_second))
	  {
	  	 printf("\r");

         maj_horloge();

	     if (time_S%2)
		 {
		    printf("tic ");

         }
		 else {
		    printf("tac ");
		 }

		 printf(" %d:%02d:%02d ",time_H,time_M,time_S);

         // printf("[%0#*d]        ", time_special, 0);        // print d'une bar de chargemenet tous les 1/10e de secondes
			
         printf("\n\nTension aux bornes du potentiom�tre (Binaire) : %d", AnBiPot);         
         printf("\nTension aux bornes du potentiom�tre (V) : %d\n\n", AnVoPot);
		 
         printf("\Temp�rature (Binaire) : %d", AnBiTemp);         
         printf("\nTemp�rature (�C) : %d\n\n\n", AnVoTemp);  

         if (~SW2) {                                       // la DEL reste fixe si on appuie sur le switch (P0.7)
            LED = 1;
         }
         else {
            LED = ~LED;
         }

      }
   }
}

//-----------------------------------------------------------------------------
// SYSCLK_Init
//-----------------------------------------------------------------------------
//
// This routine initializes the system clock to use the internal 24.5MHz / 8
// oscillator as its clock source.  Also enables missing clock detector reset.
//
void SYSCLK_Init (void)
{
   OSCICN = 0xc3;                         // configure internal oscillator for
                                          // its lowest frequency
   RSTSRC = 0x04;                         // enable missing clock detector
}

//-----------------------------------------------------------------------------
// PORT_Init
//-----------------------------------------------------------------------------
//
// Configure the Crossbar and GPIO ports.
// P3.3 - LED (push-pull)
//
void PORT_Init (void)
{
                                          // assignments
   XBR0     = 0x01;                       // no digital peripherals selected
   XBR1     = 0x40;                       // Enable crossbar and weak pull-ups
   P0MDOUT |= 0x10;                       // TxD (P0.4) as push-pull
   P3MDOUT |= 0x08;                       // enable LED as a push-pull output

   P1MDIN  &= ~0x02;                      // permet de passer P1.1 en entr�e analogique
}

//-----------------------------------------------------------------------------
// Timer2_Init
//-----------------------------------------------------------------------------
//
// Configure Timer2 to 16-bit auto-reload and generate an interrupt at
// interval specified by <counts> using SYSCLK/48 as its time base.
//
void Timer2_Init (int n)
{
                                          // use SYSCLK/12 as timebase
   CKCON  &= ~0x10;                       // Timer2 clocked based on T2XCLK;

   TMR2CN  = 0x04;                        // Stop Timer2; Clear TF2; Voir datasheet p199
   TMR2RL  = -n;                          // Init reload values

   ET2 = 1;

}


//-----------------------------------------------------------------------------
// UART0_Init
//-----------------------------------------------------------------------------
//


void UART0_Init()
{
   //Timer 1 init
   TH1     = -213;                        // d�bordement � 57600Hz
   TMOD   |= 0x20;                        // mode2 autoreload
   CKCON  |= 0x08;                        // sysclk pour timer1
   TR1     = 1;                           // timer1 run


   //UART Init
   REN0    = 1;                           // reception autoris�e
   SBUF0   = '\n';                        // pour commencer nouvelle  
}

//-----------------------------------------------------------------------------
// Put_char_
//-----------------------------------------------------------------------------
//

void Put_char_(unsigned char c)
{
   while (TI0 == 0) {}
   TI0     = 0;
   SBUF0   = c;
}

//-----------------------------------------------------------------------------
// Interrupt Service Routines
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Timer2_ISR
//-----------------------------------------------------------------------------
// This routine changes the state of the LED whenever Timer2 overflows.
//
void Timer2_ISR (void) interrupt 5
{
   

   static unsigned char counter = 50;
   counter--;
   if (counter == 0) {

      event_trigger(&top_second);
      
	  counter = 50;
   }
      

   TF2H = 0;                              // clear Timer2 interrupt flag
}

//-----------------------------------------------------------------------------
// ADC_ISR
//-----------------------------------------------------------------------------
// This routine changes everytime the potentiometer is changed
//

void ADC_ISR() interrupt 10 
{
   AD0INT       = 0;                // Remise du bit d'interruption � 0
   
   switch(AMX0P)                         // Il n'y a qu'un seul convertisseur : on ne peut pas mesurer la temp et la tension en m�me temps
   {
      case 0x01:
         AnBiPot = ADC0;
		 AMX0P = 0x1E;
		 break;
      case 0x1E:
         AnBiTemp = ADC0;
		 AMX0P = 0x01;	     
   }

   L                 = (long) AnBiPot * 3300 / 1023; // On d�passe les 16 bits si on reste en int
   AnVoPot           = (unsigned int) L;

   M                 = (long) AnBiTemp * 3300 / 1023; // Pareil. Facteur de conversion � chercher dans la doc
   AnVoTemp          = (unsigned int) L;


}
