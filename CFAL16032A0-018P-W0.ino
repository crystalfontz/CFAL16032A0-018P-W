//===========================================================================
//
//
//  CRYSTALFONTZ 
//
//  This code uses the 3-wire SPI MCU mode of the display.
//TODO: add link to website when completed
//  https://www.crystalfontz.com/product/
//
//  The controller is a Solomen Systech SSD1320:
//TODO: add link to controller datasheet
//    https://www.crystalfontz.com/controllers/
//
//  Seeeduino v4.2, an open-source 3.3v capable Arduino clone.
//    https://www.seeedstudio.com/Seeeduino-V4.2-p-2517.html
//    https://github.com/SeeedDocument/SeeeduinoV4/raw/master/resources/Seeeduino_v4.2_sch.pdf
//============================================================================
//
//
//
//===========================================================================
//This is free and unencumbered software released into the public domain.
//
//Anyone is free to copy, modify, publish, use, compile, sell, or
//distribute this software, either in source code form or as a compiled
//binary, for any purpose, commercial or non-commercial, and by any
//means.
//
//In jurisdictions that recognize copyright laws, the author or authors
//of this software dedicate any and all copyright interest in the
//software to the public domain. We make this dedication for the benefit
//of the public at large and to the detriment of our heirs and
//successors. We intend this dedication to be an overt act of
//relinquishment in perpetuity of all present and future rights to this
//software under copyright law.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
//OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
//ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
//OTHER DEALINGS IN THE SOFTWARE.
//
//For more information, please refer to <http://unlicense.org/>
//============================================================================

//============================================================================
// LCD & USD control lines
//   ARD      | Port | CFAL16032A0 |  Function                          | Wire
//------------+------+-------------+------------------------------------+------------
//  12V       |      | #1 #13      |  POWER 12V                         | Red
//  GND       |      | #2-3 #8-9   |  GROUND                            | Black
//  3.3V      |      | #10         |  POWER 3.3V                        | Green
// -----------+------+-------------+------------------------------------+------------
// #10/D10    |  PB2 | #4          |  Chip Select               (CS)    | Purple
// #11/D11    |  PC3 | #7          |  Serial Data Input Signal  (SDIN)  | Grey
// #12/D12    |  PC4 | #6          |  Serial Data Input Clock   (SCLK)  | Yellow
//  #8/D8     |  PC0 | #5          |  Reset                     (RESET) | Blue
// -----------+------+-------------+------------------------------------+------------
//============================================================================
//
//============================================================================

#include "images.h"
#include <SD.h>
#include <avr/io.h>
#include <SPI.h>


// Map which pins are which for Serial communication
#define SS    PORTB2
#define MOSI  PORTB3
#define CLK   PORTB5

#define CLR_CS   (PORTB &= ~(0x04)) //pin #10 - Chip Select
#define SET_CS   (PORTB |=  (0x04)) 
#define CLR_RES  (PORTB &= ~(0x01)) //pin #8 - Reset
#define SET_RES  (PORTB |=  (0x01)) //pin #8 - Reset 

// Set "sendCommand" to send the command bit low followed by the command data
#define sendCommand(cmd)	soft_spi_send_byte(0,cmd)
// Set "sendData" to send the command bit high followed by the data
#define sendData(data)		soft_spi_send_byte(1,data)



