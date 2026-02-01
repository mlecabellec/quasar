/*****************************************************************************
 *                                                                           *
 * File:         pcie6509_tst.c                                              *
 *                                                                           *
 * Description:  Interactive Driver test                                     *
 *                                                                           *
 * Syntax:                                                                   *
 *   pcie6509_tst             ==> start interactive test on device 0.        *
 *   pcie6509_tst <dn>        ==> start interactive test on device <dn>      *
 *                                (where dn is device number)                *
 *                                                                           *
 * Date:        02/13/2014                                                   *
 * History:                                                                  *
 *                                                                           *
 *  1  02/13/14 D. Dubash                                                    *
 *              Initial release                                              *
 *                                                                           *
 *****************************************************************************
 *                                                                           *
 * Copyright (C) 2010 and beyond Concurrent Computer Corporation             *
 * All rights reserved                                                       *
 *                                                                           *
 *****************************************************************************/

/*****************************************************************************
 *                                                                           *
 *                            == DISCLAIMER ==                               *
 *                                                                           *
 *    The source code enclosed has been included as an aid in the            *
 *    development of your application, and while believed to be accurate and *
 *    fully functional code, is in NO WAY to be held to the standard of      *
 *    normal supported and maintained source code that has been stringently  *
 *    tested and debugged for the purposes currently offered.                *
 *    The attached source code is offered "AS IS" and as such will not be    *
 *    supported by its author or any other employee of Concurrent Computer   *
 *    Corporation.                                                           *
 *                                                                           *
 *****************************************************************************/

/*
 * Headers
 */

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <sys/mman.h>
#include <math.h>
#include <time.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <unistd.h>

#include "pcie6509_user.h"
#include "pcie6509_snap.h"

#define DEF_DEV_NAME   "/dev/" PCIE6509_DRIVER_NAME
#define    DEF_BOARD_NO    "0"

#define FLUSH_SIZE      200

// volatile struct pcie6509_registers parm;
void quit(int s);
int user_input(char *input, char **retptr);
void PortDirection(pcie6509_local_ctrl_data_t *lp, pcie6509_shadow_regs_t *sp, int port, int output);
void PortDirectionAll(pcie6509_local_ctrl_data_t *lp, pcie6509_shadow_regs_t *sp, int output);

// Global variables
int           c, pcie6509_break_received;
char          devname[15];
int           fp, status;
pcie6509_driver_info_t driver_info;
char flush[FLUSH_SIZE];
char *cp;

pcie6509_local_ctrl_data_t *local_ptr;
pcie6509_shadow_regs_t *shadow_ptr;

typedef struct _what {
    uint    cmd;
    char    *name;
    char    *desc;
} what;

#define MMAP_PCIE6509_SELECT_LOCAL_MMAP     -1
#define MMAP_PCIE6509_SELECT_SHADOW_REGS    -2
#define MMAP_PCIE6509_SELECT_PHYS_MEM       -3 
#define MUNMAP_PCIE6509_PHYS_MEM            -4 
#define PCIE6509_SELECT_READ                -5
#define PCIE6509_SELECT_WRITE               -6

#define CMD_BASE _IO(IOCTL_DEVICE_MAGIC,0)
#define CMD(WhichCmd) (WhichCmd - CMD_BASE)

static int munmap_dma_mem_size, *munmap_dma_mem_ptr;

