/* 
 * File:   main.c
 *
 * $Id$
 */

// #include <stdlib.h>
#include <p24Fxxxx.h>

_CONFIG1 (JTAGEN_OFF & GCP_OFF & GWRP_OFF & COE_OFF & FWDTEN_OFF & ICS_PGx2)
_CONFIG2 (FNOSC_FRCPLL)

#define		TMR1_PRE		3
#define		TMR1_PERIOD		15625
#define		TMR3_PRE		3
#define		ADC_CONV_PERIOD		15

#define		RC_ZERO_MIN		60
#define		RC_ZERO_MAX		90
#define		RC_ONE_MIN		120
#define		RC_ONE_MAX		180
#define		RC_START_MIN		675
#define		RC_START_MAX		1013

#define		U1_BRG		0
#define		U1_BRGH		1
// rate		[bit/s]		4,000,000

// MMA8452QL -- Digital Accelerometer
// R8 = short
// #define		I2C_ACC			0x38
// R8 = open
#define		I2C_ACC			0x3A

// MAG3110 -- Three-Axis, Digital Magnetometer
#define		I2C_MAG			0x1C

// MCP9800 -- High-Accuracy Temperature Sensor
#define		I2C_TMP			0x90

// L3GD20 -- three-axis digital output gyroscope
#define		I2C_GYR			0xD4

unsigned led_green_state;
unsigned led_red_state;

unsigned rc_last_time;
unsigned rc_state;
unsigned long rc_command;
unsigned state;

void Wait (unsigned int time) {
	unsigned int i;
	while (time--) {
		i = 1000;
		while (i--) { Nop (); } // unknown delay
	}
}

void StatusLED (unsigned status) { // Yellow color (for some reason)
	TRISCbits.TRISC4 = 0;    // output
	if (status) {
		LATCbits.LATC4 = 0; // on
	} else {
		LATCbits.LATC4 = 1; // off
	}
}

void LEDs_Init (void) {
	TRISC = 0xffff; // off
	StatusLED (1);
	Wait (1000);
	StatusLED (0);
	Wait (1000);
}


void CLK_Init (void) {
    CLKDIVbits.RCDIV = 0;
    __builtin_write_OSCCONH(1);
    __builtin_write_OSCCONL(1);
    OSCCONbits.OSWEN = 1;
}
void T1_Init(void) {
    TMR1            = 0;           // clear timer1 register
    PR1             = TMR1_PERIOD; // set period1 register
    T1CONbits.TSYNC = 0; // 
    T1CONbits.TCKPS = TMR1_PRE; // ( x, x/8, x/64, x/256 )
    T1CONbits.TCS   = 0; // set internal clock source
//  IPC0bits.T1IP   = 4; // set priority level
    IFS0bits.T1IF   = 0; // clear interrupt flag
    IEC0bits.T1IE   = 1; // enable interrupts
//  SRbits.IPL      = 3; // enable CPU priority levels 4-7
    T1CONbits.TON   = 1; // start the timer
}
void T3_Init(void) {
    T2CONbits.T32   = 0;
    T3CONbits.TCKPS = TMR3_PRE;
    T3CONbits.TCS   = 0; // set internal clock source
//    T3CONbits.TGATE = 0;
    TMR3            = 0;      // clear timer register
    PR3             = 0xFFFF; // set period register
//  IPC0bits.T1IP   = 4; // set priority level
//    IFS0bits.T1IF   = 0; // clear interrupt flag
//    IEC0bits.T1IE   = 1; // enable interrupts
//  SRbits.IPL      = 3; // enable CPU priority levels 4-7
    T3CONbits.TON   = 1; // start the timer
}
void IC1_Init (void) {
    // TMR3 init
    RPINR7bits.IC1R  = 7; // RPN7 --> IC1
    IC1CONbits.ICM   = 2; // @ (negedge IC1)
    IC1CONbits.ICI   = 0; // interrupt for every capture
    IC1CONbits.ICTMR = 0; // TMR3
    IFS0bits.IC1IF   = 0; // clear IC1 interrupt flag
    IEC0bits.IC1IE   = 1; // enable interrupts
}
void TestPorts (void) {
    unsigned i, j;
    TRISA = 0; // on
    TRISB = 0; // on
    TRISC = 0; // on
    j = 0;
    while (1) {
        LATA = j;
        LATB = j;
        LATC = j;
        for (i = 0; i < 50; i++) {}
        j++;
    }
}
unsigned int I2C_Init (void) {
    TRISBbits.TRISB2 = 0; // on SDA2

    I2C2BRG = 39; // 100KHz
    I2C2CON = 0x1200;
    I2C2RCV = 0x0000;
    I2C2TRN = 0x0000;
    // Now we can enable the peripheral
    I2C2CON = 0x9200;
}
void I2C_wait_for_idle (void) {
    while (I2C2STATbits.TRSTAT); // Wait for bus Idle
}
void I2C_write (unsigned char data) {
	I2C2TRN = data;
	while (I2C2STATbits.TBF);
        I2C_wait_for_idle ();
}
void I2C_ByteWrite (unsigned char a0, unsigned char a1, unsigned char d) {
	I2C_wait_for_idle ();
	I2C2CONbits.SEN = 1; while (I2C2CONbits.SEN); // Start
	I2C_write (a0); // 7-bit address + rw=0 (write)
	if (!I2C2STATbits.ACKSTAT) {
		I2C_wait_for_idle ();
		I2C_write (a1);
		if (!I2C2STATbits.ACKSTAT) {
			I2C_wait_for_idle ();
			I2C_write (d);
		}
	}
	I2C_wait_for_idle ();
	I2C2CONbits.PEN = 1; while (I2C1CONbits.PEN); // Stop
}

