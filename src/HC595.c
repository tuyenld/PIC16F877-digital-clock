#include "defines.h"
#include <xc.h>
char i, j, k;
/*
+-----------------------------------------------------------------------+
|               Difine 74HC595 Function  			
|		ST: Shift_STorage (Common Use )			
|		SH0: Shift-Data 1				
|		DS0: Data 1 (Serial Connect)			
|								
|		SH1: Shift - Data to display Time		
|		DS1: Data 1 (one 74HC595) to display Time	
|									
|		SH0_Temp: Shift -Data to display Temperature		
|		DS0_Temp: Data 2 (one 74HC595) to display Temperature       
|									
+-----------------------------------------------------------------------+
 */
// Function make Pulse to Export STorage bit
void Export595()
{
    ST = 0;
    ST = 1;
}

// Make pulse to Inport 1 bit Serial
void Inport0()
{
    SH0 = 0;
    SH0 = 1;
}

// Make Pulse to Inport 1 bit on 74HC595_2
void Inport1()
{
    SH1 = 0;
    SH1 = 1;
}

// Inport 1 byte to 74HC595_2 Serial
// LSB firST
void Input595(char data)
{
    for (char i = 0; i < 8; i++) // Shift 5 BECAUSE: Font 5
    {
        DS0 = (data >> i) & 0x01;
        Inport0();
    }
}

// Control ROW when 74HC595_2 selection ROW
// 0 -> led turn on
//row-based font. row0=0; row1=1
const char Font1[11][8] = {
    {0b00010001, 0b11101110, 0b11101110, 0b11101110, 0b11101110, 0b11101110, 0b11101110, 0b00010001}, //0
    {0b00011011, 0b00010011, 0b00011011, 0b00011011, 0b00011011, 0b00011011, 0b00011011, 0b00010001}, //1
    {0b00011001, 0b00010110, 0b00001110, 0b00011110, 0b00011101, 0b00011011, 0b00010111, 0b00000000}, //2
    {0b00000000, 0b00011101, 0b00011011, 0b00010001, 0b00011110, 0b00011110, 0b00011110, 0b00000001}, //3
    {0b00011101, 0b00011001, 0b00010101, 0b00001101, 0b00000000, 0b00011101, 0b00011101, 0b00011101}, //4
    {0b00000001, 0b00001111, 0b00001111, 0b00000001, 0b00011110, 0b00011110, 0b00011110, 0b00000001}, //5
    {0b00010001, 0b00001111, 0b00001111, 0b00000001, 0b00001110, 0b00001110, 0b00001110, 0b00010001}, //6
    {0b00000000, 0b00011101, 0b00011011, 0b00010111, 0b00001111, 0b00001111, 0b00001111, 0b00001111}, //7
    {0b00010001, 0b00001110, 0b00001110, 0b00010001, 0b00001110, 0b00001110, 0b00001110, 0b00010001}, //8
    {0b00010001, 0b00001110, 0b00001110, 0b00010000, 0b00011110, 0b00011110, 0b00001110, 0b00010001}, //9
    {0b00010001, 0b00001110, 0b00001111, 0b00001111, 0b00001111, 0b00001111, 0b00001110, 0b00010001}  // 10 -> C
};

const char Font2[4][8] = {

    {0b00011000, 0b00010111, 0b00001111, 0b00000000, 0b00001111, 0b00001111, 0b00010111, 0b00011000}, // 11 -> E
    {0b00000011, 0b00001101, 0b00001110, 0b00001110, 0b00001110, 0b00001110, 0b00001101, 0b00000001}, // 12 -> D
    {0b00000000, 0b00011011, 0b00011011, 0b00011011, 0b00011011, 0b00011011, 0b00011011, 0b00000000}, // 13 -> I
    {0b00000000, 0b00011011, 0b00011011, 0b00011011, 0b00011011, 0b00011011, 0b00011011, 0b00011011}, // 14 -> T
};

/*
* Display: "n m l k u v" on Led Matrix
* Row control is "100...0" while Colum control is "0b00000000"
* Or Row control is "0100...00" while Colum control is "0b00001110" etc,
*/

void Scan595()
{
    char i;
    char j;
    DS1 = 1;

    for (i = 0; i < 8; i++)
    {
        for (j = 0; j < 8; j++)
        {
            Input595(display_SRAM.row[i].block[7 - j]);
        }

        //Input595(Font1[3][i]);
        Inport1();
        Export595();
        DS1 = 0;
        //Inport0();
    }
}
//Clear bit n (0-7) in byte x
void ClearBit(char *x, char n)
{
    (*x) &= ~(1 << n);
}
//Set bit n (0-7) in byte x
void SetBit(char *x, char n)
{
    (*x) |= (1 << n);
}
// display a 5-column number y from position x (0-64)
void DisplayNumber(char y, char x)
{
    char tmp_Col = x % 8;
    char tmp_Block = x / 8;

    for (i = 0; i < 8; i++)
    {
        tmp_Block = x / 8;
        k = tmp_Col;
        for (j = tmp_Col; j < tmp_Col + 5; j++) // x/8:Block; x%8: starting collumn
        {
            if (((j & 0x07) == 0) && j > 0) /////////////////// j=0
            {
                tmp_Block++;
                k = 0;
            }
            if (((Font2[y][i] & (1 << (4 - j + tmp_Col))) >> (4 - j + tmp_Col)) == 0)
                ClearBit(&tmp_SRAM.row[i].block[tmp_Block], 7 - k);
            else
                SetBit(&tmp_SRAM.row[i].block[tmp_Block], 7 - k);
            k++;
        }
    }
}

// display a 5-column letter y from position x (took from Font2)
void DisplayChar(char y, char x)
{
    char tmp_Col = x % 8;
    char tmp_Block = x / 8;

    for (i = 0; i < 8; i++)
    {
        tmp_Block = x / 8;
        k = tmp_Col;
        for (j = tmp_Col; j < tmp_Col + 5; j++) // x/8:Block; x%8: starting collumn
        {
            if (((j & 0x07) == 0) && j > 0) /////////////////// j=0
            {
                tmp_Block++;
                k = 0;
            }
            if (((Font2[y][i] & (1 << (4 - j + tmp_Col))) >> (4 - j + tmp_Col)) == 0)
                ClearBit(&tmp_SRAM.row[i].block[tmp_Block], 7 - k);
            else
                SetBit(&tmp_SRAM.row[i].block[tmp_Block], 7 - k);
            k++;
        }
    }
}

// clear all display
void ClearSRAM()
{
    for (i = 0; i < 8; i++)
        for (j = 0; j < 8; j++)
            tmp_SRAM.row[i].block[j] = 0xFF;
}