/*
 * RTEMS SpaceWire packet library demonstration application. The example
 * takes commands from STDIN and generates SpaceWire packets with the path
 * or logical address the user specififes. If a on-chip router is found
 * the routing table will be set up as described in the README file, you
 * can also see a console log from a typical execution in the README file.
 *
 * The application consists of three threads:
 *
 *  TA01. Input task, the user commands from STDIN are interpreted into
 *        packet scheduling, time-code generation or status printing.
 *
 *  TA02. Link monitor task. Prints out whenever a SpaceWire link switch
 *        from run-state to any other state or vice versa.
 *
 *  TA03. SpaceWire DMA task. Handles reception and transmission of SpaceWire
 *        packets on all SpaceWire devices.
 *
 *
 *  Tick-out IRQs are catched by the time-code ISR, and printed on STDOUT.
 *
 *
 */

/* Define INCLUDE_PCI_DRIVERS to include PCI host and PCI board drivers with
 * SpaceWire interfaces
 */
/*#define INCLUDE_PCI_DRIVERS*/

/* SpaceWire parameters */
#define SPW_PROT_ID 155

/* Number of SpaceWire ports supported */
#define DEVS_MAX 4 //32

#include <rtems.h>

/* configuration information */

#define CONFIGURE_INIT

#include <bsp.h> /* for device driver prototypes */

rtems_task Init( rtems_task_argument argument);	/* forward declaration needed */

/* configuration information */

#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER

/* Set some values if someone should modify the example. The shared IRQ layer
 * need one semaphore.
 */
#define CONFIGURE_MAXIMUM_TASKS             8
#define CONFIGURE_MAXIMUM_SEMAPHORES        (5 + (DEVS_MAX * 5))
#define CONFIGURE_MAXIMUM_MESSAGE_QUEUES    20
#define CONFIGURE_MAXIMUM_FILE_DESCRIPTORS 32
#define CONFIGURE_MAXIMUM_DRIVERS 32
#define CONFIGURE_MAXIMUM_PERIODS             1

#define CONFIGURE_RTEMS_INIT_TASKS_TABLE
#define CONFIGURE_INIT_TASK_ATTRIBUTES    RTEMS_DEFAULT_ATTRIBUTES | RTEMS_FLOATING_POINT
#define CONFIGURE_EXTRA_TASK_STACKS         (40 * RTEMS_MINIMUM_STACK_SIZE)
#define CONFIGURE_MICROSECONDS_PER_TICK     RTEMS_MILLISECONDS_TO_MICROSECONDS(2)


/* Configure Driver manager */
#if defined(RTEMS_DRVMGR_STARTUP) && defined(LEON3) /* if --drvmgr was given to configure */
 /* Add Timer and UART Driver for this example */
 #ifdef CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER
  #define CONFIGURE_DRIVER_AMBAPP_GAISLER_GPTIMER
 #endif
 #ifdef CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER
  #define CONFIGURE_DRIVER_AMBAPP_GAISLER_APBUART
 #endif
#endif

#define CONFIGURE_DRIVER_AMBAPP_GAISLER_SPW_ROUTER /* SpaceWire Router  */
#define CONFIGURE_DRIVER_AMBAPP_GAISLER_GRSPW2     /* SpaceWire Packet driver */

#ifdef INCLUDE_PCI_DRIVERS
/* Configure PCI Library to auto configuration. This can be substituted with
 * a static configuration by setting PCI_LIB_STATIC, see pci/. Static
 * configuration can be generated automatically by print routines in PCI
 * library.
 */
#define RTEMS_PCI_CONFIG_LIB
/*#define CONFIGURE_PCI_LIB PCI_LIB_STATIC*/
#define CONFIGURE_PCI_LIB PCI_LIB_AUTO

/*#define CONFIGURE_DRIVER_AMBAPP_GAISLER_PCIF*//* GRLIB PCIF Host driver  */
#define CONFIGURE_DRIVER_AMBAPP_GAISLER_GRPCI   /* GRPCI Host driver */
#define CONFIGURE_DRIVER_AMBAPP_GAISLER_GRPCI2  /* GRPCI2 Host Driver */
#define CONFIGURE_DRIVER_PCI_GR_RASTA_IO        /* GR-RASTA-IO PCI Target Driver */
#define CONFIGURE_DRIVER_PCI_GR_701             /* GR-701 PCI Target Driver */
#define CONFIGURE_DRIVER_PCI_GR_RASTA_TMTC      /* GR-RASTA-TMTC PCI Target Driver */
#endif