//============================================================================
void soft_spi_send_byte(uint8_t cmd, char data)
{
  // The SPI.transfer function sends a byte at a time and since we need to send 9 bits we need a work around
  // In order to send 9 bits of data instead of just a byte, we need to bitbang the controller
  // We are going to hard code the pins to be high or low using assembly instructions, the asm function

  // enable chip_sel
  asm("cbi %0, %1" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (SS));		// clear chip select

  // Send the first bit, the data/command flag bit
  asm("cbi %0, %1" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (CLK));		//	clear the clock
  asm("cbi %0, %1" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (MOSI));	//	clear the SDIN
  asm("sbrc %0, 0" : : "a" (cmd));								              //	check if the data should be set or not
  asm("sbi %0, %1" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (MOSI));	//	set the data
  asm("sbi %0, %1" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (CLK));		//	set the clock



  // Now send all the data bits
  // Send Bit 7 of data
  asm("cbi %0, %1" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (CLK));		//	clear the clock
  asm("cbi %0, %1" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (MOSI));	//	clear the SDIN
  asm("sbrc %0, 7" : : "a" (data));								              //	check if the data should be set or not
  asm("sbi %0, %1" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (MOSI));	//	set the data
  asm("sbi %0, %1" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (CLK));		//	set the clock

  // Send Bit 6 of data
  asm("cbi %0, %1" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (CLK));		//	clear the clock
  asm("cbi %0, %1" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (MOSI));	//	clear the SDIN
  asm("sbrc %0, 6" : : "a" (data));								              //	check if the data should be set or not
  asm("sbi %0, %1" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (MOSI));	//	set the data
  asm("sbi %0, %1" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (CLK));		//	set the clock

  // Send bit 5 of data
  asm("cbi %0, %1" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (CLK));		//	clear the clock
  asm("cbi %0, %1" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (MOSI));	//	clear the SDIN
  asm("sbrc %0, 5" : : "a" (data));								              //	check if the data should be set or not
  asm("sbi %0, %1" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (MOSI));	//	set the data
  asm("sbi %0, %1" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (CLK));		//	set the clock

  // Send bit 4 of data
  asm("cbi %0, %1" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (CLK));		//	clear the clock
  asm("cbi %0, %1" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (MOSI));	//	clear the SDIN
  asm("sbrc %0, 4" : : "a" (data));             								//	check if the data should be set or not
  asm("sbi %0, %1" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (MOSI));	//	set the data
  asm("sbi %0, %1" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (CLK));		//	set the clock

  // Send bit 3 of data
  asm("cbi %0, %1" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (CLK));		//	clear the clock
  asm("cbi %0, %1" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (MOSI));	//	clear the SDIN
  asm("sbrc %0, 3" : : "a" (data));								              //	check if the data should be set or not
  asm("sbi %0, %1" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (MOSI));	//	set the data
  asm("sbi %0, %1" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (CLK));		//	set the clock

  // Send bit 2 of data
  asm("cbi %0, %1" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (CLK));		//	clear the clock
  asm("cbi %0, %1" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (MOSI));	//	clear the SDIN
  asm("sbrc %0, 2" : : "a" (data));								              //	check if the data should be set or not
  asm("sbi %0, %1" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (MOSI));	//	set the data
  asm("sbi %0, %1" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (CLK));		//	set the clock

  // Send bit 1 of data
  asm("cbi %0, %1" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (CLK));		//	clear the clock
  asm("cbi %0, %1" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (MOSI));	//	clear the SDIN
  asm("sbrc %0, 1" : : "a" (data));								              //	check if the data should be set or not
  asm("sbi %0, %1" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (MOSI));	//	set the data
  asm("sbi %0, %1" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (CLK));		//	set the clock

  // Send bit 0 of data
  asm("cbi %0, %1" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (CLK));		//	clear the clock
  asm("cbi %0, %1" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (MOSI));	//	clear the SDIN
  asm("sbrc %0, 0" : : "a" (data));								              //	check if the data should be set or not
  asm("sbi %0, %1" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (MOSI));	//	set the data
  asm("sbi %0, %1" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (CLK));		//	set the clock


  // disable chip_sel
  asm("sbi %0, %1" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (SS));
  asm("cbi %0, %1" : : "I" (_SFR_IO_ADDR(PORTB)), "I" (CLK));	// clear the clock
}


