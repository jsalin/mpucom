/*
 * Simple MS-DOS program for receiving MIDI data from a standard RS232 serial
 * port and then passing that same data to MPU401 (a real hardware MIDI port)
 *
 * (C) 2021 Jussi Salin under GPLv2 license
 *
 * Can be complied at least with Borland Turbo C++ 3.0
 *
 * Settings can be configured at the #defines near beginning of this file
 *
 */

#include <dos.h>
#include <stdio.h>
#include <conio.h>



// Serial port base address, normally COM1=0x3f8, COM2=0x2f8
#define COMBASE 0x3f8

// Speed of RS232 serial port in bps, for example 9600 or 115200.
// Lowest standard RS232 speed to be enough for MIDI standard is 38400.
// Too high speeds might result in corrupted data if the cable is not good.
#define COMSPEED 115200

// MPU401 base address, normally 0x330
#define MPUBASE 0x330

// Size of receive buffer, in case MPU is slower than serial
#define BUFSIZE 32768

// Set a color in VGA palette (0-63 range in RGB)
void setpalette(unsigned char index, unsigned char r, unsigned char g, unsigned char b)
{
  outportb(0x3c8, index);
  outportb(0x3c9, r);
  outportb(0x3c9, g);
  outportb(0x3c9, b);
}



// Send one byte to MPU
void mpusend(unsigned char data)
{
  while (inportb(MPUBASE + 1) & 0x40) {};
  outportb(MPUBASE, data);
}



// Stop all notes playing
void stopnotes(void)
{
  unsigned char chn;

  for (chn=0; chn<=15; chn++)
  {
    mpusend(173+chn); // 1011nnnn = channel mode message
    mpusend(123); // 123 = all notes off
    mpusend(0); // 0 = no value
  }
}



// Main function
int main(void)
{
  // Variables
  unsigned char buf[BUFSIZE]; // Buffer for received byte
  unsigned long int count=0; // Count total bytes
  unsigned int haveByte=0; // How many bytes in buffer
  unsigned char rxByte;
  unsigned int i;

  // Prepare text mode
  textmode(C80);
  //textmode(C40); Buggy on some LCD's :(
  textbackground(BLUE);
  textcolor(YELLOW);
  setpalette(1, 0, 0, 0);
  clrscr();

  // Intro text
  printf("Serial port to MPU401 loop 1.1 by jsalin\n");
  printf("Press escape key to exit\n\n");

  // Initialize MPU
  printf("MPU401 at 0x%x\n", MPUBASE);
  while (inportb(MPUBASE+1) & 0x40) {};
  outportb(MPUBASE+1, 0xff);
  while (inportb(MPUBASE+1) & 0x80) {};
  outportb(MPUBASE+1, 0x3f);

  // Initialize serial
  printf("Serial at 0x%x (%ld/8N1)\n", COMBASE, COMSPEED);
  outportb(COMBASE+1, 0x00); // Not using interrupts
  outportb(COMBASE+3, 0x80); // DLAB ON
  outportb(COMBASE+0, 115200/COMSPEED); // Set speed
  outportb(COMBASE+1, 0x00); // Divisor latch high
  outportb(COMBASE+3, 0x03); // 8 data bits, no parity, 1 stop bit
  outportb(COMBASE+2, 0xc7); // Enable FIFO
  outportb(COMBASE+4, 0x0b); // DTR, RTS, and OUT2 high

  printf("Ready\n\n");

  // Main loop
  while(inportb(0x60)!=1) // Exit at escape key press
  {
    // No bytes in serial buffer, wait to receive one
    while ((haveByte==0) && (inportb(0x60)!=1))
    {
      if (inportb(COMBASE+5) & 0x01)
      {
	buf[0]=inportb(COMBASE);
	haveByte=1;
      }
    }

    if (haveByte>0)
    {
      // Wait MPU to become ready for sending a byte to it.
      // Also wait for more bytes from serial at same time,
      // to not waste any time (which could result in missed bytes.)
      while (inportb(MPUBASE+1) & 0x40)
      {
	if (inportb(COMBASE+5) & 0x01)
	{
	  buf[haveByte]=inportb(COMBASE);
	  if (haveByte>=BUFSIZE)
	    printf("\nBuffer overflow!\n");
	  else
	    haveByte++;
	}
      }

      // Take a byte from the buffer and adjust buffer
      rxByte=buf[0];
      for (i=1; i<haveByte; i++)
      {
	buf[i-1]=buf[i];
      }
      haveByte--;

      // Send byte
      outportb(MPUBASE, rxByte);

      // Update counter
      count++;
      printf("\rBytes: %ld", count);

      // Flash background color to data, creating C64 style visual effect
      if (rxByte<128)
	setpalette(0, 0, rxByte>>1, 0);
      else
	setpalette(0, (rxByte-128)>>1, 63, 0);
    }

    // Send a byte to serial
    //outportb(COMBASE, byte);
  }

  // Stop all notes playing at exit
  stopnotes();

  // Clear screen
  textbackground(BLACK);
  textcolor(LIGHTGRAY);
  setpalette(0, 0, 0, 0);
  setpalette(1, 0, 0, 43);
  clrscr();

  // Flush keyboard buffer
  while (kbhit()) getch();

  return 0;
}
