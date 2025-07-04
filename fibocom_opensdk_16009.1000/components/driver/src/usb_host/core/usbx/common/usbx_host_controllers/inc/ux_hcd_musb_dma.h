/*
 * MUSB OTG driver DMA controller abstraction
 *
 * Copyright 2005 Mentor Graphics Corporation
 * Copyright (C) 2005-2006 by Texas Instruments
 * Copyright (C) 2006-2007 Nokia Corporation
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
 * NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef __UX_HCD_MUSB_DMA_H__
#define __UX_HCD_MUSB_DMA_H__

#include "ux_api.h"
#include <stdbool.h>
#include "ux_typedef.h"

struct musb_hw_ep;

/*
 * DMA Controller Abstraction


 
 *
 * DMA Controllers are abstracted to allow use of a variety of different
 * implementations of DMA, as allowed by the Inventra USB cores.  On the
 * host side, usbcore sets up the DMA mappings and flushes caches; on the
 * peripheral side, the gadget controller driver does.  Responsibilities
 * of a DMA controller driver include:
 *
 *  - Handling the details of moving multiple USB packets
 *    in cooperation with the Inventra USB core, including especially
 *    the correct RX side treatment of short packets and buffer-full
 *    states (both of which terminate transfers).
 *
 *  - Knowing the correlation between dma channels and the
 *    Inventra core's local endpoint resources and data direction.
 *
 *  - Maintaining a list of allocated/available channels.
 *
 *  - Updating channel status on interrupts,
 *    whether shared with the Inventra core or separate.
 */

#define DMA_ADDR_INVALID (~(dma_addr_t)0)

#ifdef CONFIG_MUSB_PIO_ONLY
#define is_dma_capable() (0)
#else
#define is_dma_capable() (1)
#endif

#ifdef CONFIG_USB_SPRD_DMA
#define musb_dma_sprd(musb) (musb->io.quirks & MUSB_DMA_SPRD)
#else
#define musb_dma_sprd(musb) 0
#endif

#ifdef CONFIG_USB_UX500_DMA
#define musb_dma_ux500(musb) (musb->io.quirks & MUSB_DMA_UX500)
#else
#define musb_dma_ux500(musb) 0
#endif

#ifdef CONFIG_USB_TI_CPPI41_DMA
#define musb_dma_cppi41(musb) (musb->io.quirks & MUSB_DMA_CPPI41)
#else
#define musb_dma_cppi41(musb) 0
#endif

#ifdef CONFIG_USB_TI_CPPI_DMA
#define musb_dma_cppi(musb) (musb->io.quirks & MUSB_DMA_CPPI)
#else
#define musb_dma_cppi(musb) 0
#endif

#ifdef CONFIG_USB_TUSB_OMAP_DMA
#define tusb_dma_omap(musb) (musb->io.quirks & MUSB_DMA_TUSB_OMAP)
#else
#define tusb_dma_omap(musb) 0
#endif

#ifdef CONFIG_USB_INVENTRA_DMA
#define musb_dma_inventra(musb) (musb->io.quirks & MUSB_DMA_INVENTRA)
#else
#define musb_dma_inventra(musb) 0
#endif

#if defined(CONFIG_USB_TI_CPPI_DMA) || defined(CONFIG_USB_TI_CPPI41_DMA)
#define is_cppi_enabled(musb) \
    (musb_dma_cppi(musb) || musb_dma_cppi41(musb))
#else
#define is_cppi_enabled(musb) 0
#endif

/* Anomaly 05000456 - USB Receive Interrupt Is Not Generated in DMA Mode 1
 *	Only allow DMA mode 1 to be used when the USB will actually generate the
 *	interrupts we expect.
 */
#ifdef CONFIG_BLACKFIN
#undef USE_MODE1
#if !ANOMALY_05000456
#define USE_MODE1
#endif
#endif

/*
 * DMA channel status ... updated by the dma controller driver whenever that
 * status changes, and protected by the overall controller spinlock.
 */