//============================================================================
void SSD1320_init()
{

  CLR_RES;
  delay(500);
  SET_RES;
  delay(500);

//Set Display ON/OFF: OFF
sendCommand(0xAE);

//TODO: Initial Setting Configuration

//Set Display Clock Divide Ratio / Oscillator Frequency
sendCommand(0xD5);
sendCommand(0xC2);
    // 1100 0010
    // |||| ||||--- Define divide ratio of display clock
    // ||||           0000: Divide Ratio = 1
    // ||||            .
    // ||||            .
    // ||||           1111: Divide Ratio = 256
    // ||||-------- Set Oscillator Frequency.Oscillator Frequency increases
    //                with the value of [7:4]and vice versa

//Set Multiplex Ratio to N+1 MUX
sendCommand(0xA8);
sendCommand(0x1F);
    // 0001 1111
    // |||| ||||--- Set MUX ratio to N+1 MUX
    //                0x00 - 0x0E: invalid
    //                0x0F - 0x9F: 16MUX to 160MUX

//Set Display Offset
sendCommand(0xD3);
sendCommand(0x60);
    // 0110 0000
    // |||| ||||--- Set vertical shift by COM from 0d~159d

//Set Display Start Line
sendCommand(0xA2);
sendCommand(0x00);
    //0000 0000
    // |||| ||||--- Set display RAM display start line register
    //                0x00: set start line to 0
    //                 .
    //                 .
    //                0x9F: set start line to 159

//Set Segment Re-map
sendCommand(0xA0);  //column address 0 is mapped to SEG0 (reset)

//Set COM Output Scan Direction
sendCommand(0xC0);  //normal mode (reset) scan from COM0 to COM[N-1]

//Set SEG Pins Hardware Configuration
sendCommand(0xDA);
sendCommand(0x12);
    // 0001 0010
    //   ||-------- SEG pin configuration
    //   |            0: Sequential SEG pin configuration
    //   |            1: Alternative (odd/even) SEG pin configuration (reset)
    //   |
    //   |--------- SEG Left/Right remap
    //                0: Disable SEG Left/Right remap (reset)
    //                1: Enable SEG Left/Right remap

//Set Contrast Control
sendCommand(0x81);
sendCommand(0x5A);
    // 0101 1010
    // |||| ||||--- Brightness/Contrast Control
    //                0x00: Brightness/Contrast set to 0
    //                 .
    //                 .
    //                0xFF: Brightness/Contrast set to full

//Set Pre-Charge Period
sendCommand(0xD9);
sendCommand(0x22);
    // 0010 0010
    // |||| ||||--- Sets the length of phase 1 of segments waveform of the driver
    // ||||           0000 - 1111: Set the period for Phase 1 in the unit of DCLK
    // ||||                         A larger capacitance of the OLED pixel may require
    // ||||                         a longer period to discharge the previous data charge
    // ||||                        
    // ||||-------- Sets the length of phase 2 of segments waveform of the driver
    //                0000 - 1111: Set the period for Phase 2 in the unit of DCLK. 
    //                              A larger period is needed to charge up a larger
    //                              capacitance of the OLED pixel to the targe voltage

//Set VCOMH Deselect Level
sendCommand(0xDB);
sendCommand(0x30);
    // 0011 0000
    //       |||--- Set COM deselect voltage level
    //                000:~.72 x Vcc
    //                010:~.76 x Vcc
    //                100:~.80 x Vcc
    //                110:~.84 x Vcc

//Set Internal IREF Enable
sendCommand(0xAD);
sendCommand(0x10);
    // 0001 0000
    //    |-------- Select external or internal Iref
    //                0: Select external Iref (reset)
    //                1: Enable internal Iref during display ON

//Set Memory Addressing Mode
sendCommand(0x20);
sendCommand(0x00);
    // 0000 0000
    //         |--- Set memory addressing mode
    //                0: Horizontal Addressing Mode
    //                1: Vertical Addressing Mode

//Disable Internal Charge Pump
sendCommand(0x8D);
sendCommand(0x01);
sendCommand(0xAC);
sendCommand(0x00);

//Set Entire Display ON
sendCommand(0xA4);

//Set Normal/Inverse Display: Normal
sendCommand(0xA6);

clean();

//Set Display ON/OFF: ON
sendCommand(0xAF);

//mfg recommended delay
delay(100);
}

//============================================================================
void clean()
{
  //set the display window
  setDefaultWindow();

  //turn off all the pixels
  for (int i = 0; i < 80; i++)
  {
    for (int j = 0; j < 32; j++)
    {
      sendData(0x00);
    }
  }
}


