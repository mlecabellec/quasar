/******************************************************************************
 *                                                                            *
 * File:        pcie6509_user.h                                               *
 *                                                                            *
 * Description: The interface to the PCIE6509 Linux device driver.            *
 *                                                                            *
 * Date:         02/13/2014                                                   *
 * History:                                                                   *
 *                                                                            *
 *  1  02/13/14  D. Dubash                                                    *
 *               Initial release                                              *
 *                                                                            *
 ******************************************************************************
 *                                                                            *
 *  Copyright (C) 2010 and beyond Concurrent Computer Corporation             *
 *  All rights reserved                                                       *
 *                                                                            *
 ******************************************************************************/

#ifndef __PCIE6509_IOCTL_H_INCLUDED__
#define __PCIE6509_IOCTL_H_INCLUDED__

//#include <linux/pci_ids.h> /*only for PCI_VENDOR_ID and PCI_DEVICE_ID codes */

#define DEFAULT_TIMEOUT  0      /* 0 = don't timeout */
#define DEFAULT_TIMEOUT_MSEC MSECS_TO_SLEEP(DEFAULT_TIMEOUT*1000)

/* These defines are duplicated here so that they can be used by the 
 * library PCIE6509_MAGIC define in lib/pcie6509_lib.h file.
 */
/*** BOARD SPECIFIC INFORMATION IN .../include/linux/pci_ids.h ***/
#define PCIE6509_VENDOR_ID              0x1093       /* board vendor id */
#define PCIE6509_DEVICE_ID_PCIE_6509    0xC4C4       /* board device id */
#define PCIE6509_IDENTIFICATION         0xC0107AD0   /* board identification */

#define PCIE6509_DRIVER_NAME            "pcie6509" /* Must match name in makefile */
#define PCIE6509_MAX_BOARDS             10
                      /* maximum number of boards that driver supports. 
                         Arbitrarily chosen, change if you can actully 
                         support more. Various scripts will need to be
                         enhanced as well. */
#define PCIE6509_LINES_PER_PORT         8
#define PCIE6509_MAX_PORTS              12
#define PCIE6509_MAX_LINES              (PCIE6509_LINES_PER_PORT * PCIE6509_MAX_PORTS)
#define PCIE6509_NUMBER_OF_RTSI_LINES   (8)

/*** Since hardware has some write only registers, we need to shadow them in s/w ***/
/*** Master is at offset 0x020000, Slave is at offset 0x040000 ***/
typedef struct {
    u_short CSJointReset;                   /* 0x000064: (W) ChpServices Joint Reset */
    u_int   CSWatchdogTimeout;              /* 0x000068: (W) ChpServices Watchdog Timeout */
    u_short CSWatchdogConfiguration;        /* 0x00006C: (W) ChpServices Watchdog Configuration */
    u_short CSWatchdogControl;              /* 0x00006E: (W) ChpServices Watchdog Control */
    u_int   CSWatchdogTimerInterrupt1;      /* 0x000070: (W) ChpServices Watchdog Timer Interrupt 1 */
    u_int   CSWatchdogTimerInterrupt2;      /* 0x000074: (W) ChpServices Watchdog Timer Interrupt 2 */
    u_int   CSGlobalInterruptEnable;        /* 0x000078: (W) ChpServices Global Interrupt Enable */
    u_short PFIDirection;                   /* 0x0000A4: (W) PfiPorts PFI Direction */
    u_short CSRTSITrigDirection;            /* 0x0000A6: (W) ChpServices RTSI Trig Direction */
    u_char  CSRTSIOutputSelect[PCIE6509_NUMBER_OF_RTSI_LINES];          
                                            /* 0x0000A8 - 0x0000AF: (W) Line 0..7 */
    u_short PFIFilterPort0Low;              /* 0x0000B0: (W) PfiPorts PFI Filter Port0 Low */
    u_short PFIFilterPort0High;             /* 0x0000B2: (W) PfiPorts PFI Filter Port0 High */
    u_short PFIFilterPort1Low;              /* 0x0000B4: (W) PfiPorts PFI Filter Port1 Low */
    u_short PFIFilterPort1High;             /* 0x0000B6: (W) PfiPorts PFI Filter Port1 High */
    u_char  PFIOutputSelectPort0[PCIE6509_LINES_PER_PORT]; /* 0x0000BA - 0x0000C1 */
    u_char  PFIOutputSelectPort1[PCIE6509_LINES_PER_PORT]; /* 0x0000C2 - 0x0000C9 */
    u_short PFIStaticDigitalOutput;         /* 0x0000E0: (W) PfiPorts Static Digital Output */
    u_short PFIWDTSafeState;                /* 0x0000E2: (W) PfiPortsPFI WDT Safe State */
    u_int   PFIWDTModeSelect;               /* 0x0000E4: (W) PfiPortsPFI WDT Mode Select */
    u_int   DioPortsStaticDigitalOutput;    /* 0x0004B0: (W) DioPorts Static Digital Output */
    u_int   DioPortsDIODirection;           /* 0x0004B4: (W) DioPorts DIO Direction */
    u_int   DioPortsDOWDTSafeState;         /* 0x0004D0: (W) DioPorts DO WDT Safe State */
    u_int   DioPortsDOWDTModeSelectP01;     /* 0x0004D4: (W) DioPorts DO WDT Mode Select Port 0/1 */
    u_int   DioPortsDOWDTModeSelectP23;     /* 0x0004D8: (W) DioPorts DO WDT Mode Select Port 2/3 */
    u_int   DioPortsDIChangeIrqRE;          /* 0x000540: (W) DioPorts DI Change Irq Rising Edge (RE) */
    u_int   DioPortsDIChangeIrqFE;          /* 0x000544: (W) DioPorts DI Change Irq Falling Edge (FE) */
    u_int   PFIChangeIrq;                   /* 0x000548: (W) PfiPorts PFI Change Irq */
    u_int   DioPortsDIFilterP01;            /* 0x00054C: (W) DioPorts DI Filter Port 0/1 */
    u_int   DioPortsDIFilterP23;            /* 0x000550: (W) DioPorts DI Filter Port 2/3 */
    u_int   CSChangeDetectionIrq;           /* 0x000554: (W) ChpServicesChangeDetectionIrq */
} _pcie6509_master_slave_shadow_t;