what    command[]= {
    { IOCTL_PCIE6509_ADD_IRQ,
      "IOCTL_PCIE6509_ADD_IRQ",                 "add irq" },
    { IOCTL_PCIE6509_DISABLE_PCI_INTERRUPTS,
      "IOCTL_PCIE6509_DISABLE_PCI_INTERRUPTS",  "disable pci interrupts" },
    { IOCTL_PCIE6509_ENABLE_PCI_INTERRUPTS,
      "IOCTL_PCIE6509_ENABLE_PCI_INTERRUPTS",   "enable pci interrupts" },
    { IOCTL_PCIE6509_GET_DRIVER_ERROR,
      "IOCTL_PCIE6509_GET_DRIVER_ERROR",        "get device error" },
    { IOCTL_PCIE6509_GET_DRIVER_INFO,
      "IOCTL_PCIE6509_GET_DRIVER_INFO",         "get driver info" },
    { IOCTL_PCIE6509_GET_PHYSICAL_MEMORY,
      "IOCTL_PCIE6509_GET_PHYSICAL_MEMORY",     "get physical mem" },
    { IOCTL_PCIE6509_INIT_BOARD,
      "IOCTL_PCIE6509_INIT_BOARD",              "init board" },
    { IOCTL_PCIE6509_MMAP_SELECT,
      "IOCTL_PCIE6509_MMAP_SELECT",             "mmap select" },
    { MMAP_PCIE6509_SELECT_SHADOW_REGS,
      "mmap(PCIE6509_SELECT_SHADOW_REGS)",      "mmap(Shadow registers)" },
    { MMAP_PCIE6509_SELECT_LOCAL_MMAP,
      "mmap(PCIE6509_SELECT_LOCAL_MMAP)",       "mmap(LOCAL registers)" },
    { MMAP_PCIE6509_SELECT_PHYS_MEM,
      "mmap(PCIE6509_SELECT_PHYS_MEM)",         "mmap(physical memory)" },
    { MUNMAP_PCIE6509_PHYS_MEM,
      "munmap(PCIE6509_PHYS_MEM)",              "munmap(physical memory)" },
    { IOCTL_PCIE6509_NO_COMMAND,
      "IOCTL_PCIE6509_NO_COMMAND",              "no command" },
    { PCIE6509_SELECT_READ,
      "read()",                                 "read operation" },
    { IOCTL_PCIE6509_REMOVE_IRQ,
      "IOCTL_PCIE6509_REMOVE_IRQ",              "remove irq" },
    { IOCTL_PCIE6509_RESET_BOARD,
      "IOCTL_PCIE6509_RESET_BOARD",             "reset board" },
    { PCIE6509_SELECT_WRITE,
      "write()",                                "write operation" },

    { 0, 0, 0, }    /* list terminator */
};

/* prototype */
void Ioctl_Range_Selection(char *desc, int min, int max,
                int ioctl_request, char *ioctl_desc );
int  Ioctl_Print_Selection(what *table);
int  Table_Selection(what *table, char *desc, void *ret_parm );
void Ioctl_Table_Selection(what *table, char *desc, int ioctl_request, 
                            char *ioctl_desc, void *ret_parm );
int  Initialize_Board();
void BadArg(char *arg);
void table_1();
int  Do_Selection(char *cp);
void Reset_Board();

void ioctl_PCIE6509_get_driver_info();
void ioctl_PCIE6509_get_physical_memory();
void ioctl_PCIE6509_get_device_error();
void ioctl_PCIE6509_mmap_get_offset();
void mmap_PCIE6509_select_local_mmap();
void mmap_PCIE6509_select_shadow_regs();
void mmap_PCIE6509_select_phys_mem();
void munmap_PCIE6509_phys_mem();
void PCIE6509_select_read();
void PCIE6509_select_write();

/*
 * Main entry point...
 */
int main(int argc, char **argv)
{
    char    *endptr;
    char    *bnp=0;

    if(argc > 2)    /* more than one argument, error out */
        BadArg(0);

    if(argc == 2) {    /* only one argument specified */
        bnp=argv[1];
        strtol(bnp, &endptr, 10);
        if(!endptr || *endptr != 0) 
            BadArg(argv[1]);
    }
    
    if(!bnp)    /* if no device number entered, use default */
        bnp = DEF_BOARD_NO;

    sprintf(devname,"%s%s",DEF_DEV_NAME,bnp);
    fprintf(stderr,"\nDevice Name: %s\n",devname);

    signal(SIGINT, quit);

    Initialize_Board();
            
    table_1();

    do {
        fprintf(stderr,"Main Selection ('h'=display menu, 'q'=quit)-> ");

        if(user_input(flush,&cp) == 0)   /* if break received, skip */
            continue;

        if(cp[0] == '\0') /* if no entry....repeat selection */
            continue;

        if(strcmp(cp,"h") == 0) {   /* if help, display table */
            table_1();
            continue;
        }

        if(strcmp(cp,"q") == 0) {   /* if quit, exit test */
            break;
        } 
                
        Do_Selection(cp);

    } while (1);

    if (fp) {
        Reset_Board();
        close(fp);
    }

    exit(0);
}

/****************************************************************************
 * user input                                                               *
 ****************************************************************************/
int
user_input(char *input, char **retptr)
{
    int     i;
    char    *cp;

    pcie6509_break_received = 0;

    fgets(input, FLUSH_SIZE, stdin);

    /* clean up input string */
    cp = flush;
    while((*cp == ' '))cp++; /* skip leading blanks */
    i = strlen(cp) - 1; /* point 2 end of string and remove blanks*/
    while((cp[i] == '\n') || (cp[i] == ' ')) {
        cp[i] = 0;
        if(i==0) break;
        i--;
    }

    if(pcie6509_break_received) {
        if((strcmp(cp,"y") == 0) || (strcmp(cp,"q")==0)) {
            if (fp) {
                Reset_Board();
                close(fp);
            }
            exit(1);
        } else
            signal(SIGINT, quit);
    }

    if(retptr)
        *retptr = cp;

    /* if break received, return zero */
    return(!pcie6509_break_received);
}

