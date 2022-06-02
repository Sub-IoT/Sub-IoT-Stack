#include "hwexternalbusuncoupler.h"

#include "debug.h"
#include "stm32_device.h"
#include "stm32_common_mcu.h"
#include "stm32_common_gpio.h"

typedef struct
{
    pin_id_t pin;
    bool is_active_low;
} priv_uncoupler_handle_t;

static void uncoupler_set(uncoupler_handle_t *handle, bool enable);

uncoupler_driver_t uncoupler_driver = { .uncoupler_set = uncoupler_set};

void uncoupler_init(uncoupler_handle_t *handle, pin_id_t pin, bool is_active_low)
{
    assert(handle != NULL);
    handle->driver = &uncoupler_driver;
    priv_uncoupler_handle_t* phandle = (priv_uncoupler_handle_t*)handle->priv_data;
    phandle->pin = pin;
    phandle->is_active_low = is_active_low;
}

static void uncoupler_set(uncoupler_handle_t *handle, bool enable)
{
    priv_uncoupler_handle_t* phandle = (priv_uncoupler_handle_t*)handle->priv_data;
    GPIO_InitTypeDef GPIO_InitStruct;
    if(enable)
    {
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    }
    else
    {
        GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    }
    error_t err = hw_gpio_configure_pin_stm(phandle->pin, &GPIO_InitStruct);
    assert(err == SUCCESS || err == EALREADY);
    if(enable)
    {
        if(phandle->is_active_low)
        {
            hw_gpio_clr(phandle->pin);
        }
        else
        {
            hw_gpio_set(phandle->pin);
        }
    }
}