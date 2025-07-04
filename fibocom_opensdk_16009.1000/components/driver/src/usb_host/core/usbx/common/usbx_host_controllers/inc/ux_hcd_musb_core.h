/*
 * MUSB OTG driver defines
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

#ifndef __UX_HCD_MUSB_CORE_H__
#define __UX_HCD_MUSB_CORE_H__

//#define CONFIG_USB_MUSB_SPRD
#include "ch9.h"
#include "ch11.h"
#include "ux_api.h"
#include "ux_bitops.h"
#include "ux_hcd_musb.h"
#include "ux_hcd_musb_host.h"
struct musb;
struct musb_hw_ep;
//struct musb_ep;

/* Helper defines for struct musb->hwvers */
#define MUSB_HWVERS_MAJOR(x) ((x >> 10) & 0x1f)
#define MUSB_HWVERS_MINOR(x) (x & 0x3ff)
#define MUSB_HWVERS_RC 0x8000
#define MUSB_HWVERS_1300 0x52C
#define MUSB_HWVERS_1400 0x590
#define MUSB_HWVERS_1800 0x720
#define MUSB_HWVERS_1900 0x784
#define MUSB_HWVERS_2000 0x800

//#include "musb_debug.h"
#include "ux_hcd_musb_dma.h"

#include "ux_hcd_musb_io.h"

//#include "musb_gadget.h"
//#include <linux/usb/hcd.h>
//#include "musb_host.h"

/* NOTE:  otg and peripheral-only state machines start at B_IDLE.
 * OTG or host-only go to A_IDLE when ID is sensed.
 */
#define is_peripheral_active(m) (!(m)->is_host)
#define is_host_active(m) ((m)->is_host)
#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

#define USB_RESUME_TIMEOUT 40
#define MUSB_RECOVER_TIMEOUT 100

enum
{
    MUSB_PORT_MODE_HOST = 1,
    MUSB_PORT_MODE_GADGET,
    MUSB_PORT_MODE_DUAL_ROLE,
};

/****************************** CONSTANTS ********************************/

#ifndef MUSB_C_NUM_EPS
#define MUSB_C_NUM_EPS ((u8)16)
#endif

#ifndef MUSB_MAX_END0_PACKET
#define MUSB_MAX_END0_PACKET ((u16)MUSB_EP0_FIFOSIZE)
#endif

/* host side ep0 states */
enum musb_h_ep0_state
{
    MUSB_EP0_IDLE,
    MUSB_EP0_START,  /* expect ack of setup */
    MUSB_EP0_IN,     /* expect IN DATA */
    MUSB_EP0_OUT,    /* expect ack of OUT DATA */
    MUSB_EP0_STATUS, /* expect ack of STATUS */
} __attribute__((packed));
#if 0
/* peripheral side ep0 states */
enum musb_g_ep0_state {
	MUSB_EP0_STAGE_IDLE,		/* idle, waiting for SETUP */
	MUSB_EP0_STAGE_SETUP,		/* received SETUP */
	MUSB_EP0_STAGE_TX,		/* IN data */
	MUSB_EP0_STAGE_RX,		/* OUT data */
	MUSB_EP0_STAGE_STATUSIN,	/* (after OUT data) */
	MUSB_EP0_STAGE_STATUSOUT,	/* (after IN data) */
	MUSB_EP0_STAGE_ACKWAIT,		/* after zlp, before statusin */
} __attribute__ ((packed));
#endif
/*
 * OTG protocol constants.  See USB OTG 1.3 spec,
 * sections 5.5 "Device Timings" and 6.6.5 "Timers".
 */
#define OTG_TIME_A_WAIT_VRISE 100 /* msec (max) */
#define OTG_TIME_A_WAIT_BCON 1100 /* min 1 second */
#define OTG_TIME_A_AIDL_BDIS 200  /* min 200 msec */
#define OTG_TIME_B_ASE0_BRST 100  /* min 3.125 ms */

/****************************** FUNCTIONS ********************************/

#define MUSB_HST_MODE(_musb)     \
    {                            \
        (_musb)->is_host = true; \
    }
#define MUSB_DEV_MODE(_musb)      \
    {                             \
        (_musb)->is_host = false; \
    }