typedef struct {
    _pcie6509_master_slave_shadow_t Master;
    _pcie6509_master_slave_shadow_t Slave;
} pcie6509_shadow_regs_t;

typedef enum {
    PCIE6509_PORT_OUTPUT   =1,             /* set port for output */
    PCIE6509_PORT_INPUT    =0,             /* set port for input */
} pcie6509_operation_t;

typedef enum {
    PCIE6509_LINE_LOW   =0,                /* line state low */
    PCIE6509_LINE_HIGH  =1,                /* line state high */
} pcie6509_line_hilo_t;

typedef enum {
    PCIE6509_INPUT_LOW     =0,             /* get port input low */
    PCIE6509_INPUT_HIGH    =1,             /* get port input high */
} pcie6509_input_hilo_t;

typedef enum {
    PCIE6509_OUTPUT_LOW    =0,             /* set output to low */
    PCIE6509_OUTPUT_HIGH   =1,             /* set output to high */
} pcie6509_output_hilo_t;

typedef enum {
    PCIE6509_DISABLE       =0,             /* disable */
    PCIE6509_ENABLE        =1,             /* enable */
} pcie6509_enable_disable_t;

typedef enum {
    PCIE6509_TIMER_STATUS_NORMAL   =0,     /* timer expiration status - normal */
    PCIE6509_TIMER_STATUS_EXPIRED  =1,     /* timer expiration status - expired */
} pcie6509_timer_status_t;

typedef enum {
    PCIE6509_SLAVE_STC      = 0,            /* Slave STC3 selection */
    PCIE6509_MASTER_STC     = 1,            /* Master STC3 selection */
} pcie6509_master_slave_select_t;

/* clear register */
typedef enum {
    PCIE6509_CLEAR_INTERRUPT_WDT   =0x40,  /* clear watchdog timer interrupt */
    PCIE6509_RESET_WDT             =0x20,  /* reset watchdog timer running */
    PCIE6509_CLEAR_WDT_EXPIRATION  =0x10,  /* clear watchdog timer expiration */
    PCIE6509_CLEAR_EDGE            =0x08,  /* clear edge detector status */
    PCIE6509_CLEAR_OVERFLOW        =0x04,  /* clear overflow status */
} pcie6509_clear_state_t;

#define PCIE6509_CLEAR_ALL_STATES  (PCIE6509_CLEAR_INTERRUPT_WDT | PCIE6509_RESET_WDT | \
                                 PCIE6509_CLEAR_WDT_EXPIRATION| PCIE6509_CLEAR_EDGE | \
                                 PCIE6509_CLEAR_OVERFLOW)

/* change status register */
typedef enum {
    PCIE6509_WDT_EXP_INT_STATUS    =0x20,  /* watchdog timer expired interrupt */
    PCIE6509_FALLING_EDGE_STATUS   =0x10,  /* falling edge interrupt occurred */
    PCIE6509_RISING_EDGE_STATUS    =0x08,  /* rising edge interrupt occurred */ 
    PCIE6509_MASTER_INT_STATUS     =0x04,  /* device asserting an interrupt */
    PCIE6509_OVERFLOW_STATUS       =0x02,  /* one more edge detected since interrupt */
    PCIE6509_EDGE_STATUS           =0x01,  /* edge has been detected */
} pcie6509_change_status_t;

#define PCIE6509_ALL_CHANGE_STATUS (PCIE6509_WDT_EXP_INT_STATUS | PCIE6509_FALLING_EDGE_STATUS | \
                                 PCIE6509_RISING_EDGE_STATUS | PCIE6509_MASTER_INT_STATUS  | \
                                 PCIE6509_OVERFLOW_STATUS | PCIE6509_EDGE_STATUS)

/* master interrupt control register */
typedef enum {
    PCIE6509_NO_INTERRUPTS         =0x00,  /* clear interrupts */
    PCIE6509_WDT_EXP_INT           =0x20,  /* enable interrupt of WDT expiration */
    PCIE6509_FALLING_EDGE_INT      =0x10,  /* enable interrupt on falling edge detect */ 
    PCIE6509_RISING_EDGE_INT       =0x08,  /* enable interrupt on rising edge detect */ 
    PCIE6509_MASTER_INT            =0x04,  /* enable for any interrupt to occur */
    PCIE6509_OVERFLOW_INT          =0x02,  /* enable overflow interrupt */
    PCIE6509_EDGE_INT              =0x01,  /* enable edge detection interrupt */
} pcie6509_master_interrupt_t;

#define PCIE6509_ALL_MASTER_INT  (PCIE6509_WDT_EXP_INT | PCIE6509_FALLING_EDGE_INT | \
                               PCIE6509_RISING_EDGE_INT | PCIE6509_MASTER_INT | \
                               PCIE6509_OVERFLOW_INT | PCIE6509_EDGE_INT)