//============================================================================
void fillScreen(uint8_t bmap)
{
  //set the display window
  setDefaultWindow();


  //turn on pixels to match the bitmap sent. Each byte has 2 pixels so bmap will repeat
  //every two pixels
  for (int i = 0; i < 80; i++)
  {
    for (int j = 0; j < 32; j++)
    {
      sendData(bmap);
    }
  }
}


//============================================================================
void OLED_Line(uint16_t x0, uint16_t y0,
  uint16_t x1, uint16_t y1,
  uint8_t g)
{
  int16_t
    dx;
  int16_t
    sx;
  int16_t
    dy;
  int16_t
    sy;
  int16_t
    err;
  int16_t
    e2;
  uint8_t
    p = 1;


  dx = abs((int16_t)x1 - (int16_t)x0);
  sx = x0 < x1 ? 1 : -1;
  dy = abs((int16_t)y1 - (int16_t)y0);
  sy = y0 < y1 ? 1 : -1;
  err = (dx > dy ? dx : -dy) / 2;
  for (;;)
  {
    Put_Pixel(x0, y0, p, g);
    if ((x0 == x1) && (y0 == y1))
      break;
    e2 = err;
    if (e2 > -dx)
    {
      err -= dy;
      x0 = (uint16_t)((int16_t)x0 + sx);
    }
    if (e2 < dy)
    {
      err += dx;
      y0 = (uint16_t)((int16_t)y0 + sy);
    }
  }

}


//============================================================================
void Put_Pixel(uint8_t x, uint8_t y, uint8_t p, uint8_t g)
{
  //This function places a square, p pixels wide with a corner at (x,y) shaded g
  uint8_t startCol = x;
  uint8_t endCol = (x + p) - 1;
  uint8_t startRow = y * 2;
  uint8_t endRow = (y + p) * 2 - 1;
  uint8_t dr = endRow - startRow + 2;
  uint8_t dc = endCol - startCol + 1;

  sendCommand(0x21);
  sendCommand(startCol);  //Start Column Address
  sendCommand(endCol);  //End Column Address

  sendCommand(0x22);
  sendCommand(startRow);  //Start Row Address
  sendCommand(endRow);  //End Row Address
  

  int i = 0;
  int j = 0;
  for (i = 0; i < dr; i++) {
    for (j = 0; j < dc; j++) {
      sendData(g);
    }
  }
}


//============================================================================
void spanningLinesDemo() {
  //this function draws consecutive lines to create the appearance of spanning lines being drawn across the screen
  uint16_t speed = 0;
  uint8_t g = 0xFF; //0x00
  uint8_t x;
  uint8_t y;
  clean();
  x = 0;
  y = 15;
  //start of span is (0,0)
  while (x < 80)
  {

    OLED_Line(0, 0, x, 15, g);
    x++;
    delay(speed);
  }
  while (y != 0)
  {
    y--;
    OLED_Line(0, 0, 79, (y), g);
    delay(speed);
  }
  g = ~g;

  //start of span is (39,0);
  while (y != 16)
  {
    OLED_Line(39, 0, 0, y, g);
    y++;
    delay(speed);
  }
  x = 0;
  while (x < 80)
  {

    OLED_Line(39, 0, x, 15, g);
    x++;
    delay(speed);
  }
  while (y != 0)
  {
    y--;
    OLED_Line(39, 0, 79, (y), g);
    delay(speed);
  }

  g = ~g;
  //start of span is (79,0)
  while (y != 16)
  {
    OLED_Line(79, 0, 0, y, g);
    y++;
    delay(speed);
  }
  x = 0;
  while (x < 80)
  {

    OLED_Line(79, 0, x, 15, g);
    x++;
    delay(speed);
  }
  g = ~g;


  y = 0;
  x = 80;
  //start of span is (79,15)
  while (x != 0)
  {
    x--;
    OLED_Line(79, 15, x, 0, g);
    delay(speed);
  }
  while (y < 16)
  {
    OLED_Line(79, 15, 0, y, g);
    y++;
    delay(speed);
  }
  g = ~g;


  //start of span is (39,15);
  while (y != 0)
  {
    y--;
    OLED_Line(39, 15, 79, y, g);
    delay(speed);
  }
  x = 80;
  while (x != 0)
  {
    x--;
    OLED_Line(39, 15, x, 0, g);
    delay(speed);
  }
  while (y < 16)
  {
    OLED_Line(39, 15, 0, y, g);
    y++;
    delay(speed);
  }
  g = ~g;


  //start of span is (0,15)
  while (y != 0)
  {
    y--;
    OLED_Line(0, 15, 79, y, g);
    delay(speed);
  }
  x = 80;
  while (x != 0)
  {
    x--;
    OLED_Line(0, 15, x, 0, g);
    delay(speed);

  }
}