#define test_devctl_hst_mode(_x) \
    (musb_readb((_x)->mregs, MUSB_DEVCTL) & MUSB_DEVCTL_HM)

#define MUSB_MODE(musb) ((musb)->is_host ? "Host" : "Peripheral")

/******************************** TYPES *************************************/

struct musb_io;

/**
 * struct musb_platform_ops - Operations passed to musb_core by HW glue layer
 * @quirks:	flags for platform specific quirks
 * @enable:	enable device
 * @disable:	disable device
 * @ep_offset:	returns the end point offset
 * @ep_select:	selects the specified end point
 * @fifo_mode:	sets the fifo mode
 * @fifo_offset: returns the fifo offset
 * @readb:	read 8 bits
 * @writeb:	write 8 bits
 * @readw:	read 16 bits
 * @writew:	write 16 bits
 * @readl:	read 32 bits
 * @writel:	write 32 bits
 * @read_fifo:	reads the fifo
 * @write_fifo:	writes to fifo
 * @dma_init:	platform specific dma init function
 * @dma_exit:	platform specific dma exit function
 * @init:	turns on clocks, sets up platform-specific registers, etc
 * @exit:	undoes @init
 * @set_mode:	forcefully changes operating mode
 * @try_idle:	tries to idle the IP
 * @recover:	platform-specific babble recovery
 * @vbus_status: returns vbus status if possible
 * @set_vbus:	forces vbus status
 * @adjust_channel_params: pre check for standard dma channel_program func
 * @pre_root_reset_end: called before the root usb port reset flag gets cleared
 * @post_root_reset_end: called after the root usb port reset flag gets cleared
 * @phy_callback: optional callback function for the phy to call
 */
//struct musb_platform_ops {
struct musb_ops
{
#define MUSB_DMA_SPRD BIT(10)
#define MUSB_G_NO_SKB_RESERVE BIT(9)
#define MUSB_DA8XX BIT(8)
#define MUSB_PRESERVE_SESSION BIT(7)
#define MUSB_DMA_UX500 BIT(6)
#define MUSB_DMA_CPPI41 BIT(5)
#define MUSB_DMA_CPPI BIT(4)
#define MUSB_DMA_TUSB_OMAP BIT(3)
#define MUSB_DMA_INVENTRA BIT(2)
#define MUSB_IN_TUSB BIT(1)
#define MUSB_INDEXED_EP BIT(0)
    u32 quirks;

    int (*init)(struct musb *musb);
    int (*exit)(struct musb *musb);

    void (*enable)(struct musb *musb);
    void (*disable)(struct musb *musb);

    u32 (*ep_offset)(u8 epnum, u16 offset);
    void (*ep_select)(void *mbase, u8 epnum);
    u16 fifo_mode;
    u32 (*fifo_offset)(u8 epnum);
    u32 (*busctl_offset)(u8 epnum, u16 offset);
    u8 (*readb)(const void *addr, unsigned offset);
    void (*writeb)(void *addr, unsigned offset, u8 data);
    u16 (*readw)(const void *addr, unsigned offset);
    void (*writew)(void *addr, unsigned offset, u16 data);
    u32 (*readl)(const void *addr, unsigned offset);
    void (*writel)(void *addr, unsigned offset, u32 data);
    void (*read_fifo)(struct musb_hw_ep *hw_ep, u16 len, u8 *buf);
    void (*write_fifo)(struct musb_hw_ep *hw_ep, u16 len, const u8 *buf);
    struct dma_controller *(*dma_init)(struct musb *musb, void *base);
    void (*dma_exit)(struct dma_controller *c);
    int (*set_mode)(struct musb *musb, u8 mode);
    void (*try_idle)(struct musb *musb);
    int (*recover)(struct musb *musb);

    int (*vbus_status)(struct musb *musb);
    void (*set_vbus)(struct musb *musb, int on);

    int (*adjust_channel_params)(struct dma_channel *channel,
                                 u16 packet_sz, u8 *mode,
                                 dma_addr_t *dma_addr, u32 *len);
    void (*pre_root_reset_end)(struct musb *musb);
    void (*post_root_reset_end)(struct musb *musb);
    int (*phy_callback)(enum musb_vbus_id_status status);
    void (*clear_ep_rxintr)(struct musb *musb, int epnum);
    void (*phy_set_emphasis)(struct musb *musb, bool enabled);
};