/*** Port ***/
typedef enum {
    PCIE6509_PORT_MASK_P0          =0x001, /* port 0 mask */
    PCIE6509_PORT_MASK_P1          =0x002, /* port 1 mask */
    PCIE6509_PORT_MASK_P2          =0x004, /* port 2 mask */
    PCIE6509_PORT_MASK_P3          =0x008, /* port 3 mask */
    PCIE6509_PORT_MASK_P4          =0x010, /* port 4 mask */
    PCIE6509_PORT_MASK_P5          =0x020, /* port 5 mask */
    PCIE6509_PORT_MASK_P6          =0x040, /* port 6 mask */
    PCIE6509_PORT_MASK_P7          =0x080, /* port 7 mask */
    PCIE6509_PORT_MASK_P8          =0x100, /* port 8 mask */
    PCIE6509_PORT_MASK_P9          =0x200, /* port 9 mask */
    PCIE6509_PORT_MASK_P10         =0x400, /* port 10 mask */
    PCIE6509_PORT_MASK_P11         =0x800, /* port 11 mask */
} pcie6509_port_mask_t;

#define PCIE6509_ALL_PORTS_MASK    (PCIE6509_PORT_MASK_P0|PCIE6509_PORT_MASK_P1| \
                                 PCIE6509_PORT_MASK_P2|PCIE6509_PORT_MASK_P3| \
                                 PCIE6509_PORT_MASK_P4|PCIE6509_PORT_MASK_P5| \
                                 PCIE6509_PORT_MASK_P6|PCIE6509_PORT_MASK_P7| \
                                 PCIE6509_PORT_MASK_P8|PCIE6509_PORT_MASK_P9| \
                                 PCIE6509_PORT_MASK_P10|PCIE6509_PORT_MASK_P11)

#define PCIE6509_PORT_GET(Mask,Port) ((Mask & (1 << Port)) >> Port)
#define PCIE6509_PORT_SET(Mask,Port) (Mask = (Mask | (1 << Port)))
#define PCIE6509_PORT_RESET(Mask,Port) (Mask = ((~(1 << Port) & Mask) & PCIE6509_ALL_PORTS_MASK))

#define PCIE6509_WHICH_PORT(Line) (Line/PCIE6509_LINES_PER_PORT)
#define PCIE6509_GET_PORT_MASK(Port) (1 << Port)
#define PCIE6509_GET_LINE_MASK(Line) (1 << Line%PCIE6509_LINES_PER_PORT)

/*** Line ***/
typedef enum {
    PCIE6509_LINE_MASK_L0          =0x01,  /* line 0 mask */
    PCIE6509_LINE_MASK_L1          =0x02,  /* line 1 mask */
    PCIE6509_LINE_MASK_L2          =0x04,  /* line 2 mask */
    PCIE6509_LINE_MASK_L3          =0x08,  /* line 3 mask */
    PCIE6509_LINE_MASK_L4          =0x10,  /* line 4 mask */
    PCIE6509_LINE_MASK_L5          =0x20,  /* line 5 mask */
    PCIE6509_LINE_MASK_L6          =0x40,  /* line 6 mask */
    PCIE6509_LINE_MASK_L7          =0x80,  /* line 7 mask */
} pcie6509_line_mask_t;

#define PCIE6509_ALL_LINES_MASK    (PCIE6509_LINE_MASK_L0|PCIE6509_LINE_MASK_L1| \
                                 PCIE6509_LINE_MASK_L2|PCIE6509_LINE_MASK_L3| \
                                 PCIE6509_LINE_MASK_L4|PCIE6509_LINE_MASK_L5| \
                                 PCIE6509_LINE_MASK_L6|PCIE6509_LINE_MASK_L7)

#define PCIE6509_LINE_GET(Mask,Line) ((Mask & (1 << Line)) >> Line)
#define PCIE6509_LINE_SET(Mask,Line) (Mask = (Mask | (1 << Line)))
#define PCIE6509_LINE_RESET(Mask,Line) (Mask = ((~(1 << Line) & Mask) & PCIE6509_ALL_LINES_MASK))
#define PCIE6509_LINE_ENABLE(Mask,Line) PCIE6509_LINE_SET(Mask,Line)
#define PCIE6509_LINE_DISABLE(Mask,Line) PCIE6509_LINE_RESET(Mask,Line)

/*** Filter Interval */
#define PCIE6509_MIN_FILTER_INTERVAL_RAW       0
#define PCIE6509_MAX_FILTER_INTERVAL_RAW       0x000fffff
#define PCIE6509_FILTER_INTERVAL_MASK          (0x000fffff)

/******************************************************************************
 ***                                                                        ***
 ***                  <<<< Define Board Types Here >>>>                     ***
 ***                                                                        ***
 ******************************************************************************/

enum pcie6509_type_index{
    N6509_0,
};

struct pcie6509_board_entry {
    int  device;
    int  subsystem_device;
    char name[40];
    int  board_type;
};

typedef struct {
    u_short port_mask;                     /* mask of ports to be read/written */
    u_char  line_mask[PCIE6509_MAX_PORTS]; /* ports read/written */ 
} pcie6509_io_port_t;

typedef	struct {
    uint   physical_address;
    uint   size;
    uint   flags;
    uint   *virtual_address;
} pcie6509_dev_region_t;

typedef struct {
    u_int               status;
    u_int               mask;
    u_int64_t           count;
    u_int               pending;
} pcie6509_driver_int_t;


/******************************************************************************
 ***                                                                        ***
 ***        <<<< Define Board Specific Register Strutures Here >>>>         ***
 ***                                                                        ***
 ******************************************************************************/

