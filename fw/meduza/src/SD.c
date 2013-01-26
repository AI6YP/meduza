/*
 * $Id$
 */

#include <p24Fxxxx.h>

void BYTE2UART (int dat) {
	while (U1STAbits.UTXBF);
	U1TXREG = dat;
}

void INT2UART (int dat) {
	while (U1STAbits.UTXBF);
	U1TXREG = dat >> 8;
	while (U1STAbits.UTXBF);
	U1TXREG = dat;
}

void SPI2UART (void) {
	int dat;
	while ((SPI1STATbits.SRXMPT == 0) | (SPI1STATbits.SRMPT == 0)) {
		dat = SPI1BUF;
		INT2UART (dat);
	}
}

void SPI2TRASH (void) {
	int dat;
	while ((SPI1STATbits.SRXMPT == 0) | (SPI1STATbits.SRMPT == 0)) {
		dat = SPI1BUF;
	}
}

void SD_CMD_Send (unsigned a, unsigned b, unsigned c) {
	while (SPI1STATbits.SPITBF); SPI1BUF = a;
	while (SPI1STATbits.SPITBF); SPI1BUF = b;
	while (SPI1STATbits.SPITBF); SPI1BUF = c;
	while (SPI1STATbits.SPIBEC); // TX buffer is empty
	while (SPI1STATbits.SRMPT == 0); // SR is empty
	SPI2TRASH ();
}