/****************************************************************************
 * break interrupt received                                                 *
 ****************************************************************************/
void quit(int s)
{
    pcie6509_break_received = 1;

    fprintf(stderr,
            "\nTERMINATE RECEIVED!!! Shutdown/quit Test? (q,y/n)->");
}

/*** Initialize Board Routine ***/
int
Initialize_Board()
{
    /*** Open the device ***/
    fp = open(devname, O_RDWR);
    if (fp == -1) {
        printf("open() failure on %s: [%s]\n", devname, strerror(errno));
        exit(1);
    }

            
    //
    // Initialize Board          
    //
    status = ioctl(fp, IOCTL_PCIE6509_INIT_BOARD, NULL);
    if (status < 0) {
        fprintf(stderr,
                "%s: ioctl IOCTL_PCIE6509_INIT_BOARD failed - [%s]\n",
                devname, strerror(errno));
        return (1);
    }

    mmap_PCIE6509_select_local_mmap();
    mmap_PCIE6509_select_shadow_regs();
    
    //
    // Get Firmware Revision
    //
    status =
        ioctl(fp, IOCTL_PCIE6509_GET_DRIVER_INFO,
              &driver_info);
    if (status != 0) {
        fprintf(stderr,
                "%s: ioctl IOCTL_PCIE6509_GET_DRIVER_INFO failed - [%s]\n",
                devname, strerror(errno));
        return (1);
    }
    fprintf(stderr, "Board Signature: Master 0x%x, Slave 0x%x  successful\n",
        driver_info.MasterSignature,driver_info.SlaveSignature);

    return (0);
}


/******************************************************************************
 *** Bad argument message and abort                                         ***
 ******************************************************************************/
void
BadArg(char *arg)
{
    if(arg)
        fprintf(stderr,"\n*** Invalid argument [%s] ***\n",arg);
    else
        fprintf(stderr,"\n*** Only one argument must be specified ***\n");

    fprintf(stderr,"Usage: pcie6509_tst <device number>\n");
    exit(1);
}

#define STRING_LEN   30
/****************************************************************************
 * Print Table 1 Menu                                                       *
 ****************************************************************************/
void table_1()
{
    int     i, j;
    char    s[STRING_LEN+1];
    
    i = 0;
    while(command[i].cmd) {
        strncpy(s, command[i].desc, STRING_LEN);
        for(j=strlen(s);j<STRING_LEN;j++) s[j]=' '; /* pad with space */
        s[j] = '\0';
        if(i%2)
            fprintf(stderr,"%02d = %s\n",i+1,s);
        else
            fprintf(stderr,"  %02d = %s",i+1,s);
        i++;
    }

    fprintf(stderr,"\n");

    if(i%2)
        fprintf(stderr,"\n");
}

// reset board
void
Reset_Board()
{
    //
    // Reset Board          
    //
    status = ioctl(fp, IOCTL_PCIE6509_RESET_BOARD, NULL);
    if (status < 0) {
        fprintf(stderr,
                "%s: ioctl IOCTL_PCIE6509_RESET_BOARD failed - [%s]\n",
                devname, strerror(errno));
    }
}