void I2C_Read (unsigned char a0, unsigned char a1, unsigned char length) {
	unsigned i, tmp;
	I2C_wait_for_idle ();
	I2C2CONbits.SEN = 1; while (I2C2CONbits.SEN); // Start
	I2C_write (a0); // 7-bit address + rw=0 (write)
	if (!I2C2STATbits.ACKSTAT) { // slave sends an acknowledgement
		I2C_wait_for_idle ();
		// master transmits the address of the register to read
		I2C_write (a1); // DR_STATUS
		if (!I2C2STATbits.ACKSTAT) { // slave sends an acknowledgement
			I2C_wait_for_idle ();
			I2C2CONbits.RSEN = 1; while (I2C2CONbits.RSEN); // Restart
			// followed by the slave address with the R/W bit set to “1” for a read from the previously selected register
			I2C_write (a0 | 1); // 0x0E + read=1
			if (!I2C2STATbits.ACKSTAT) { // slave then acknowledges
				for (i = 0; i < (length - 1); i++) {
					I2C2CONbits.RCEN = 1; //  and transmits the data from the requested register.
					I2C2CONbits.ACKDT = 0;
					while(!I2C2STATbits.RBF); // Wait for receive bufer to be full
					tmp = I2C2RCV;
					I2C2CONbits.ACKEN = 1; // ACK
					while (I2C2CONbits.ACKEN); // wait for ACK to complete
					I2C_wait_for_idle ();
				}
				I2C2CONbits.RCEN  = 1; //  and transmits the data from the requested register.
				I2C2CONbits.ACKDT = 1;
				while(!I2C2STATbits.RBF); // Wait for receive bufer to be full
				tmp = I2C2RCV;
				I2C2CONbits.ACKEN = 1; // NACK
				while(I2C2CONbits.ACKEN);		//wait for ACK to complete
				I2C_wait_for_idle ();
			}
		}
	}
	// followed by a stop condition (SP) signaling the end of transmission.
	I2C2CONbits.PEN = 1; while (I2C1CONbits.PEN); // Stop
}




