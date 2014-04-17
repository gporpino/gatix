/*
 * Gatix
 * Copyright (C) 2014  Daniel Gatis Carrazzoni
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "output/monitor.h"
#include "std/io.h"

// the VGA framebuffer.
uint16_t *video_memory = 0;

// stores the cursor position.
uint8_t cursor_x = 0;
uint8_t cursor_y = 0;

// colors
uint16_t attr_byte = 0;

// chars
uint8_t SPACE = 0x20;
uint8_t TAB = 0x09;
uint8_t BACKSPACE = 0x08;

void k_init_monitor()
{
  video_memory = (uint16_t *)0xB8000;

  k_monitor_clr();

  // foreground white and background black
  k_set_text_color(15, 0);
}

// set foreground and background color.
void k_set_text_color(uint8_t foreground_color, uint8_t background_color)
{
  // top 4 bytes are the background.
  // bottom 4 bytes are the foreground color.
  attr_byte = (background_color << 4) | (foreground_color & 0x0F);
}

// returns a char with foreground and background color
static uint16_t k_char(uint16_t c)
{
  return c | (attr_byte << 8);
}

// updates the hardware cursor.
static void k_move_cursor()
{
  // the screen is 80 characters wide.
  uint16_t cursorLocation = cursor_y * 80 + cursor_x;

  // This sends a command to indicies 14 and 15 in the
  // CRT Control Register of the VGA controller. These
  // are the high and low bytes of the index that show
  // where the hardware cursor is to be 'blinking'. To
  // learn more, you should look up some VGA specific
  // programming documents. A great start to graphics:
  // http://www.brackeen.com/home/vga.
  k_outb(0x3D4, 14);                  // tell the VGA board we are setting the high cursor byte.
  k_outb(0x3D5, cursorLocation >> 8); // send the high cursor byte.
  k_outb(0x3D4, 15);                  // tell the VGA board we are setting the low cursor byte.
  k_outb(0x3D5, cursorLocation);      // send the low cursor byte.
}

// scrolls the text on the screen up by one line.
static void k_scroll()
{
  // row 25 is the end, this means we need to scroll up
  if (cursor_y >= 25)
  {
    // move the current text chunk that makes up the screen back in the buffer by a line.
    for (int i = 0 * 80; i < 24 * 80; i++)
    {
      video_memory[i] = video_memory[i + 80];
    }

    // the last line should now be blank.
    // do this by writing 80 spaces to it.
    for (int i = 24 * 80; i < 25 * 80; i++)
    {
      video_memory[i] = k_char(SPACE);
    }

    // the cursor should now be on the last line.
    cursor_y = 24;
  }
}

// clears the screen, by copying lots of spaces to the framebuffer.
void k_monitor_clr()
{
  for (int i = 0; i < 80 * 25; i++)
  {
    video_memory[i] = k_char(SPACE);
  }

  // move the hardware cursor back to the start.
  cursor_x = 0;
  cursor_y = 0;
  k_move_cursor();
}

// writes a single char.
void k_monitor_puts_c(char c)
{
  // handle a backspace, by moving the cursor back one space.
  if (c == BACKSPACE && cursor_x)
  {
    cursor_x--;
  }

  // handle a tab by increasing the cursor_x, but only to a point where it is divisible by 4.
  else if (c == TAB)
  {
    cursor_x = (cursor_x + 4) - (cursor_x % 4);
  }

  // handle carriage return.
  else if (c == '\r')
  {
    cursor_x = 0;
  }

  // handle newline by moving cursor back to left and increasing the row.
  else if (c == '\n')
  {
    cursor_x = 0;
    cursor_y++;
  }

  // handle any other printable character.
  else if (c >= SPACE)
  {
    uint16_t *location = video_memory + (cursor_y * 80 + cursor_x);
    *location = k_char(c);
    cursor_x++;
  }

  // check if we need to insert a new line because we have reached the end of the screen.
  if (cursor_x >= 80)
  {
    cursor_x = 0;
    cursor_y ++;
  }

  // scroll the screen if needed.
  k_scroll();

  // move the hardware cursor.
  k_move_cursor();
}

// writes a null-terminated string.
void k_monitor_puts_s(char *c)
{
  while (*c != '\0')
  {
    k_monitor_puts_c(*c);
    c++;
  }
}

void k_monitor_puts_hex(int32_t n)
{
  char* hex = "0123456789ABCDEF";
  k_monitor_puts_s("0x");
  for(int i = 1; i <= 8; i++) {
    k_monitor_puts_c(hex[(n >> (32 - 4 * i)) & 0xF]);
  }
}

void k_monitor_puts_dec(int32_t n)
{
  /* enough for 64 bit integer */
  int INT_DIGITS = 19;

  char buf[INT_DIGITS + 2];

  /* points to terminating '\0' */
  char *p = buf + INT_DIGITS + 1;

  if (n >= 0)
  {
    do
    {
      *--p = '0' + (n % 10);
      n /= 10;
    } while (n != 0);
  }
  else {
    do
    {
      *--p = '0' - (n % 10);
      n /= 10;
    } while (n != 0);
    *--p = '-';
  }

  k_monitor_puts_s(p);
}