int
Do_Selection(char *cp)
{
    char        *ep;
    int         cmd, num_cmds, i;
    long int    iSelection;

    iSelection = strtol(cp, &ep, 10) - 1;

    i=0;
    while(ep[i]== ' ')i++;      /* skip trailing blanks */

    num_cmds = sizeof(command)/sizeof(what) - 1; /* last entry not a command */

    if((ep[i] != '\0') || (iSelection < 0) || (iSelection > (num_cmds -1))) {
        fprintf(stderr,"### Invalid Selection [%s] ###\n",cp);
        return (1);
    }

    cmd = command[iSelection].cmd;

    fprintf(stderr,"   Command: %s\n",command[iSelection].name);

    switch (cmd) {
        case IOCTL_PCIE6509_ADD_IRQ:
        case IOCTL_PCIE6509_DISABLE_PCI_INTERRUPTS:
        case IOCTL_PCIE6509_ENABLE_PCI_INTERRUPTS:
        case IOCTL_PCIE6509_INIT_BOARD:
        case IOCTL_PCIE6509_NO_COMMAND:
        case IOCTL_PCIE6509_REMOVE_IRQ:
        case IOCTL_PCIE6509_RESET_BOARD:
            status = ioctl(fp, cmd, NULL);
            if (status < 0) {
                fprintf(stderr,
                    "%s: ioctl %s failed - [%s]\n", devname, 
                    command[iSelection].name, strerror(errno));
                return (1);
            }
        break;

        case IOCTL_PCIE6509_GET_DRIVER_ERROR:
            ioctl_PCIE6509_get_device_error();
        break;

        case IOCTL_PCIE6509_GET_DRIVER_INFO:
            ioctl_PCIE6509_get_driver_info();
        break;

        case IOCTL_PCIE6509_GET_PHYSICAL_MEMORY:
            ioctl_PCIE6509_get_physical_memory();
        break;

        case IOCTL_PCIE6509_MMAP_SELECT:
            ioctl_PCIE6509_mmap_get_offset();
        break;

        case MMAP_PCIE6509_SELECT_SHADOW_REGS:
            mmap_PCIE6509_select_shadow_regs();
        break;

        case MMAP_PCIE6509_SELECT_LOCAL_MMAP:
            mmap_PCIE6509_select_local_mmap();
        break;

        case MMAP_PCIE6509_SELECT_PHYS_MEM:
            mmap_PCIE6509_select_phys_mem();
        break;

        case MUNMAP_PCIE6509_PHYS_MEM:
            munmap_PCIE6509_phys_mem();
        break;

        case PCIE6509_SELECT_READ:
            PCIE6509_select_read();
        break;

        case PCIE6509_SELECT_WRITE:
            PCIE6509_select_write();
        break;

        default:
            fprintf(stderr,"Command %ld: %s not implemented\n",
                   iSelection+1, command[iSelection].name);

        break;
    }

    return (0);
}
/***********************************************************************/

/***********************************************************************/
void
ioctl_PCIE6509_get_physical_memory()
{
    pcie6509_phys_mem_t pcie6509_phys_mem;

    pcie6509_phys_mem.phys_mem = 0;
    pcie6509_phys_mem.phys_mem_size = 0;
        
    status = ioctl(fp, IOCTL_PCIE6509_GET_PHYSICAL_MEMORY, 
                                &pcie6509_phys_mem);
    if (status) {
        fprintf(stderr,
            "ioctl(IOCTL_PCIE6509_GET_PHYSICAL_MEMORY) failed: [%s]\n",
            strerror(errno));
        fprintf(stderr,"Make sure physical memory is first mmapped()\n");
        return;
    }

    munmap_dma_mem_size = pcie6509_phys_mem.phys_mem_size;

    fprintf(stderr,"Physical Memory: address=%p, size=%d (0x%x)\n",
                pcie6509_phys_mem.phys_mem, pcie6509_phys_mem.phys_mem_size, 
                pcie6509_phys_mem.phys_mem_size);
    
}
/***********************************************************************/

/***********************************************************************/
void
Ioctl_Range_Selection(char *desc, int min, int max, 
                      int ioctl_request, char *ioctl_desc )
{
    long int input;
    fprintf(stderr,"   Enter %s value [%d-%d] (0x%x-0x%x) ->", 
                desc, min, max, min, max);

    if(user_input(flush,&cp) == 0)  /* if break received, skip */
        return;

    input = strtoul(cp,NULL,0);

    if((input < min) || (input > max)) {
        fprintf(stderr,"### ERROR! Invalid %s value %ld ###\n",desc,
                input);
        return;
    }

    status = ioctl(fp, ioctl_request,(void *)&input);

    if(status) {
        fprintf(stderr,"ioctl(%s) failed: %s\n", ioctl_desc,
                            strerror(errno));
        return;
    }
}
/***********************************************************************/

/***********************************************************************/
int
Ioctl_Print_Selection(what *table)
{
    int i;
    i = 0;
    while(table[i].name) {
        fprintf(stderr,"      %d: %s\n",i+1, table[i].name);
        i++;
    }
    return(i);
}

int
Table_Selection(what *table, char *desc, void *ret_parm )
{
    char  	    *cp;
    char            *ep;
    int             num_entries;
    long    int     iSelection, parm;
    int             ret_code=0;

    num_entries = Ioctl_Print_Selection(table);

    do {
        fprintf(stderr,
            "   Input %s Selection ('h'=display menu, 'q'=quit)-> ", desc);

        if(user_input(flush,&cp) == 0)  /* if break received, skip */
            break;

        if(cp[0] == '\0') /* if no entry....repeat selection */
            continue;

        if(strcmp(cp,"h") == 0) {   /* if help, display table */
            Ioctl_Print_Selection(table);
            continue;
        }

        if(strcmp(cp,"q") == 0) {   /* if quit, exit test */
            break;
        } 

        iSelection = strtol(cp, &ep, 10) - 1;

        if((ep[0] != '\0') || (iSelection > (num_entries-1))
            || (iSelection < 0)) {
            fprintf(stderr,"### Invalid Selection [%s] ###\n",cp);
            continue;
        }

        parm = table[iSelection].cmd;
        ret_code=1; /* entry selected return */

        if(ret_parm)
            *((int *)ret_parm) = parm;

        break;  /* done....break out */

    } while(1);

    return(ret_code);
}

