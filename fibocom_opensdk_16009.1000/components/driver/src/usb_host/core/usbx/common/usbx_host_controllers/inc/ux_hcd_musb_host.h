/*
 * MUSB OTG driver host defines
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

#ifndef __UX_HCD_MUSB_HOST_H
#define __UX_HCD_MUSB_HOST_H

//#include <linux/scatterlist.h>
//#include "ux_hcd_musb_core.h"

#define URB_SHORT_NOT_OK 0x0001        /* report short reads as errors */
#define URB_ISO_ASAP 0x0002            /* iso-only; use the first unexpired slot in the schedule */
#define URB_NO_TRANSFER_DMA_MAP 0x0004 /* urb->transfer_dma valid on submit */
#define URB_NO_FSBR 0x0020             /* UHCI-specific */
#define URB_ZERO_PACKET 0x0040         /* Finish bulk OUT with short packet */
#define URB_NO_INTERRUPT 0x0080        /* HINT: no non-error interrupt*/

/* stored in "usb_host_endpoint.hcpriv" for scheduled endpoints */
struct musb_qh
{
    //struct usb_host_endpoint *hep;		/* usbcore info */
    UX_ENDPOINT *hep;
    //struct usb_device	*dev;
    UX_DEVICE *dev;
    struct musb_hw_ep *hw_ep; /* current binding */

    struct list_head ring;            /* of musb_qh */
    /* struct musb_qh		*next; */ /* for periodic tree */
    u8 mux;                           /* qh multiplexed to hw_ep */

    unsigned offset;  /* in urb->transfer_buffer */
    unsigned segsize; /* current xfer fragment */

    u8 type_reg;   /* {rx,tx} type register */
    u8 intv_reg;   /* {rx,tx} interval register */
    u8 addr_reg;   /* device address register */
    u8 h_addr_reg; /* hub address register */
    u8 h_port_reg; /* hub port register */

    u8 is_ready; /* safe to modify hw_ep */
    u8 type;     /* XFERTYPE_* */
    u8 epnum;
    u8 hb_mult; /* high bandwidth pkts per uf */
    u16 maxpacket;
    u16 frame;        /* for periodic schedule */
    unsigned iso_idx; /* in urb->iso_frame_desc[] */
    //struct sg_mapping_iter sg_miter;	/* for highmem in PIO mode */
    bool use_sg; /* to track urb using sglist */
};

struct usb_iso_packet_descriptor
{
    u32 offset;
    u32 length;
    u32 actual_length;
    int status;
};

typedef struct UX_MUSB_ED_STRUCT
{
    struct musb_qh qh;
    struct list_head ed_list;
    struct list_head td_list;
} UX_MUSB_ED;

typedef struct UX_MUSB_TD_STRUCT
{
    UX_TRANSFER *transfer_request;
    void *transfer_buffer;
    u32 transfer_buffer_length;
    u32 actual_length;
    u32 transfer_flags;
    u8 *setup_packet;
    u32 dir;
    u32 transfer_dma;
    int unlinked;
    //UX_MUSB_ED *ed;
    struct list_head td_list;
    int number_of_packets;
    int start_frame;
    int status;
    int error_count;
    struct usb_iso_packet_descriptor iso_frame_desc[0];

} UX_MUSB_TD;

/* map from control or bulk queue head to the first qh on that ring */
static inline struct musb_qh *first_qh(struct list_head *q)
{
    if (list_empty(q))
        return NULL;
    return list_entry(q->next, struct musb_qh, ring);
}

#if 0
//extern struct musb *hcd_to_musb(struct usb_hcd *);
//extern irqreturn_t musb_h_ep0_irq(struct musb *);
void musb_host_start(struct musb *musb);
extern int musb_host_alloc(struct musb *);
extern int musb_host_setup(struct musb *, int);
extern void musb_host_cleanup(struct musb *);
extern void musb_host_tx(struct musb *, u8);
extern void musb_host_rx(struct musb *, u8);
extern void musb_root_disconnect(struct musb *musb);
extern void musb_host_free(struct musb *);
extern void musb_host_cleanup(struct musb *);
extern void musb_host_tx(struct musb *, u8);
extern void musb_host_rx(struct musb *, u8);
extern void musb_root_disconnect(struct musb *musb);
extern void musb_host_resume_root_hub(struct musb *musb);
extern void musb_host_poke_root_hub(struct musb *musb);
extern int musb_port_suspend(struct musb *musb, bool do_suspend);
//extern void musb_host_finish_resume(struct work_struct *work);
#endif

#if 0
extern bool musb_tx_dma_program(struct dma_controller *dma,
		struct musb_hw_ep *hw_ep, struct musb_qh *qh,
		struct urb *urb, u32 offset, u32 length);

extern void musb_rx_dma_sprd(struct dma_channel *dma_channel,
		struct musb *musb, u8 epnum,
		struct musb_qh *qh,
		struct urb *urb,
		u32 offset, size_t len);



//static inline void musb_host_finish_resume(struct work_struct *work) {}
#endif

//struct usb_hcd;

//extern int musb_hub_status_data(struct usb_hcd *hcd, char *buf);
//extern int musb_hub_control(struct usb_hcd *hcd,
//			u16 typeReq, u16 wValue, u16 wIndex,
//			char *buf, u16 wLength);
//extern void musb_advance_schedule(struct musb *musb, struct urb *urb,
//				  struct musb_hw_ep *hw_ep, int is_in);
#if 0
static inline struct urb *next_urb(struct musb_qh *qh)
{
	struct list_head	*queue;

	if (!qh)
		return NULL;
	queue = &qh->hep->urb_list;
	if (list_empty(queue))
		return NULL;
	return list_entry(queue->next, struct urb, urb_list);
}
#endif
static inline UX_MUSB_TD *next_td(UX_MUSB_ED *ed)
{
    struct list_head *queue;

    if (!ed)
        return NULL;
    queue = &ed->td_list;
    if (list_empty(queue))
        return NULL;
    return list_entry(queue->next, UX_MUSB_TD, td_list);
}

#endif /* _MUSB_HOST_H */
