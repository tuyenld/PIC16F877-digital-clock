#include<xc.h>
/*
+------------------------------------------------------------+
|               I2C Function                                 |
+------------------------------------------------------------+
 */


void Init_1307()
{
    TRISCbits.TRISC3 = 1;
    TRISCbits.TRISC4 = 1;
    SSPCONbits.SSPEN=1;     //Enable MSSP
    SSPCONbits.SSPM=0b1000; //Mode Master
    SSPSTATbits.SMP=1;      // Disable Slew Rate for Standar Mode
    SSPADD = 0x02;        //-------------------------Set Baud Rate--//
}

void Wait_1307()
{
    while (PIR1bits.SSPIF==0);      // Wait for SSPIF=1
    PIR1bits.SSPIF =0;
}

//RECEIVE mode only
void ACK_1307()
{
    SSPCON2bits.ACKDT=0;    //Acknowledge from Slave
    SSPCON2bits.ACKEN=1;    // Send ACK
    while (SSPCON2bits.ACKEN);
    Wait_1307();            // Wait for it Complete
}

// RECEIVE mode only
void NACK_1307()
{
    SSPCON2bits.ACKDT=1;    // Not Acknowledge
    SSPCON2bits.ACKEN=1;    // SEnd notACK
    Wait_1307();            // Wait for it Complete
}

void Start_1307()
{
    SSPCON2bits.SEN=1;      // Set Start Condition
    while (SSPSTATbits.S==0);   // Indicate Start bit
    Wait_1307();                // Wait for it complete
}

void Stop_1307()
{
    SSPCON2bits.PEN=1;          // Set Stop condition
    while(SSPCON2bits.PEN==1);  // Indicate Stop bit
    Wait_1307();                // Wait for complete
}


// Write "data" to register have address "add" of Slave
void Write_1307(unsigned add,unsigned data)
{
    Start_1307();
    SSPBUF=0b11010000;          // Address of Slave + '0'
    while(SSPSTATbits.BF);      //while SSPBUF read Complete
    while (SSPSTATbits.R_nW);   //While transmition complete
    while(SSPCON2bits.ACKSTAT); // ACK from Slave
    Wait_1307();

    SSPBUF=add;                 // Address Register of Slave to Write
    while (SSPSTATbits.BF);     // Wait for Transmition complete
    while (SSPSTATbits.R_nW);
    while(SSPCON2bits.ACKSTAT); // ACK from Slave
    Wait_1307();

    SSPBUF=data;
    while (SSPSTATbits.BF);     // Wait for Read from SSPBUF complete
    while (SSPSTATbits.R_nW);   //Wait for Transmition complete
    while(SSPCON2bits.ACKSTAT); // ACK from Slave
    Wait_1307();

    Stop_1307();
}


// Write Address of Register of Slave before Read
void Write_Init_1307(unsigned add)
{
    Start_1307();
    SSPBUF=0b11010000;          // Address of Slave + '0'
    while(SSPSTATbits.BF);      // while read from SSPBUF
    while (SSPSTATbits.R_nW);   // While transmittion
    while(SSPCON2bits.ACKSTAT); // ACK from Slave
    Wait_1307();                // Wait for complete

    SSPBUF=add;                 // Address Register of Slave
    while (SSPSTATbits.BF);     // Wait for Read From SSPBUF complete
    while (SSPSTATbits.R_nW);   // Wait for Transmittion Complete
    while(SSPCON2bits.ACKSTAT); // ACK from Slave
    Wait_1307();
    Stop_1307();

}

/*
* Write_Init_1307(0x00) before "Read_1307(char *s, char *m, char *h)" if want to read "s,m,h" and so on...
* s mean "second"
* m mean "minute"
* h mean "hour"
*/
void  Read_1307(unsigned *s, unsigned *m, unsigned *h)
{
//
    Start_1307();
    SSPBUF=0b11010001;          // Address of Slave + '1'
    while(SSPSTATbits.BF);      // while Read From SSPBUF complete
    while(SSPSTATbits.R_nW);    // While transmition complete
    while(SSPCON2bits.ACKSTAT); // ACK from Slave
    Wait_1307();

    SSPCON2bits.RCEN =1;

    while(SSPSTATbits.BF==0);
    while(SSPSTATbits.R_nW);
    Wait_1307();
    (*s) = SSPBUF;
    ACK_1307();
    SSPCON2bits.RCEN =1;

    while(SSPSTATbits.BF==0);
    while(SSPSTATbits.R_nW);
    Wait_1307();
    (*m) = SSPBUF;
    ACK_1307();
    SSPCON2bits.RCEN =1;

    while(SSPSTATbits.BF==0);
    while(SSPSTATbits.R_nW);
    Wait_1307();
    (*h) = SSPBUF;
    NACK_1307();

    Stop_1307();
}