void SD_Init (void) {
	unsigned i, j, dat;
//	TRISCbits.TRISC6  = 0;
//	RPOR11bits.RP23R  = 8; // SCK1OUT // pin 3 --> RP23 --> RC7  --> SCLK
//	RPOR12bits.RP24R  = 7; // SDO1 // pin 4 --> RP24 --> RC8  --> MOSI
//	RPINR20bits.SDI1R = 12; // pin 10 <-- RP12 <-- RB12 <-- MISO
/*
	// 4MHz
	SPI1CON1bits.PPRE   = 3; // 1:1 Primary Prescale
	SPI1CON1bits.SPRE   = 7; // 1:1 Secondary Prescale
*/
	// 250KHz
	SPI1CON1bits.PPRE   = 0; // 64:1 Primary Prescale
	SPI1CON1bits.SPRE   = 7; // 1:1 Secondary Prescale

	SPI1CON1bits.MODE16 = 1; // Communication is word-wide (16 bits)
	SPI1CON1bits.MSTEN  = 1; // Write the desired settings to the SPIxCON register with MSTEN (SPIxCON1<5>) = 1.
	SPI1STATbits.SPIROV = 0; // Clear the SPIROV bit (SPIxSTAT<6>).
	SPI1CON2bits.SPIBEN = 1; // Select Enhanced Buffer mode
	SPI1CON1bits.CKE    = 1; // Clock Edge Select bit
	SPI1CON1bits.SMP    = 1;
	SPI1STATbits.SPIEN  = 1; // Enable SPI operation by setting the SPIEN bit (SPIxSTAT<15>).

	// reset ??? LOW SPEED !!!
	LATCbits.LATC9 = 1;

	for (i = 0; i < 100; i++) {
		while (SPI1STATbits.SPITBF); // Tx fifo is not-full
		SPI1BUF = 0xFFFF;
		SPI2TRASH ();
	}
	while (SPI1STATbits.SPIBEC);     // Tx buffer is empty
	while (SPI1STATbits.SRMPT == 0); // SR is empty
	SPI2TRASH ();

	LATCbits.LATC9 = 0;

	SD_CMD_Send (0x4000, 0x0000, 0x0095); // CMD0

	// Expect: R1 = FF 01
	while (SPI1STATbits.SPITBF); SPI1BUF = 0xFFFF;
	while (SPI1STATbits.SPITBF); SPI1BUF = 0xFFFF;
	while (SPI1STATbits.SPIBEC); // TX buffer is empty
	while (SPI1STATbits.SRMPT == 0); // SR is empty
	if (SPI1BUF != 0xFF01) { while (1) { BYTE2UART (0xEB); }}
	SPI2TRASH ();

	SD_CMD_Send (0x4800, 0x0001, 0xAA87); // CMD8

	// Expect: R7 = FF 01 00 00 01 AA FF FF (5byte)
	while (SPI1STATbits.SPITBF); SPI1BUF = 0xFFFF;
	while (SPI1STATbits.SPITBF); SPI1BUF = 0xFFFF;
	while (SPI1STATbits.SPITBF); SPI1BUF = 0xFFFF;
	while (SPI1STATbits.SPITBF); SPI1BUF = 0xFFFF;
	while (SPI1STATbits.SPIBEC); // TX buffer is empty
	while (SPI1STATbits.SRMPT == 0); // SR is empty
	SPI2TRASH ();

	SD_CMD_Send (0x7a00, 0x0001, 0x00fd); // CMD58

	// Expect: R3: { R1: [FF 01], OCR: [00 FF 80 00]}, TAIL: [FF FF]
	while (SPI1STATbits.SPITBF); SPI1BUF = 0xFFFF;
	while (SPI1STATbits.SPITBF); SPI1BUF = 0xFFFF;
	while (SPI1STATbits.SPITBF); SPI1BUF = 0xFFFF;
	while (SPI1STATbits.SPITBF); SPI1BUF = 0xFFFF;
	while (SPI1STATbits.SPIBEC); // TX buffer is empty
	while (SPI1STATbits.SRMPT == 0); // SR is empty
	SPI2TRASH ();

	for (i = 0; i < 16384; i++) {
		SD_CMD_Send (0x7700, 0x0000, 0x0065); // CMD55

		// Expect: R1: [FF 01], TAIL: [FF FF] (1byte)
		while (SPI1STATbits.SPITBF); SPI1BUF = 0xFFFF;
		while (SPI1STATbits.SPITBF); SPI1BUF = 0xFFFF;
		while (SPI1STATbits.SPIBEC); // TX buffer is empty
		while (SPI1STATbits.SRMPT == 0); // SR is empty
		SPI2TRASH ();

		SD_CMD_Send (0x6940, 0x0000, 0x0077); // ACMD41 + HCS

		// Expect: R1: [FF 01], TAIL: [FF FF] (1byte)
		while (SPI1STATbits.SPITBF); SPI1BUF = 0xFFFF;
		while (SPI1STATbits.SPITBF); SPI1BUF = 0xFFFF;
		while (SPI1STATbits.SPIBEC); // TX buffer is empty
		while (SPI1STATbits.SRMPT == 0); // SR is empty
		if (SPI1BUF != 0xFF01) { break; }
//		SPI2UART ();
		SPI2TRASH ();
	}
//    SPI2TRASH ();

	SD_CMD_Send (0x7a00, 0x0001, 0x00fd); // CMD58

	// Expect: R3: {R1: [FF 00], OCR: [C0 FF 80 00]}, TAIL: [FF FF]
	while (SPI1STATbits.SPITBF); SPI1BUF = 0xFFFF;
	while (SPI1STATbits.SPITBF); SPI1BUF = 0xFFFF;
	while (SPI1STATbits.SPITBF); SPI1BUF = 0xFFFF;
	while (SPI1STATbits.SPITBF); SPI1BUF = 0xFFFF;
	while (SPI1STATbits.SPIBEC); // TX buffer is empty
	while (SPI1STATbits.SRMPT == 0); // SR is empty
	SPI2TRASH ();

	// 4 Mbit speed SPI
	SPI1STATbits.SPIEN  = 0;
	SPI1CON1bits.PPRE   = 2; // 4:1 Primary Prescale
	SPI1CON1bits.SPRE   = 7; // 1:1 Secondary Prescale
	SPI1STATbits.SPIEN  = 1;
}
/*
    // CMD17 + Addr:0
    while (SPI1STATbits.SPITBF); SPI1BUF = 0x5100;
    while (SPI1STATbits.SPITBF); SPI1BUF = 0x0800;
    while (SPI1STATbits.SPITBF); SPI1BUF = 0x0055; // crc error
    while (SPI1STATbits.SPIBEC); // TX buffer is empty
    while (SPI1STATbits.SRMPT == 0); // SR is empty
    SPI2TRASH ();

    for (i = 0; i < 72; i++) {
        for (j = 0; j < 4; j++) {
            while (SPI1STATbits.SPITBF); // Tx fifo is not-full
            SPI1BUF = 0xFFFF;
        }
        while (SPI1STATbits.SPIBEC); // TX buffer is empty
        while (SPI1STATbits.SRMPT == 0); // SR is empty
        SPI2UART ();
    }
*/