/*******************************************/

#ifdef LEON2
  /* PCI support for AT697 */
  #define CONFIGURE_DRIVER_LEON2_AT697PCI
  /* AMBA PnP Support for GRLIB-LEON2 */
  #define CONFIGURE_DRIVER_LEON2_AMBAPP
#endif

#include <rtems/confdefs.h>
#include <drvmgr/drvmgr_confdefs.h>

#include <rtems/bspIo.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#undef ENABLE_NETWORK
#undef ENABLE_NETWORK_SMC_LEON3

#include "/opt/rcc-1.3.1-gcc/src/samples/config.c"

rtems_task test_app(rtems_task_argument ignored);
rtems_id tid, tid_link, tid_dma;
rtems_id dma_sem;

int nospw = 0;
int tasks_stop = 0;

rtems_task Init(
  rtems_task_argument ignored
)
{
	/* Initialize Driver manager and Networking, in config.c */
	system_init();

	/* Print device topology */
	/*drvmgr_print_topo();*/
	rtems_task_wake_after(4);

	tasks_stop = nospw = 0;

	/* Run SpaceWire Test application */
	rtems_task_create(
			rtems_build_name( 'T', 'A', '0', '1' ),
			10, RTEMS_MINIMUM_STACK_SIZE * 10, RTEMS_DEFAULT_MODES,
			RTEMS_FLOATING_POINT, &tid);
	rtems_task_create(
			rtems_build_name( 'T', 'A', '0', '2' ),
			10, RTEMS_MINIMUM_STACK_SIZE * 10, RTEMS_DEFAULT_MODES,
			RTEMS_FLOATING_POINT, &tid_link);
	rtems_task_create(
			rtems_build_name( 'T', 'A', '0', '3' ),
			10, RTEMS_MINIMUM_STACK_SIZE * 10, RTEMS_DEFAULT_MODES,
			RTEMS_FLOATING_POINT, &tid_dma);
	/* Device Semaphore created with count = 1 */
	if (rtems_semaphore_create(rtems_build_name('S', 'E', 'M', '0'), 1,
	    RTEMS_FIFO | RTEMS_SIMPLE_BINARY_SEMAPHORE | \
	    RTEMS_NO_INHERIT_PRIORITY | RTEMS_LOCAL | \
	    RTEMS_NO_PRIORITY_CEILING, 0, &dma_sem) != RTEMS_SUCCESSFUL) {
		printf("Failed creating Semaphore\n");
		exit(0);
	}

	rtems_task_start(tid, test_app, 0);
	rtems_task_suspend( RTEMS_SELF );
}

//#include <grlib/grspw_pkt.h>
//#include "grspw_pkt_lib.h"

#include "pkt.h"

rtems_task link_ctrl_task(rtems_task_argument unused);
rtems_task dma_task(rtems_task_argument unused);


/* All packet buffers used by application */
struct spwpkt pkts[DEVS_MAX][DATA_MAX];

