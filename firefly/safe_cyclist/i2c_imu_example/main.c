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
#include <TWI_Master.h>



#define TWI_GEN_CALL         0x00  // The General Call address is 0

// Sample TWI transmission commands
#define TWI_CMD_MASTER_WRITE 0x10
#define TWI_CMD_MASTER_READ  0x20

// Sample TWI transmission states, used in the main application.
#define SEND_DATA             0x01
#define REQUEST_DATA          0x02
#define READ_DATA_FROM_BUFFER 0x03


/* Constants for ADXL345 */

//LSB of the X,Y,Z data registers
#define ADXL345_REGISTER_XLSB 0x32

//Power control register
#define ADXL_REGISTER_PWRCTL 0x2D
//Fifo control register
#define ADXL_REGISTER_FIFOCTL 0x38
//Data format register
#define ADXL_REGISTER_DATAFMT 0x31

//Stream fifo bits
#define ADXL_FIFOCTL_STREAM 1<<7
//Measure mode
#define ADXL_PWRCTL_MEASURE 1 << 3
//Standy mode
#define ADXL_PWRCTL_STBY 0

// -+4g mode
#define ADXL_DATAFMT_4G 0x01
// address of adxl345
#define ADXL345_ADDRESS 0xA6


//Constants for the gyroscope
#define ITG3200_ADDRESS 0xD0
//request burst of 6 bytes from this address
#define ITG3200_REGISTER_XMSB 0x1D
#define ITG3200_REGISTER_DLPF 0x16
#define ITG3200_FULLSCALE 0x03 << 3
#define ITG3200_42HZ 0x03


#define HMC5843_ADDRESS 0x3C
//First data address of 6 is XMSB.  Also need to set a configuration register for
//continuous measurement
 #define HMC5843_REGISTER_XMSB 0x03
 #define HMC5843_REGISTER_MEASMODE 0x02
 #define HMC5843_MEASMODE_CONT 0x00

uint8_t messageBuf[16];

unsigned char TWI_Act_On_Failure_In_Last_Transmission ( unsigned char TWIerrorMsg )
{
                    // A failure has occurred, use TWIerrorMsg to determine the nature of the failure
                    // and take appropriate actions.
                    // Se header file for a list of possible failures messages.
                    
                    // Here is a simple sample, where if received a NACK on the slave address,
                    // then a retransmission will be initiated.
if ( (TWIerrorMsg == TWI_MTX_ADR_NACK) | (TWIerrorMsg == TWI_MRX_ADR_NACK) ){
    TWI_Start_Transceiver();
}
printf("%c \n",TWIerrorMsg);
    
  return TWIerrorMsg; 
}




NRK_STK Stack1[NRK_APP_STACKSIZE];
nrk_task_type TaskOne;

NRK_STK Stack2[NRK_APP_STACKSIZE];
nrk_task_type TaskTwo;

NRK_STK Stack3[NRK_APP_STACKSIZE];
nrk_task_type TaskThree;

void init_adxl345(void);
void init_itg3200(void);
void init_hmc5843(void);

void nrk_create_taskset();

int
main ()
{
  nrk_setup_ports();
  nrk_setup_uart(UART_BAUDRATE_115K2);

  TWI_Master_Initialise();
  sei();
  /* initialize the adxl345 */
  init_adxl345();
  init_itg3200();
  init_hmc5843();
  
  nrk_init();

  nrk_led_clr(ORANGE_LED);
  nrk_led_clr(BLUE_LED);
  nrk_led_clr(GREEN_LED);
  nrk_led_clr(RED_LED);
 
  nrk_time_set(0,0);
  nrk_create_taskset ();
  nrk_start();
  
  return 0;
}

void init_itg3200() {
    /* put in standby mode while we change fifo control bits */
  messageBuf[0] = ITG3200_ADDRESS | FALSE<<TWI_READ_BIT;
  messageBuf[1] = ITG3200_REGISTER_DLPF;
  messageBuf[2] = ITG3200_FULLSCALE | ITG3200_42HZ;
  TWI_Start_Transceiver_With_Data(messageBuf, 3);
}