/*** CHInCh Registers ***/
typedef volatile struct {
    u_int   Identification;                 /* 0x000000 (R)   CHInCh Identification 0xC0107AD0 */
    u_int   __spare_0x000004_0x00005B[22];  /* 0x000004 - 0x00005B */
    u_int   InterruptMask;                  /* 0x00005C (R/W) CHInCh Interrupt Mask */ 
    u_int   InterruptStatus;                /* 0x000060 (R)   CHInCh Interrupt Status */ 
    u_int   __spare_0x000064;               /* 0x000064 - 0x000067 */
    u_int   VolatileInterruptStatus;        /* 0x000068 (R)   CHInCh Volatile Interrupt StatusRegister */ 
    u_int   __spare_0x00006C_0x0001FF[101]; /* 0x00006C - 0x0001FF */
    u_int   Scrap;                          /* 0x000200 (R/W) CHInCh Interrupt Status */ 
    u_int   __spare_0x000204_0x0010AB[938]; /* 0x000204 - 0x0010AB */
    u_int   PCISubsystemID;                 /* 0x0010AC (R)   CHInCh PCI Subsystem ID (ProductID/VendorID) */
    u_int   __spare_0x0010B0_0x01FFFF[31700];/* 0x0010B0 - 0x01FFFF */
} pcie6509_CHInCh_t;

/*** Master/Slave STC3s (offset=0x020000/0x040000) ***/
typedef volatile struct {
    u_int   __spare_0x000000;               /* 0x000000 - 0x000003 */
    u_int   CSScratchPad;                   /* 0x000004: (R/W) ChpServices Scratch Pad */
    u_int   __spare_0x000008_0x00005F[22];  /* 0x000008 - 0x00005F */
    u_int   CSSignature;                    /* 0x000060: (R) ChpServices Signature */

    union {
        u_int   CSTimeSincePowerUp;         /* 0x000064: (R) ChpServices Time Since Power Up */
        u_short CSJointReset;               /* 0x000064: (W) ChpServices Joint Reset */
    };

    union {
        u_int   CSWatchdogStatus;           /* 0x000068: (R) ChpServices Watchdog Status */
        u_int   CSWatchdogTimeout;          /* 0x000068: (W) ChpServices Watchdog Timeout */
    };

    u_short CSWatchdogConfiguration;        /* 0x00006C: (W) ChpServices Watchdog Configuration */
    u_short CSWatchdogControl;              /* 0x00006E: (W) ChpServices Watchdog Control */
  
    union {                                 /* Interrupt Status and WDT Interrupt 1 */
        u_short CSGlobalInterruptStatus;    /* 0x000070: (R) ChpServices Global Interrupt Status */
        u_int   CSWatchdogTimerInterrupt1;  /* 0x000070: (W) ChpServices Watchdog Timer Interrupt 1 */
    };

    u_int   CSWatchdogTimerInterrupt2;      /* 0x000074: (W) ChpServices Watchdog Timer Interrupt 2 */
    u_int   CSGlobalInterruptEnable;        /* 0x000078: (W) ChpServices Global Interrupt Enable */

    u_short __spare_0x00007C;               /* 0x00007C - 0x00007D */

    u_short CSDIInterruptStatus;            /* 0x00007E: (R) ChpServices DI Interrupt Status */

    u_short __spare_0x000080_0x000085[3];   /* 0x000080 - 0x000085 */

    u_short CSWatchdogTimerInterruptStatus; /* 0x000086: (R) ChpServices Watchdog Timer Interrupt Status */

    u_short __spare_0x000088_0x0000A3[14];  /* 0x000088 - 0x0000A3 */

    u_short PFIDirection;                   /* 0x0000A4: (W) PfiPorts PFI Direction */
    u_short CSRTSITrigDirection;            /* 0x0000A6: (W) ChpServices RTSI Trig Direction */
    u_char  CSRTSIOutputSelect[PCIE6509_NUMBER_OF_RTSI_LINES];          
                                            /* 0x0000A8 - 0x0000AF: (W) Line 0..7 */

    u_short PFIFilterPort0Low;              /* 0x0000B0: (W) PfiPorts PFI Filter Port0 Low */
    u_short PFIFilterPort0High;             /* 0x0000B2: (W) PfiPorts PFI Filter Port0 High */
    u_short PFIFilterPort1Low;              /* 0x0000B4: (W) PfiPorts PFI Filter Port1 Low */
    u_short PFIFilterPort1High;             /* 0x0000B6: (W) PfiPorts PFI Filter Port1 High */

    u_short __spare_0x0000B8;               /* 0x0000B8 - 0x0000B9 */

    u_char  PFIOutputSelectPort0[PCIE6509_LINES_PER_PORT];        
                                            /* 0x0000BA - 0x0000C1: (W) PCIe-6509 Port 4/6, Line 0..7 */
    u_char  PFIOutputSelectPort1[PCIE6509_LINES_PER_PORT];        
                                            /* 0x0000C2 - 0x0000C9: (W) PCIe-6509 Port 5/7, Line 0..7 */

    u_short __spare_0x0000CA_0x0000DF[11];  /* 0x0000CA - 0x0000DF */

    union {
        u_short PFIStaticDigitalInput;      /* 0x0000E0: (R) PfiPorts Static Digital Input */
        u_short PFIStaticDigitalOutput;     /* 0x0000E0: (W) PfiPorts Static Digital Output */
    };

    u_short PFIWDTSafeState;                /* 0x0000E2: (W) PfiPortsPFI WDT Safe State */
    u_int   PFIWDTModeSelect;               /* 0x0000E4: (W) PfiPortsPFI WDT Mode Select */

    u_int   __spare_0x0000E8_0x0004AF[242]; /* 0x0000E8 - 0x0004AF */

    u_int   DioPortsStaticDigitalOutput;    /* 0x0004B0: (W) DioPorts Static Digital Output */
    u_int   DioPortsDIODirection;           /* 0x0004B4: (W) DioPorts DIO Direction */

    u_int   __spare_0x0004B8_0x0004CF[6];   /* 0x0004B8 - 0x0004CF */

    u_int   DioPortsDOWDTSafeState;         /* 0x0004D0: (W) DioPorts DO WDT Safe State */
    u_int   DioPortsDOWDTModeSelectP01;     /* 0x0004D4: (W) DioPorts DO WDT Mode Select Port 0/1 */
    u_int   DioPortsDOWDTModeSelectP23;     /* 0x0004D8: (W) DioPorts DO WDT Mode Select Port 2/3 */

    u_int   __spare_0x0004DC_0x00052F[21];  /* 0x0004DC - 0x00052F */

    u_int   DioPortsStaticDigitalInput;     /* 0x000530: (R) DioPorts Static Digital Input */

    u_int   __spare_0x000534_0x00053F[3];   /* 0x000534 - 0x00053F */

    union {
        u_int   CSDIChangeDetectionStatus;  /* 0x000540: (R) ChpServices DI Change Detection Status */
        u_int   DioPortsDIChangeIrqRE;      /* 0x000540: (W) DioPorts DI Change Irq Rising Edge (RE) */
    };

    union {
        u_int   DioPortsDIChangeDetectionLatched; /* 0x000544: (R) DioPorts DI Change Detection Latched */
        u_int   DioPortsDIChangeIrqFE;      /* 0x000544: (W) DioPorts DI Change Irq Falling Edge (FE) */
    };

    union {
        u_short PFIChangeDetectionLatched;  /* 0x000548: (R) PfiPorts PFI Change Detection Latched */
        u_int   PFIChangeIrq;               /* 0x000548: (W) PfiPorts PFI Change Irq */
    };

    u_int   DioPortsDIFilterP01;            /* 0x00054C: (W) DioPorts DI Filter Port 0/1 */
    u_int   DioPortsDIFilterP23;            /* 0x000550: (W) DioPorts DI Filter Port 2/3 */

    u_int   CSChangeDetectionIrq;           /* 0x000554: (W) ChpServicesChangeDetectionIrq */

    u_int   __spare_0x000558_0x002203[1835];/* 0x000558 - 0x002203 */

    u_int   CSIntForwardingControlStatus;   /* 0x002204: (R/W) ChpServices IntForwarding Control Status */
    u_int   CSIntForwardingDestination;     /* 0x002208: (R/W) ChpServices IntForwarding Destination */

    u_int   __spare_0x00220C_0x01FFFF[30589];/* 0x00220C - 0x01FFFF */

} pcie6509_Master_Slave_t;