static struct grspw_device devs[DEVS_MAX];

 void app_tc_isr(void *data, int tc);


 /* All packet buffers used by application */
 struct spwpkt pkts[DEVS_MAX][DATA_MAX];

 static struct grspw_device devs[DEVS_MAX];

 struct grspw_config dev_def_cfg =
 {
 		.adrcfg =
 		{
 			.promiscuous = 1, /* Detect all packets */
 			.def_addr = 32, /* updated bu dev_init() */
 			.def_mask = 0,
 			.dma_nacfg =
 			{
 				/* Since only one DMA Channel is used, only
 				 * the default Address|Mask is used.
 				 */
 				{
 					.node_en = 0,
 					.node_addr = 0,
 					.node_mask = 0,
 				},
 				{
 					.node_en = 0,
 					.node_addr = 0,
 					.node_mask = 0,
 				},
 				{
 					.node_en = 0,
 					.node_addr = 0,
 					.node_mask = 0,
 				},
 				{
 					.node_en = 0,
 					.node_addr = 0,
 					.node_mask = 0,
 				},
 			},
 		},
 		.rmap_cfg = 0,		/* Disable RMAP */
 		.rmap_dstkey = 0,	/* No RMAP DESTKEY needed when disabled */
 		.tc_cfg = TCOPTS_EN_TX|TCOPTS_EN_RX,/* Enable TimeCode */
 		.tc_isr_callback = app_tc_isr,/* TimeCode ISR */
 		.tc_isr_arg = NULL,	/* No TimeCode ISR Argument */
 		.enable_chan_mask = 1,	/* Enable only the first DMA Channel */
 		.chan =
 		{
 			{
 				.flags = DMAFLAG_NO_SPILL,
 				.rxmaxlen = PKT_SIZE+4,
 				.rx_irq_en_cnt = 0, /* Disable RX IRQ generation */
 				.tx_irq_en_cnt = 0, /* Disable TX IRQ generation */
 			},
 			/* The other 3 DMA Channels are unused */

 		},
 };

 extern int router_setup_custom(void);
 extern int router_print_port_status(void);


 int dev_init(int idx)
 {
 	struct grspw_device *dev = &devs[idx];
 	int i, ctrl, clkdiv, tc;

 	printf(" Initializing SpaceWire device %d\n", idx);

 	memset(dev, 0, sizeof(struct grspw_device));

 	dev->index = idx;
 	dev->dh = grspw_open(idx);
 	if (dev->dh == NULL) {
 		printf("Failed to open GRSPW device %d\n", idx);
 		return -1;
 	}
 	grspw_hw_support(dev->dh, &dev->hwsup);
 #ifdef PRINT_GRSPW_RESET_CFG
 	grspw_config_read(DEV(dev), &dev->cfg);
 	printf("\n\n---- DEFAULT CONFIGURATION FROM DRIVER/HARDWARE ----\n");
 	grspw_cfg_print(&dev->hwsup, &dev->cfg);
 #endif
 	dev->cfg = dev_def_cfg;
 	dev->cfg.adrcfg.def_addr = 32 + idx;
 	dev->cfg.tc_isr_arg = dev;
 	tc = TCOPTS_EN_TX | TCOPTS_EN_RX | TCOPTS_EN_RXIRQ;
 	grspw_tc_ctrl(dev->dh, &tc);

 	if (grspw_cfg_set(DEV(dev), &dev->cfg)) {
 		grspw_close(dev->dh);
 		return -1;
 	}
 #ifdef PRINT_GRSPW_RESET_CFG
 	printf("\n\n---- APPLICATION CONFIGURATION ----\n");
 	grspw_cfg_print(&dev->hwsup, &dev->cfg);
 	printf("\n\n");
 #endif

 	/* This will result in an error if only one port available */
 	if (dev->hwsup.nports < 2) {
 		int port = 1;
 		if ( grspw_port_ctrl(dev->dh, &port) == 0 ) {
 			printf("Succeeded to select port1, however only one PORT on dev %d!\n", dev->index);
 			return -1;
 		}
 	}

 	/* Try to bring link up at fastest clockdiv but do not touch
 	 * start-up clockdivisor */
 	clkdiv = -1;
 	grspw_link_ctrl(dev->dh, NULL, NULL, &clkdiv);
 	ctrl = LINKOPTS_ENABLE | LINKOPTS_AUTOSTART | LINKOPTS_START;
 	clkdiv &= 0xff00;
 	grspw_link_ctrl(dev->dh, &ctrl, NULL, &clkdiv);

 	if ( (dev->hwsup.hw_version >> 16) != GAISLER_SPW2_DMA ){
 		printf(" NOTE: running on SPW-ROUTER DMA SpaceWire link (no link-state available)\n");
 	}
 	else
 		printf(" After Link Start: %d\n", (int)grspw_link_state(dev->dh));
 	dev->run = 0;

 	grspw_stats_clr(dev->dh);

 	for (i=0; i<dev->hwsup.ndma_chans; i++) {
 		if (dev->dma[i])
 			grspw_dma_stats_clr(dev->dma[i]);
 	}

 	grspw_list_clr(&dev->rx_list);
 	grspw_list_clr(&dev->tx_list);
 	grspw_list_clr(&dev->tx_buf_list);
 	dev->rx_list_cnt = dev->tx_list_cnt = dev->tx_buf_list_cnt = 0;

 	return 0;
 }

 int dev_dma_close_all(int idx)
 {
 	struct grspw_device *dev = &devs[idx];
 	int i, rc;
 	for (i=0; i<dev->hwsup.ndma_chans; i++) {
 		if (dev->dma[i]) {
 			rc = grspw_dma_close(dev->dma[i]);
 			if (rc)
 				return rc;
 			dev->dma[i] = NULL;
 		}
 	}
 	return 0;
 }

 void dev_cleanup(int idx)
 {
 	struct grspw_device *dev = &devs[idx];

 	if (dev->dh == NULL)
 		return;

 	/* Stop all DMA activity first */
 	grspw_stop(DEV(dev));

 	/* wait for other tasks to be thrown out from driver */
 	rtems_task_wake_after(4);

 	/* close all DMA channels */
 	if (dev_dma_close_all(idx)) {
 		printf("FAILED to close GRSPW%d DMA\n", idx);
 	}

 	if (grspw_close(dev->dh)) {
 		printf("FAILED to close GRSPW%d\n", idx);
 	}
 	dev->dh = NULL;
 }

