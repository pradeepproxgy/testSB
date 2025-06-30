/* Copyright (C) 2020 RDA Technologies Limited and/or its affiliates("RDA").
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

#include "cfw.h"
#include "osi_api.h"
#include "osi_log.h"
#include "nonip.h"
#include "drv_ps_path.h"
#include "at_cfw.h"

#define SIM_CID(sim, cid) ((((sim)&0xf) << 4) | ((cid)&0xf))

extern osiThread_t *netGetTaskID();

#define MAX_SIM_ID 2
#define MAX_CID 7

static NonIPSession_t gNonIPSession[MAX_SIM_ID][MAX_CID] = {0};

NonIPSession_t *getNonIPSession(uint8_t nSim, uint8_t nCid)
{
    OSI_LOGI(0x10007af9, "gNonIPSession[%d][%d] = %p", nSim, nCid, &gNonIPSession[nSim][nCid]);

    if (nSim >= MAX_SIM_ID || nCid >= MAX_CID)
        return NULL;
    return &gNonIPSession[nSim][nCid];
}

static void _nonIP_pspathDataInput(void *ctx, drvPsIntf_t *p)
{
    OSI_LOGI(0x10007afa, "nonip_pspathDataInput osiThreadCallback in ");

    NonIPSession_t *nonIPSession = (NonIPSession_t *)ctx;
    if (nonIPSession->cb != NULL)
        nonIPSession->cb(nonIPSession->cb_ctx, nonIPSession);
    OSI_LOGI(0x10007afb, "nonip_pspathDataInput osiThreadCallback out");
}

int8_t NonIPDataSend(uint8_t nSimId, uint8_t nCid, const uint8_t *data, uint32_t len, uint8_t rai, int seqno)
{
    OSI_LOGI(0x10007afc, "NonIPDataSend ---------tot_len=%d----", len);

    NonIPSession_t *nonIPSession = NULL;
    nonIPSession = getNonIPSession(nSimId, nCid);
    if (nonIPSession->used == 0 || nonIPSession->dataPsPath == NULL)
    {
        return -1;
    }

    extern bool NetifGetDPSDFlag(CFW_SIM_ID nSim);
    if (!NetifGetDPSDFlag(nonIPSession->simID))
        drvPsIntfWrite(nonIPSession->dataPsPath, data, len, rai, seqno);

    return 0;
}

int32_t NonIPDataRecv(uint8_t nSimId, uint8_t nCid, uint8_t *data, uint32_t len)
{
    OSI_LOGI(0x10007afd, "NonIPDataRecv ---------tot_len=%d----", len);

    NonIPSession_t *nonIPSession = NULL;
    nonIPSession = getNonIPSession(nSimId, nCid);
    if (nonIPSession->used == 0 || nonIPSession->dataPsPath == NULL)
    {
        return -1;
    }
    OSI_LOGI(0x10007afe, "NonIPDataRecv ---------nonIPSession->dataPsPath=%x----", nonIPSession->dataPsPath);
    return drvPsIntfRead(nonIPSession->dataPsPath, data, len);
}

bool NonIPSessionSetDataArriveCB(uint8_t nSimId, uint8_t nCid, nonIPDataArriveCB_t cb, void *cb_ctx)
{
    NonIPSession_t *nonIPSession = NULL;
    nonIPSession = getNonIPSession(nSimId, nCid);
    if (nonIPSession->used == 0)
    {
        OSI_LOGE(0x10009512, "NonIPSession is not created for simid %d cid %d", nSimId, nCid);
        return false;
    }

    nonIPSession->cb = cb;
    nonIPSession->cb_ctx = cb_ctx;
    return true;
}

void NonIPSessionCreate(uint8_t nSimId, uint8_t nCid, nonIPDataArriveCB_t cb, void *cb_ctx)
{
    NonIPSession_t *nonIPSession = NULL;
    nonIPSession = getNonIPSession(nSimId, nCid);
    if (nonIPSession->used != 0)
    {
        return;
    }
    nonIPSession->cb = cb;
    nonIPSession->cb_ctx = cb_ctx;
#ifndef CONFIG_NET_LTE_PSPACKET_SUPPORT
    drvPsIntf_t *dataPsPath = drvPsIntfOpen(nSimId, nCid, _nonIP_pspathDataInput, (void *)nonIPSession);
    if (dataPsPath != NULL)
    {
        nonIPSession->dataPsPath = dataPsPath;
    }
#endif
    nonIPSession->cID = nCid;
    nonIPSession->simID = nSimId;
    nonIPSession->used = 1;
    OSI_LOGI(0x10007aff, "NonIPSessionCreate, nCid: %d nSimId: %d, nonIPSession->dataPsPath %x\n", nCid, nSimId, nonIPSession->dataPsPath);
}

void NonIPSessionDestory(uint8_t nSimId, uint8_t nCid)
{
    NonIPSession_t *nonIPSession = NULL;
    nonIPSession = getNonIPSession(nSimId, nCid);

    if (nonIPSession->used == 1)
    {
#ifndef CONFIG_NET_LTE_PSPACKET_SUPPORT
        if (nonIPSession->dataPsPath != NULL)
            drvPsIntfClose(nonIPSession->dataPsPath);
#endif
        nonIPSession->cID = 0;
        nonIPSession->simID = 0;
        nonIPSession->dataPsPath = NULL;
        nonIPSession->cb = NULL;
        nonIPSession->cb_ctx = NULL;
        nonIPSession->used = 0;
    }
    OSI_LOGI(0x10007b00, "NonIPSessionDestory, nCid: %d nSimId: %d\n", nCid, nSimId);
}
