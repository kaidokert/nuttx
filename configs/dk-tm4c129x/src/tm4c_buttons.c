/****************************************************************************
 * configs/dk-tm4c/src/tm4c_buttons.c
 *
 *   Copyright (C) 2014-2015 Gregory Nutt. All rights reserved.
 *   Author: Gregory Nutt <gnutt@nuttx.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name NuttX nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <stdint.h>

#include <nuttx/arch.h>
#include <nuttx/board.h>

#include <nuttx/irq.h>
#include <arch/board/board.h>

#include "dk-tm4c129x.h"

#ifdef CONFIG_ARCH_BUTTONS

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/****************************************************************************
 * Private Data
 ****************************************************************************/
/* Pin configuration for each STM3210E-EVAL button.  This array is indexed by
 * the BUTTON_* and JOYSTICK_* definitions in board.h
 */

static const uint32_t g_buttons[NUM_BUTTONS] =
{
  GPIO_SW2, GPIO_SW3, GPIO_SW4
};

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: board_button_initialize
 *
 * Description:
 *   board_button_initialize() must be called to initialize button resources.
 *   After that, board_buttons() may be called to collect the current state
 *   of all buttons or board_button_irq() may be called to register button
 *   interrupt handlers.
 *
 ****************************************************************************/

void board_button_initialize(void)
{
  int i;

  /* Configure the GPIO pins as inputs. */

  for (i = 0; i < NUM_BUTTONS; i++)
    {
      tiva_configgpio(g_buttons[i]);
    }
}

/****************************************************************************
 * Name: board_buttons
 ****************************************************************************/

uint8_t board_buttons(void)
{
  uint8_t ret = 0;
  int i;

  /* Check that state of each key */

  for (i = 0; i < NUM_BUTTONS; i++)
    {
      /* A LOW value means that the key is pressed. */

      bool released = tiva_gpioread(g_buttons[i]);

      /* Accumulate the set of depressed (not released) keys */

      if (!released)
        {
          ret |= (1 << i);
        }
    }

  return ret;
}

/************************************************************************************
 * Button support.
 *
 * Description:
 *   board_button_initialize() must be called to initialize button resources.  After
 *   that, board_buttons() may be called to collect the current state of all
 *   buttons or board_button_irq() may be called to register button interrupt
 *   handlers.
 *
 *   After board_button_initialize() has been called, board_buttons() may be called to
 *   collect the state of all buttons.  board_buttons() returns an 8-bit bit set
 *   with each bit associated with a button.  See the BUTTON_*_BIT and JOYSTICK_*_BIT
 *   definitions in board.h for the meaning of each bit.
 *
 *   board_button_irq() may be called to register an interrupt handler that will
 *   be called when a button is depressed or released.  The ID value is a
 *   button enumeration value that uniquely identifies a button resource. See the
 *   BUTTON_* and JOYSTICK_* definitions in board.h for the meaning of enumeration
 *   value.  The previous interrupt handler address is returned (so that it may
 *   restored, if so desired).
 *
 ************************************************************************************/

#if defined(CONFIG_ARCH_IRQBUTTONS) && defined(CONFIG_TIVA_GPIOP_IRQS)
xcpt_t board_button_irq(int id, xcpt_t irqhandler)
{
  static xcpt_t handler = NULL;
  xcpt_t oldhandler = handler;
  irqstate_t flags;
  int ret;

  /* Interrupts are supported only on ports P and Q and, hence, only on button SW4 */

  if (id >= BUTTON_SW4)
    {
      /* The following should be atomic */

      flags = enter_critical_section();

      /* Detach and disable the button interrupt */

      up_disable_irq(IRQ_SW4);
      irq_detach(IRQ_SW4);
      handler = NULL;

      /* Attach the new handler if so requested */

      if (irqhandler)
        {
          ret = irq_attach(IRQ_SW4, irqhandler);
          if (ret == OK)
            {
              handler = irqhandler;
              up_enable_irq(IRQ_SW4);
            }
        }

      leave_critical_section(flags);
    }

  return oldhandler;
}
#endif
#endif /* CONFIG_ARCH_BUTTONS */
