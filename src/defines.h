#ifndef FINAL_H
#define	FINAL_H
/*
+-----------------------------------+
| Declare Data and Shift of 74HC595 |
+-----------------------------------+
 */
#include<xc.h>

#define Timer0_Reset 255
#define _XTAL_FREQ 20000000
/*
+-------------------------------------------------------------------------------+
|               Block0                  |               Block1                  |
|bit7|bit6|bit5|bit4|bit3|bit2|bit1|bit0|bit7|bit6|bit5|bit4|bit3|bit2|bit1|bit0|
+-------------------------------------------------------------------------------+
 */
struct line
{
	char block[8];
};
struct SRAM
{
    struct line row[8];
    char i_row; // present Colum Display
}tmp_SRAM,display_SRAM;

#define SH0 PORTDbits.RD0    // RD0 wire to SH_CP 74HC595_1
#define DS0 PORTDbits.RD1    // RD1 wire to DS of 74HC595_1
#define ST  PORTDbits.RD2    // RD2 wire to ST_CP common Dual 74HC595
#define SH1 PORTDbits.RD3    // RD3 wire to SH_CP of Block Selection Row
#define DS1 PORTDbits.RD4     // RD4 wire to DS of 74HC595 of Block Selec Row


extern void Init_1307();
extern void Write_Init_1307(unsigned add);
extern void Write_1307(unsigned add,unsigned data);
extern void  Read_1307(unsigned *s, unsigned *m, unsigned *h);
extern const char Font1[11][8];

extern void ClearBit(char* x, char n);
extern void SetBit(char* x, char n);
extern void Scan595();
extern void DisplayNumber(char y,char x);
extern void DisplayChar(char y,char x);
extern void ClearSRAM();
#endif	/* FINAL_H */

