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

//-----------------------------------------------------------------------------
// Global CONSTANTS
//-----------------------------------------------------------------------------

#define SYSCLK       24500000 / 8         // SYSCLK frequency in Hz

sbit LED = P3^3;                          // LED='1' means ON
sbit SW2 = P0^7;                          // SW2='0' means switch pressed

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
   Timer2_Init (0xFFF0);                  // Init Timer2
   UART0_Init();

   EA = 1;                                // enable global interrupts

   while (1) {                            // spin forever
      printf("Hey");
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
   TH1     = -213;                        // débordement à 57600Hz
   TMOD   |= 0x20;                        // mode2 autoreload
   CKCON  |= 0x08;                        // sysclk pour timer1
   TR1     = 1;                           // timer1 run


   //UART Init
   REN0    = 1;                           // reception autorisée
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
   if (~SW2) {
      LED = 1;
   }
   else {
      LED = ~LED;
   }

   TF2H = 0;                              // clear Timer2 interrupt flag
}