void
Ioctl_Table_Selection(what *table, char *desc, int ioctl_request, 
                      char *ioctl_desc, void *ret_parm )
{
    long int local_parm;

    if(Table_Selection(table, desc, &local_parm ) == 0)
        return;

    status = ioctl(fp, ioctl_request, &local_parm);
    if (status) {
        fprintf(stderr, "ioctl(%s) failed: [%s]\n",ioctl_desc,
                            strerror(errno));
        if(ret_parm)
            *((int *)ret_parm) = -1;

         return;
    }

    if(ret_parm)
        *((int *)ret_parm) = local_parm;
}
/***********************************************************************/

/***********************************************************************/
what get_device_error[]= {
    { 1,    "GET DEVICE ERROR","" },
    { 2,    "CLEAR DEVICE ERROR","" },

    { 0, 0, 0, }    /* list terminator */
};
void
ioctl_PCIE6509_get_device_error()
{
    pcie6509_user_error_t   ret_err;
    int                     local_parm;

    if(Table_Selection(get_device_error, "Get Device Error", &local_parm ) == 0)
        return;

    switch(local_parm) {
        case 1: /* GET DEVICE ERROR */
            status = ioctl(fp, IOCTL_PCIE6509_GET_DRIVER_ERROR, 
                                &ret_err);
            if (status) {
                fprintf(stderr,
                    "ioctl(IOCTL_PCIE6509_GET_DRIVER_ERROR) failed: [%s]\n",
                    strerror(errno));
                return;
            }

            fprintf(stderr,"===============================================\n");
            fprintf(stderr,"driver error information:\n");
            fprintf(stderr,"   error:   %d\n",ret_err.error);
            fprintf(stderr,"    name:   %s\n",ret_err.name);
            fprintf(stderr,"    desc:   %s\n",ret_err.desc);
            fprintf(stderr,"===============================================\n");
        break;

        case 2: /* CLEAR DEVICE ERROR */
            status = ioctl(fp, IOCTL_PCIE6509_GET_DRIVER_ERROR, 
                                NULL);
            if (status) {
                fprintf(stderr,
                    "ioctl(IOCTL_PCIE6509_GET_DRIVER_ERROR) failed: [%s]\n",
                    strerror(errno));
                return;
            }
        break;
    }
}
/***********************************************************************/

/***********************************************************************/
what mmap_get_offset[]= {
    { PCIE6509_SELECT_LOCAL_MMAP,       "PCIE6509_SELECT_LOCAL_MMAP","" },
    { PCIE6509_SELECT_SHADOW_REG_MMAP,  "PCIE6509_SELECT_SHADOW_REG_MMAP","" },
    { PCIE6509_SELECT_PHYS_MEM_MMAP,    "PCIE6509_SELECT_PHYS_MEM_MMAP","" },

    { 0, 0, 0, }    /* list terminator */
};

void
ioctl_PCIE6509_mmap_get_offset()
{
    int                     ret_parm;
    pcie6509_mmap_select_t  mmap_select;

    if(Table_Selection(mmap_get_offset, "Get MMAP Offset", &ret_parm ) == 0)
        return;

    mmap_select.select = ret_parm;
    mmap_select.size = 0;
    mmap_select.offset = 0;

    status = ioctl(fp, IOCTL_PCIE6509_MMAP_SELECT, &mmap_select);
    if (status) {
        if(errno == ENOMEM) {
            fprintf(stderr, "Selected Region Not Present\n");
        } else {
            fprintf(stderr, "ioctl(IOCTL_PCIE6509_MMAP_SELECT) failed: [%s]\n",
                strerror(errno));
        }
        return;
    }

    fprintf(stderr,"Returned Offset: 0x%lx, size=%ld (0x%lx)\n",mmap_select.offset,
                                 mmap_select.size, mmap_select.size);
}
/***********************************************************************/