/*
 * struct musb_hw_ep - endpoint hardware (bidirectional)
 *
 * Ordered slightly for better cacheline locality.
 */
struct musb_hw_ep
{
    struct musb *musb;
    void *fifo;
    void *regs;
    /* index in musb->endpoints[]  */
    u8 epnum;

    /* hardware configuration, possibly dynamic */
    bool is_shared_fifo;
    bool tx_double_buffered;
    bool rx_double_buffered;
    u16 max_packet_sz_tx;
    u16 max_packet_sz_rx;

    struct dma_channel *tx_channel;
    struct dma_channel *rx_channel;

    /* currently scheduled peripheral endpoint */
    struct musb_qh *in_qh;
    struct musb_qh *out_qh;

    u8 rx_reinit;
    u8 tx_reinit;
#if 0
	/* peripheral side */
	struct musb_ep		ep_in;			/* TX */
	struct musb_ep		ep_out;			/* RX */
#endif
#ifdef CONFIG_USB_SPRD_DMA
#ifdef CONFIG_USB_SPRD_LINKFIFO
    struct linklist_node_s *dma_linklist[CHN_MAX_QUEUE_SIZE];
    dma_addr_t list_dma_addr[CHN_MAX_QUEUE_SIZE];
#else
    struct linklist_node_s *dma_linklist;
    dma_addr_t list_dma_addr;
#endif
#endif
    //#ifdef CONFIG_USB_MUSB_SPRD
    UX_ENDPOINT *hep[2];
    //#endif
};
#if 0
static inline struct musb_request *next_in_request(struct musb_hw_ep *hw_ep)
{
	return next_request(&hw_ep->ep_in);
}

static inline struct musb_request *next_out_request(struct musb_hw_ep *hw_ep)
{
	return next_request(&hw_ep->ep_out);
}
#endif
struct musb_csr_regs
{
    /* FIFO registers */
    u16 txmaxp, txcsr, rxmaxp, rxcsr;
    u16 rxfifoadd, txfifoadd;
    u16 s_rxfifoadd, s_txfifoadd;
    u8 txtype, txinterval, rxtype, rxinterval;
    u8 rxfifosz, txfifosz;
    u8 s_rxfifosz, s_txfifosz;
    u8 txfunaddr, txhubaddr, txhubport;
    u8 rxfunaddr, rxhubaddr, rxhubport;
};

struct musb_context_registers
{

    u8 power;
    u8 intrusbe;
    u16 frame;
    u8 index, testmode;

    u8 devctl, busctl, misc;
    u32 otg_interfsel;

    struct musb_csr_regs index_regs[MUSB_C_NUM_EPS];
};

/*
 * struct musb - Driver instance data.
 */
struct musb
{
    /* device lock */
    //spinlock_t		lock;
    UX_MUTEX *mutex;
    UX_THREAD *delayed_work_thread;
    //spinlock_t		list_lock;	/* resume work list lock */

    struct musb_io io;
    const struct musb_ops *ops;
    struct musb_context_registers context;

    void (*isr)(void *);
    //struct delayed_work	irq_work;
    //struct delayed_work	deassert_reset_work;
    //struct delayed_work	finish_resume_work;
    //struct delayed_work	gadget_work;
    u16 hwvers;

    u16 intrrxe;
    u16 intrtxe;
/* this hub status bit is reserved by USB 2.0 and not seen by usbcore */
#define MUSB_PORT_STAT_RESUME (1 << 31)

    u32 port1_status;

    unsigned long rh_timer;

    enum musb_h_ep0_state ep0_stage;

    /* bulk traffic normally dedicates endpoint hardware, and each
	 * direction has its own ring of host side endpoints.
	 * we try to progress the transfer at the head of each endpoint's
	 * queue until it completes or NAKs too much; then we try the next
	 * endpoint.
	 */
    struct musb_hw_ep *bulk_ep;