void SD_dump (void) {
	unsigned i, j, k, dat, tmp;
	unsigned code_table [16] = {
		0xFF, 0xFD, 0xFB, 0xF7, 0xEF, 0xDF, 0xBF, 0x7F,
		0xF5, 0xEB, 0xED, 0xDD, 0xBB, 0x77, 0xB7, 0x5F
	};

/*
    for (i = 0; i < 100; i++) {
        for (j = 0; j < 16; j++) {
            while (U1STAbits.UTXBF);
            U1TXREG = code_table [j];
//          for (k = 0; k < 120; k++) { }
        }
    }
*/
	SD_CMD_Send (0x5200, 0x0080, 0x00e1); // CMD18 + Addr:0x00008000

	for (i = 0; i < 100; i++) {
		for (j = 0; j < 1000; j++) {
			while (SPI1STATbits.SPITBF); // while TX fifo is full
			SPI1BUF = 0xFFFF;
			while (SPI1STATbits.SRXMPT); // While Receive FIFO Empty bit (valid in Enhanced Buffer mode)
			dat = SPI1BUF;

			tmp = code_table [(dat & 0xF)];
			while (U1STAbits.UTXBF);
			U1TXREG = tmp;

			tmp = code_table [((dat >> 4) & 0xF)];
			while (U1STAbits.UTXBF);
			U1TXREG = tmp;

			tmp = code_table [((dat >> 8) & 0xF)];
			while (U1STAbits.UTXBF);
			U1TXREG = tmp;

			tmp = code_table [((dat >> 12) & 0xF)];
			while (U1STAbits.UTXBF);
			U1TXREG = tmp;
		}
	}
	SD_CMD_Send (0x4c00, 0x0000, 0x0061); // CMD12

	while (SPI1STATbits.SPITBF); SPI1BUF = 0xFFFF;
	while (SPI1STATbits.SPITBF); SPI1BUF = 0xFFFF;
	while (SPI1STATbits.SPIBEC); // TX buffer is empty
	while (SPI1STATbits.SRMPT == 0); // SR is empty
	SPI2TRASH ();
}

void SD_write_head (unsigned offset) { // 0x0003
	unsigned i;
	SD_CMD_Send (0x5900, 0x0080, offset); // CMD25 + Addr:0x00008000

	while (SPI1STATbits.SPITBF); SPI1BUF = 0xFFFF;
	while (SPI1STATbits.SPITBF); SPI1BUF = 0xFFFF;
	while (SPI1STATbits.SPITBF); SPI1BUF = 0xFFFF;
	while (SPI1STATbits.SPITBF); SPI1BUF = 0xFFFF;
	while (SPI1STATbits.SPIBEC); // TX buffer is empty
	while (SPI1STATbits.SRMPT == 0); // SR is empty
	SPI2UART ();

	while (SPI1STATbits.SPITBF); SPI1BUF = 0xFCAD; // data token
}

void SD_write_data (unsigned dat) { // call 255 times
	while (SPI1STATbits.SPITBF); SPI1BUF = dat;
	while (SPI1STATbits.SPIBEC); // TX buffer is empty
	while (SPI1STATbits.SRMPT == 0); // SR is empty
	SPI2TRASH ();
}

void SD_write_crc  (void) {
	while (SPI1STATbits.SPITBF); SPI1BUF = 0x0000; // last byte + fake CRC start
	while (SPI1STATbits.SPITBF); SPI1BUF = 0x00FF; // fake CRC end + pause
	while (SPI1STATbits.SPITBF); SPI1BUF = 0xFFFF; // pause
	while (SPI1STATbits.SPITBF); SPI1BUF = 0xFFFF; // pause
	while (SPI1STATbits.SPIBEC); // TX buffer is empty
	while (SPI1STATbits.SRMPT == 0); // SR is empty
	SPI2TRASH ();
}

void SD_write_tail (void) {
	unsigned i;
	for (i = 0; i < 16384; i++) {
		while (SPI1STATbits.SPITBF); SPI1BUF = 0xFFFF; // pause
		while (SPI1STATbits.SPIBEC); // TX buffer is empty
		while (SPI1STATbits.SRMPT == 0); // SR is empty
		if (SPI1BUF == 0xFFFF) { break; }
		SPI2TRASH ();
	}

	while (SPI1STATbits.SPITBF); SPI1BUF = 0xFDFF; // stop tran

	for (i = 0; i < 16384; i++) {
		while (SPI1STATbits.SPITBF); SPI1BUF = 0xFFFF; // pause
		while (SPI1STATbits.SPIBEC); // TX buffer is empty
		while (SPI1STATbits.SRMPT == 0); // SR is empty
		if (SPI1BUF == 0xFFFF) { break; }
		SPI2TRASH ();
	}
}
