/* Copyright (C) 2018 FIBOCOM Technologies Limited and/or its affiliates("FIBOCOM").
 * All rights reserved.
 *
 * This software is supplied "AS IS" without any warranties.
 * FIBOCOM assumes no responsibility or liability for the use of the software,
 * conveys no license or title under any patent, copyright, or mask work
 * right to the product. FIBOCOM reserves the right to make changes in the
 * software without notification.  FIBOCOM also make no representation or
 * warranty that such application will be suitable for the specified use
 * without further testing or modification.
 */

#define OSI_LOG_TAG OSI_MAKE_LOG_TAG('M', 'Y', 'A', 'P')

#include "osi_log.h"
#include "osi_api.h"
#include "fibo_opencpu.h"
#include "drv_keypad.h"

#define _BL_LIGHT_ONE_STEP_NUM              (10)
#define DISP_SQUARE_SIZE                    (10)


static uint8_t g_bl_light = 0;
static void *gp_bl_keep_add_timer = NULL;


static void _prv_bl_keep_add_timer_func(void *arg)
{
    g_bl_light += _BL_LIGHT_ONE_STEP_NUM;
    if (g_bl_light >= 255)
    {
        g_bl_light = 0;
    }
    fibo_sinkLevel_Set(1, g_bl_light);
}

// Callback function used for record only to respond quickly.
static void _prv_key_callback_func(int keyid, int state, void *ctx)
{
    fibo_textTrace("GJH: [%s.%d]key%d,state=%d", __func__, __LINE__, keyid, state);
    /*
        ----------------------------------------------------------------------------------------------------------------
        Case      | Short or long press detection are needed only.
        ----------------------------------------------------------------------------------------------------------------
        Suggestion| User can use the released signal as a key state marker, which means 'KEY_STATE_SHORT_RELEASE' 
                  | and 'KEY_STATE_SHORT_RELEASE' are needed for those scenarios.
        ----------------------------------------------------------------------------------------------------------------
        Example   | key 'SOFT_R': 
                  |     short release: turn on backlight.
                  |     long release : turn off backlight.
        ----------------------------------------------------------------------------------------------------------------
    */ 
    if (keyid == KEY_MAP_SOFT_R)
    {
        if (state == KEY_STATE_SHORT_RELEASE)
        {
            fibo_textTrace("GJH: [%s.%d] turn on backlight.", __func__, __LINE__);
            fibo_sink_OnOff(1, true);
        }
        else if (state == KEY_STATE_LONG_RELEASE)
        {
            fibo_textTrace("GJH: [%s.%d] turn off backlight.", __func__, __LINE__);
            fibo_sink_OnOff(1, false);
        }
    } 
    
    /*
        ----------------------------------------------------------------------------------------------------------------
        Case      | Short press, continue press (while long press kept pressed) scence, such as keeping adding backlight 
                  | light while key keeping pressed.
        ----------------------------------------------------------------------------------------------------------------
        Suggestion| User can use 'KEY_STATE_SHORT_PRESS', 'KEY_STATE_LONG_PRESS' and 'KEY_STATE_SHORT_RELEASE' states 
                    to mark short press and continue press.
        ----------------------------------------------------------------------------------------------------------------
        Example   | key 'OK': 
                  |     short press     : light add.
                  |     long press      : start and keep adding every 500ms 
                  |     long release    : stop adding.
        ----------------------------------------------------------------------------------------------------------------
    */ 
    else if (keyid == KEY_MAP_OK)
    {
        if (state == KEY_STATE_SHORT_PRESS)
        {
            fibo_textTrace("GJH: [%s.%d] lcd backlight light adds once.", __func__, __LINE__);
            g_bl_light += _BL_LIGHT_ONE_STEP_NUM;
            if (g_bl_light >= 255)
            {
                g_bl_light = 0;
            }
            fibo_sinkLevel_Set(1, g_bl_light);
        }
        else if (state == KEY_STATE_LONG_PRESS)
        {
            fibo_textTrace("GJH: [%s.%d] lcd backlight light keeps adding every 500ms.", __func__, __LINE__);
            fibo_timer_start(gp_bl_keep_add_timer, 500, true);

        }
        else if (state == KEY_STATE_LONG_RELEASE)
        {
            fibo_textTrace("GJH: [%s.%d] stop keeping adding.", __func__, __LINE__);
            fibo_timer_stop(gp_bl_keep_add_timer);
        }
    }    
}