/***********************************************************************/
void
mmap_PCIE6509_select_local_mmap()
{
    int                     *localptr;
    int                     *munmap_localptr;
    pcie6509_mmap_select_t  mmap_select;

    /*** Select LOCAL Registers ***/
    /*** This IOCTL must be called before mmap() ***/
    mmap_select.select = PCIE6509_SELECT_LOCAL_MMAP;
    mmap_select.offset=0;
    mmap_select.size=0;
    status = ioctl(fp, IOCTL_PCIE6509_MMAP_SELECT,(void *)&mmap_select);

    if(status) {
        if(errno == ENOMEM) {
            fprintf(stderr, "Local Region Not Present\n");
        } else {
            fprintf(stderr, "ioctl(IOCTL_PCIE6509_MMAP_SELECT) failed: [%s]\n",
                strerror(errno));
        }
        return;
    }
        
    /*** Map LOCAL PCIE6509 CONTROL AND STATUS REGISTERS ***/
    localptr = munmap_localptr = (int *) mmap((caddr_t)0 ,
                        mmap_select.size,
                        (PROT_READ|PROT_WRITE), MAP_SHARED, fp, 0);

    if(localptr == MAP_FAILED) {
        fprintf(stderr,"mmap() failure on %s: %s\n",devname,strerror(errno));
        exit(1);
    }

    localptr = (int *)((char *)localptr + mmap_select.offset);

    local_ptr = (pcie6509_local_ctrl_data_t *)localptr;

    fprintf(stderr,"Local Register virtual Address: %p\n", localptr);
}
/***********************************************************************/

/***********************************************************************/
void
mmap_PCIE6509_select_shadow_regs()
{
    int                     *shadowptr;
    int                     *munmap_shadowptr;
    pcie6509_mmap_select_t  mmap_select;

    /*** Select SHADOW Registers ***/
    /*** This IOCTL must be called before mmap() ***/
    mmap_select.select = PCIE6509_SELECT_SHADOW_REG_MMAP;
    mmap_select.offset=0;
    mmap_select.size=0;
    status = ioctl(fp, IOCTL_PCIE6509_MMAP_SELECT,(void *)&mmap_select);

    if(status) {
        if(errno == ENOMEM) {
            fprintf(stderr, "Shadow Registers Not Present\n");
        } else {
            fprintf(stderr, "ioctl(IOCTL_PCIE6509_MMAP_SELECT) failed: [%s]\n",
                strerror(errno));
        }
        return;
    }
        
    /*** Map LOCAL PCIE6509 CONTROL AND STATUS REGISTERS ***/
    shadowptr = munmap_shadowptr = (int *) mmap((caddr_t)0 ,
                        mmap_select.size,
                        (PROT_READ|PROT_WRITE), MAP_SHARED, fp, 0);

    if(shadowptr == MAP_FAILED) {
        fprintf(stderr,"mmap() failure on %s: %s\n",devname,strerror(errno));
        exit(1);
    }

    shadowptr = (int *)((char *)shadowptr + mmap_select.offset);

    shadow_ptr = (pcie6509_shadow_regs_t *)shadowptr;

    fprintf(stderr,"Shadow Register virtual Address: %p\n", shadowptr);
}
/***********************************************************************/

/***********************************************************************/
void
mmap_PCIE6509_select_phys_mem()
{
    int                     physical_size;
    int                     *dma_mem_ptr;
    pcie6509_mmap_select_t  mmap_select;

    fprintf(stderr,"   Enter physical memory size in bytes ->");

    if(user_input(flush,&cp) == 0)  /* if break received, skip */
        return;

    physical_size = strtoul(cp,NULL,0);

    if(physical_size == 0) {
        fprintf(stderr,"### ERROR! Length of zero is invalid ###\n");
        return;
    }

    /*** Select Physical Memory */
    /*** This IOCTL must be called before mmap() ***/
    mmap_select.select = PCIE6509_SELECT_PHYS_MEM_MMAP;
    mmap_select.offset=0;
    mmap_select.size=0;
    status = ioctl(fp, IOCTL_PCIE6509_MMAP_SELECT,(void *)&mmap_select);

    if(status) {
        fprintf(stderr,"ioctl(IOCTL_PCIE6509_MMAP_SELECT) failed: %s\n",
                            strerror(errno));
        return;
    }
        
    /*** Map PHYSICAL MEMORY ***/
    dma_mem_ptr = (int *) mmap((caddr_t)0 ,physical_size,
                            (PROT_READ|PROT_WRITE), MAP_SHARED, fp, 0);

    if(dma_mem_ptr == MAP_FAILED) {
        fprintf(stderr,"mmap() failure on %s: %s\n",devname,strerror(errno));
        return;
    }
        
    munmap_dma_mem_ptr = dma_mem_ptr;

    dma_mem_ptr = (int *)((char *)dma_mem_ptr + mmap_select.offset);

    fprintf(stderr,"Virtual Address: %p\n", dma_mem_ptr);

    /*** now display the physical memory that was allocated by the driver ***/
    ioctl_PCIE6509_get_physical_memory();
}
/***********************************************************************/

