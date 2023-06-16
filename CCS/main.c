#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include <stdbool.h>

#include "main.h"
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "inc/hw_gpio.h"
#include "inc/hw_adc.h"
#include "inc/hw_udma.h"
#include "driverlib/adc.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/fpu.h"
#include "driverlib/pin_map.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/systick.h"
#include "driverlib/timer.h"
#include "driverlib/udma.h"
#include "driverlib/ssi.h"

#define RED_LED   GPIO_PIN_1
#define BLUE_LED  GPIO_PIN_2
#define GREEN_LED GPIO_PIN_3


#define     Set_Bit(val, bitn)      (val |= (/*1 <<*/(bitn)))   // Set the bit as 1
#define     Clr_Bit(val, bitn)      (val &= ~(/*1<<*/(bitn)))   // Clear the bit as 0
#define     Get_Bit(val, bitn)      (val & (1<<(bitn)) )        // get the bit value
#define     LED_PIN_NUM         (GREEN_LED)
#define     LED_IO_INIT                     (Set_Bit(PF->DIR,LED_PIN_NUM))
#define     LED_Set()                       (Set_Bit(PF->DATA,LED_PIN_NUM))
#define     LED_Clr()                       (Clr_Bit(PF->DATA,LED_PIN_NUM))

//*****************************************************************************
//
// Definition for ADC buffer size.
//
//*****************************************************************************
#define ADC_SAMPLE_BUF_SIZE     1

//*****************************************************************************
//
// Definition for ADC Sampling Frequency.
//
//*****************************************************************************
#define ADC_SAMPLING_FREQ       100000

// must less than sysclock / 12 = 6.7M
#define SSI2_BitRate        (10000*100)

//*****************************************************************************
//
// The control table used by the uDMA controller.  This table must be aligned
// to a 1024 byte boundary.
//
//*****************************************************************************
#if defined(ewarm)
#pragma data_alignment=1024
uint8_t pui8ControlTable[1024];
#elif defined(ccs)
#pragma DATA_ALIGN(pui8ControlTable, 1024)
uint8_t pui8ControlTable[1024];
#else
uint8_t pui8ControlTable[1024] __attribute__ ((aligned(1024)));
#endif

//*****************************************************************************
//
// Global buffers to store ADC sample data.
//
//*****************************************************************************
uint16_t pui16ADCBuffer1[ADC_SAMPLE_BUF_SIZE];
uint16_t pui16ADCBuffer2[ADC_SAMPLE_BUF_SIZE];
uint16_t pui16ADCBuffer1_buf;
uint16_t pui16ADCBuffer2_buf;
uint16_t spi_buf[16];
uint8_t spi_buf_ptr1 = 0;
uint8_t spi_buf_ptr2 = 0;

//*****************************************************************************
//
// Each possible state of the fill status for an ADC buffer.
//
//*****************************************************************************
enum BUFFER_STATUS
{
    EMPTY,
    FILLING,
    FULL
};

//*****************************************************************************
//
// Global variable to keep track of the fill status of each ADC buffer.
//
//*****************************************************************************
static uint8_t pui32BufferStatus;

#define ISEMPTY(n) (((pui32BufferStatus>>(n*2))&0x3)==EMPTY)
#define ISFILLING(n) (((pui32BufferStatus>>(n*2))&0x3)==FILLING)
#define ISFULL(n) (((pui32BufferStatus>>(n*2))&0x3)==FULL)
#define SETEMPTY(n) (pui32BufferStatus=((pui32BufferStatus&~(3<<(2*n))) | (EMPTY<<(2*n))))
#define SETFILLING(n) (pui32BufferStatus=((pui32BufferStatus&~(3<<(2*n))) | (FILLING<<(2*n))))
#define SETFULL(n) (pui32BufferStatus=((pui32BufferStatus&~(3<<(2*n))) | (FULL<<(2*n))))

