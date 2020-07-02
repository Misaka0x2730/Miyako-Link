#define WEAK __attribute__ ((weak))
#define ALIAS(f) __attribute__ ((weak, alias (#f)))

#define STACK_SIZE       0x00000040

__attribute__ ((section(".stack")))
unsigned long stack[STACK_SIZE];

extern void SystemInit(void);
extern void main(void);

static void ResetISR(void);
static void Default_Handler(void);

__attribute__ ((used))
WEAK void NMI_Handler(void);
WEAK void HardFault_Handler(void);
WEAK void SVC_Handler(void);
WEAK void PendSV_Handler(void);
WEAK void SysTick_Handler(void);

void SPI0_IRQHandler(void)      ALIAS(Default_Handler);
void SPI1_IRQHandler(void)      ALIAS(Default_Handler);
void UART0_IRQHandler(void)     ALIAS(Default_Handler);
void UART1_IRQHandler(void)     ALIAS(Default_Handler);
void UART2_IRQHandler(void)     ALIAS(Default_Handler);
void I2C_IRQHandler(void)       ALIAS(Default_Handler);
void SCT_IRQHandler(void)       ALIAS(Default_Handler);
void MRT_IRQHandler(void)       ALIAS(Default_Handler);
void CMP_IRQHandler(void)       ALIAS(Default_Handler);
void WDT_IRQHandler(void)       ALIAS(Default_Handler);
void BOD_IRQHandler(void)       ALIAS(Default_Handler);
void WKT_IRQHandler(void)       ALIAS(Default_Handler);
void PININT0_IRQHandler(void)   ALIAS(Default_Handler);
void PININT1_IRQHandler(void)   ALIAS(Default_Handler);
void PININT2_IRQHandler(void)   ALIAS(Default_Handler);
void PININT3_IRQHandler(void)   ALIAS(Default_Handler);
void PININT4_IRQHandler(void)   ALIAS(Default_Handler);
void PININT5_IRQHandler(void)   ALIAS(Default_Handler);
void PININT6_IRQHandler(void)   ALIAS(Default_Handler);
void PININT7_IRQHandler(void)   ALIAS(Default_Handler);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wincompatible-pointer-types"
__attribute__ ((used, section(".isr_vector")))
void (* const g_pfnVectors[])(void) = {
    &stack[STACK_SIZE],                     // The initial stack pointer
    ResetISR,                               // The reset handler
    NMI_Handler,                            // The NMI handler
    HardFault_Handler,                      // The hard fault handler
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    0,                                      // Reserved
    SVC_Handler,                            // SVCall handler
    0,                                      // Reserved
    0,                                      // Reserved
    PendSV_Handler,                         // The PendSV handler
    SysTick_Handler,                        // The SysTick handler

    // Chip Level - LPC8xx
    SPI0_IRQHandler,                         // SPI0 controller
    SPI1_IRQHandler,                         // SPI1 controller
    0,                                       // Reserved
    UART0_IRQHandler,                        // UART0
    UART1_IRQHandler,                        // UART1
    UART2_IRQHandler,                        // UART2
    0,                                       // Reserved
    0,                                       // Reserved
    I2C_IRQHandler,                          // I2C controller
    SCT_IRQHandler,                          // Smart Counter Timer
    MRT_IRQHandler,                          // Multi-Rate Timer
    CMP_IRQHandler,                          // Comparator
    WDT_IRQHandler,                          // Watchdog
    BOD_IRQHandler,                          // Brown Out Detect
    0,                                       // Reserved
    WKT_IRQHandler,                          // Wakeup timer
    0,                                       // Reserved
    0,                                       // Reserved
    0,                                       // Reserved
    0,                                       // Reserved
    0,                                       // Reserved
    0,                                       // Reserved
    0,                                       // Reserved
    0,                                       // Reserved
    PININT0_IRQHandler,                      // PIO INT0
    PININT1_IRQHandler,                      // PIO INT1
    PININT2_IRQHandler,                      // PIO INT2
    PININT3_IRQHandler,                      // PIO INT3
    PININT4_IRQHandler,                      // PIO INT4
    PININT5_IRQHandler,                      // PIO INT5
    PININT6_IRQHandler,                      // PIO INT6
    PININT7_IRQHandler,                      // PIO INT7
}; /* End of g_pfnVectors */

#pragma GCC diagnostic pop

extern unsigned long _sidata;    /*!< Start address for the initialization
                                      values of the .data section.            */
extern unsigned long _sdata;     /*!< Start address for the .data section     */
extern unsigned long _edata;     /*!< End address for the .data section       */
extern unsigned long _sbss;      /*!< Start address for the .bss section      */
extern unsigned long _ebss;      /*!< End address for the .bss section        */

static void ResetISR(void)
{
    volatile unsigned long *src, *dst;

    for(dst = &_sdata, src = &_sidata; dst < &_edata; ) {
        *(dst++) = *(src++);
    }
  

    for(dst = &_sbss; dst < &_ebss; ) {
        *(dst++) = 0;
    }

    SystemInit();

    main();
}

static void Default_Handler(void)
{
    while(1)
    {
    }
}

void NMI_Handler(void)
{
    while(1)
    {
    }
}

void HardFault_Handler(void)
{
    while(1)
    {
    }
}

void SVC_Handler(void)
{
    while(1)
    {
    }
}

void PendSV_Handler(void)
{
    while(1)
    {
    }
}

void SysTick_Handler(void)
{
    while(1)
    {
    }
}