    struct list_head control;      /* of musb_qh */
    struct list_head in_bulk;      /* of musb_qh */
    struct list_head out_bulk;     /* of musb_qh */
    struct list_head pending_list; /* pending work list */

    //struct timer_list	otg_timer;
    //struct notifier_block	nb;

    struct dma_controller *dma_controller;

    //struct device		*controller;
    void *ctrl_base;
    void *mregs;

    /* passed down from chip/board specific irq handlers */
    u8 int_usb;
    u16 int_rx;
    u16 int_tx;
    u32 reg_dma;

    //struct usb_phy		*xceiv;
    //struct phy		*phy;

    int nIrq;
    unsigned irq_wake : 1;

    struct musb_hw_ep endpoints[MUSB_C_NUM_EPS];
#define control_ep endpoints

#define VBUSERR_RETRY_COUNT 3
    u16 vbuserr_retry;
    u16 epmask;
    u8 nr_endpoints;

    int (*board_set_power)(int state);

    u8 min_power; /* vbus for periph, in mA/2 */

    int port_mode; /* MUSB_PORT_MODE_* */
    bool session;
    unsigned long quirk_retries;
    bool is_host;
    bool is_offload; /* i2s mode for usb audio */
    bool offload_used;

    int a_wait_bcon;            /* VBUS timeout in msecs */
    unsigned long idle_timeout; /* Next timeout in jiffies */

    unsigned is_initialized : 1;
    unsigned is_runtime_suspended : 1;

    /* active means connected and not suspended */
    unsigned is_active : 1;

    unsigned is_multipoint : 1;

    unsigned hb_iso_rx : 1; /* high bandwidth iso rx? */
    unsigned hb_iso_tx : 1; /* high bandwidth iso tx? */
    unsigned dyn_fifo : 1;  /* dynamic FIFO supported? */

    unsigned bulk_split : 1;
#define can_bulk_split(musb, type) \
    (((type) == USB_ENDPOINT_XFER_BULK) && (musb)->bulk_split)

    unsigned bulk_combine : 1;
#define can_bulk_combine(musb, type) \
    (((type) == USB_ENDPOINT_XFER_BULK) && (musb)->bulk_combine)

    /* is_suspended means USB B_PERIPHERAL suspend */
    unsigned is_suspended : 1;

    /* may_wakeup means remote wakeup is enabled */
    unsigned may_wakeup : 1;

    /* is_self_powered is reported in device status and the
	 * config descriptor.  is_bus_powered means B_PERIPHERAL
	 * draws some VBUS current; both can be true.
	 */
    unsigned is_self_powered : 1;
    unsigned is_bus_powered : 1;

    unsigned set_address : 1;
    unsigned test_mode : 1;
    unsigned softconnect : 1;

    unsigned flush_irq_work : 1;

    u8 address;
    u8 test_mode_nr;
    u16 ackpend; /* ep0 */
#if 0
	enum musb_g_ep0_state	ep0_state;
	struct usb_gadget	g;			/* the gadget */
	struct usb_gadget_driver *gadget_driver;	/* its driver */
#endif
    UX_HCD *hcd; /* the usb hcd */
    u16 ux_hcd_nb_root_hubs;
    u32 ux_hcd_musb_interrupt_count;
    struct list_head ux_hcd_musb_ed_list;
    struct list_head ux_hcd_musb_td_list;
    //UX_MUSB_ED *ux_hcd_musb_ed_list;   /*ed list*/
    //UX_MUSB_TD *ux_hcd_musb_host_td_list;

    /*
	 * FIXME: Remove this flag.
	 *
	 * This is only added to allow Blackfin to work
	 * with current driver. For some unknown reason
	 * Blackfin doesn't work with double buffering
	 * and that's enabled by default.
	 *
	 * We added this flag to forcefully disable double
	 * buffering until we get it working.
	 */
    unsigned double_buffer_not_ok : 1;
    unsigned fixup_ep0fifo : 1;

    struct musb_hdrc_config *config;

    int xceiv_old_state;
#ifdef CONFIG_DEBUG_FS
    struct dentry *debugfs_root;
#endif
    int shutdowning;
#ifdef CONFIG_USB_MUSB_SPRD
    bool power_always_on;
#endif
    bool restore_complete;
};