uint16_t tag = 0;
//*****************************************************************************
//
// Interrupt handler for ADC0 Sequence Zero.
//
//*****************************************************************************
void
ADCSeq0Handler(void)
{
    uint32_t j;
    //
    // Clear the Interrupt Flag.
    //
    ADCIntClear(ADC0_BASE, 0);

    //
    // Determine which buffer as been filled based on the UDMA_MODE_STOP flag
    // and update the buffer status.
    //
    if ((uDMAChannelModeGet(UDMA_CHANNEL_ADC0 | UDMA_PRI_SELECT) ==
            UDMA_MODE_STOP) && ISFILLING(0))
    {
        pui32BufferStatus = FULL;
//        pui32BufferStatus = (FILLING << 2) | (FULL);
//        pui16ADCBuffer1_buf = pui16ADCBuffer1[0] & 0xfff | tag & 0xf000;

//        spi_buf[spi_buf_ptr2] = pui16ADCBuffer1[0] | tag;
//        tag += 0x0001;
//        uint8_t spi_buf_ptr2_tmp = spi_buf_ptr2 + 1;
//        spi_buf_ptr2_tmp &= 0xf;
//        // not full
//        if (spi_buf_ptr2_tmp != spi_buf_ptr1) {
//            spi_buf_ptr2 = spi_buf_ptr2_tmp;
//        }

//        SSIDataPutNonBlocking(SSI2_BASE, pui16ADCBuffer1[0]|tag);
//        tag += 0x1000;
//        tag &= 0x7000;
//        SSIDataPutNonBlocking(SSI2_BASE, pui16ADCBuffer1[1]|tag);
//        tag += 0x1000;
//        if(!SSIBusy(SSI2_BASE))
//        SSIDataPut(SSI2_BASE, tag++);
//        SSIDataPutNonBlocking(SSI2_BASE, tag++);
//        SSIDataPut(SSI2_BASE, 0x1234);
    }
//    else if ((uDMAChannelModeGet(UDMA_CHANNEL_ADC0 | UDMA_ALT_SELECT) ==
//            UDMA_MODE_STOP) && ISFILLING(1))
//    {
//        pui32BufferStatus = (FULL << 2) | (FILLING);
//        pui16ADCBuffer2_buf = pui16ADCBuffer2[0] & 0xfff | tag & 0xf000;

//        spi_buf[spi_buf_ptr2] = pui16ADCBuffer2[0] | tag;
//        tag += 0x0001;
//        uint8_t spi_buf_ptr2_tmp = spi_buf_ptr2 + 1;
//        spi_buf_ptr2_tmp &= 0xf;
//        // not full
//        if (spi_buf_ptr2_tmp != spi_buf_ptr1) {
//            spi_buf_ptr2 = spi_buf_ptr2_tmp;
//        }

//        SSIDataPutNonBlocking(SSI2_BASE, pui16ADCBuffer2[0]|tag);
//        tag += 0x1000;
//        tag &= 0x7000;
//        SSIDataPutNonBlocking(SSI2_BASE, pui16ADCBuffer2[1]|tag);
//        tag += 0x1000;
//        if(!SSIBusy(SSI2_BASE))
//        SSIDataPut(SSI2_BASE, tag++);
//        SSIDataPutNonBlocking(SSI2_BASE, tag++);
//        SSIDataPut(SSI2_BASE, 0x5678);
//    }
}