void pktlist_set_flags(struct grspw_list *lst, unsigned short flags)
{
	struct grspw_pkt *pkt = lst->head;
	while (pkt) {
		pkt->flags = flags;
		pkt = pkt->next;
	}
}

#include <grlib/grspw_router.h>
extern struct router_hw_info router_hw;
extern void *router;
int router_present = 0;

int dma_process(struct grspw_device *dev);

rtems_task test_app(rtems_task_argument ignored)
{
	int i, nb_pkts_to_transmit;
	struct grspw_pkt *pkt;
	struct route_entry route;

	int src_port, dest_port;
	int devno;
	int pkt_cnt=0;

	/* Initialize two GRSPW AMBA ports */
	printf("Setting up SpaceWire router\n");
	if (router_setup_custom()) {
		printf("Failed router initialization, assuming that it does not exists\n");
	} else {
		/* on-chip router found */
		if (router_hw.nports_amba < 2) {
			printf("Error. Router with less than 2 AMBA ports not supported\n");
			exit(0);
		}
		router_present = 1;
	}

	nospw = grspw_dev_count();
	if (nospw < 1) {
		printf("Found no SpaceWire cores, aborting\n");
		exit(0);
	}
	if (nospw > DEVS_MAX) {
		printf("Limiting to %d SpaceWire devices\n", DEVS_MAX);
		nospw = DEVS_MAX;
	}

	memset(devs, 0, sizeof(devs));
	for (i=0; i<nospw; i++) {
		if (dev_init(i)) {
			printf("Failed to initialize GRSPW%d\n", i);
			exit(0);
		}
		fflush(NULL);
	}

	/* Initialize GRSPW */
	init_pkts(devs, pkts);

	printf("\n\nStarting SpW DMA channels\n");
	for (i = 0; i < nospw; i++) {
		printf("Starting GRSPW%d: ", i);
		fflush(NULL);
		if (grspw_start(DEV(&devs[i]))) {
			printf("Failed to initialize GRSPW%d\n", i);
			exit(0);
		}
		printf("DMA Started Successfully\n");
	}

	fflush(NULL);
	rtems_task_start(tid_link, link_ctrl_task, 0);
	rtems_task_start(tid_dma, dma_task, 0);
	rtems_task_wake_after(12);


	printf("\n***********  PKT TX/RX TEST  **************\n\n");

	//devno = -1;
	src_port = 3;
	dest_port = 6;
	/// The number of packets to transmit
	nb_pkts_to_transmit=4;

	memset(&route, 0, sizeof(route));
	route.dstadr[0]=src_port;
	route.dstadr[1]=dest_port;

	printf("SPW src port : %d\n", route.dstadr[0]);
	printf("SPW dest port : %d\n", route.dstadr[1]);
	printf("%d pkts are waiting for transmission\n\n", nb_pkts_to_transmit);


	while(nb_pkts_to_transmit!=0)
	{

		rtems_task_wake_after(1000);

		// the device used is changed as a new packet is sent
		devno = nb_pkts_to_transmit%4;
		//devno++;
		pkt_cnt++;
		printf("------ PKT %d ------\n-------------------\n", pkt_cnt);
		printf("TX on GRSPW device %d (AMBA port %d)\n", devno, devno+1);

		/* Get a TX packet buffer */
		rtems_semaphore_obtain(dma_sem, RTEMS_WAIT, RTEMS_NO_TIMEOUT);
		pkt = devs[devno].tx_buf_list.head;
		if (pkt == NULL) {
			printf(" No free transmit buffers available\n");
		}
		devs[devno].tx_buf_list.head = pkt->next;
		devs[devno].tx_buf_list_cnt--;
		if (pkt->next == NULL)
			devs[devno].tx_buf_list.tail = NULL;
		pkt_init_hdr(pkt, &route, devno);

		nb_pkts_to_transmit--;

		/* Send packet by adding it to the tx_list */
		grspw_list_append(&devs[devno].tx_list, pkt);
		devs[devno].tx_list_cnt++;
		rtems_semaphore_release(dma_sem);
	}

	rtems_task_wake_after(1000);
	tasks_stop = 1;
	for ( i=0; i<nospw; i++)
		dev_cleanup(i);
	rtems_task_wake_after(8);

	printf("\n\nEXAMPLE COMPLETED.\n\n");
	exit(0);

}

 /* Interrupt handler for TimeCode reception on RXDEV */
 void app_tc_isr(void *data, int tc)
 {
 	struct grspw_device *dev = data;

 	printk("GRSPW%d: TC-ISR received 0x%02x\n", dev->index, tc);
 }