/* This must be included after struct musb is defined */
#include "ux_hcd_musb_regs.h"
#if 0
static inline struct musb *gadget_to_musb(struct usb_gadget *g)
{
	return container_of(g, struct musb, g);
}
#endif

#if 1
static inline char *musb_ep_xfertype_string(u8 type)
{
    char *s;

    switch (type)
    {
    case USB_ENDPOINT_XFER_CONTROL:
        s = "ctrl";
        break;
    case USB_ENDPOINT_XFER_ISOC:
        s = "iso";
        break;
    case USB_ENDPOINT_XFER_BULK:
        s = "bulk";
        break;
    case USB_ENDPOINT_XFER_INT:
        s = "int";
        break;
    default:
        s = "";
        break;
    }
    return s;
}
#endif
#ifdef CONFIG_BLACKFIN
static inline int musb_read_fifosize(struct musb *musb,
                                     struct musb_hw_ep *hw_ep, u8 epnum)
{
    musb->nr_endpoints++;
    musb->epmask |= (1 << epnum);

    if (epnum < 5)
    {
        hw_ep->max_packet_sz_tx = 128;
        hw_ep->max_packet_sz_rx = 128;
    }
    else
    {
        hw_ep->max_packet_sz_tx = 1024;
        hw_ep->max_packet_sz_rx = 1024;
    }
    hw_ep->is_shared_fifo = false;

    return 0;
}

static inline void musb_configure_ep0(struct musb *musb)
{
    musb->endpoints[0].max_packet_sz_tx = MUSB_EP0_FIFOSIZE;
    musb->endpoints[0].max_packet_sz_rx = MUSB_EP0_FIFOSIZE;
    musb->endpoints[0].is_shared_fifo = true;
}

#else

static inline int musb_read_fifosize(struct musb *musb,
                                     struct musb_hw_ep *hw_ep, u8 epnum)
{
    void *mbase = musb->mregs;
    u8 reg = 0;

    /* read from core using indexed model */
    reg = musb_readb(mbase, musb->io.ep_offset(epnum, MUSB_FIFOSIZE));
    /* 0's returned when no more endpoints */
    if (!reg)
        return UX_CONTROLLER_INIT_FAILED;

    musb->nr_endpoints++;
    musb->epmask |= (1 << epnum);

    hw_ep->max_packet_sz_tx = 1 << (reg & 0x0f);

    /* shared TX/RX FIFO? */
    if ((reg & 0xf0) == 0xf0)
    {
        hw_ep->max_packet_sz_rx = hw_ep->max_packet_sz_tx;
        hw_ep->is_shared_fifo = true;
        return 0;
    }
    else
    {
        hw_ep->max_packet_sz_rx = 1 << ((reg & 0xf0) >> 4);
        hw_ep->is_shared_fifo = false;
    }

    return 0;
}

static inline void musb_configure_ep0(struct musb *musb)
{
    musb->endpoints[0].max_packet_sz_tx = MUSB_EP0_FIFOSIZE;
    musb->endpoints[0].max_packet_sz_rx = MUSB_EP0_FIFOSIZE;
    musb->endpoints[0].is_shared_fifo = true;
}
#endif /* CONFIG_BLACKFIN */

/***************************** Glue it together *****************************/

extern const char musb_driver_name[];
int musb_init_controller(struct musb *musb);
void musb_port_reset(struct musb *musb);
struct musb_ops *sprd_musb_ops_get(void);
struct musb_hdrc_config *sprd_musb_config_get(void);
UX_MUSB_ED *_ux_hcd_musb_ed_obtain(struct musb *musb);
void _ux_hcd_musb_ed_init(struct musb *musb, UX_MUSB_ED *ed);
void _ux_hcd_musb_ed_release(struct musb *musb, UX_MUSB_ED *ed);
UINT _ux_hcd_musb_request_transfer(struct musb *musb, UX_TRANSFER *transfer_request);
UINT _ux_hcd_musb_endpoint_create(struct musb *musb, UX_ENDPOINT *endpoint);
UINT _ux_hcd_musb_endpoint_destroy(struct musb *musb, UX_ENDPOINT *endpoint);
UINT _ux_hcd_musb_transfer_abort(struct musb *musb, UX_TRANSFER *transfer_request);
void musb_advance_schedule(struct musb *musb, UX_MUSB_TD *td,
                           struct musb_hw_ep *hw_ep, int is_in);