// 该中断未启用，只是懒得删
void SPI2IntHandler(void) {
    unsigned long ulStatus;
    uint32_t j;

    //
    // Read interrupt status.
    //
    ulStatus = SSIIntStatus(SSI2_BASE, 1);

    //
    // Check the reason for the interrupt.
    //
    if (ulStatus & SSI_TXFF)
    {
        // 将数据放到缓冲区，注意若主机再次获取数据时没有新数据放到缓冲区，则主机会获取到旧数据
//        SSIDataPut(SSI2_BASE, spi_buf[spi_buf_ptr1]);
//        SSIDataPut(SSI2_BASE, tag);
//        while(SSIBusy(SSI2_BASE));
//        SSIDataGetNonBlocking(SSI2_BASE, &j);
//        while(SSIBusy(SSI2_BASE));
//        // not empty
//        if (spi_buf_ptr1 != spi_buf_ptr2) {
//            spi_buf_ptr1++;
//            spi_buf_ptr1 &= 0xf;
//        } else {
//            Set_Bit(PF->DATA,GPIO_PIN_1);
//        }
//        if (ISFULL(0)) {
//            SSIDataPut(SSI2_BASE, pui16ADCBuffer1_buf);
//            while(SSIBusy(SSI2_BASE));
//            SSIDataGet(SSI2_BASE, &j);
//            while(SSIBusy(SSI2_BASE));
//        } else if (ISFULL(1)) {
//            SSIDataPut(SSI2_BASE, pui16ADCBuffer2_buf);
//            while(SSIBusy(SSI2_BASE));
//            SSIDataGet(SSI2_BASE, &j);
//            while(SSIBusy(SSI2_BASE));
//        }
    }

    //
    // Clear interrupts.
    //
    SSIIntClear(SSI2_BASE, ulStatus);
}

void
ConfigureDMA(void) {
    //
    // Enable the uDMA controller.
    //
    uDMAEnable();

    //
    // Point at the control table to use for channel control structures.
    //
    uDMAControlBaseSet(pui8ControlTable);

    //
    // Put the attributes in a known state for the uDMA ADC0 channel.  These
    // should already be disabled by default.
    //
    uDMAChannelAttributeDisable(UDMA_CHANNEL_ADC0,
                                UDMA_ATTR_ALTSELECT | UDMA_ATTR_HIGH_PRIORITY |
                                UDMA_ATTR_REQMASK);
//    uDMAChannelAttributeDisable(UDMA_CHANNEL_SSI0TX,
//                                UDMA_ATTR_ALTSELECT | UDMA_ATTR_HIGH_PRIORITY |
//                                UDMA_ATTR_REQMASK);

    //
    // Configure the control parameters for the primary control structure for
    // the ADC0 channel.  The primary control structure is used for the "A"
    // part of the ping-pong receive.  The transfer data size is 16 bits, the
    // source address does not increment since it will be reading from a
    // register.  The destination address increment is 16-bits.  The
    // arbitration size is set to one byte transfers.
    //
    uDMAChannelControlSet(UDMA_CHANNEL_ADC0 | UDMA_PRI_SELECT, UDMA_SIZE_16 |
                          UDMA_SRC_INC_NONE | UDMA_DST_INC_16 | UDMA_ARB_1);
//    uDMAChannelControlSet(UDMA_CHANNEL_SSI0TX | UDMA_PRI_SELECT, UDMA_SIZE_16 |
//                          UDMA_SRC_INC_16 | UDMA_DST_INC_NONE | UDMA_ARB_1);

    //
    // Configure the control parameters for the alternate control structure for
    // the ADC0 channel.  The alternate control structure is used for the
    // "B" part of the ping-pong receive.  The configuration is identical to
    // the primary/A control structure.
    //
//    uDMAChannelControlSet(UDMA_CHANNEL_ADC0 | UDMA_ALT_SELECT, UDMA_SIZE_16 |
//                          UDMA_SRC_INC_NONE | UDMA_DST_INC_16 | UDMA_ARB_1);
//    uDMAChannelControlSet(UDMA_CHANNEL_SSI0TX | UDMA_ALT_SELECT, UDMA_SIZE_16 |
//                          UDMA_SRC_INC_16 | UDMA_DST_INC_NONE | UDMA_ARB_1);

    //
    // Set up the transfer parameters for the ADC0 primary control structure
    // The mode is set to ping-pong, the transfer source is the ADC Sample
    // Sequence Result FIFO 0 register, and the destination is the receive
    // "A" buffer.  The transfer size is set to match the size of the buffer.
    //
    uDMAChannelTransferSet(UDMA_CHANNEL_ADC0 | UDMA_PRI_SELECT,
                           UDMA_MODE_BASIC,
                           (void *)(ADC0_BASE + ADC_O_SSFIFO0),
                           &pui16ADCBuffer1, ADC_SAMPLE_BUF_SIZE);
//    uDMAChannelTransferSet(UDMA_CHANNEL_ADC0 | UDMA_PRI_SELECT,
//                           UDMA_MODE_PINGPONG,&pui16ADCBuffer1,
//                           (void *)(SSI2_BASE + ),
//                            ADC_SAMPLE_BUF_SIZE);

    //
    // Set up the transfer parameters for the ADC0 primary control structure
    // The mode is set to ping-pong, the transfer source is the ADC Sample
    // Sequence Result FIFO 0 register, and the destination is the receive
    // "B" buffer.  The transfer size is set to match the size of the buffer.
    //
//    uDMAChannelTransferSet(UDMA_CHANNEL_ADC0 | UDMA_ALT_SELECT,
//                           UDMA_MODE_PINGPONG,
//                           (void *)(ADC0_BASE + ADC_O_SSFIFO0),
//                           &pui16ADCBuffer2, ADC_SAMPLE_BUF_SIZE);

    //
    // Set the USEBURST attribute for the uDMA ADC0 channel.  This will force
    // the controller to always use a burst when transferring data from the
    // TX buffer to the UART.  This is somewhat more efficient bus usage than
    // the default which allows single or burst transfers.
    //
    uDMAChannelAttributeEnable(UDMA_CHANNEL_ADC0, UDMA_ATTR_USEBURST);

    //
    // Enables DMA channel so it can perform transfers.  As soon as the
    // channels are enabled, the peripheral will issue a transfer request and
    // the data transfers will begin.
    //
    uDMAChannelEnable(UDMA_CHANNEL_ADC0);

}

