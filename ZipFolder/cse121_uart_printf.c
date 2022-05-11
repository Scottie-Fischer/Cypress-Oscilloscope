/* ================================================
UART functions for printing to terminal through USB
 * =================================================
*/

#include "project.h"

/* Initialize UART */

void init_uart_printf(void){
 Cy_SCB_UART_Init(UART_printf_HW, &UART_printf_config, &UART_printf_context);
 Cy_SCB_UART_Enable(UART_printf_HW);
}

/* Print string */

void uart_printf(char *print_string){
  Cy_SCB_UART_PutString(UART_printf_HW, print_string);
}
