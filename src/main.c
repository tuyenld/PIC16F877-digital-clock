#include "defines.h"
#include <xc.h>

#pragma config FOSC = HS      // Oscillator Selection bits (HS oscillator: High-speed crystal/resonator on RA6/OSC2/CLKOUT and RA7/OSC1/CLKIN)
#pragma config WDTE = OFF     // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = OFF    // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = ON     // RE3/MCLR pin function select bit (RE3/MCLR pin function is MCLR)
#pragma config CP = OFF       // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF      // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = ON     // Brown Out Reset Selection bits (BOR enabled)
#pragma config IESO = ON      // Internal External Switchover bit (Internal/External Switchover mode is enabled)
#pragma config FCMEN = ON     // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is enabled)
#pragma config LVP = OFF      // Low Voltage Programming Enable bit (RB3 pin has digital I/O, HV on MCLR must be used for programming)
#pragma config BOR4V = BOR40V // Brown-out Reset Selection bit (Brown-out Reset set to 4.0V)
#pragma config WRT = OFF      // Flash Program Memory Self Write Enable bits (Write protection off)

unsigned s, min, hour, s0, s1, m0, m1, h0, h1;
char tmp0, tmp1, tmp2, no_use; // temperature digits
int temp;
char mode = 1;
char h24 = 0, f_h24 = 0; //h24 = 0: Mode 24h; h24 = 1:Mode 12h
char edit = 1;           //	edit=1(Edit hour); edit=2(Edit min);

char i, j, k;

/* Convert ADC value to temperature digits */
void Convert()
{
    int k;
    k = (int)(temp * 4.8875);
    tmp2 = k / 100;
    k = k % 100;
    tmp1 = k / 10;
    tmp0 = k % 10;
}

/* Decimal to binary-coded decimal (BCD) */
char D2BCD(unsigned x)
{
    char m1, m2;
    m1 = (x % 10);
    m2 = (x / 10) << 4;
    return (m1 + m2);
}

/*
 +------------------------------------+
 |        Interupt routine            |
 +------------------------------------+
 */
void interrupt isbbr()
{

    if (INTCONbits.T0IF)
    {
        TMR0 = Timer0_Reset; //Timer0 Register(8 bit) = 255 -> T = 5ms
        INTCONbits.T0IF = 0;
    }

    if (INTCONbits.INTF)
    {
        /*
 +-------------------------------------------------+
 |              Read TIME                          |
 +-------------------------------------------------+
 */
        Write_Init_1307(0x00);
        Read_1307(&s, &min, &hour);

        s0 = s & 0b00001111;
        s1 = s >> 4;

        m0 = min & 0b00001111;
        m1 = min >> 4;

        if (h24 == 0)
            hour = hour & 0b00111111;
        else
            hour = hour & 0b00011111;
        h0 = hour & 0b00001111;
        h1 = hour >> 4;
        /*
 +----------------------------------------------+
 |                  Read TEMPARATURE            |
 +----------------------------------------------+
 */
        ADCON0bits.GO_nDONE = 1;
        while (ADCON0bits.GO_nDONE)
            ;
        temp = (ADRESH << 8) + ADRESL;
        Convert();

        INTF = 0;
    }

    /*
 +----------------------------------------------+
 |                  Change Mode					|
 +----------------------------------------------+
 */

    if (INTCONbits.RBIF)
    {
        switch (mode)
        {
        case 1:
            if (RB5 == 1)
            {
                while (RB5 == 1)
                    ; //debouncing
                if (h24 == 0)
                {
                    h24 = 1;
                    hour = h0 + h1 * 10; // Decimal
                    if (hour > 12)
                    {
                        hour = hour - 12;
                        f_h24 = 1; // Change Time
                    }
                    else
                        f_h24 = 0;

                    hour = D2BCD(hour);
                    hour = hour | 0b01000000;
                    Write_1307(0x02, hour);
                }
                else
                {

                    h24 = 0;
                    if (f_h24 == 1)
                    {
                        hour = h0 + h1 * 10; // Decimal

                        hour = hour + 12;
                        hour = D2BCD(hour);
                    };
                    hour = hour & 0b10111111;
                    Write_1307(0x02, hour);
                }
            }
            if (RB6 == 1)
            {
                while (RB6 == 1)
                    ; //debouncing
                mode = 2;
            }
            break;
        ////////////////////////////////////////////////////////////
        case 2:
            if (RB7 == 1)
            {
                while (RB7 == 1)
                    ; //debouncing

                switch (edit)
                {
                case 1:
                    hour = h0 + h1 * 10;
                    if (hour >= 23)
                        hour = 0;
                    else
                        hour = hour + 1;
                    Write_1307(0x02, D2BCD(hour));
                    break;
                case 2:
                    min = m0 + m1 * 10;
                    if (min >= 59)
                        min = 0;
                    else
                        min = min + 1;
                    Write_1307(0x01, D2BCD(min));
                    break;
                case 3:
                    s = 0;
                    Write_1307(0x00, 0x00);
                    break;
                }
            }

            if (RB6 == 1)
            {
                while (RB6 == 1)
                    ; //debouncing
                mode = 3;
            }
            if (RB5 == 1)
            {
                while (RB5 == 1)
                    ;
                switch (edit)
                {
                case 1:
                    edit = 2;
                    break;
                case 2:
                    edit = 3;
                    break;
                case 3:
                    edit = 1;
                    break;
                }
            }

            break;
        ///////////////////////////////////////////////////////////////
        case 3:
            if (RB6 == 1)
            {
                while (RB6 == 1)
                    ; //debouncing
                mode = 1;
            }

            break;
        }
        INTCONbits.RBIF = 0;
        no_use = PORTB;
    }
}

