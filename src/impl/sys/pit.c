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

#include "sys/pit.h"
#include "std/io.h"
#include "desc/idt.h"

uint32_t timer_ticks = 0;

void k_timer_phase(uint32_t frequency)
{
  uint32_t divisor = 1193180 / frequency;
  k_outb(0x43, 0x36);

  uint8_t l = (uint8_t)(divisor & 0xFF);
  uint8_t h = (uint8_t)((divisor>>8) & 0xFF);

  k_outb(0x40, l);
  k_outb(0x40, h);
}

void k_timer_handler(registers_t registers)
{
  timer_ticks++;
}

void k_init_timer()
{
  k_timer_phase(50);
  k_idt_set_handler(0, k_timer_handler);
}