/***********************************************************************/
void
munmap_PCIE6509_phys_mem()
{
    if(munmap_dma_mem_ptr && munmap_dma_mem_size) {
        fprintf(stderr, "munmap_dma_mem_ptr=%p, size=0x%x\n",
            munmap_dma_mem_ptr, munmap_dma_mem_size);
        munmap(munmap_dma_mem_ptr, munmap_dma_mem_size);
        munmap_dma_mem_ptr=0;
        munmap_dma_mem_size=0;
    }
}
/***********************************************************************/

/***********************************************************************/
void
PCIE6509_select_read()
{
    int     bytes_read;
    int     port;
    u_short port_mask;
    pcie6509_io_port_t   rp;
    
    port_mask   = 0x003F;   /* read ports 0..6 */

    rp.port_mask = port_mask;

    for(port=0;port<6;port++) 
        PortDirection(local_ptr,shadow_ptr,port,0); /* set ports for input */

    bytes_read = read(fp, &rp, sizeof(rp));
    
    if(bytes_read == -1) {
        fprintf(stderr,"read() failed: %s\n",strerror(errno));
        return;
    }

    fprintf(stderr,"Number of Ports Read: %d\n",bytes_read);

    fprintf(stderr,"Port Mask: 0x%04x\n",rp.port_mask);
    for(port=0; port < 6; port++)
        fprintf(stderr,"   Port%02d: 0x%02x\n",port, rp.line_mask[port]);

}
/***********************************************************************/

/***********************************************************************/
void
PCIE6509_select_write()
{
    int     bytes_written;
    int     port;
    u_short port_mask;
    pcie6509_io_port_t   rp;
    
    port_mask   = 0x0FC0;   /* write 6..11 ports */

    rp.port_mask = port_mask;

    for(port=6;port<12;port++) 
        PortDirection(local_ptr,shadow_ptr,port,1); /* set all ports for input */

    fprintf(stderr,"Writing port numbers to each of the 6 ports\n");

    for(port=6; port < 12; port++)
        rp.line_mask[port] = port;

    bytes_written = write(fp, &rp, sizeof(rp));

    if(bytes_written == -1) {
        fprintf(stderr,"write() failed: %s\n",strerror(errno));
        return;
    }

    fprintf(stderr,"Number of Ports Written: %d\n",bytes_written);
}
/***********************************************************************/

/***********************************************************************/
void
ioctl_PCIE6509_get_driver_info()
{
    #define FMT             "%17s: "
    int                     i;
    char                    str[15];
    pcie6509_driver_info_t  info;

    status = ioctl(fp, IOCTL_PCIE6509_GET_DRIVER_INFO,(void *)&info);

    if(status) {
        fprintf(stderr,"ioctl(IOCTL_PCIE6509_GET_DRIVER_INFO) failed: %s\n",
                            strerror(errno));
        return;
    }

    fprintf(stderr,"\n");
    fprintf(stderr,FMT " %s\n",     "Version",    info.version);
    fprintf(stderr,FMT " %s\n",     "Build",      info.built);
    fprintf(stderr,FMT " %s\n",     "Module",     info.module_name);
    fprintf(stderr,FMT " %d (%s)\n","Board Type", info.board_type, 
                                                  info.board_desc);
    fprintf(stderr,FMT " %d\n",     "Bus",        info.bus);
    fprintf(stderr,FMT " %d\n",     "Slot",       info.slot);
    fprintf(stderr,FMT " %d\n",     "Func",       info.func);
    fprintf(stderr,FMT " 0x%x\n",   "Vendor ID",  info.vendor_id);
    fprintf(stderr,FMT " 0x%x\n",   "Device ID",  info.device_id);
    fprintf(stderr,FMT " N%x\n",    "Device",     info.device);
    fprintf(stderr,FMT " 0x%x\n",   "Board ID",   info.board_id);
    fprintf(stderr,FMT " 0x%x\n",   "Master Signature", info.MasterSignature);
    fprintf(stderr,FMT " 0x%x\n",   "Slave Signature",  info.SlaveSignature);
    fprintf(stderr,FMT " %ld\n",    "Interrupt.count", (ulong)info.interrupt.count);
    fprintf(stderr,FMT " 0x%04x\n",    "Interrupt.mask", info.interrupt.mask);
    fprintf(stderr,FMT " 0x%04x\n",    "Interrupt.status", info.interrupt.status);
    fprintf(stderr,FMT " 0x%04x\n",    "Interrupt.pending", info.interrupt.pending);

    /* now, display memory regions */
    for(i=0; i < PCIE6509_MAX_REGION; i++) {
        if(info.mem_region[i].size) {
            sprintf(str,"Region%2d",i);
            fprintf(stderr,FMT " Addr=%-10p  Size=%-5d (0x%x)\n",str,
                        (char *)(long)info.mem_region[i].physical_address,
                        info.mem_region[i].size,
                        info.mem_region[i].size);
        }
    }
}
/***********************************************************************/

