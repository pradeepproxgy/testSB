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

#include "util.h"
#include "ql_ipc_dev.h"
#include "ql_ipc.h"

#define OSI_LOCAL_LOG_LEVEL OSI_LOG_LEVEL_DEBUG
#include "osi_log.h"

static const char *TIPC_TEST_PORT = "com.android.ipc-unittest.srv.echo";

static struct trusty_dev tdev; /* There should only be one trusty device */
static struct trusty_ipc_dev* tipc_dev;
static struct trusty_ipc_chan tipc_chan;

#define MAX_MESSAGE_SIZE 256


static int ql_tipc_test_entry(struct trusty_ipc_dev* dev) {
    char *msg = "hello world!\n";
    char tx_buf[MAX_MESSAGE_SIZE];
    char rx_buf[MAX_MESSAGE_SIZE];
    int num_iovs = 1;
    struct trusty_ipc_iovec req_iovs[1] = {
            {.base = tx_buf, .len = MAX_MESSAGE_SIZE},
    };
    struct trusty_ipc_iovec resp_iovs[1] = {
            {.base = rx_buf, .len = MAX_MESSAGE_SIZE},
    };
    int rc = TRUSTY_ERR_GENERIC;

    trusty_assert(dev);

    trusty_debug(OSI_LOGPAR_S, "%s Initializing trusty IPC channel\n", TRUSTY_STR);
    trusty_ipc_chan_init(&tipc_chan, dev);
    trusty_debug(OSI_LOGPAR_S, "%s Initializing trusty IPC channel ... OK\n", TRUSTY_STR);

    /* connect to trusty IPC test TA and wait for connect to complete */
    trusty_debug(OSI_LOGPAR_S, "%s Connecting to trusty IPC test TA\n", TRUSTY_STR);
    rc = trusty_ipc_connect(&tipc_chan, TIPC_TEST_PORT, true);
    if (rc < 0) {
        trusty_error(OSI_LOGPAR_SIS, "%s Failed (%d) to connect to '%s'\n",
		     TRUSTY_STR, rc, TIPC_TEST_PORT);
        return rc;
    }

    trusty_strcpy(tx_buf, msg);
    trusty_debug(OSI_LOGPAR_SS, "%s Messages:%s\n", TRUSTY_STR, tx_buf);

    trusty_debug(OSI_LOGPAR_S, "%s Sending messages to trusty IPC test TA\n", TRUSTY_STR);
    rc = trusty_ipc_send(&tipc_chan, req_iovs, num_iovs, true);
    if (rc < 0) {
        trusty_error(OSI_LOGPAR_SI, "%s Failed (%d) to send request message", TRUSTY_STR, rc);
        return rc;
    }
    trusty_debug(OSI_LOGPAR_S, "%s Sending messages to test TA ... OK\n", TRUSTY_STR);

    trusty_debug(OSI_LOGPAR_S, "%s Receiving messages from trusty IPC test TA\n", TRUSTY_STR);
    rc = trusty_ipc_recv(&tipc_chan, resp_iovs, num_iovs, true);
    if (rc < 0) {
        trusty_error(OSI_LOGPAR_SI, "%s Failed (%d) to receive response message", TRUSTY_STR, rc);
        return rc;
    }
    trusty_debug(OSI_LOGPAR_SS, "%s Messages:%s\n", TRUSTY_STR, rx_buf);

    if (rc != MAX_MESSAGE_SIZE) {
        trusty_error(OSI_LOGPAR_SII, "%s Receive message truncated (%d vs %d)",
		     TRUSTY_STR, rc, MAX_MESSAGE_SIZE);
	return rc;
    }

    if (trusty_memcmp(tx_buf, rx_buf, MAX_MESSAGE_SIZE)) {
        trusty_error(OSI_LOGPAR_S, "%s Message mismatch\n", TRUSTY_STR);
	return rc;
    }
    trusty_debug(OSI_LOGPAR_S, "%s Receiving messages from test TA ... OK\n", TRUSTY_STR);

    return TRUSTY_ERR_NONE;
}

void ql_tipc_test(void) {
    int rc = TRUSTY_ERR_NONE;

    trusty_debug(OSI_LOGPAR_S, "%s Initializing trusty device\n", TRUSTY_STR);
    rc = trusty_dev_init(&tdev, NULL);
    if (rc != 0) {
        trusty_error(OSI_LOGPAR_SI, "%s Initializing trusty device failed (%d)\n",
		     TRUSTY_STR, rc);
	goto err_dev_shutdown;
    }
    trusty_debug(OSI_LOGPAR_S, "%s Initializing trusty device ... OK\n", TRUSTY_STR);

    trusty_debug(OSI_LOGPAR_S, "%s Initializing trusty IPC device\n", TRUSTY_STR);
    rc = trusty_ipc_dev_create(&tipc_dev, &tdev, PAGE_SIZE);
    if (rc != 0) {
        trusty_error(OSI_LOGPAR_SI, "%s Initializing trusty IPC device failed (%d)\n",
		     TRUSTY_STR, rc);
	goto err_dev_shutdown;
    }
    trusty_debug(OSI_LOGPAR_S, "%s Initializing trusty IPC device ... OK\n", TRUSTY_STR);

    rc = ql_tipc_test_entry(tipc_dev);
    if (rc != 0) {
        trusty_error(OSI_LOGPAR_SI, "%s Initlializing trusty IPC test client failed (%d)\n",
		     TRUSTY_STR, rc);
	goto err_ipc_dev_shutdown;
    }

    //trusty_debug(OSI_LOGPAR_S, "%s Disconnecting to trusty IPC test TA\n", TRUSTY_STR);
    rc = trusty_ipc_close(&tipc_chan);
    if (rc != 0) {
        trusty_error(OSI_LOGPAR_SI, "%s disconnecting to trusty IPC test TA failed (%d)\n",
		     TRUSTY_STR, rc);
	goto err_ipc_dev_shutdown;
    }

err_ipc_dev_shutdown:
    //trusty_debug(OSI_LOGPAR_S, "%s Shutdown trusty IPC device\n", TRUSTY_STR);
    trusty_ipc_dev_shutdown(tipc_dev);

err_dev_shutdown:
    //trusty_debug(OSI_LOGPAR_S, "%s Shutdown trusty device\n", TRUSTY_STR);
    trusty_dev_shutdown(&tdev);

    osiThreadExit();
}
