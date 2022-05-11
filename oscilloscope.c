/* ========================================
 *
 * Code Written by Scott Fischer
 * 
 * Oscilloscope Project Main File
 *
 * ========================================
*/
#include "project.h"
#include <stdio.h>

#define BUFFER_SIZE 256

void init_uart_printf(void);
void uart_printf(char *print_string);
void lcd_init(void);
void lcd_write(char *data, uint8_t size);
void lcd_cursor(uint8_t row, uint8_t col);
void lcd_clear(void);



int DMA_flag = -1;  //Global Flag where -1 is none. 0 is array1 is ready, 1 is array2 is ready.
//int count = 0;

void DMA_1_ISR(void){
    /* Clear interrupt */
    
    if(DMA_flag == -1 || DMA_flag == 2){
        DMA_flag = 1;
        Cy_SysLib_DelayUs(100);
    }
    else{
        DMA_flag = 2;
        Cy_SysLib_DelayUs(100);
    }
    Cy_DMA_Channel_ClearInterrupt(DMA_1_HW, DMA_1_DW_CHANNEL);
}
//Helper Function to see if we crossed the threshold
int does_cross(uint16_t x, uint16_t y){
    uint16_t avg = 0x400;
    uint16_t threshold = 0x80;
    if((x >= avg && y <= avg) || (x >= avg+threshold && y <= avg)){
        return 1;
    }else{
        return 0;   
    }
}
//Helper Function to print to LCD
void handle_lcd(int freq, int freq_count){
    lcd_cursor(0,1);
    char string[16];
    sprintf(string,"Freq: %03i",freq/freq_count);
    lcd_write(string,sizeof("Freq:    "));
}

int main(void)
{
    //Start of channel configuring
    cy_stc_dma_channel_config_t channelConfig;
    /* Set parameters based on settings of DMA component */
    channelConfig.descriptor = &DMA_1_Descriptor_1;
    /* Start of descriptor chain */
    channelConfig.preemptable = DMA_1_PREEMPTABLE;
    channelConfig.priority = DMA_1_PRIORITY;
    channelConfig.enable = false;
    channelConfig.bufferable = DMA_1_BUFFERABLE;
    //----------------------------------------------------------
    //Init the UART so that we can print to the terminal
    init_uart_printf();
    char init_string[32] = "\n\r----Initialized----\n\r";
    uart_printf(init_string);
    //--------------------------------------------------
    lcd_init();
    lcd_cursor(0,1);
    
    __enable_irq(); /* Enable global interrupts. */ 

    
    //Initialize Destination Address so that it is all 0s
    uint16_t array1[BUFFER_SIZE];
    uint16_t array2[BUFFER_SIZE];
    
    
    //Init DMA--------------------------------
    //Init the Descriptor
    Cy_DMA_Descriptor_Init(&DMA_1_Descriptor_1,&DMA_1_Descriptor_1_config);
    Cy_DMA_Descriptor_SetSrcAddress(&DMA_1_Descriptor_1,(uint32_t *)&(SAR->CHAN_RESULT[0]));
    Cy_DMA_Descriptor_SetDstAddress(&DMA_1_Descriptor_1,array1);
    
    Cy_DMA_Descriptor_Init(&DMA_1_Descriptor_2,&DMA_1_Descriptor_2_config);
    Cy_DMA_Descriptor_SetSrcAddress(&DMA_1_Descriptor_2,(uint32_t *)&(SAR->CHAN_RESULT[0]));
    Cy_DMA_Descriptor_SetDstAddress(&DMA_1_Descriptor_2,array2);
    
    //Init Channel
    Cy_DMA_Channel_Init(DMA_1_HW, DMA_1_DW_CHANNEL,&channelConfig);
    
    //Init the DMA Interrupt
    Cy_SysInt_Init(&DMA_1_int_cfg, DMA_1_ISR);
    NVIC_EnableIRQ(DMA_1_int_cfg.intrSrc);
    
    //Enable the DMA interrupt source
    Cy_DMA_Channel_SetInterruptMask(DMA_1_HW,DMA_1_DW_CHANNEL, CY_DMA_INTR_MASK);
    
    //Enable the Channel then enable the DMA
    Cy_DMA_Channel_Enable(DMA_1_HW, DMA_1_DW_CHANNEL);
    Cy_DMA_Enable(DMA_1_HW);
    //----------------------------------------- 
    
    ADC_1_Start();
    ADC_1_StartConvert();
    
    
    uint16_t avg = 0x400;   //Average voltage
    
    uint16_t prev_val = 0;  //Previous Value of the Array
    uint16_t curr_val = 0;  //Current  Value of the Array
    uint64_t time = 0;           //Tells us the time difference between threshold crossings
    uint64_t freq = 0;           //Tells us the combined times so that we can divide by freq_count to get freq
    uint32_t prev_t = 0;
    uint32_t curr_t = 0;
    int loop_count = 0;     //Tells us how many times the descriptors were executed
    int freq_count = 0;     //Tells us how many times we find the frequency
    char s[32];             //Print string for the LCD
    for(;;)
    {
        //We are ready to process Array1
        if(DMA_flag == 1){
            for(int i = 0; i < 256; i++){
                prev_val = curr_val;
                curr_val = array1[i];
                //Look at our window to see if the threshold is crossed
                if(prev_val >= avg && curr_val <= avg){
                    //Crossed threshold
                    freq += 1000/time;
                    freq_count++;
                    //Reset Time
                    if(freq/freq_count > 45){
                        time = 2;
                    }
                    else{time = 0;}
                }
                //Threshhold not crossed so just inc time
                else{
                    time++;
                }
            }
            DMA_flag = 3;   //Clearing Flag
            loop_count++;
        }
        //We are ready to process Array2
        else if(DMA_flag == 2){
            for(int i = 0; i < 256; i++){
                prev_val = curr_val;
                curr_val = array2[i];
                //Look at our window to see if the threshold is crossed
                if(prev_val >= avg && curr_val <= avg){
                    //Crossed threshold
                    freq += 1000/time;
                    freq_count++;
                    if(freq/freq_count > 45){
                        time = 2;
                    }
                    else{time = 0;}
                }
                //Threshhold not crossed so just inc time
                else{
                    time++;
                }
            }
            DMA_flag = -1;  //Clearing Flag
            loop_count++;
        }
        //If we have no array to process then we find the average across many loops
        else if(loop_count >= 3500){
            int f = freq/freq_count;
            if(f >= 1 && f <= 100){
                sprintf(s,"Freq:%03i       ",f);
            }else{
                sprintf(s,"Freq:OutOfRange");
            }
            lcd_cursor(0,0);
            lcd_write(s,sizeof("Freq:OutOfRange"));
            loop_count = 0;
            freq = 0;
            freq_count = 0;
        }
       
    }
    
}

/* [] END OF FILE */