int main()
{
    OSCCONbits.IRCF = 0b111; //Internal Oscillator Frequency: 8MHz

    TRISD = 0x00;
    PORTD = 0x00;
    /*
 +----------------------------------------------+
 |              ADC Config                      |
 +----------------------------------------------+
 */
    ANSELbits.ANS0 = 1;
    ADCON0bits.ADCS = 00; // Selection FOSC/2 --> T_AD = 2 us
    ADCON1bits.VCFG1 = 0;
    ADCON1bits.VCFG0 = 0;    // VOltage Reference is VSS-VDD
    ADCON0bits.CHS = 0b0000; // Analog Chanel: AN0
    ADCON1bits.ADFM = 1;     // Right Justified;
    ADCON0bits.ADON = 1;     // Enable ADC;

    /*
 +----------------------------------------------------------+
 |           Interrupt Configure                            |
 +----------------------------------------------------------+
  */
    /*External Interrupt */
    ANSELHbits.ANS12 = 0; /* Interrupt at RB0 For DS1307 */
    TRISBbits.TRISB0 = 1; /* RB0 la input */
    INTCONbits.INTE = 1;
    OPTION_REGbits.INTEDG = 1; /*External interrupt*/
    INTCONbits.INTF = 0;
    INTCONbits.GIE = 1; /* Enable Enterrupt va khoi tao*/
    /*Interrupt ON Change PortB*/
    TRISBbits.TRISB5 = 1;
    ANSELHbits.ANS13 = 0;
    TRISBbits.TRISB6 = 1; //ANSELHbits.ANS8 = 0;
    TRISBbits.TRISB7 = 1; //ANSELHbits.ANS9 = 0;
    INTCONbits.RBIE = 1;
    INTCONbits.RBIF = 0;

    IOCB = 0xFF;

    WPUB = 0xFF;
    OPTION_REGbits.nRBPU = 0;
    /*
    +------------------------------------------------------+
    |                      Timer0 Setup                    |
    |       T = (256-TMR0) / (Fosc /(4*PS))                |
    |               Fosc(defaut) = 4Mhz                    |
    +------------------------------------------------------+
     */
    INTCONbits.GIE = 1;  // Enable Global Interrupt
    INTCONbits.T0IE = 1; //Enable Timer0 Interrupt
    INTCONbits.T0IF = 0; //Timer0 Flag = 0

    OPTION_REGbits.T0CS = 0;
    OPTION_REGbits.PSA = 0;
    OPTION_REGbits.PS = 0b111; //Prescale 1:256
    /*
     +==================================================+
     I                  Main Loop                       I
     +==================================================+
     */
    Init_1307();
    Write_1307(0x00, 0x18); // Declare: second = 30
    Write_1307(0x01, 0x59); // Declare: Minute = 41
    Write_1307(0x02, 0x14); // Declare: Hour = 12
    Write_1307(0x03, 0x04);
    Write_1307(0x04, 0x25);
    Write_1307(0x05, 0x12);
    Write_1307(0x06, 0x29);
    Write_1307(0x07, 0b10010000);

    ClearSRAM();

    while (1)
    {
        switch (mode)
        {

        case 2:
            switch (edit)
            {
            case 1:
                DisplayChar(0, 0);
                DisplayChar(1, 6);
                DisplayChar(2, 14);
                DisplayChar(3, 20);
                DisplayNumber(h1, 28);
                DisplayNumber(h0, 34);

                DisplayNumber(tmp2, 40);
                DisplayNumber(tmp1, 46);
                DisplayNumber(tmp0, 53);
                break;
            case 2:
                DisplayChar(0, 0);
                DisplayChar(1, 6);
                DisplayChar(2, 14);
                DisplayChar(3, 20);
                DisplayNumber(m1, 28);
                DisplayNumber(m0, 34);

                DisplayNumber(tmp2, 40);
                DisplayNumber(tmp1, 46);
                DisplayNumber(tmp0, 53);
                break;
            case 3:
                DisplayChar(0, 0);
                DisplayChar(1, 6);
                DisplayChar(2, 14);
                DisplayChar(3, 20);
                DisplayNumber(s1, 28);
                DisplayNumber(s0, 34);

                DisplayNumber(tmp2, 40);
                DisplayNumber(tmp1, 46);
                DisplayNumber(tmp0, 53);
                break;
            }

            break;
        default:
            DisplayNumber(h1, 0);
            DisplayNumber(h0, 6);
            DisplayNumber(m1, 14);
            DisplayNumber(m0, 20);
            DisplayNumber(s1, 28);
            DisplayNumber(s0, 34);
            DisplayNumber(tmp2, 40);
            DisplayNumber(tmp1, 46);
            DisplayNumber(tmp0, 53);
        }

        display_SRAM = tmp_SRAM;
        Scan595();
    }
    return 1;
}