void
ConfigureADC() {
    //
    // Use ADC0 sequence 0 to sample channel 3 once for each timer period.
    //
    // TODO
    ADCClockConfigSet(ADC0_BASE, ADC_CLOCK_SRC_PIOSC | ADC_CLOCK_RATE_FULL, 1);

    //
    // Wait for the clock configuration to set.
    //
    SysCtlDelay(10);

    //
    // Disable the ADC0 sequence 0 interrupt on the processor (NVIC).
    //
    IntDisable(INT_ADC0SS0);

    //
    // Disable interrupts for ADC0 sample sequence 0 to configure it.
    //
    ADCIntDisable(ADC0_BASE, 0);

    //
    // Disable ADC0 sample sequence 0.  With the sequence disabled, it is now
    // safe to load the new configuration parameters.
    //
    ADCSequenceDisable(ADC0_BASE, 0);

    //
    // Enable sample sequence 0 with a processor signal trigger.  Sequence 0
    // will do a single sample when the processor sends a signal to start the
    // conversion.
    //
    ADCSequenceConfigure(ADC0_BASE, 0, ADC_TRIGGER_TIMER, 0);

    //
    // Configure step 0 on sequence 0.  Sample channel 3 (ADC_CTL_CH3) in
    // single-ended mode (default) and configure the interrupt flag
    // (ADC_CTL_IE) to be set when the sample is done.  Tell the ADC logic
    // that this is the last conversion on sequence 0 (ADC_CTL_END).  Sequence
    // 0 has 8 programmable steps.  Since we are only doing a single conversion
    // using sequence 0 we will only configure step 0.  For more information
    // on the ADC sequences and steps, reference the datasheet.
    //
    ADCSequenceStepConfigure(ADC0_BASE, 0, 0, ADC_CTL_CH3 | ADC_CTL_END |
                             ADC_CTL_IE);

    //
    // Since sample sequence 0 is now configured, it must be enabled.
    //
    ADCSequenceEnable(ADC0_BASE, 0);

    //
    // Clear the interrupt status flag.  This is done to make sure the
    // interrupt flag is cleared before we sample.
    //
    ADCIntClear(ADC0_BASE, 0);

    //
    // Enables the DMA channel for the ADC0 sample sequence 0.
    //
    ADCSequenceDMAEnable(ADC0_BASE, 0);

    //
    // Enable the ADC 0 sample sequence 0 interrupt.
    //
    ADCIntEnable(ADC0_BASE, 0);

    //
    // Enable the interrupt for ADC0 sequence 0 on the processor (NVIC).
    //
    IntEnable(INT_ADC0SS0);

    //
    // Configure a 32-bit periodic timer.
    //
    TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);

    //
    // Set ADC sampling frequency to be 16KHz i.e. every 62.5uS.
    //
    TimerLoadSet(TIMER0_BASE, TIMER_A, (SysCtlClockGet()/ADC_SAMPLING_FREQ) - 1);

    //
    // Enable the ADC trigger output for Timer A.
    //
    TimerControlTrigger(TIMER0_BASE, TIMER_A, true);

    //
    // Enable processor interrupts.
    //
    IntMasterEnable();

    //
    // Enable Timer 0 which will start the whole application process.
    //
    TimerEnable(TIMER0_BASE, TIMER_A);
}