void
PortDirection(pcie6509_local_ctrl_data_t *lp, pcie6509_shadow_regs_t *sp, int port, int output)
{
    u_char  value, line_value;
    int     lines;
    u_char  port_shift;

    value = (output)?0xFF:0x00;
    line_value = (output)?0x10:0x00;

    switch(port) {
        case 0 ... 3:
            port_shift = (port * 8); /* 0..3 -> 0..3 */
            sp->Master.DioPortsDIODirection &= ~(0xFF << port_shift);
            sp->Master.DioPortsDIODirection |= (value << port_shift);
            lp->Master.DioPortsDIODirection = sp->Master.DioPortsDIODirection;
        break;

        case 4 ... 5:
            port_shift = ((port - 4) * 8); /* 4..5 -> 0..1 */
            sp->Master.PFIDirection &= ~(0xFF << port_shift);
            sp->Master.PFIDirection |= (value << port_shift);
            lp->Master.PFIDirection = sp->Master.PFIDirection;
            for(lines=0; lines < PCIE6509_LINES_PER_PORT; lines++) {
                if(port==4) {
                    lp->Master.PFIOutputSelectPort0[lines] = 
                    sp->Master.PFIOutputSelectPort0[lines] = line_value; /* port 4 */
                }
                else {
                    lp->Master.PFIOutputSelectPort1[lines] =
                    sp->Master.PFIOutputSelectPort1[lines] = line_value; /* port 5 */
                }
            }
        break;

        case 6 ... 7:
            port_shift = ((port - 6) * 8); /* 6..7 -> 0..1 */
            sp->Slave.PFIDirection &= ~(0xFF << port_shift);
            sp->Slave.PFIDirection |= (value << port_shift);
            lp->Slave.PFIDirection = sp->Slave.PFIDirection;
            for(lines=0; lines < PCIE6509_LINES_PER_PORT; lines++) {
                if(port==6) {
                    lp->Slave.PFIOutputSelectPort0[lines] =
                    sp->Slave.PFIOutputSelectPort0[lines] = line_value; /* port 6 */
                }
                else {
                    lp->Slave.PFIOutputSelectPort1[lines] = 
                    sp->Slave.PFIOutputSelectPort1[lines] = line_value; /* port 7 */
                }
            }
        break;

        case 8 ... 11:
            port_shift = ((port - 8) * 8); /* 8..11 -> 0..3 */
            sp->Slave.DioPortsDIODirection &= ~(0xFF << port_shift);
            sp->Slave.DioPortsDIODirection |= (value << port_shift);
            lp->Slave.DioPortsDIODirection = sp->Slave.DioPortsDIODirection;
        break;

        default:
            fprintf(stderr,"Invalid Port %d\n",port);
        break;
    }
}

void
PortDirectionAll(pcie6509_local_ctrl_data_t *lp, pcie6509_shadow_regs_t *sp, int output)
{
    int     lines;
    u_int   dio_out;
    u_char  pfi_out;
    if(output) {
        dio_out = 0xffffffff;
        pfi_out = 0x10;
    } else {
        dio_out = 0;
        pfi_out = 0;
    }

    lp->Master.DioPortsDIODirection = 
    sp->Master.DioPortsDIODirection = dio_out;     /* port 0..3 */
    lp->Slave.DioPortsDIODirection =
    sp->Slave.DioPortsDIODirection = dio_out;      /* port 8..11 */
    lp->Master.PFIDirection =
    sp->Master.PFIDirection = dio_out;             /* port 4..5 */
    lp->Slave.PFIDirection =
    sp->Slave.PFIDirection = dio_out;              /* port 6..7 */

    for(lines=0; lines < PCIE6509_LINES_PER_PORT; lines++) {
        lp->Master.PFIOutputSelectPort0[lines] =
        sp->Master.PFIOutputSelectPort0[lines] = pfi_out; /* set port 4 as output */
        lp->Master.PFIOutputSelectPort1[lines] = 
        sp->Master.PFIOutputSelectPort1[lines] = pfi_out; /* set port 5 as output */
        lp->Slave.PFIOutputSelectPort0[lines]  = 
        sp->Slave.PFIOutputSelectPort0[lines]  = pfi_out; /* set port 6 as output */
        lp->Slave.PFIOutputSelectPort1[lines]  = 
        sp->Slave.PFIOutputSelectPort1[lines]  = pfi_out; /* set port 7 as output */
    }
}