//============================================================================
void displayImage(const uint8_t pointer[][160])
{
  //This image takes a greyscale 8bit bitmap and converts it to 47bits greyscale
  setDefaultWindow();
  uint8_t holder = 0;
  for (int i = 0; i < 32; i++)
  {
    for (int j = 0; j < 160; j++)
    {
      holder = pgm_read_byte(&pointer[i][j]) >> 4;
      //holder = 0xF0;
      j++;
      holder |= (pgm_read_byte(&pointer[i][j]) & 0xF0);
      sendData(holder);
    }
  }

}

//============================================================================
void flickering()
{
  //
  setDefaultWindow();

  for(int i = 0; i < 50; i++)
  {
    for (int i = 0; i < 80 * 32; i++)
    {
      sendData(0xFF);
    }
    for (int i = 0; i < 80 * 32; i++)
    {
      sendData(0x00);
    }
  }
}

//============================================================================
void checkerboard()
{
  setDefaultWindow();
  uint8_t pattern = 0x00;
  for (int i = 0; i < 32; i++)
  {
    pattern = ~pattern;
    for (int j = 0; j < 160 / 2; j++)
    {
      sendData(pattern);
      sendData(~pattern);
    }
  }
}

//============================================================================
void invertedCheckerboard()
{
  setDefaultWindow();
  uint8_t pattern = 0xFF;
  for (int i = 0; i < 32; i++)
  {
    pattern = ~pattern;
    for (int j = 0; j < 160 / 2; j++)
    {
      sendData(pattern);
      sendData(~pattern);
    }
  }
}


//============================================================================
void setXY(uint8_t startX, uint8_t startY, uint8_t endX, uint8_t endY)
{
  sendCommand(0x21);
  sendData(startX);
  sendData(endX);
  sendCommand(0x22);
  sendData(startY);
  sendData(endY);
}

//============================================================================
void setDefaultWindow()
{
  //Set Column Address
  sendCommand(0x21);
  sendCommand(0);
  sendCommand(79);
  //Set Row Address
  sendCommand(0x22);
  sendCommand(0);
  sendCommand(31);
}



//============================================================================
void setup()
{
  SET_CS;
  Serial.begin(9600);
  CLR_CS;
  delay(100);
  DDRB = 0x2F;
  DDRC = 0x3F;
  PORTB = 0x00;

  SSD1320_init();

  fillScreen(0x00);
  Serial.println(PINB, HEX);

  while (1)
  {
    displayImage(screenMemory);
    delay(15000);
    displayImage(screenMemory3);
    delay(15000);
    displayImage(screenMemory4);
    delay(15000);
    displayImage(screenMemory5);
    delay(15000);
  }
}


//============================================================================
void loop()
{
  Serial.println("in main loop");
  checkerboard();
  delay(1000);
//  displayImage(screenMemory);
  delay(1000);
  checkerboard();
  delay(1000);
  invertedCheckerboard();
  delay(1000);
  fillScreen(0xFF);
  delay(1000);
  flickering();
  delay(500);
  fillScreen(0x00);
  delay(500);
  fillScreen(0xFF);
  delay(500);
  spanningLinesDemo();
  clean();
  delay(1000);
  OLED_Line(0, 0, 79, 15, 0xFF);
  OLED_Line(79, 0, 0, 15, 0xFF);
  delay(1000);
  fillScreen(0x00);
  delay(500);
  fillScreen(0xFF);
  delay(500);
}