void spi_init()     //SPI1初始化
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI2);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_SSI2));
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOB));

    GPIOPinConfigure(GPIO_PB4_SSI2CLK);
    GPIOPinConfigure(GPIO_PB5_SSI2FSS);
    GPIOPinConfigure(GPIO_PB6_SSI2RX);
    GPIOPinConfigure(GPIO_PB7_SSI2TX);

    GPIOPinTypeSSI(GPIO_PORTB_BASE,  GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7);

    SSIConfigSetExpClk(SSI2_BASE, SysCtlClockGet(), SSI_FRF_MOTO_MODE_0, SSI_MODE_SLAVE, SSI2_BitRate, 16);
    SSIEnable(SSI2_BASE);
}

inline uint16_t div20(uint16_t x) {
    return x / 20;
}

/**
 * main.c
 */
int main(void)
{
    //
    // Set the system clock to run from the PLL at 80MHz.
    //
    SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN |
                       SYSCTL_XTAL_16MHZ);
    // 400 / 2 / 2.5 = 80MHz
    uint32_t clk = SysCtlClockGet();

    //
    // Initialize the buffer status.
    //
    SETFILLING(0);
    SETEMPTY(1);

    //
    // Enable the peripherals used by this application.
    //
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_UDMA);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);

    //
    // Enable the GPIO pin for ADC0 Channel 3 (PE0) which configures it for
    // analog functionality.
    //
    MAP_GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_0);

    ConfigureDMA();

    ConfigureADC();



//    SysCtlClockSet(SYSCTL_SYSDIV_2|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);
//    // Enable the GPIO port that is used for the on-board LED.
//    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
//    // Check if the peripheral access is enabled.
//    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA));
//    // Enable 1.3“ oled GPIOs for output
//    GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF));
    // Configure the GPIO port for the LED operation.
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, RED_LED|BLUE_LED|GREEN_LED);

    spi_init();
    // initial OLED controller