enum dma_channel_status
{
    /* unallocated */
    MUSB_DMA_STATUS_UNKNOWN,
    /* allocated ... but not busy, no errors */
    MUSB_DMA_STATUS_FREE,
    /* busy ... transactions are active */
    MUSB_DMA_STATUS_BUSY,
    /* transaction(s) aborted due to ... dma or memory bus error */
    MUSB_DMA_STATUS_BUS_ABORT,
    /* transaction(s) aborted due to ... core error or USB fault */
    MUSB_DMA_STATUS_CORE_ABORT
};

struct dma_controller;

/**
 * struct dma_channel - A DMA channel.
 * @private_data: channel-private data
 * @max_len: the maximum number of bytes the channel can move in one
 *	transaction (typically representing many USB maximum-sized packets)
 * @actual_len: how many bytes have been transferred
 * @status: current channel status (updated e.g. on interrupt)
 * @desired_mode: true if mode 1 is desired; false if mode 0 is desired
 *
 * channels are associated with an endpoint for the duration of at least
 * one usb transfer.
 */
struct dma_channel
{
    void *private_data;
    /* FIXME not void* private_data, but a dma_controller * */
    size_t max_len;
    size_t actual_len;
    enum dma_channel_status status;
    bool desired_mode;
    bool rx_packet_done;
};

/*
 * dma_channel_status - return status of dma channel
 * @c: the channel
 *
 * Returns the software's view of the channel status.  If that status is BUSY
 * then it's possible that the hardware has completed (or aborted) a transfer,
 * so the driver needs to update that status.
 */
static inline enum dma_channel_status
dma_channel_status(struct dma_channel *c)
{
    return (is_dma_capable() && c) ? c->status : MUSB_DMA_STATUS_UNKNOWN;
}

/**
 * struct dma_controller - A DMA Controller.
 * @musb: the usb controller
 * @start: call this to start a DMA controller;
 *	return 0 on success, else negative errno
 * @stop: call this to stop a DMA controller
 *	return 0 on success, else negative errno
 * @channel_alloc: call this to allocate a DMA channel
 * @channel_release: call this to release a DMA channel
 * @channel_abort: call this to abort a pending DMA transaction,
 *	returning it to FREE (but allocated) state
 * @dma_callback: invoked on DMA completion, useful to run platform
 *	code such IRQ acknowledgment.
 *
 * Controllers manage dma channels.
 */
struct dma_controller
{
    struct musb *musb;
    struct dma_channel *(*channel_alloc)(struct dma_controller *,
                                         struct musb_hw_ep *, u8 is_tx);
    void (*channel_release)(struct dma_channel *);
    int (*channel_program)(struct dma_channel *channel,
                           u16 maxpacket, u8 mode,
                           dma_addr_t dma_addr,
                           u32 length);
    int (*channel_abort)(struct dma_channel *);
    int (*is_compatible)(struct dma_channel *channel,
                         u16 maxpacket,
                         void *buf, u32 length);
    void (*dma_callback)(struct dma_controller *);
};

/* called after channel_program(), may indicate a fault */
extern void musb_dma_completion(struct musb *musb, u8 epnum, u8 transmit);

#ifdef CONFIG_MUSB_PIO_ONLY
static inline struct dma_controller *
musb_dma_controller_create(struct musb *m, void *io)
{
    return NULL;
}

static inline void musb_dma_controller_destroy(struct dma_controller *d) {}

#else

extern struct dma_controller *(*musb_dma_controller_create)(struct musb *, void *);

extern void (*musb_dma_controller_destroy)(struct dma_controller *);
#endif

/* Platform specific DMA functions */
extern struct dma_controller *
musbhs_dma_controller_create(struct musb *musb, void *base);
extern void musbhs_dma_controller_destroy(struct dma_controller *c);

extern struct dma_controller *
tusb_dma_controller_create(struct musb *musb, void *base);
extern void tusb_dma_controller_destroy(struct dma_controller *c);

extern struct dma_controller *
cppi_dma_controller_create(struct musb *musb, void *base);
extern void cppi_dma_controller_destroy(struct dma_controller *c);

extern struct dma_controller *
cppi41_dma_controller_create(struct musb *musb, void *base);
extern void cppi41_dma_controller_destroy(struct dma_controller *c);

extern struct dma_controller *
ux500_dma_controller_create(struct musb *musb, void *base);
extern void ux500_dma_controller_destroy(struct dma_controller *c);

#endif /* __MUSB_DMA_H__ */
