
#include "guix_base_thread.h"
#include "lcd.h"
#include "guix_base_project_resources.h"

#define LCDTEST 0
#define GUIX 1
#define TOUCH 1

#if(GUIX)
#include "gx_api.h"

GX_WINDOW_ROOT * p_window_root;
GX_WINDOW p_splash_screen;
#endif

void g_lcd_spi_callback(spi_callback_args_t * p_args);

#if (TOUCH)
static void guix_test_send_touch_message(sf_touch_panel_payload_t * p_payload);
#endif

void g_lcd_spi_callback(spi_callback_args_t * p_args)
{
    SSP_PARAMETER_NOT_USED(p_args);
    tx_semaphore_ceiling_put(&g_gui_semaphore, 1);
}

#if (LCDTEST)
#define RED_    0xf800
#define BLUE_   0x001f
#define GREEN_  0x07e0

#define WHITE   0xFFFF
#define BLACK   0x0000

#define DISPLAY_WIDTH           240
#define DISPLAY_HEIGHT          320
#define DISPLAY_STRIDE          256

static void test_LCD(void);

static void test_LCD(void)
{
    uint16_t * display_frame_buffer = (uint16_t *)g_display0_runtime_cfg_bg.input.p_base;
    int x,y;

        /* Fill display buffer with color bars */
    for (y = 0; y < DISPLAY_HEIGHT; y++)
    {
        for (x = 0; x < DISPLAY_WIDTH/3; x++)
        {
            *display_frame_buffer++ = RED_;
        }
        for (x = 0; x < DISPLAY_WIDTH/3; x++)
        {
            *display_frame_buffer++ = GREEN_;
        }
        for (x = 0; x < DISPLAY_WIDTH/3; x++)
        {
            *display_frame_buffer++ = BLUE_;
        }
        display_frame_buffer += DISPLAY_STRIDE-DISPLAY_WIDTH;
    }

}
#endif

void guix_base_thread_entry(void);

/* GUIX Base Thread entry function */
void guix_base_thread_entry(void)
{
    ssp_err_t err;
#if(GUIX)
#if (TOUCH)

    sf_message_header_t * p_message = NULL;
#endif
    UINT      status = TX_SUCCESS;
    /* Initializes GUIX. */
    status = gx_system_initialize();
    if(TX_SUCCESS != status)
    {
        while(1); //Debug trap
    }

    /* Initializes GUIX drivers. */
    err = g_sf_el_gx0.p_api->open (g_sf_el_gx0.p_ctrl, g_sf_el_gx0.p_cfg);
    if(SSP_SUCCESS != err)
    {
        while(1); //Debug trap
    }

    gx_studio_display_configure ( MAIN_DISPLAY,
                                  g_sf_el_gx0.p_api->setup,
                                  LANGUAGE_ENGLISH,
                                  MAIN_DISPLAY_THEME_1,
                                  &p_window_root );

    err = g_sf_el_gx0.p_api->canvasInit(g_sf_el_gx0.p_ctrl, p_window_root);
    if(SSP_SUCCESS != err)
    {
        while(1); //Debug trap
    }

    /* Creates the primary screen. */
    status = gx_studio_named_widget_create("splash_screen", (GX_WIDGET *)p_window_root, (GX_WIDGET **) &p_splash_screen);
    if(TX_SUCCESS != status)
    {
        while(1); //Debug trap
    }

    /* Shows the root window to make it visible. */
    status = gx_widget_show(p_window_root);
    if(TX_SUCCESS != status)
    {
        while(1); //Debug trap
    }

    /* Lets GUIX run. */
    status = gx_system_start();
    if(TX_SUCCESS != status)
    {
        while(1); //Debug trap
    }
#endif
    /** Open the SPI driver to initialize the LCD **/
    err = g_rspi_lcd.p_api->open(g_rspi_lcd.p_ctrl, g_rspi_lcd.p_cfg);
    if(SSP_SUCCESS != err)
    {
        while(1); //Debug trap
    }

    /** Setup the ILI9341V **/
    ILI9341V_Init();

    err = g_rspi_lcd.p_api->close(g_rspi_lcd.p_ctrl);
    if(SSP_SUCCESS != err)
    {
        while(1); //Debug trap
    }

#if (LCDTEST)
/* Opens display driver. */
    err = g_display0.p_api->open(g_display0.p_ctrl, g_display0.p_cfg);
    if (err)
    {
        while(1);
    }

    /* Starts displaying. */
    err = g_display0.p_api->start(g_display0.p_ctrl);
    if (err)
    {
        while(1);
    }

    test_LCD();
#endif


    while (1)
    {
#if(TOUCH)
        g_sf_message0.p_api->pend(g_sf_message0.p_ctrl, &guix_base_thread_message_queue, (sf_message_header_t **) &p_message, TX_WAIT_FOREVER);
        switch (p_message->event_b.class_code)
        {
        case SF_MESSAGE_EVENT_CLASS_TOUCH:
            if (SF_MESSAGE_EVENT_NEW_DATA == p_message->event_b.code)
            {
                //sf_touch_panel_payload_t * p_touch_message = (sf_touch_panel_payload_t *) p_message;

                /** Translate a touch event into a GUIX event */
                guix_test_send_touch_message((sf_touch_panel_payload_t *) p_message);
            }
            break;
        default:
            break;
        }

        /** Message is processed, so release buffer. */
        err = g_sf_message0.p_api->bufferRelease(g_sf_message0.p_ctrl, (sf_message_header_t *) p_message, SF_MESSAGE_RELEASE_OPTION_NONE);
        if (err)
        {
            while(1); //Debug trap
        }
        tx_thread_sleep(1);
#endif
    }
}

#if(TOUCH)
/*******************************************************************************************************************//**
 * @brief  Sends a touch event to GUIX internal thread to call the GUIX event handler function
 *
 * @param[in] p_payload Touch panel message payload
***********************************************************************************************************************/
static void guix_test_send_touch_message(sf_touch_panel_payload_t * p_payload)
{
    bool send_event = true;
    GX_EVENT gxe;

    switch (p_payload->event_type)
    {
    case SF_TOUCH_PANEL_EVENT_DOWN:
        gxe.gx_event_type = GX_EVENT_PEN_DOWN;
        break;
    case SF_TOUCH_PANEL_EVENT_UP:
        gxe.gx_event_type = GX_EVENT_PEN_UP;
        break;
    case SF_TOUCH_PANEL_EVENT_HOLD:
    case SF_TOUCH_PANEL_EVENT_MOVE:
        gxe.gx_event_type = GX_EVENT_PEN_DRAG;
        break;
    case SF_TOUCH_PANEL_EVENT_INVALID:
        send_event = false;
        break;
    default:
        break;
    }

    if (send_event)
    {
        /** Send event to GUI */
        gxe.gx_event_sender         = GX_ID_NONE;
        gxe.gx_event_target         = 0;  /* the event to be routed to the widget that has input focus */
        gxe.gx_event_display_handle = 0;

        gxe.gx_event_payload.gx_event_pointdata.gx_point_x = (GX_VALUE)(p_payload->x);
        gxe.gx_event_payload.gx_event_pointdata.gx_point_y = (GX_VALUE)(320 - p_payload->y);

        gx_system_event_send(&gxe);
    }
}
#endif