//    OLED_Init();
    LED_IO_INIT;
    LED_Clr();


    int ui32Count;
    uint32_t j;

    while(1) {
        //
        // Check if the first buffer is full, if so process data.
        //
        if(ISFULL(0))
        {
            SETFILLING(0);
//            if (ISEMPTY(1)) {
//                SETFILLING(0);
//            } else {
//                SETEMPTY(0);
//            }

//            OLED_Clear();
            for(ui32Count = 0; ui32Count < ADC_SAMPLE_BUF_SIZE; ui32Count++)
            {
                SSIDataPut(SSI2_BASE, pui16ADCBuffer1[0]|tag);
                tag += 0x1000;
//                SSIDataPut(SSI2_BASE, tag++);
//                SSIDataPut(SSI2_BASE, spi_buf[spi_buf_ptr1]);
//                while(SSIBusy(SSI2_BASE));
//                SSIDataGetNonBlocking(SSI2_BASE, &j);
//                while(SSIBusy(SSI2_BASE));
//                // not empty
//                if (spi_buf_ptr1 != spi_buf_ptr2) {
//                    spi_buf_ptr1++;
//                    spi_buf_ptr1 &= 0xf;
//                } else {
//                    Set_Bit(PF->DATA,GPIO_PIN_1);
//                }

//                OLED_Draw_horizontal(div20(pui16ADCBuffer1[ui32Count]));
                if((pui16ADCBuffer1[ui32Count]&0xfff) > 600) {
                    LED_Set();
                } else {
                    LED_Clr();
                }
//                OLED_ShowNum(0,ui32Count * 2,pui16ADCBuffer1[ui32Count],4,16);
////                OLED_add_a_wave_point(pui16ADCBuffer1[ui32Count] / 20.315);
////                pui16ADCBuffer1[ui32Count] = 0;
            }
//            OLED_Draw_seq(pui16ADCBuffer1, ADC_SAMPLE_BUF_SIZE, div20);
//            spi_write_buf(0, zeros, sizeof(pui16ADCBuffer1));


            //
            // Enable for another uDMA block transfer.
            //
            uDMAChannelTransferSet(UDMA_CHANNEL_ADC0 | UDMA_PRI_SELECT,
                                   UDMA_MODE_BASIC,
                                   (void *)(ADC0_BASE + ADC_O_SSFIFO0),
                                   &pui16ADCBuffer1, ADC_SAMPLE_BUF_SIZE);
            //
            // Enable DMA channel so it can perform transfers.
            //
            uDMAChannelEnable(UDMA_CHANNEL_ADC0 | UDMA_PRI_SELECT);
        }

        // Check if the second buffer is full, if so process data.
        //
        if(ISFULL(1)&0)
        {
            if (ISEMPTY(0)) {
                SETFILLING(1);
            } else {
                SETEMPTY(1);
            }

//            OLED_Clear();
            for(ui32Count =0; ui32Count < ADC_SAMPLE_BUF_SIZE; ui32Count++)
            {
//                SSIDataPut(SSI2_BASE, spi_buf[spi_buf_ptr1]);
//                while(SSIBusy(SSI2_BASE));
//                SSIDataGetNonBlocking(SSI2_BASE, &j);
//                while(SSIBusy(SSI2_BASE));
//                // not empty
//                if (spi_buf_ptr1 != spi_buf_ptr2) {
//                    spi_buf_ptr1++;
//                    spi_buf_ptr1 &= 0xf;
//                } else {
//                    Set_Bit(PF->DATA,GPIO_PIN_1);
//                }

//                OLED_Draw_horizontal(div20(pui16ADCBuffer2[ui32Count]));
                if((pui16ADCBuffer2[ui32Count]&0xfff) > 600) {
                    LED_Set();
                } else {
                    LED_Clr();
                }
//                OLED_ShowNum(0,ui32Count * 2 + 4,pui16ADCBuffer2[ui32Count],4,16);
////                OLED_add_a_wave_point(pui16ADCBuffer2[ui32Count] / 20.315);
////                pui16ADCBuffer2[ui32Count] = 0;
            }
//            OLED_Draw_seq(pui16ADCBuffer2, ADC_SAMPLE_BUF_SIZE, div20);
//            spi_write_buf(0, zeros, sizeof(pui16ADCBuffer2));

            //
            // Enable for another uDMA block transfer.
            //
            uDMAChannelTransferSet(UDMA_CHANNEL_ADC0 | UDMA_ALT_SELECT,
                                   UDMA_MODE_PINGPONG,
                                   (void *)(ADC0_BASE + ADC_O_SSFIFO0),
                                   &pui16ADCBuffer2, ADC_SAMPLE_BUF_SIZE);

            //
            // Enable DMA channel so it can perform transfers.
            //
            uDMAChannelEnable(UDMA_CHANNEL_ADC0 | UDMA_ALT_SELECT);
        }

    }
}