/*** Registers: Control and Data Registers ***/
typedef volatile struct {
    pcie6509_CHInCh_t   CHInCh;     /* 0x000000 - 0x01FFFF */
    pcie6509_Master_Slave_t Master; /* 0x020000 - 0x03FFFF */
    pcie6509_Master_Slave_t Slave;  /* 0x020000 - 0x03FFFF */
} pcie6509_local_ctrl_data_t;


/*** CHInCh Identification Register (R) - (0x000000) ***/
/*** CHInCh PCISubsystemID Register (R) - (0x0010AC) ***/
/*** ChpServices Signature          (R) - (0x020060/0x040060) ***/
typedef struct {
    u_int       BoardId;
    u_short     SubsystemPid;
    u_short     SubsystemVid;
    u_int       MasterSignature;
    u_int       SlaveSignature;
} pcie6509_board_info_t;
#define PCIE6509_SUBPID_SHIFT               (16)
#define PCIE6509_SUBPID_MASK                (0xFFFF)
#define PCIE6509_SUBVID_SHIFT               (0)
#define PCIE6509_SUBVID_MASK                (0xFFFF)

/*** CHInCh Interrupt Mask Register (R/W) - (0x00005C) ***/
#define PCIE6509_IMR_CLEAR_CPU_INT      (1 << 30)   /* Clear CPU Interrupt */
#define PCIE6509_IMR_SET_STC3_INT       (1 << 11)   /* Set SET STC3 Interrupt */
#define PCIE6509_IMR_CLEAR_STC3_INT     (1 << 10)   /* Clear STC3 Interrupt */

/*** ChpServices Time Since Power Up (R) - (0x020064/0x040064) ***/
typedef struct {
    u_int   raw_time;           /* raw time - 0.65536 ms * N */
    u_char  days;
    u_char  hours;
    u_char  minutes;
    u_char  seconds;
    u_short milli_seconds;
} pcie6509_uptime_t;

#define PCIE6509_POWERUP_TIME_MULTIPLIER    (0.65536)   /* milli-seconds * N */

/*** ChpServices Watchdog Status (R) (0x020068/0x040068) ***/
#define PCIE6509_WDTS_EXPIRE_COUNT_SHIFT    (8)
#define PCIE6509_WDTS_EXPIRE_COUNT_MASK     (0xFF)
#define PCIE6509_WDTS_STATE_SHIFT           (0)
#define PCIE6509_WDTS_STATE_MASK            (0x7)
typedef enum {
    PCIE6509_WDTS_STATE_SYNCH_RESET         = (0),
    PCIE6509_WDTS_STATE_COUNT_DOWN_FEED     = (1),
    PCIE6509_WDTS_STATE_COUNT_DOWN_FOOD     = (2),
    PCIE6509_WDTS_STATE_SLEEPING            = (3),
    PCIE6509_WDTS_STATE_EXPIRED_PULSE       = (5),
    PCIE6509_WDTS_STATE_EXPIRED             = (6),
} pcie6509_wdts_state_t;

typedef struct {
    u_char                  expiration_count;
    pcie6509_wdts_state_t   state;
} pci6509_watchdog_status_t;

/*** ChpServices Watchdog Timeout (W) (0x020068/0x040068) ***/
#define PCIE6509_BUS_CLOCK_FREQUENCY        (31250000.0)
#define PCIE6509_BUS_CLOCK_PERIOD           (1.0/PCIE6509_BUS_CLOCK_FREQUENCY)

