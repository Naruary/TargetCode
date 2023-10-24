#include <intrinsics.h>
#include <stm32f4xx.h>
#include <stdbool.h>
#include "portable.h"
#include "board.h"
#include "ModemDriver.h"

//============================================================================//
// DATA DEFINITIONS //
//============================================================================//

static BOOL bModemIsPresent = false;

//============================================================================//
// FUNCTION IMPLEMENTATIONS //
//============================================================================//

void ModemDriver_InitPins(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_StructInit(&GPIO_InitStructure);

    // Modem RESET pin set up parameters.
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;

    // GPIO PORTA Pin4: This is the Yitran Modem Hardware Reset
    GPIO_InitStructure.GPIO_Pin  = MODEM_RESET_PIN;
    GPIO_Init(MODEM_RESET_PORT, &GPIO_InitStructure);

    // GPIO PORTA Pin11: This is the Yitran Modem Power control
    GPIO_InitStructure.GPIO_Pin  = MODEM_POWER_PIN;
    GPIO_Init(MODEM_POWER_PORT, &GPIO_InitStructure);

    // Initialize the modem power to OFF (GND)
    GPIO_ResetBits(MODEM_POWER_PORT, MODEM_POWER_PIN);

    ModemDriver_Power(true); // Turn off the modem initially
}

void ModemDriver_PutInHardwareReset(BOOL bState)
{
    GPIO_WriteBit(MODEM_RESET_PORT, MODEM_RESET_PIN, (bState ? Bit_RESET : Bit_SET));
}

void SetModemIsPresent(BOOL bState)
{
    bModemIsPresent = bState;
}

BOOL GetModemIsPresent(void)
{
    return bModemIsPresent;
}

void ModemDriver_Power(BOOL bState)
{
    if (bState) {
        // Turn on the modem (set the modem power pin high)
        GPIO_SetBits(MODEM_POWER_PORT, MODEM_POWER_PIN);
    } else {
        // Turn off the modem (set the modem power pin low)
        GPIO_ResetBits(MODEM_POWER_PORT, MODEM_POWER_PIN);
    }
}