static int32_t _prv_init_lcd_and_show_something(void)
{
    int32_t ret = -1;
    uint32_t lcd_devid = 0;
    uint32_t lcd_width = 0;
    uint32_t lcd_height = 0;
    lcdDisplay_t startPoint = {0};

    // fibo_pmu_setlevel(0, 1);    // if LCD IOs' level is 3.2V
    ret = fibo_lcd_init();
    if ((ret != DRV_LCD_SUCCESS) && (ret != DRV_LCD_HAS_INITED))
    {
        goto error;
    }

    ret = fibo_lcd_Getinfo(&lcd_devid, &lcd_width, &lcd_height);
    if (ret != 0)
    {
        goto error;
    }

    ret = fibo_lcd_SetBrushDirection(LCD_DIRECT_NORMAL);
    if (ret != 0)
    {
        goto error;
    }
    
    startPoint.x = 0;
    startPoint.y = 0;
    startPoint.height = lcd_height;
    startPoint.width = lcd_width;
    fibo_lcd_FillRect(&startPoint, BLACK);

    // drawing square
    startPoint.x = 0;
    startPoint.y = 0;
    startPoint.width = DISP_SQUARE_SIZE;
    startPoint.height = DISP_SQUARE_SIZE;
    fibo_lcd_FillRect(&startPoint, RED);
    fibo_taskSleep(100);

    for (uint32_t i = 0; i < 5; i++)
    {
        startPoint.x = startPoint.x + DISP_SQUARE_SIZE;
        startPoint.y = startPoint.y + DISP_SQUARE_SIZE;
        startPoint.width = DISP_SQUARE_SIZE;
        startPoint.height = DISP_SQUARE_SIZE;
        fibo_lcd_FillRect(&startPoint, YELLOW);
        fibo_taskSleep(100);

        startPoint.x = startPoint.x + DISP_SQUARE_SIZE;
        startPoint.y = startPoint.y + DISP_SQUARE_SIZE;
        startPoint.width = DISP_SQUARE_SIZE;
        startPoint.height = DISP_SQUARE_SIZE;
        fibo_lcd_FillRect(&startPoint, BLUE);
        fibo_taskSleep(100);

        startPoint.x = startPoint.x + DISP_SQUARE_SIZE;
        startPoint.y = startPoint.y + DISP_SQUARE_SIZE;
        startPoint.width = DISP_SQUARE_SIZE;
        startPoint.height = DISP_SQUARE_SIZE;
        fibo_lcd_FillRect(&startPoint, RED);
        fibo_taskSleep(100);
    }

    startPoint.x = startPoint.x + DISP_SQUARE_SIZE;
    startPoint.y = startPoint.y + DISP_SQUARE_SIZE;
    startPoint.width = DISP_SQUARE_SIZE;
    startPoint.height = DISP_SQUARE_SIZE;
    fibo_lcd_FillRect(&startPoint, BLUE);
    fibo_taskSleep(100);
    
    fibo_sinkLevel_Set(1, 0);
    fibo_sink_OnOff(1, false); // turn off backlight

    return 0;

error:
    fibo_lcd_deinit();
    return ret;
}

static void hello_world_demo(void *param)
{
    int32_t ret = -1;

    fibo_textTrace("application thread enter, param 0x%x", param);

    /* init lcd and show something */
    ret = _prv_init_lcd_and_show_something();
    if (ret != 0)
    {
        fibo_textTrace("GJH: err: _prv_init_lcd_and_show_something");
        goto error;
    }

    /* create a timer for backlight adding 1 every 500ms */
    gp_bl_keep_add_timer = fibo_timer_create(_prv_bl_keep_add_timer_func, NULL, true);
    if (gp_bl_keep_add_timer == NULL)
    {
        fibo_textTrace("GJH: err: fibo_timer_create");
        goto error;
    }

    /* register key callback, use long press detection function and set 3s as long press timeout. */
    /* NOTE: 
        While param 'long_press_time' < 1s:  don't use long press detection function, key callback will 
                                             report only 'KEY_STATE_PRESS'(while key pressed down) and 
                                             'KEY_STATE_RELEASE' (while key released) event.
        While param 'long_press_time' >= 1s: use long press detection function, and key event will be 
                                             distinguished as short press and long press, so it will 
                                             report 'KEY_STATE_SHORT_PRESS', 'KEY_STATE_SHORT_RELEASE', 
                                             'KEY_STATE_LONG_PRESS',and 'KEY_STATE_LONG_RELEASE' event.
    */
    ret = fibo_key_register_callback(_prv_key_callback_func, NULL, 3 * 1000);
    if (ret != 0)
    {
        fibo_textTrace("GJH: err: fibo_key_register_callback");
        goto error;
    }

    fibo_thread_delete();
    return;

error:
    if (gp_bl_keep_add_timer != NULL)
    {
        fibo_timer_delete(gp_bl_keep_add_timer);
        gp_bl_keep_add_timer = NULL;
    }
    fibo_lcd_deinit();
    fibo_thread_delete();
    return;
}

int appimg_enter(void *param)
{
    fibo_textTrace("application image enter, param 0x%x", param);

    fibo_thread_create(hello_world_demo, "mythread", 1024, NULL, OSI_PRIORITY_NORMAL);
    return 0;
}

void appimg_exit(void)
{
    fibo_textTrace("application image exit");
}