/*** ChpServices Watchdog Configuration (W) - (0x02006C/0x04006C) ***/
#define PCIE6509_WDTC_EXTTRIGSEL_SHIFT        (0)
#define PCIE6509_WDTC_EXTTRIGSEL_MASK         (7)
#define PCIE6509_WDTC_EXTTRIGPOL_SHIFT        (7)
#define PCIE6509_WDTC_EXTTRIGPOL_MASK         (1)
#define PCIE6509_WDTC_EXTTRIGEN_SHIFT         (8)
#define PCIE6509_WDTC_EXTTRIGEN_MASK          (1)
#define PCIE6509_WDTC_INTTRIGEN_SHIFT         (9)
#define PCIE6509_WDTC_INTTRIGEN_MASK          (1)
typedef enum {
    PCIE6509_WDTC_RTSI0                     = (0),
    PCIE6509_WDTC_RTSI1                     = (1),
    PCIE6509_WDTC_RTSI2                     = (2),
    PCIE6509_WDTC_RTSI3                     = (3),
    PCIE6509_WDTC_RTSI4                     = (4),
    PCIE6509_WDTC_RTSI5                     = (5),
    PCIE6509_WDTC_RTSI6                     = (6),
    PCIE6509_WDTC_RTSI7                     = (7),
} pcie6509_wdtc_exttrigsel_t;

typedef enum {
    PCIE6509_WDTC_ACTIVE_HIGH               = (0),
    PCIE6509_WDTC_ACTIVE_LOW                = (1),
} pcie6509_wdtc_exttrigpol_t;

typedef enum {
    PCIE6509_WDTC_EXT_DISABLED              = (0),
    PCIE6509_WDTC_EXT_USE_EXTERNAL_TRIGGER  = (1),
} pcie6509_wdtc_exttrigen_t;

typedef enum {
    PCIE6509_WDTC_INT_DISABLED              = (0),
    PCIE6509_WDTC_INT_USE_INTERNAL_COUNTER  = (1),
} pcie6509_wdtc_inttrigen_t;

typedef struct {
    pcie6509_wdtc_exttrigsel_t              ExtTrigSel;
    pcie6509_wdtc_exttrigpol_t              ExtTrigPol;
    pcie6509_wdtc_exttrigen_t               ExtTrigEn;
    pcie6509_wdtc_inttrigen_t               IntTrigEn;
} pcie6509_wdt_config_t;

/*** ChpServices Watchdog Control (W) - ((0x02006E/0x04006E) ***/
#define PCIE6509_WDTCON_5678                0x5678
#define PCIE6509_WDTCON_FEED                0xFEED
#define PCIE6509_WDTCON_FOOD                0xF00D
#define PCIE6509_WDTCON_1234                0x1234
#define PCIE6509_WDTCON_DEAD                0xDEAD
#define PCIE6509_WDTCON_ACED                0xACED

typedef enum {
    PCIE6509_WDTCON_START                   = (PCIE6509_WDTCON_5678),
    PCIE6509_WDTCON_RESTART_FEED            = (PCIE6509_WDTCON_FEED),
    PCIE6509_WDTCON_RESTART_FOOD            = (PCIE6509_WDTCON_FOOD),
    PCIE6509_WDTCON_PAUSE                   = (PCIE6509_WDTCON_1234),
    PCIE6509_WDTCON_TERMINATE               = (PCIE6509_WDTCON_DEAD),
    PCIE6509_WDTCON_RESTART_ACED            = (PCIE6509_WDTCON_ACED),
    PCIE6509_WDTCON_RESTART_FEED_FOOD       = (PCIE6509_WDTCON_FEED+PCIE6509_WDTCON_FOOD)
                                                /* for set routine only */
}  pcie6509_wdt_control_t;
 
/*** ChpServices Watchdog Timer Interrupt 1 Register (W) - (0x020070/0x040070) ***/
#define PCIE6509_WTI1R_WDT_TRIGGER_IRQ_ACK      (1 << 16)   /* WDT Trigger IRQ Ack */
#define PCIE6509_WTI1R_WDT_TRIGGER_IRQ_ENABLE   (1 <<  0)   /* WDT Trigger IRQ Enable */

/*** ChpServices Watchdog Timer Interrupt 2 Register (W) - (0x020074/0x040074) ***/
#define PCIE6509_WTI2R_WDT_TRIGGER_IRQ_ACK      (1 << 16)   /* WDT Trigger IRQ Ack */
#define PCIE6509_WTI2R_WDT_TRIGGER_IRQ_DISABLE  (1 <<  0)   /* WDT Trigger IRQ Disable */

/*** ChpServices Global Interrupt Enable Register (W) - (0x020078/0x040078) ***/
#define PCIE6509_GIER_WTI_DISABLE       (1 << 26)   /* Watchdog Timer Interrupt Disable */
#define PCIE6509_GIER_DII_DISABLE       (1 << 22)   /* DI Interrupt Disable */
#define PCIE6509_GIER_WTI_ENABLE        (1 << 10)   /* Watchdog Timer Interrupt Enable */
#define PCIE6509_GIER_DII_ENABLE        (1 <<  6)   /* DI Interrupt Enable */

/*** ChpServices RTSI Configuration (W) - (0x0200A6-0x0200AF/0x0400A6-0x0400AF) ***/
#define PCIE6509_RTSI_DIRECTION_SHIFT    (8)
#define PCIE6509_RTSI_DIRECTION_MASK     (0xFF)

typedef enum {
    PCIE6509_RTSICON_EXPORT_DIO_CHANGE  = 13,
    PCIE6509_RTSICON_EXPORT_WDT_EXPIRE  = 14,
} pcie6509_rtsi_out_t;

typedef struct {
    u_char               CSRTSITrigDirection;
    pcie6509_rtsi_out_t  CSRTSIOutputSelect[PCIE6509_NUMBER_OF_RTSI_LINES];          
} pcie6509_rtsi_config_t;