int musb_schedule(struct musb *musb, UX_MUSB_ED *ed, int is_in);
int musb_h_ep0_irq(struct musb *musb);
void musb_host_rx(struct musb *musb, u8 epnum);
void musb_host_tx(struct musb *musb, u8 epnum);
int musb_host_setup(struct musb *musb);

void sprd_musb_work(struct musb *musb, int state);
void musb_h_disable(struct musb *musb, UX_ENDPOINT *hep);
int musb_urb_dequeue(struct musb *musb, UX_MUSB_TD *td, struct musb_qh *qh);
void musb_host_finish_resume(struct musb *musb);
int musb_port_suspend(struct musb *musb, bool do_suspend);
void musb_delayed_work_thread_entry(ULONG input);
void musb_delayed_work_callback(void *ctx);
int musb_suspend(struct musb *musb);
int musb_resume(struct musb *musb);
void musb_recover_from_babble(struct musb *musb);
int musb_reset_all_fifo_2_default(struct musb *musb);
void musb_host_start(struct musb *musb);
void musb_read_fifo(struct musb_hw_ep *hw_ep, u16 len, u8 *dst);
void musb_write_fifo(struct musb_hw_ep *hw_ep, u16 len, const u8 *src);
void musb_start(struct musb *musb);
void musb_test_mode(struct musb *musb, int wIndex);

static inline void musb_platform_set_vbus(struct musb *musb, int is_on)
{
    if (musb->ops->set_vbus)
        musb->ops->set_vbus(musb, is_on);
}

static inline void musb_platform_enable(struct musb *musb)
{
    if (musb->ops->enable)
        musb->ops->enable(musb);
}

static inline void musb_platform_disable(struct musb *musb)
{
    if (musb->ops->disable)
        musb->ops->disable(musb);
}

static inline int musb_platform_set_mode(struct musb *musb, u8 mode)
{
    if (!musb->ops->set_mode)
        return 0;

    return musb->ops->set_mode(musb, mode);
}

static inline void musb_platform_try_idle(struct musb *musb)
{
    if (musb->ops->try_idle)
        musb->ops->try_idle(musb);
}

static inline int musb_platform_recover(struct musb *musb)
{
    if (!musb->ops->recover)
        return 0;

    return musb->ops->recover(musb);
}
#if 0
static inline int musb_platform_get_vbus_status(struct musb *musb)
{
	if (!musb->ops->vbus_status)
		return -EINVAL;

	return musb->ops->vbus_status(musb);
}
#endif
static inline int musb_platform_init(struct musb *musb)
{
    if (!musb->ops->init)
        return UX_ERROR;

    return musb->ops->init(musb);
}

static inline int musb_platform_exit(struct musb *musb)
{
    if (!musb->ops->exit)
        return UX_ERROR;

    return musb->ops->exit(musb);
}

static inline void musb_platform_pre_root_reset_end(struct musb *musb)
{
    if (musb->ops->pre_root_reset_end)
        musb->ops->pre_root_reset_end(musb);
}

static inline void musb_platform_post_root_reset_end(struct musb *musb)
{
    if (musb->ops->post_root_reset_end)
        musb->ops->post_root_reset_end(musb);
}

static inline void musb_platform_clear_ep_rxintr(struct musb *musb, int epnum)
{
    if (musb->ops->clear_ep_rxintr)
        musb->ops->clear_ep_rxintr(musb, epnum);
}

static inline void musb_platform_emphasis_set(struct musb *musb, bool enabled)
{
    if (musb->ops->phy_set_emphasis)
        musb->ops->phy_set_emphasis(musb, enabled);
}

/*
 * gets the "dr_mode" property from DT and converts it into musb_mode
 * if the property is not found or not recognized returns MUSB_OTG
 */
//extern enum musb_mode musb_get_mode(struct device *dev);

#endif /* __MUSB_CORE_H__ */