rtems_task link_ctrl_task(rtems_task_argument unused)
{
	int i, run;
	struct grspw_device *dev;
	spw_link_state_t state;
	uint32_t sts;
	spw_link_state_t rtrp[32]; /* router ports link status */

	memset(rtrp, 0, sizeof(rtrp));

	printf("Started link control task\n");

	while (tasks_stop == 0) {
		for (i = 0; i < nospw; i++) {
			dev = &devs[i];
			if (dev->dh == NULL)
				continue;

			/* Check if link status has changed */
			state = grspw_link_state(dev->dh);
			run = 0;
			if (state == SPW_LS_RUN)
				run = 1;
			if ((run && dev->run == 0) || (run == 0 && dev->run)) {
				if (run)
					printf("GRSPW%d: link state entering run-state\n", i);
				else
					printf("GRSPW%d: link state leaving run-state\n", i);
				dev->run = run;
			}
		}

		/* Check link-status of router ports */
		if (router_present) {
			for (i = 0; i < router_hw.nports_spw; i++) {
				router_port_status(router, i+1, &sts, 0x0);
				state = (sts >> 12) & 0x7;

				run = 0;
				if (state == SPW_LS_RUN)
					run = 1;
				if ((run && rtrp[i] == 0) || (run == 0 && rtrp[i])) {
					if (run)
						printf("ROUTER SpW PORT%d: link state entering run-state\n", i+1);
					else
						printf("ROUTER SpW PORT%d: link state leaving run-state\n", i+1);
					rtrp[i] = run;
				}
			}
		}

		rtems_task_wake_after(4);
	}

	printf(" Link control task shutdown\n");

	rtems_task_delete(RTEMS_SELF);
}

rtems_task dma_task(rtems_task_argument unused)
{
	int i;
	struct grspw_device *dev;

	printf("Started DMA control task\n");

	while (tasks_stop == 0) {
		rtems_semaphore_obtain(dma_sem, RTEMS_WAIT, RTEMS_NO_TIMEOUT);

		for (i = 0; i < nospw; i++) {
			dev = &devs[i];
			if (dev->dh == NULL)
				continue;

			dma_process(dev);
		}
		rtems_semaphore_release(dma_sem);
		rtems_task_wake_after(20);
	}

	printf(" DMA task shutdown\n");

	rtems_task_delete(RTEMS_SELF);

}

int dma_process(struct grspw_device *dev)
{
	int cnt, rc, i;
	struct grspw_list lst;
	struct grspw_pkt *pkt;
	unsigned char *c;

	int rx_ready, rx_sched, rx_recv, tx_send, tx_sched, tx_sent, tx_hwcnt, rx_hwcnt;

	grspw_dma_rx_count(dev->dma[0], &rx_ready, &rx_sched, &rx_recv, &rx_hwcnt);
	grspw_dma_tx_count(dev->dma[0], &tx_send, &tx_sched, &tx_sent, &tx_hwcnt);
	if (rx_hwcnt >= 127) {
		printf(" DMA DRVQ RX_READY: %d\n", rx_ready);
		printf(" DMA DRVQ RX_SCHED: %d\n", rx_sched);
		printf(" DMA DRVQ RX_RECV: %d\n", rx_recv);
		printf(" DMA DRVQ RX_HWCNT: %d\n", rx_hwcnt);
		printf(" DMA DRVQ TX_SEND: %d\n", tx_send);
		printf(" DMA DRVQ TX_SCHED: %d\n", tx_sched);
		printf(" DMA DRVQ TX_SENT: %d\n", tx_sent);
		printf(" DMA DRVQ TX_HWCNT: %d\n", tx_hwcnt);

	}

	dma_RX(dev);

	dma_TX(dev);


	return 0;

}