/*** DioPortsDOWDTModeSelectP01 - (W) (0x0204D4/0x0404D4) ***/
/*** DioPortsDOWDTModeSelectP23 - (W) (0x0204D8/0x0404D8) ***/
/*** PFIWDTModeSelect - (W) (0x0200E4/0x0400E4) ***/
typedef enum {
    PCIE6509_WDT_MODE_DISABLE           = (0),    /* disabled */
    PCIE6509_WDT_MODE_FREEZE            = (1),    /* freeze */
    PCIE6509_WDT_MODE_TRISTATE          = (2),    /* tristate */
    PCIE6509_WDT_MODE_SAFE_VALUE        = (3),    /* safe value */
} pcie6509_watchdog_mode_t;

/*** ChpServices DI Change Detection Status (R) (0x020540/0x040540) ***/
#define PCIE6509_CDS_STATUS_SHIFT         (0)
#define PCIE6509_CDS_STATUS_MASK          (0x1)
#define PCIE6509_CDS_ERROR_SHIFT          (1)
#define PCIE6509_CDS_ERROR_MASK           (0x1)

typedef enum {
    PCIE6509_CDS_NO_CHANGE              = (0),
    PCIE6509_CDS_CHANGE_DETECTED        = (1),
} pcie6509_cds_status_t;

typedef enum {
    PCIE6509_CDS_NO_ERROR               = (0),
    PCIE6509_CDS_MULTI_CHANGE_DETECT    = (1),
} pcie6509_cds_error_t;

typedef struct {
    pcie6509_cds_status_t   status;
    pcie6509_cds_error_t    error;
} pcie6509_change_detection_status_t;

/*** DioPorts DI Filter - (W) (0x2054C/0x20550/0x4054C/0x40550) ***/
/*** DioPorts PFI Filter - (W) (0x200B0-0x200B6 and 0x400B0-0x400B6) ***/
/*** Port 0..3 and 8..9 (DI) - value set from value 0,1,2,3 ***/
/*** Port 4..7 (PFI) - value set from value 0,2,3,4 ***/
typedef enum {
    PCIE6509_FILTER_NONE        =      (0),        /* no filter */
    PCIE6509_FILTER_SMALL       =      (1),        /* Small Filter rejects less than 100ns */
    PCIE6509_FILTER_MEDIUM      =      (2),        /* Medium Filter rejects less than 6.4us */
    PCIE6509_FILTER_LARGE       =      (3),        /* Large Filter rejects less than 2.54ms */
} pcie6509_filter_option_t;

#define PCIE6509_DI_FILTER_VAL_NONE     (0)         /* no filter */
#define PCIE6509_DI_FILTER_VAL_SMALL    (1)         /* Small Filter rejects less than 100ns */
#define PCIE6509_DI_FILTER_VAL_MEDIUM   (2)         /* Medium Filter rejects less than 6.4us */
#define PCIE6509_DI_FILTER_VAL_LARGE    (3)         /* Large Filter rejects less than 2.54ms */

#define PCIE6509_PFI_FILTER_VAL_NONE    (0)         /* no filter */
#define PCIE6509_PFI_FILTER_VAL_SMALL   (2)         /* Small Filter rejects less than 100ns */
#define PCIE6509_PFI_FILTER_VAL_MEDIUM  (3)         /* Medium Filter rejects less than 6.4us */
#define PCIE6509_PFI_FILTER_VAL_LARGE   (4)         /* Large Filter rejects less than 2.54ms */

/*** ChpService Change Detection Irq Register (W) - (0x20554/0x40554) ***/
#define PCIE6509_CDIR_CHANGE_DETECTION_ERROR_IRQ_ENABLE     (1 << 7)    /* Change Detection Error IRQ Enable */
#define PCIE6509_CDIR_CHANGE_DETECTION_ERROR_IRQ_DISABLE    (1 << 6)    /* Change Detection Error IRQ Disable */
#define PCIE6509_CDIR_CHANGE_DETECTION_IRQ_ENABLE           (1 << 5)    /* Change Detection IRQ Enable */
#define PCIE6509_CDIR_CHANGE_DETECTION_IRQ_DISABLE          (1 << 4)    /* Change Detection IRQ Disable */
#define PCIE6509_CDIR_CHANGE_DETECTION_ERROR_IRQ_ACK        (1 << 1)    /* Change Detection ERROR IRQ Ack */
#define PCIE6509_CDIR_CHANGE_DETECTION_IRQ_ACK              (1 << 0)    /* Change Detection IRQ Ack */

/*** ChpService IntForwarding Control Status Register (R/W) - (0x022204/0x042204) ***/
#define PCIE6509_ICSR_INT_FORWARDING_RESET      (1 << 1)    /* Interrupt Forwarding Reset */
#define PCIE6509_ICSR_INT_FORWARDING_ENABLE     (1 << 0)    /* Interrupt Forwarding Enable */

/*** ChpServices Joint Reset (W) - (0x020004/0x040004) ***/
#define PCIE6509_JRR_SOFTWARE_RESET             (1 << 0)    /* Software Reset */

/*** PfiPorts PFI Output Select Register (W) - 0x0200BA-0x0200C9/0x0400BA-0x0400C9 ***/
#define PCIE6509_PFI_OUTPUT_SELECT_VALUE         (0x10)

/******************************************************************************
 ***                                                                        ***
 ***              <<<< Define Board Specific IOCTLs Here >>>>               ***
 ***                                                                        ***
 ******************************************************************************/

#define IOCTL_PCIE6509_MAGIC   'a'

 /**
  ** IOCTL defines.  See below for API details.
  **
  **/

