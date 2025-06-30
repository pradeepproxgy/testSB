
/* Copyright (C) 2018 RDA Technologies Limited and/or its affiliates("RDA").
 * All rights reserved.
 *
 * This software is supplied "AS IS" without any warranties.
 * RDA assumes no responsibility or liability for the use of the software,
 * conveys no license or title under any patent, copyright, or mask work
 * right to the product. RDA reserves the right to make changes in the
 * software without notification.  RDA also make no representation or
 * warranty that such application will be suitable for the specified use
 * without further testing or modification.
 */

#include "lcd_panel_desc.h"
#include "drv_config.h"

extern const drvLcdPanelDesc_t gLcdGc9305Desc;
extern const drvLcdPanelDesc_t gLcdSt7789h2Desc;
extern const drvLcdPanelDesc_t gLcdGc9306Desc;

const drvLcdPanelDesc_t *gLcdPanels[] = {
#ifdef CONFIG_SUPPORT_LCD_GC9305
    &gLcdGc9305Desc,
#endif
#ifdef CONFIG_SUPPORT_LCD_ST7789h2
    &gLcdSt7789h2Desc,
#endif
#ifdef CONFIG_SUPPORT_LCD_GC9306
    &gLcdGc9306Desc,
#endif

    NULL,
};
