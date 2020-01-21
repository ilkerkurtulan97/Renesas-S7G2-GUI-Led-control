/*
 * my_gui_event_handler.c
 *
 *  Created on: Feb 10, 2016
 *      Author: godocha01
 */


//#include "gx_api.h"
#include "guix_base_project_resources.h"
#include "guix_base_project_specifications.h"
#include "hal_data.h"

#define TIME_EVENT_TIMER     (100)
#define SPLASH_EVENT_TIMER 1
#define BUTTON_EVENT 1

/* Splash Screen Event Handler */
UINT SplashScreenEventHandler (GX_WINDOW * widget, GX_EVENT * event_ptr)
{
#if (BUTTON_EVENT)
    bsp_leds_t  pbsp_leds;
    ioport_port_pin_t led1_pin;
    ioport_port_pin_t led2_pin;


    R_BSP_LedsGet(&pbsp_leds);
    led1_pin = pbsp_leds.p_leds[BSP_LED_LED1];
    led2_pin = pbsp_leds.p_leds[BSP_LED_LED2];

#endif
#if (SPLASH_EVENT_TIMER)
    UINT status;
    GX_MULTI_LINE_TEXT_VIEW * my_text_view = &splash_screen.splash_screen_text_view;
#endif
    switch (event_ptr->gx_event_type)
    {
        case GX_EVENT_SHOW:
            gx_system_timer_start(widget, TIME_EVENT_TIMER, 20 * 5, 0);
            return gx_window_event_process(widget, event_ptr);
        break;
#if (SPLASH_EVENT_TIMER)
        case GX_EVENT_TIMER:
            status = gx_multi_line_text_view_text_set(my_text_view, "ILKER KURTULAN GUI LED APPLICATION !!!");
            if (GX_SUCCESS != status)
            {
                while(1);
            }
        break;
#endif
#if (BUTTON_EVENT)
        case GX_SIGNAL(BUTTON1, GX_EVENT_CLICKED):
        {
            ioport_level_t pin_state;

            g_ioport.p_api->pinRead(led1_pin, &pin_state);
            if (IOPORT_LEVEL_LOW == pin_state)

                g_ioport.p_api->pinWrite(led1_pin, IOPORT_LEVEL_HIGH);
            else
                g_ioport.p_api->pinWrite(led1_pin, IOPORT_LEVEL_LOW);
        }
        break;



#endif
        default:
        return gx_window_event_process(widget, event_ptr);
    }
    return 0;
}


