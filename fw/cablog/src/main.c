/* 
 * File:   main.c
 *
 * $Id$
 */

#include <p24Fxxxx.h>

_CONFIG1 (JTAGEN_OFF & GCP_OFF & GWRP_OFF & COE_OFF & FWDTEN_OFF & ICS_PGx2)
_CONFIG2 (FNOSC_FRC)

#define		DELAY	1

int main(int argc, char** argv) {
    unsigned i;
    // 8MHz
    TRISA = 0xFFFF; // off
    TRISB = 0xFFFF; // off
    TRISC = 0xFFFF; // off
    AD1PCFG = 0xFFFF; // all AN pins now digital

    TRISCbits.TRISC1 = 0; // SD  = pin26 / RC1 / RP17
    TRISBbits.TRISB2 = 0; // TXD = pin23 / RB2 / RP2

    LATCbits.LATC1   = 0; // SD = 0
    LATBbits.LATB2   = 0; // TXD = 0
    for (i = 0; i < DELAY; i++) { } // delay >200 ns
    LATCbits.LATC1   = 1; // SD = 1
    for (i = 0; i < DELAY; i++) { } // delay >200 ns
    LATBbits.LATB2   = 1; // TXD = 1
    for (i = 0; i < DELAY; i++) { } // delay >200 ns
    LATCbits.LATC1   = 0; // SD = 0
    for (i = 0; i < DELAY; i++) { } // delay >200 ns
    LATBbits.LATB2   = 0; // TXD = 0

    while (1) Idle ();
}