#define IOCTL_PCIE6509_ADD_IRQ                _IO  (IOCTL_PCIE6509_MAGIC,1)
#define IOCTL_PCIE6509_DISABLE_PCI_INTERRUPTS _IO  (IOCTL_PCIE6509_MAGIC,2)
#define IOCTL_PCIE6509_ENABLE_PCI_INTERRUPTS  _IO  (IOCTL_PCIE6509_MAGIC,3)
#define IOCTL_PCIE6509_GET_DRIVER_ERROR       _IOR (IOCTL_PCIE6509_MAGIC,4, pcie6509_user_error_t *)
#define IOCTL_PCIE6509_GET_DRIVER_INFO        _IOR (IOCTL_PCIE6509_MAGIC,5,pcie6509_driver_info_t *)
#define IOCTL_PCIE6509_GET_PHYSICAL_MEMORY    _IOR (IOCTL_PCIE6509_MAGIC,6,pcie6509_phys_mem_t *)
#define IOCTL_PCIE6509_INIT_BOARD             _IO  (IOCTL_PCIE6509_MAGIC,7)
#define IOCTL_PCIE6509_MMAP_SELECT            _IOR (IOCTL_PCIE6509_MAGIC,8, unsigned long *)
#define IOCTL_PCIE6509_NO_COMMAND             _IO  (IOCTL_PCIE6509_MAGIC,9)
#define IOCTL_PCIE6509_REMOVE_IRQ             _IO  (IOCTL_PCIE6509_MAGIC,10)
#define IOCTL_PCIE6509_RESET_BOARD            _IO  (IOCTL_PCIE6509_MAGIC,11)
#define IOCTL_PCIE6509_WAIT_FOR_INTERRUPT     _IO  (IOCTL_PCIE6509_MAGIC,15)
#define IOCTL_PCIE6509_WAKE_INTERRUPT         _IO  (IOCTL_PCIE6509_MAGIC,16)
#define IOCTL_PCIE6509_GET_TIMEOUT            _IOR (IOCTL_PCIE6509_MAGIC,17, int)
#define IOCTL_PCIE6509_SET_TIMEOUT            _IOW (IOCTL_PCIE6509_MAGIC,18, int)
#define IOCTL_PCIE6509_GET_INTERRUPT_COUNTER  _IOR (IOCTL_PCIE6509_MAGIC,19, int)
#define IOCTL_PCIE6509_SET_INTERRUPT_COUNTER  _IOW (IOCTL_PCIE6509_MAGIC,20, int)

/******************************************************************************
 ***                                                                        ***
 ***              <<<< Define Board Specific Errors Here >>>>               ***
 ***                                                                        ***
 ******************************************************************************/
/*** The following pcie6509_driver_error_t structure is used for the user ioctl
 *** call.
 ***/
#define PCIE6509_ERROR_NAME_SIZE    64
#define PCIE6509_ERROR_DESC_SIZE    128
typedef struct _pcie6509_user_error_t {
    uint    error;                            /* error number */
    char    name[PCIE6509_ERROR_NAME_SIZE];   /* error name used in driver */
    char    desc[PCIE6509_ERROR_DESC_SIZE];   /* error description */
} pcie6509_user_error_t;

enum    {
    PCIE6509_SUCCESS = 0,
    PCIE6509_INVALID_PARAMETER,
    PCIE6509_TIMEOUT,
    PCIE6509_OPERATION_CANCELLED,
    PCIE6509_RESOURCE_ALLOCATION_ERROR,
    PCIE6509_INVALID_REQUEST,
    PCIE6509_FAULT_ERROR,
    PCIE6509_BUSY,
    PCIE6509_ADDRESS_IN_USE,
};


/******************************************************************************
 ***                                                                        ***
 ***              <<<< Define Board Specific MMAPs Here >>>>                ***
 ***                                                                        ***
 ******************************************************************************/
typedef struct {
    void            *phys_mem;      /* physical memory: physical address    */
    unsigned int    phys_mem_size;  /* physical memory: memory size - bytes */
} pcie6509_phys_mem_t;
 
/* IOCTL_MMAP_SELECT
 *
 * MMAP Selection Commands 
 */
typedef struct {
    unsigned    int     select;         /* mmap() selection */
    unsigned    long    offset;         /* returned offset */
    unsigned    long    size;           /* returned size */
} pcie6509_mmap_select_t;

#define PCIE6509_SELECT_LOCAL_MMAP      1         /* Select local Register */
#define PCIE6509_SELECT_SHADOW_REG_MMAP 2         /* MMAP shadow memory */
#define PCIE6509_SELECT_PHYS_MEM_MMAP   3         /* MMAP physical memory */

#define PCIE6509_LOCAL_MMAP_SIZE      (512*1024)/* LOCAL MMAP Size */

/******************************************************************************
 ***                                                                        ***
 ***              <<<< Define Driver Information  Here >>>>                 ***
 ***                                                                        ***
 ******************************************************************************/

#define PCIE6509_MAX_REGION 32

typedef struct
{
    char                    version[12];        /* driver version */
    char                    built[32];          /* driver date built */
    char                    module_name[16];    /* driver name */
    int                     board_type;         /* board type */
    int                     device;             /* device */
    char                    board_desc[32];     /* board description */
    int                     bus;                /* bus number */
    int                     slot;               /* slot number */
    int                     func;               /* function number */
    int                     vendor_id;          /* vendor id */
    int                     device_id;          /* device id */
    int                     board_id;           /* board id */
    int                     MasterSignature;    /* Master Signature number if applicable */
    int                     SlaveSignature;     /* Slave Signature number if applicable */
    pcie6509_driver_int_t   interrupt;          /* interrupt struct */
    pcie6509_dev_region_t   mem_region[PCIE6509_MAX_REGION];
} pcie6509_driver_info_t;


#endif    /* __PCIE6509_IOCTL_H_INCLUDED__ */

