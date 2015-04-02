/******************************************************************************
*  Nano-RK, a real-time operating system for sensor networks.
*  Copyright (C) 2007, Real-Time and Multimedia Lab, Carnegie Mellon University
*  All rights reserved.
*
*  This is the Open Source Version of Nano-RK included as part of a Dual
*  Licensing Model. If you are unsure which license to use please refer to:
*  http://www.nanork.org/nano-RK/wiki/Licensing
*
*  This program is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation, version 2.0 of the License.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*******************************************************************************/

#include <nrk.h>
#include <include.h>
#include <ulib.h>
#include <stdio.h>
#include <hal.h>
#include <nrk_error.h>
#include <nrk_timer.h>
#include <nrk_stack_check.h>
#include <nrk_stats.h>
#include <pcf_tdma.h>
#include <tdma_cons.h>

// Constants for the accelrometer
//There are 6 data registers, they are sequential starting 
//with the LSB of X.  We'll read all 6 in a burst and won't
//address them individually
#define ADXL345_REGISTER_XLSB 0x32

//Need to set power control bit to wake up the adxl345
#define ADXL_REGISTER_PWRCTL 0x2D
#define ADXL_REGISTER_FIFOCTL 0x38
#define ADXL_FIFOCTL_STREAM 1<<7
#define ADXL_PWRCTL_MEASURE 1 << 3
#define ADXL_PWRCTL_STBY 0
#define ADXL345_ADDRESS 0xA6
#define ADXL_SIZE 6


//Constants for the gyroscope
#define ITG3200_ADDRESS 0xD0
//request burst of 6 bytes from this address
#define ITG3200_REGISTER_XMSB 0x1D
#define ITG3200_REGISTER_DLPF 0x16
#define ITG3200_FULLSCALE 0x03 << 3
#define ITG3200_42HZ 0x03
#define ITG3200_SIZE 6


#define HMC5843_ADDRESS 0x3C
//First data address of 6 is XMSB.  Also need to set a configuration register for
//continuous measurement
#define HMC5843_REGISTER_XMSB 0x03
#define HMC5843_REGISTER_MEASMODE 0x02
#define HMC5843_MEASMODE_CONT 0x00
#define HMC5843_SIZE 6


tdma_info tx_tdma_fd;

uint8_t tx_buf[TDMA_MAX_PKT_SIZE];

uint16_t mac_address;

uint8_t aes_key[] = {0x00,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0xaa,0xbb,0xcc,0xdd,0xee, 0xff};

NRK_STK Stack1[NRK_APP_STACKSIZE];
nrk_task_type TaskOne;
void task_cadence(void);

NRK_STK tx_task_stack[NRK_APP_STACKSIZE];
nrk_task_type tx_task_info;
void tx_task (void);

void nrk_create_taskset();

int
main ()
{
  nrk_setup_ports();
  nrk_setup_uart(UART_BAUDRATE_115K2);

  tdma_init (TDMA_CLIENT, DEFAULT_CHANNEL, mac_address);

  tdma_aes_setkey(aes_key);
  tdma_aes_enable();

  tdma_tx_slot_add (mac_address&0xFFFF);
  
  nrk_init();

  mac_address = CLIENT_MAC;

  nrk_led_clr(ORANGE_LED);
  nrk_led_clr(BLUE_LED);
  nrk_led_clr(GREEN_LED);
  nrk_led_clr(RED_LED);
 
  nrk_time_set(0,0);
  nrk_create_taskset();
  nrk_start();
  
  return 0;
}

void timer_callback(){
  nrk_led_toggle(RED_LED);
}

void task_cadence()
{
  nrk_time_t t;
  uint8_t val,v;
  uint16_t time;  

  // // setup a software watch dog timer
  nrk_sw_wdt_init(0, &t, NULL);
  nrk_sw_wdt_start(0);

  //set 400 Hz timer
  if (nrk_timer_int_configure
      (NRK_APP_TIMER_0,2,2000, &timer_callback) != NRK_OK)
    nrk_kprintf(PSTR("ERROR setting up timer\r\n"));

  nrk_timer_int_reset(NRK_APP_TIMER_0);

  nrk_gpio_direction(NRK_PORTD_0, NRK_PIN_INPUT);

  nrk_timer_int_start(NRK_APP_TIMER_0);

  while (1) {
    // Update watchdog timer
    nrk_sw_wdt_update(0);
    nrk_led_toggle(RED_LED);

    val = nrk_gpio_get(NRK_PORTD_0);
    if (val)
    {
      time = nrk_timer_int_read(NRK_APP_TIMER_0);
      nrk_timer_int_reset(NRK_APP_TIMER_0);
      
      v = tdma_send (&tx_tdma_fd, &time, 2, TDMA_BLOCKING);
      if (!v)
      {
        nrk_kprintf(PSTR("tx error\r\n"));
      }

      printf("Task1 timer = %u\r\n",time);
    }
  }
}




void
nrk_create_taskset()
{
  nrk_task_set_entry_function( &TaskOne, task_cadence);
  nrk_task_set_stk( &TaskOne, Stack1, NRK_APP_STACKSIZE);
  TaskOne.prio = 2;
  TaskOne.FirstActivation = TRUE;
  TaskOne.Type = BASIC_TASK;
  TaskOne.SchType = PREEMPTIVE;
  TaskOne.period.secs = 0;
  TaskOne.period.nano_secs = 250 * NANOS_PER_MS;
  TaskOne.cpu_reserve.secs = 0;
  TaskOne.cpu_reserve.nano_secs = 0;
  TaskOne.offset.secs = 1;
  TaskOne.offset.nano_secs= 0;
  nrk_activate_task (&TaskOne);

  tdma_task_config ();
}