void U1_Init (void) {
    U1MODEbits.BRGH   = U1_BRGH;
    U1BRG             = U1_BRG;
    U1MODEbits.UARTEN = 1;  // U1 enable

// IRDA section
//    U1MODEbits.IREN   = 1;  // IRDA !!!
    U1STAbits.UTXINV = 1;

    U1STAbits.UTXEN   = 1;  // transmit enable
    RPOR4bits.RP8R    = 3;  // U1TX --> RP8 --> pin 44
//  RPOR1bits.RP2R    = 3;  // U1TX --> RP2 --> pin 23
//  RPOR9bits.RP19R   = 3;  // U1TX --> RP19 --> pin 36
//  RPOR10bits.RP21R   = 3;  // U1TX --> RP21 --> pin 38
}
void RC_StateMachine_Init (void) {
    rc_state = 0; // Idle state
    rc_command = 0;
}
unsigned long RC_StateMachine (void) {
    unsigned time, delta;
    unsigned long tmp_command;
    time = IC1BUF;
    delta = time - rc_last_time;
    rc_last_time = time;
    if (rc_state) { // counting bits
        if ((delta >= RC_ZERO_MIN) & (delta <= RC_ZERO_MAX)) {
            rc_state++;
            rc_command = rc_command << 1;
        } else if ((delta >= RC_ONE_MIN) & (delta <= RC_ONE_MAX)) {
            rc_state++;
            rc_command = rc_command << 1;
            rc_command = rc_command | 1;
        } else {
            rc_state   = 0;
            rc_command = 0;
            return 0;
        }
        if (rc_state == 32) {
            rc_state = 0;
            tmp_command = rc_command;
            rc_command = 0;
            return tmp_command;
        }
    } else { // Idle
        if ((delta >= RC_START_MIN) & (delta <= RC_START_MAX)) {
            rc_state = 1;
        }
    }
    return 0;
}
void __attribute__((__interrupt__)) _IC1Interrupt (void) {
    unsigned long tmp_command;
    unsigned i;
    tmp_command = RC_StateMachine ();
    if (tmp_command) {
        if (tmp_command == 0x007FE11E) {
            state = 1;
            LATAbits.LATA4 = 1; // green on
            LATAbits.LATA9 = 0; // red off
            SD_dump ();
            state = 0;
            LATAbits.LATA4 = 0; // green off
            LATAbits.LATA9 = 0; // red off
        } else if (tmp_command == 0x007FB44B) {
            state = 2;
            LATAbits.LATA4 = 0; // green off
            LATAbits.LATA9 = 1; // red on
            SD_write ();
            state = 0;
            LATAbits.LATA4 = 0; // green off
            LATAbits.LATA9 = 0; // red off
        } else {
            state = 0;
            LATAbits.LATA4 = 0; // green off
            LATAbits.LATA9 = 0; // red off
        }
    }
    IFS0bits.IC1IF = 0; // clear IC1 interrupt flag
}
void __attribute__((__interrupt__)) _T1Interrupt (void) {
/*
//    int ADCValue;
//    AD1PCFG = 0xFFFD; // all PORTB = Digital; RB12 = analog
    AD1CON1 = 0x00E0; // SSRC<2:0> = 111 implies internal counter ends sampling
    // and starts converting.
//    AD1CHS = 1; // Connect AN1 as S/H input.
    // in this example AN12 is the input
//    AD1CSSL = 0;
    AD1CON3 = 0x1F02; // Sample time = 31Tad, Tad = 3Tcy
    AD1CON2 = 0;
    AD1CON1bits.ADON = 1; // turn ADC ON
    AD1CON1bits.SAMP = 1; // start sampling, then after 31Tad go to conversion
//    while (!AD1CON1bits.DONE){}; // conversion done?
//    ADCValue = ADC1BUF0; // yes then get ADC value

//    while (U1STAbits.UTXBF == 1);
//    U1TXREG = ADCValue;
 */
    IFS0bits.T1IF = 0; // clear T1 interrupt flag
}
void ADC_Init (void) {
    // Select how conversion results are
    // presented in the buffer
    AD1CON1bits.FORM = 0; // Integer (0000 00dd dddd dddd)
    // Select the appropriate sample/conversion sequence
    AD1CON1bits.SSRC = 3; // Internal counter ends sampling and starts conversion (auto-convert)
//    AD1CON1bits.ASAM = 1; // Sampling begins immediately after last conversion completes. SAMP bit is auto-set.
    // Select port AN1 pins as analog input
    AD1PCFG = 0xFFFF;
    AD1PCFGbits.PCFG1 = 0;
    // Set up A/D Channel Select Register to convert AN1 on Mux A input
    AD1CHS = 1;
    // Select voltage reference source to match
    // expected range on analog inputs
    AD1CON2bits.VCFG = 0;
    // Select the analog conversion clock to
    // match desired data rate with processor
    // clock
    AD1CON3bits.ADCS = ADC_CONV_PERIOD;
    AD1CON3bits.SAMC = 1;
    AD1CON3bits.ADRC = 0; // Clock derived from system clock
    // and
    // AD1CON3<12:8>).
    // Select interrupt rate
    AD1CON2bits.SMPI = 0; // Interrupts at the completion of conversion for each sample/convert sequence
    // Clear the A/D interrupt flag bit
    IFS0bits.AD1IF = 0;
    // Set the A/D interrupt enable bit
    IEC0bits.AD1IE = 1;
    // Turn on A/D module
//    AD1CON1bits.ADON = 1;
//    AD1CON1bits.SAMP = 1;
//    AD1CON1bits.ASAM = 1; // Sampling begins immediately after last conversion completes. SAMP bit is auto-set.
}
void __attribute__((__interrupt__)) _ADC1Interrupt(void) {
    // Clear the A/D Interrupt flag bit or else the CPU will
    // keep vectoring back to the ISR
    unsigned res;
    IFS0bits.AD1IF = 0;

    res = ADC1BUF0;
    while (U1STAbits.UTXBF == 1);
    U1TXREG = res >> 8;
    while (U1STAbits.UTXBF == 1);
    U1TXREG = res;
//    AD1CON1 = 0x00E0; // SSRC<2:0> = 111 implies internal counter ends sampling
}


int main(int argc, char** argv) {
	unsigned i, j;
	I2C_Init ();
	LEDs_Init ();

/*
	I2C_Read (I2C_MAG, 0, 10);
	LEDs_Init ();

	I2C_Read (I2C_ACC, 0, 10);
	LEDs_Init ();

	I2C_Read (I2C_TMP, 0, 4);
	LEDs_Init ();
*/

	I2C_Read (I2C_GYR, 0, 5);
	LEDs_Init ();

/*
	I2C_ByteWrite (0x90, 1, 0x60);
	I2C_ByteWrite (0x90, 1, 0x60);
	for (i = 0; i < 100; i++) {
		I2C_Read (0x90, 0, 2); // MCP9800
		for (j = 0; j < 60000; j++) { }
	}
*/

	LEDs_Init ();

//    CLK_Init ();
//    U1_Init ();
//    T1_Init ();
//    T3_Init ();
//    IC1_Init ();
//    RC_StateMachine_Init ();
//    ADC_Init ();
//    SD_Init ();
	while (1) Idle ();
//	return (EXIT_SUCCESS);
}