void init_hmc5843() {
    /* put in standby mode while we change fifo control bits */
  messageBuf[0] = HMC5843_ADDRESS | FALSE<<TWI_READ_BIT;
  messageBuf[1] = HMC5843_REGISTER_MEASMODE;
  messageBuf[2] = HMC5843_MEASMODE_CONT;
  TWI_Start_Transceiver_With_Data(messageBuf, 3);
}


void init_adxl345() {
  unsigned int read = 0;

  /* put in standby mode while we change fifo control bits */
  messageBuf[0] = ADXL345_ADDRESS | FALSE<<TWI_READ_BIT;
  messageBuf[1] = ADXL_REGISTER_PWRCTL;
  messageBuf[2] = ADXL_PWRCTL_STBY;
  TWI_Start_Transceiver_With_Data(messageBuf, 3);

  /* set the fifo mode to stream */
  messageBuf[0] = ADXL345_ADDRESS | FALSE<<TWI_READ_BIT;
  messageBuf[1] = ADXL_REGISTER_FIFOCTL;
  messageBuf[2] = ADXL_FIFOCTL_STREAM;
  TWI_Start_Transceiver_With_Data(messageBuf,3);


  /* set to measure mode */
  messageBuf[0] = ADXL345_ADDRESS | FALSE<<TWI_READ_BIT;
  messageBuf[1] = ADXL_REGISTER_PWRCTL;
  messageBuf[2] = ADXL_PWRCTL_MEASURE;
  TWI_Start_Transceiver_With_Data(messageBuf, 3);

    /* set the data format */
  messageBuf[0] = ADXL345_ADDRESS | FALSE<<TWI_READ_BIT;
  messageBuf[1] = ADXL_REGISTER_DATAFMT;
  messageBuf[2] = ADXL_DATAFMT_4G;
  TWI_Start_Transceiver_With_Data(messageBuf,3);
}


/* prints out data received from the adxl345 */
void Task_Accelorometer()
{
  while(1){
    messageBuf[0] = (ADXL345_ADDRESS) | (FALSE<<TWI_READ_BIT);
    messageBuf[1] = ADXL345_REGISTER_XLSB;
    TWI_Start_Transceiver_With_Data(messageBuf, 2);

    /* Read 7 bytes from the data registers, starting with the LSB of X */
    messageBuf[0] = (ADXL345_ADDRESS) | (TRUE<<TWI_READ_BIT);
    /* set the rest of the messagebuf to 0, probably not necessary */
    messageBuf[1] = 0;
    messageBuf[2] = 0;
    messageBuf[3] = 0;
    messageBuf[4] = 0;
    messageBuf[5] = 0;
    messageBuf[6] = 0; 
    TWI_Start_Transceiver_With_Data(messageBuf, 7);
    /* if successful, print data received */
    /* data comes in like so: x1, x0, y1, y0, z1, z0, sign extended format */
    /* the 0th byte of every axis represents its sign */
    if (TWI_Get_Data_From_Transceiver(messageBuf, 7)){ 
      printf("ADXL: x0:%x x1:%x y0:%x y1:%x z0:%x z1:%x \r\n",messageBuf[2],messageBuf[1],messageBuf[4],messageBuf[3],messageBuf[6],messageBuf[5]);
    }
    nrk_wait_until_next_period();
  }
}


void Task_Gyro()
{
  while(1){
    messageBuf[0] = (ITG3200_ADDRESS) | (FALSE<<TWI_READ_BIT);
    messageBuf[1] = ITG3200_REGISTER_XMSB;
    TWI_Start_Transceiver_With_Data(messageBuf, 2);

    /* Read first byte */
    messageBuf[0] = (ITG3200_ADDRESS) | (TRUE<<TWI_READ_BIT);
    messageBuf[1] = 0;
    messageBuf[2] = 0;
    messageBuf[3] = 0;
    messageBuf[4] = 0;
    messageBuf[5] = 0;
    messageBuf[6] = 0; 
    TWI_Start_Transceiver_With_Data(messageBuf, 7);
    if (TWI_Get_Data_From_Transceiver(messageBuf, 7)){
      printf("ITG: %x %x %x %x %x %x \r\n",messageBuf[1],messageBuf[2],messageBuf[3],messageBuf[4],messageBuf[5],messageBuf[6]);
    }
    nrk_wait_until_next_period();
  }
}

void Task_Magneto()
{
  while(1){
    messageBuf[0] = (HMC5843_ADDRESS) | (FALSE<<TWI_READ_BIT);
    messageBuf[1] = HMC5843_REGISTER_XMSB;
    TWI_Start_Transceiver_With_Data(messageBuf, 2);

    /* Read first byte */
    messageBuf[0] = (HMC5843_ADDRESS) | (TRUE<<TWI_READ_BIT);
    messageBuf[1] = 0;
    messageBuf[2] = 0;
    messageBuf[3] = 0;
    messageBuf[4] = 0;
    messageBuf[5] = 0;
    messageBuf[6] = 0; 
    TWI_Start_Transceiver_With_Data(messageBuf, 7);
    if (TWI_Get_Data_From_Transceiver(messageBuf, 7)){
      printf("HMC: %x %x %x %x %x %x \r\n",messageBuf[1],messageBuf[2],messageBuf[3],messageBuf[4],messageBuf[5],messageBuf[6]);
    }
    nrk_wait_until_next_period();
  }
}



void
nrk_create_taskset()
{
  nrk_task_set_entry_function( &TaskOne, Task_Accelorometer);
  nrk_task_set_stk( &TaskOne, Stack1, NRK_APP_STACKSIZE);
  TaskOne.prio = 1;
  TaskOne.FirstActivation = TRUE;
  TaskOne.Type = BASIC_TASK;
  TaskOne.SchType = PREEMPTIVE;
  TaskOne.period.secs = 0;
  TaskOne.period.nano_secs = 250*NANOS_PER_MS;
  TaskOne.cpu_reserve.secs = 0;
  TaskOne.cpu_reserve.nano_secs = 250*NANOS_PER_MS;
  TaskOne.offset.secs = 1;
  TaskOne.offset.nano_secs= 0;
  nrk_activate_task (&TaskOne);

  nrk_task_set_entry_function( &TaskTwo, Task_Gyro);
  nrk_task_set_stk( &TaskTwo, Stack2, NRK_APP_STACKSIZE);
  TaskTwo.prio = 2;
  TaskTwo.FirstActivation = TRUE;
  TaskTwo.Type = BASIC_TASK;
  TaskTwo.SchType = PREEMPTIVE;
  TaskTwo.period.secs = 0;
  TaskTwo.period.nano_secs = 250*NANOS_PER_MS;
  TaskTwo.cpu_reserve.secs = 0;
  TaskTwo.cpu_reserve.nano_secs = 250*NANOS_PER_MS;
  TaskTwo.offset.secs = 1;
  TaskTwo.offset.nano_secs= 0;
  nrk_activate_task (&TaskTwo);

  nrk_task_set_entry_function( &TaskThree, Task_Magneto);
  nrk_task_set_stk( &TaskThree, Stack3, NRK_APP_STACKSIZE);
  TaskThree.prio = 3;
  TaskThree.FirstActivation = TRUE;
  TaskThree.Type = BASIC_TASK;
  TaskThree.SchType = PREEMPTIVE;
  TaskThree.period.secs = 0;
  TaskThree.period.nano_secs = 250*NANOS_PER_MS;
  TaskThree.cpu_reserve.secs = 0;
  TaskThree.cpu_reserve.nano_secs = 250*NANOS_PER_MS;
  TaskThree.offset.secs = 1;
  TaskThree.offset.nano_secs= 0;
  nrk_activate_task (&TaskThree);
}



