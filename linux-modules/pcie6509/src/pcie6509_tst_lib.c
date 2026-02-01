/*****************************************************************************
 *                                                                           *
 * File:         pcie6509_tst_lib.c                                          *
 *                                                                           *
 * Description:  Interactive Library Test                                    *
 *                                                                           *
 * Syntax:                                                                   *
 *   pcie6509_tst_lib           ==> start interactive library test on        *
 *                                  device 0.                                *
 *   pcie6509_tst_lib <dn>      ==> start interactive library test on        *
 *                                  device <dn>. (where dn is device number) *
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
#include "pcie6509_lib.h"

#define DEF_BOARD_NO    "0"

#define FLUSH_SIZE      200

#define PRINT_VAL(IteM) {           \
    fprintf(stderr,"\t%-38s=0x%08x\t@0x%08lx\n",#IteM, local_ptr->IteM,\
    ((long)&local_ptr->IteM - (long)local_ptr)); \
}

#define PRINT_VAL_SHADOW(IteM) {           \
    fprintf(stderr,"\t%-38s=0x%08x\t@0x%08lx\n",#IteM, shadow_reg_ptr->IteM,\
    ((long)&local_ptr->IteM - (long)local_ptr)); \
}

void quit(int s);
int user_input(char *input, char **retptr);
void _pcie6509_get_change_detection_status(char *desc, int status, int error);
void _pcie6509_get_rtsi_configuration(char *who, pcie6509_rtsi_config_t *config);

// Global variables
pcie6509_local_ctrl_data_t      *local_ptr;
pcie6509_shadow_regs_t          *shadow_reg_ptr;
pcie6509_driver_info_t          info;
void                            *MyHandle;
int                             pcie6509_break_received;
int                             fp, status;
char                   		flush[FLUSH_SIZE];
char                   		*cp;

typedef struct _what {
    uint    cmd;
    char    *name;
    char    *desc;
} what;

enum {
    _First_Entry = 100,

    PCIE6509_ADD_IRQ,
    PCIE6509_CLEAR_DRIVER_ERROR,
    PCIE6509_CLEAR_LIB_ERROR,
    PCIE6509_DISABLE_PCI_INTERRUPTS,
    PCIE6509_DISPLAY_BOARD_REGISTERS,
    PCIE6509_ENABLE_PCI_INTERRUPTS,
    PCIE6509_GET_BOARD_INFO,
    PCIE6509_GET_CHANGE_DETECTION_LATCHED,
    PCIE6509_GET_CHANGE_DETECTION_LATCHED_PMASK,
    PCIE6509_GET_CHANGE_DETECTION_STATUS,
    PCIE6509_GET_DIGITAL_INPUT,
    PCIE6509_GET_DIGITAL_INPUT_PMASK,
    PCIE6509_GET_DIGITAL_OUTPUT,
    PCIE6509_GET_DIGITAL_OUTPUT_PMASK,
    PCIE6509_GET_DRIVER_ERROR,
    PCIE6509_GET_FALL_EDGE_ENABLE,
    PCIE6509_GET_FALL_EDGE_ENABLE_PMASK,
    PCIE6509_GET_FILTER,
    PCIE6509_GET_INFO,
    PCIE6509_GET_INTERRUPT_COUNTER,
    PCIE6509_GET_LIBRARY_ERROR,
    PCIE6509_GET_LINE_STATE,
    PCIE6509_GET_MAPPED_LOCAL_PTR,
    PCIE6509_GET_PHYSICAL_MEMORY,
    PCIE6509_GET_PORT_DIRECTION,
    PCIE6509_GET_PORT_DIRECTION_PMASK,
    PCIE6509_GET_RISE_EDGE_ENABLE,
    PCIE6509_GET_RISE_EDGE_ENABLE_PMASK,
    PCIE6509_GET_RTSI_CONFIGURATION,
    PCIE6509_GET_SCRAP_REGISTER,
    PCIE6509_GET_SCRATCH_PAD_REGISTER,
    PCIE6509_GET_SHADOW_REGISTERS_PTR,
    PCIE6509_GET_UPTIME,
    PCIE6509_GET_VALUE,
    PCIE6509_GET_WATCHDOG_CONFIGURATION,
    PCIE6509_GET_WATCHDOG_CONTROL,
    PCIE6509_GET_WATCHDOG_MODE,
    PCIE6509_GET_WATCHDOG_SAFE_STATE,
    PCIE6509_GET_WATCHDOG_SAFE_STATE_PMASK,
    PCIE6509_GET_WATCHDOG_STATUS,
    PCIE6509_GET_WATCHDOG_TIMEOUT,
    PCIE6509_INITIALIZE_BOARD,
    PCIE6509_JOINT_RESET,
    PCIE6509_MMAP_PHYSICAL_MEMORY,
    PCIE6509_MUNMAP_PHYSICAL_MEMORY,
    PCIE6509_READ,
    PCIE6509_REMOVE_IRQ,
    PCIE6509_RESET_BOARD,
    PCIE6509_SET_ALL_OUTPUTS,
    PCIE6509_SET_DIGITAL_OUTPUT,
    PCIE6509_SET_DIGITAL_OUTPUT_PMASK,
    PCIE6509_SET_FALL_EDGE_ENABLE,
    PCIE6509_SET_FALL_EDGE_ENABLE_PMASK,
    PCIE6509_SET_FILTER,
    PCIE6509_SET_FILTER_PMASK,
    PCIE6509_SET_INTERRUPT_COUNTER,
    PCIE6509_SET_PORT_DIRECTION,
    PCIE6509_SET_PORT_DIRECTION_PMASK,
    PCIE6509_SET_RISING_EDGE_ENABLE,
    PCIE6509_SET_RISING_EDGE_ENABLE_PMASK,
    PCIE6509_SET_RTSI_CONFIGURATION,
    PCIE6509_SET_SCRAP_REGISTER,
    PCIE6509_SET_SCRATCHPAD_REGISTER,
    PCIE6509_SET_WATCHDOG_CONFIGURATION,
    PCIE6509_SET_WATCHDOG_CONTROL,
    PCIE6509_SET_WATCHDOG_MODE,
    PCIE6509_SET_WATCHDOG_MODE_PMASK,
    PCIE6509_SET_WATCHDOG_SAFE_STATE,
    PCIE6509_SET_WATCHDOG_SAFE_STATE_PMASK,
    PCIE6509_SET_WATCHDOG_TIMEOUT,

    PCIE6509_SET_VALUE,
    PCIE6509_WRITE,
};

what    command[]= {
    { PCIE6509_ADD_IRQ,
      "PCIE6509_Add_Irq()",                 "Add Irq" },
    { PCIE6509_CLEAR_DRIVER_ERROR,
      "PCIE6509_Clear_Driver_Error()",      "Clear Driver Error" },
    { PCIE6509_CLEAR_LIB_ERROR,
      "PCIE6509_Clear_Lib_Error()",         "Clear Library Error" },
    { PCIE6509_DISABLE_PCI_INTERRUPTS,
      "PCIE6509_Disable_Pci_Interrupts()",  "Disable Pci Interrupts" },
    { PCIE6509_DISPLAY_BOARD_REGISTERS,
      "display_registers(BOARD)",           "Display BOARD Registers" },
    { PCIE6509_ENABLE_PCI_INTERRUPTS,
      "PCIE6509_Enable_Pci_Interrupts()",   "Enable Pci Interrupts" },
    { PCIE6509_GET_BOARD_INFO,
      "PCIE6509_Get_BoardInfo()",           "Get Board Info" },
    { PCIE6509_GET_CHANGE_DETECTION_LATCHED,
      "PCIE6509_Get_ChangeDetectionLatched()", "Get Change Detection Latched" },
    { PCIE6509_GET_CHANGE_DETECTION_LATCHED_PMASK,
      "PCIE6509_Get_ChangeDetectionLatchedPmask()", "Get Change Detection Latched (Pmask)" },
    { PCIE6509_GET_CHANGE_DETECTION_STATUS,
      "PCIE6509_Get_ChangeDetectionStatus()", "Get Change Detection Status" },
    { PCIE6509_GET_DIGITAL_INPUT,
      "PCIE6509_Get_DigitalInput()",        "Get Digital Input" },
    { PCIE6509_GET_DIGITAL_INPUT_PMASK,
      "PCIE6509_Get_DigitalInputPmask()",   "Get Digital Input (Pmask)" },
    { PCIE6509_GET_DIGITAL_OUTPUT,
      "PCIE6509_Get_DigitalOutput()",       "Get Digital Output [**]" },
    { PCIE6509_GET_DIGITAL_OUTPUT_PMASK,
      "PCIE6509_Get_DigitalOutputPmask()",  "Get Digital Output (Pmask) [**]" },
    { PCIE6509_GET_DRIVER_ERROR,
      "PCIE6509_Get_Driver_Error()",        "Get Driver Error" },
    { PCIE6509_GET_FALL_EDGE_ENABLE,
      "PCIE6509_Get_FallEdgeEnable()",      "Get Falling Edge Enable [**]" },
    { PCIE6509_GET_FALL_EDGE_ENABLE_PMASK,
      "PCIE6509_Get_FallEdgeEnablePmask()", "Get Falling Edge Enable (Pmask) [**]" },
    { PCIE6509_GET_FILTER,
      "PCIE6509_Get_Filter()",              "Get Filter [**]" },
    { PCIE6509_GET_INFO,
      "PCIE6509_Get_Info()",                "Get Information" },
    { PCIE6509_GET_INTERRUPT_COUNTER,
      "PCIE6509_Get_InterruptCounter()",    "Get Interrupt Counter" },
    { PCIE6509_GET_LIBRARY_ERROR,
      "PCIE6509_Get_Library_Error()",       "Get Library Error" },
    { PCIE6509_GET_LINE_STATE,
      "PCIE6509_Get_LineState()",           "Get Line State" },
    { PCIE6509_GET_MAPPED_LOCAL_PTR,
      "PCIE6509_Get_Mapped_Local_Ptr()",    "Get Mapped Local Pointer" },
    { PCIE6509_GET_PHYSICAL_MEMORY,
      "PCIE6509_Get_Physical_Memory()",     "Get Physical Memory" },
    { PCIE6509_GET_PORT_DIRECTION,
      "PCIE6509_Get_PortDirection()",       "Get Port Direction [**]" },
    { PCIE6509_GET_PORT_DIRECTION_PMASK,
      "PCIE6509_Get_PortDirectionPmask()",  "Get Port Direction (Pmask) [**]" },
    { PCIE6509_GET_RISE_EDGE_ENABLE,
      "PCIE6509_Get_RiseEdgeEnable()",      "Get Rising Edge Enable [**]" },
    { PCIE6509_GET_RISE_EDGE_ENABLE_PMASK,
      "PCIE6509_Get_RiseEdgeEnablePmask()", "Get Rising Edge Enable (Pmask) [**]" },
    { PCIE6509_GET_RTSI_CONFIGURATION,
      "PCIE6509_Get_RTSIConfiguration()",   "Get RTSI Configuration [**]" },
    { PCIE6509_GET_SCRAP_REGISTER,
      "PCIE6509_Get_ScrapRegister()",       "Get Scrap Register" },
    { PCIE6509_GET_SCRATCH_PAD_REGISTER,
      "PCIE6509_Get_ScratchPadRegister()",  "Get Scratch Pad Register" },
    { PCIE6509_GET_SHADOW_REGISTERS_PTR,
      "PCIE6509_Get_Shadow_Registers_Ptr()","Get Shadow Registers Ptr" },
    { PCIE6509_GET_UPTIME,
      "PCIE6509_Get_Uptime()",              "Get Uptime" },
    { PCIE6509_GET_VALUE,
      "PCIE6509_Get_Value()",               "Get Value" },
    { PCIE6509_GET_WATCHDOG_CONFIGURATION,
      "PCIE6509_Get_WatchdogConfiguration()", "Get Watchdog Configuration [**]" },
    { PCIE6509_GET_WATCHDOG_CONTROL,
      "PCIE6509_Get_WatchdogControl()",     "Get Watchdog Control [**]" },
    { PCIE6509_GET_WATCHDOG_MODE,
      "PCIE6509_Get_WatchdogMode()",        "Get Watchdog Mode [**]" },
    { PCIE6509_GET_WATCHDOG_SAFE_STATE,
      "PCIE6509_Get_WatchdogSafeState()",   "Get Watchdog Safe State [**]" },
    { PCIE6509_GET_WATCHDOG_SAFE_STATE_PMASK,
      "PCIE6509_Get_WatchdogSafeStatePmask()", "Get Watchdog Safe State (Pmask) [**]" },
    { PCIE6509_GET_WATCHDOG_STATUS,
      "PCIE6509_Get_WatchdogStatus()",      "Get Watchdog Status" },
    { PCIE6509_GET_WATCHDOG_TIMEOUT,
      "PCIE6509_Get_WatchdogTimeout()",     "Get Watchdog Timeout [**]" },
    { PCIE6509_INITIALIZE_BOARD,
      "PCIE6509_Initialize_Board()",        "Initialize Board" },
    { PCIE6509_JOINT_RESET,
      "PCIE6509_JointReset()",              "Joint Reset" },
    { PCIE6509_MMAP_PHYSICAL_MEMORY,
      "PCIE6509_MMap_Physical_Memory()",    "MMap Physical Memory" },
    { PCIE6509_MUNMAP_PHYSICAL_MEMORY,
     "PCIE6509_Munmap_Physical_Memory()",   "Munmap Physical Memory" },
    { PCIE6509_READ,
      "PCIE6509_Read()",                    "Read Operation" },
    { PCIE6509_REMOVE_IRQ,
      "PCIE6509_Remove_Irq()",              "Remove Irq" },
    { PCIE6509_RESET_BOARD,
      "PCIE6509_Reset_Board()",             "Reset Board" },
    { PCIE6509_SET_ALL_OUTPUTS,
      "PCIE6509_Set_DigitalOutputPmask()",  "Set All Outputs" },
    { PCIE6509_SET_DIGITAL_OUTPUT,
      "PCIE6509_Set_DigitalOutput()",       "Set Digital Output" },
    { PCIE6509_SET_DIGITAL_OUTPUT_PMASK,
      "PCIE6509_Set_DigitalOutputPmask()",  "Set Digital Output (Pmask)" },
    { PCIE6509_SET_FALL_EDGE_ENABLE,
      "PCIE6509_Set_FallEdgeEnable()",      "Set Falling Edge Enable" },
    { PCIE6509_SET_FALL_EDGE_ENABLE_PMASK,
      "PCIE6509_Set_FallEdgeEnablePmask()", "Set Falling Edge Enable (Pmask)" },
    { PCIE6509_SET_FILTER,
      "PCIE6509_Set_Filter()",              "Set Filter" },
    { PCIE6509_SET_FILTER_PMASK,
      "PCIE6509_Set_Filter_pmask()",        "Set Filter (Pmask)" },
    { PCIE6509_SET_INTERRUPT_COUNTER,
      "PCIE6509_Set_InterruptCounter()",    "Set Interrupt Counter" },
    { PCIE6509_SET_PORT_DIRECTION,
      "PCIE6509_Set_PortDirection()",       "Set Port Direction" },
    { PCIE6509_SET_PORT_DIRECTION_PMASK,
      "PCIE6509_Set_PortDirectionPmask()",  "Set Port Direction (Pmask)" },
    { PCIE6509_SET_RISING_EDGE_ENABLE,
      "PCIE6509_Set_RiseEdgeEnable()",      "Set Rising Edge Enable" },
    { PCIE6509_SET_RISING_EDGE_ENABLE_PMASK,
      "PCIE6509_Set_RiseEdgeEnablePmask()", "Set Rising Edge Enable (Pmask)" },
    { PCIE6509_SET_RTSI_CONFIGURATION,
      "PCIE6509_Set_RTSIConfiguration()",   "Set RTSI Configuration" },
    { PCIE6509_SET_SCRAP_REGISTER,
      "PCIE6509_Set_ScrapRegister()",       "Set Scrap Register" },
    { PCIE6509_SET_SCRATCHPAD_REGISTER,
      "PCIE6509_Set_ScratchPadRegister()",   "Set ScratchPad Register" },
    { PCIE6509_SET_VALUE,
      "PCIE6509_Set_Value()",               "Set Value" },
    { PCIE6509_SET_WATCHDOG_CONFIGURATION,
      "PCIE6509_Set_WatchdogConfiguration()", "Set Watchdog Configuration" },
    { PCIE6509_SET_WATCHDOG_CONTROL,
      "PCIE6509_Set_WatchdogControl()",     "Set Watchdog Control" },
    { PCIE6509_SET_WATCHDOG_MODE,
      "PCIE6509_Set_WatchdogMode()",        "Set Watchdog Mode" },
    { PCIE6509_SET_WATCHDOG_MODE_PMASK,
      "PCIE6509_Set_WatchdogModePmask()",   "Set Watchdog Mode (Pmask)" },
    { PCIE6509_SET_WATCHDOG_SAFE_STATE,
      "PCIE6509_Set_WatchdogSafeState()",   "Set Watchdog Safe State" },
    { PCIE6509_SET_WATCHDOG_SAFE_STATE_PMASK,
      "PCIE6509_Set_WatchdogSafeStatePmask()", "Set Watchdog Safe State (Pmask)" },
    { PCIE6509_SET_WATCHDOG_TIMEOUT,
      "PCIE6509_Set_WatchdogTimeout()",     "Set Watchdog Timeout" },

    { PCIE6509_WRITE,
      "PCIE6509_Write()",                   "Write Operation" },

    { 0, 0, 0, }    /* list terminator */
};

what logical_selection[]= {
    { PCIE6509_TRUE, "True","" }, 
    { PCIE6509_FALSE, "False","" }, 

    { 0, 0, 0, }    /* list terminator */
};


/* prototype */
int  Float_Range_Selection(char *desc, double min, double max, 
                           double *ret_value);
int  Range_Selection(char *desc, int min, int max, int *ret_value);
int  Print_Selection(what *table);
int  Table_Selection(what *table, char *desc, void *ret_parm, 
                     char **ret_name, char **ret_desc );
void BadArg(char *arg);
void table_1();
int  Do_Selection(char *cp);

int  Display_Driver_Error(void *MyHandle);
int  Display_Library_Error(void *MyHandle);

void display_registers();
void pcie6509_get_board_info();
void pcie6509_get_change_detection_latched();
void pcie6509_get_change_detection_latched_pmask();
void pcie6509_get_change_detection_status();
void pcie6509_get_digital_input();
void pcie6509_get_digital_input_pmask();
void pcie6509_get_digital_output();
void pcie6509_get_digital_output_pmask();
void pcie6509_get_fall_edge_enable();
void pcie6509_get_fall_edge_enable_pmask();
void pcie6509_get_filter();
void pcie6509_get_info();
void pcie6509_get_interrupt_counter();
void pcie6509_get_line_state();
void pcie6509_get_mapped_local_ptr();
void pcie6509_get_physical_memory();
void pcie6509_get_port_direction();
void pcie6509_get_port_direction_pmask();
void pcie6509_get_rise_edge_enable();
void pcie6509_get_rise_edge_enable_pmask();
void pcie6509_get_rtsi_configuration();
void pcie6509_get_scrap_register();
void pcie6509_get_scratch_pad_register();
void pcie6509_get_shadow_registers_ptr();
void pcie6509_get_uptime();
void pcie6509_get_value();
void pcie6509_get_watchdog_configuration();
void pcie6509_get_watchdog_control();
void pcie6509_get_watchdog_mode();
void pcie6509_get_watchdog_safe_state();
void pcie6509_get_watchdog_safe_state_pmask();
void pcie6509_get_watchdog_status();
void pcie6509_get_watchdog_timeout();
void pcie6509_initialize_board();
void pcie6509_joint_reset();
void pcie6509_mmap_physical_memory();
void pcie6509_munmap_physical_memory();
void pcie6509_read_operation();
void pcie6509_set_all_outputs();
void pcie6509_set_digital_output();
void pcie6509_set_digital_output_pmask();
void pcie6509_set_fall_edge_enable();
void pcie6509_set_fall_edge_enable_pmask();
void pcie6509_set_filter();
void pcie6509_set_filter_pmask();
void pcie6509_set_interrupt_counter();
void pcie6509_set_port_direction();
void pcie6509_set_port_direction_pmask();
void pcie6509_set_rising_edge_enable();
void pcie6509_set_rising_edge_enable_pmask();
void pcie6509_set_rtsi_configuration();
void pcie6509_set_scrap_register();
void pcie6509_set_scratchpad_register();
void pcie6509_set_value();
void pcie6509_set_watchdog_configuration();
void pcie6509_set_watchdog_control();
void pcie6509_set_watchdog_mode();
void pcie6509_set_watchdog_mode_pmask();
void pcie6509_set_watchdog_safe_state();
void pcie6509_set_watchdog_safe_state_pmask();
void pcie6509_set_watchdog_timeout();

void pcie6509_write_operation();

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

    MyHandle=NULL;

    /*** OPEN DEVICE ***/
    if((status=PCIE6509_Open(&MyHandle,atoi(bnp)))) {
        fprintf(stderr,"pcie6509_Open Failed=%d\n",status);
        exit(1);
    }

    signal(SIGINT, quit);

    if((status=PCIE6509_Get_Info(MyHandle, &info))) {
        Display_Library_Error(MyHandle);
        exit(1);
    }

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

    if (MyHandle) {
        // PCIE6509_Reset_Board(MyHandle);
        PCIE6509_Close(MyHandle);
    }

    exit(0);
}

/****************************************************************************
 * user input                                                               *
 ****************************************************************************/
int
user_input(char *input, char **retptr)
{
    int i;
    char *cp;
    pcie6509_break_received = 0;

    fgets(input, FLUSH_SIZE, stdin);

    /* clean up input string */
    cp = input;
    while((*cp == ' '))cp++; /* skip leading blanks */
    i = strlen(cp) - 1; /* point 2 end of string and remove blanks*/
    while((cp[i] == '\n') || (cp[i] == ' ')) {
        cp[i] = 0;
        if(i==0) break;
        i--;
    }

    if(pcie6509_break_received) {
        if((strcmp(cp,"y") == 0) || (strcmp(cp,"q")==0)) {
            if (MyHandle) {
                PCIE6509_Reset_Board(MyHandle);
                PCIE6509_Close(MyHandle);
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

    fprintf(stderr,"Usage: pcie6509_tst_lib <device number>\n");
    exit(1);
}

#define STRING_LEN   36
/****************************************************************************
 * Print Table 1 Menu                                                       *
 ****************************************************************************/
void table_1()
{
    int i, j;
    char s[STRING_LEN+1];
    
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
    fprintf(stderr,
        "  [**] -> Data read from 'SHADOW REGISTERS'\n");

    if(i%2)
        fprintf(stderr,"\n");
}

int
Do_Selection(char *cp)
{
    char *ep;
    int cmd, num_cmds, i;
    long    int iSelection;

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

        case PCIE6509_ADD_IRQ:
            PCIE6509_Add_Irq(MyHandle);
        break;

        case PCIE6509_CLEAR_DRIVER_ERROR:
            PCIE6509_Clear_Driver_Error(MyHandle);
        break;

        case PCIE6509_CLEAR_LIB_ERROR:
            PCIE6509_Clear_Lib_Error(MyHandle);
        break;

        case PCIE6509_DISABLE_PCI_INTERRUPTS:
            PCIE6509_Disable_Pci_Interrupts(MyHandle);
        break;

        case PCIE6509_DISPLAY_BOARD_REGISTERS:
            display_registers();
        break;

        case PCIE6509_ENABLE_PCI_INTERRUPTS:
            PCIE6509_Enable_Pci_Interrupts(MyHandle);
        break;

        case PCIE6509_GET_BOARD_INFO:
            pcie6509_get_board_info();
        break;

        case PCIE6509_GET_CHANGE_DETECTION_LATCHED:
            pcie6509_get_change_detection_latched();
        break;

        case PCIE6509_GET_CHANGE_DETECTION_LATCHED_PMASK:
            pcie6509_get_change_detection_latched_pmask();
        break;

        case PCIE6509_GET_CHANGE_DETECTION_STATUS:
            pcie6509_get_change_detection_status();
        break;

        case PCIE6509_GET_DIGITAL_INPUT:
            pcie6509_get_digital_input();
        break;

        case PCIE6509_GET_DIGITAL_INPUT_PMASK:
            pcie6509_get_digital_input_pmask();
        break;

        case PCIE6509_GET_DIGITAL_OUTPUT:
            pcie6509_get_digital_output();
        break;

        case PCIE6509_GET_DIGITAL_OUTPUT_PMASK:
            pcie6509_get_digital_output_pmask();
        break;

        case PCIE6509_GET_DRIVER_ERROR:
            Display_Driver_Error(MyHandle);
        break;

        case PCIE6509_GET_FALL_EDGE_ENABLE:
            pcie6509_get_fall_edge_enable();
        break;

        case PCIE6509_GET_FALL_EDGE_ENABLE_PMASK:
            pcie6509_get_fall_edge_enable_pmask();
        break;

        case PCIE6509_GET_FILTER:
            pcie6509_get_filter();
        break;

        case PCIE6509_GET_INFO:
            pcie6509_get_info();
        break;

        case PCIE6509_GET_INTERRUPT_COUNTER:
            pcie6509_get_interrupt_counter();
        break;

        case PCIE6509_GET_LIBRARY_ERROR:
            Display_Library_Error(MyHandle);
        break;

        case PCIE6509_GET_LINE_STATE:
            pcie6509_get_line_state();
        break;

        case PCIE6509_GET_MAPPED_LOCAL_PTR:
            pcie6509_get_mapped_local_ptr();
        break;

        case PCIE6509_GET_PHYSICAL_MEMORY:
            pcie6509_get_physical_memory();
        break;

        case PCIE6509_GET_PORT_DIRECTION:
            pcie6509_get_port_direction();
        break;

        case PCIE6509_GET_PORT_DIRECTION_PMASK:
            pcie6509_get_port_direction_pmask();
        break;

        case PCIE6509_GET_RISE_EDGE_ENABLE:
            pcie6509_get_rise_edge_enable();
        break;

        case PCIE6509_GET_RISE_EDGE_ENABLE_PMASK:
            pcie6509_get_rise_edge_enable_pmask();
        break;

        case PCIE6509_GET_RTSI_CONFIGURATION:
            pcie6509_get_rtsi_configuration();
        break;

        case PCIE6509_GET_SCRAP_REGISTER:
            pcie6509_get_scrap_register();
        break;

        case PCIE6509_GET_SCRATCH_PAD_REGISTER:
            pcie6509_get_scratch_pad_register();
        break;

        case PCIE6509_GET_SHADOW_REGISTERS_PTR:
            pcie6509_get_shadow_registers_ptr();
        break;

        case PCIE6509_GET_UPTIME:
            pcie6509_get_uptime();
        break;

        case PCIE6509_GET_VALUE:
            pcie6509_get_value();
        break;

        case PCIE6509_GET_WATCHDOG_CONFIGURATION:
            pcie6509_get_watchdog_configuration();
        break;

        case PCIE6509_GET_WATCHDOG_CONTROL:
            pcie6509_get_watchdog_control();
        break;

        case PCIE6509_GET_WATCHDOG_MODE:
            pcie6509_get_watchdog_mode();
        break;

        case PCIE6509_GET_WATCHDOG_SAFE_STATE:
            pcie6509_get_watchdog_safe_state();
        break;

        case PCIE6509_GET_WATCHDOG_SAFE_STATE_PMASK:
            pcie6509_get_watchdog_safe_state_pmask();
        break;

        case PCIE6509_GET_WATCHDOG_STATUS:
            pcie6509_get_watchdog_status();
        break;

        case PCIE6509_GET_WATCHDOG_TIMEOUT:
            pcie6509_get_watchdog_timeout();
        break;

        case PCIE6509_INITIALIZE_BOARD:
            pcie6509_initialize_board();
        break;

        case PCIE6509_JOINT_RESET:
            pcie6509_joint_reset();
        break;

        case PCIE6509_MMAP_PHYSICAL_MEMORY:
            pcie6509_mmap_physical_memory();
        break;

        case PCIE6509_MUNMAP_PHYSICAL_MEMORY:
            pcie6509_munmap_physical_memory();
        break;

        case PCIE6509_READ:
            pcie6509_read_operation();
        break;

        case PCIE6509_REMOVE_IRQ:
            PCIE6509_Remove_Irq(MyHandle);
        break;

        case PCIE6509_RESET_BOARD:
            PCIE6509_Reset_Board(MyHandle);
        break;

        case PCIE6509_SET_ALL_OUTPUTS:
            pcie6509_set_all_outputs();
        break;

        case PCIE6509_SET_DIGITAL_OUTPUT:
            pcie6509_set_digital_output();
        break;

        case PCIE6509_SET_DIGITAL_OUTPUT_PMASK:
            pcie6509_set_digital_output_pmask();
        break;

        case PCIE6509_SET_FALL_EDGE_ENABLE:
            pcie6509_set_fall_edge_enable();
        break;

        case PCIE6509_SET_FALL_EDGE_ENABLE_PMASK:
            pcie6509_set_fall_edge_enable_pmask();
        break;

        case PCIE6509_SET_FILTER:
            pcie6509_set_filter();
        break;

        case PCIE6509_SET_FILTER_PMASK:
            pcie6509_set_filter_pmask();
        break;

        case PCIE6509_SET_INTERRUPT_COUNTER:
            pcie6509_set_interrupt_counter();
        break;

        case PCIE6509_SET_PORT_DIRECTION:
            pcie6509_set_port_direction();
        break;

        case PCIE6509_SET_PORT_DIRECTION_PMASK:
            pcie6509_set_port_direction_pmask();
        break;

        case PCIE6509_SET_RISING_EDGE_ENABLE:
            pcie6509_set_rising_edge_enable();
        break;

        case PCIE6509_SET_RISING_EDGE_ENABLE_PMASK:
            pcie6509_set_rising_edge_enable_pmask();
        break;

        case PCIE6509_SET_RTSI_CONFIGURATION:
            pcie6509_set_rtsi_configuration();
        break;

        case PCIE6509_SET_SCRAP_REGISTER:
            pcie6509_set_scrap_register();
        break;

        case PCIE6509_SET_SCRATCHPAD_REGISTER:
            pcie6509_set_scratchpad_register();
        break;

        case PCIE6509_SET_VALUE:
            pcie6509_set_value();
        break;

        case PCIE6509_SET_WATCHDOG_CONFIGURATION:
            pcie6509_set_watchdog_configuration();
        break;

        case PCIE6509_SET_WATCHDOG_CONTROL:
            pcie6509_set_watchdog_control();
        break;

        case PCIE6509_SET_WATCHDOG_MODE:
            pcie6509_set_watchdog_mode();
        break;

        case PCIE6509_SET_WATCHDOG_MODE_PMASK:
            pcie6509_set_watchdog_mode_pmask();
        break;

        case PCIE6509_SET_WATCHDOG_SAFE_STATE:
            pcie6509_set_watchdog_safe_state();
        break;

        case PCIE6509_SET_WATCHDOG_SAFE_STATE_PMASK:
            pcie6509_set_watchdog_safe_state_pmask();
        break;

        case PCIE6509_SET_WATCHDOG_TIMEOUT:
            pcie6509_set_watchdog_timeout();
        break;

        case PCIE6509_WRITE:
            pcie6509_write_operation();
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
int
Range_Selection(char *desc, int min, int max, int *ret_value)
{
    long int input;

again:
    fprintf(stderr,"   Enter %s value [%d-%d] (0x%x-0x%x) ->", 
                desc, min, max, min, max);

    if(user_input(flush,&cp) == 0)  /* if break received, skip */
        return(1);

    input = strtoul(cp,NULL,0);

    if((input < min) || (input > max)) {
        fprintf(stderr,"### ERROR! Invalid %s value %ld ###\n",desc,
                input);
        goto again;
    }

    *ret_value = (int) input;
    return(0);
}
/***********************************************************************/

/***********************************************************************/
int
Float_Range_Selection(char *desc, double min, double max, double *ret_value)
{
    double input;

again:
    fprintf(stderr,"   Enter %s value [%f-%f] ->", desc, min, max);

    if(user_input(flush,&cp) == 0)  /* if break received, skip */
        return(1);

    input = strtod(cp,NULL);

    if((input < min) || (input > max)) {
        fprintf(stderr,"### ERROR! Invalid %s value %f ###\n",desc,
                input);
        goto again;
    }

    *ret_value = input;
    return(0);
}
/***********************************************************************/

/***********************************************************************/
int
Print_Selection(what *table)
{
    int i;
    i = 0;
    while(table[i].name) {
        fprintf(stderr,"      %3d: %s\n",i+1, table[i].name);
        i++;
    }
    return(i);
}

int
Table_Selection(what *table, char *desc, void *ret_parm,
                char **ret_name, char **ret_desc )
{
    char 	    *cp;
    char        *ep;
    int         num_entries;
    long int    iSelection, parm;
    int         ret_code=0;

    num_entries = Print_Selection(table);

    do {
        fprintf(stderr,
            "   Enter '%s' Selection ('h'=display menu, 'q'=quit)-> ", desc);

        if(user_input(flush,&cp) == 0)  /* if break received, skip */
            break;

        if(cp[0] == '\0') /* if no entry....repeat selection */
            continue;

        if(strcmp(cp,"h") == 0) {   /* if help, display table */
            Print_Selection(table);
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

        if(ret_name)
            *ret_name = table[iSelection].name;

        if(ret_desc)
            *ret_desc = table[iSelection].desc;

        break;  /* done....break out */

    } while(1);

    return(ret_code);
}

/***********************************************************************/
/***********************************************************************/

/******************************************************************************
 *** Display LOCAL Registers                                                ***
 ******************************************************************************/
void
display_registers()
{
    pcie6509_get_mapped_local_ptr();

    if(local_ptr == NULL) { /* if local region is not present */
        return;
    }

    pcie6509_get_shadow_registers_ptr();
    if(shadow_reg_ptr == NULL) { /* if local region is not present */
        return;
    }

    fprintf(stderr,"\n======= BOARD REGISTERS =========\n");
    PRINT_VAL(CHInCh.Identification);
    PRINT_VAL(CHInCh.InterruptMask);
    PRINT_VAL(CHInCh.InterruptStatus);
    PRINT_VAL(CHInCh.VolatileInterruptStatus);
    PRINT_VAL(CHInCh.Scrap);
    PRINT_VAL(CHInCh.PCISubsystemID);

    fprintf(stderr,"----------------------------------------------\n");
    PRINT_VAL(Master.CSScratchPad);
    PRINT_VAL(Master.CSSignature);
    PRINT_VAL(Master.CSTimeSincePowerUp);
    PRINT_VAL(Master.CSWatchdogStatus);
    PRINT_VAL(Master.CSGlobalInterruptStatus);
    PRINT_VAL(Master.CSDIInterruptStatus);
    PRINT_VAL(Master.CSWatchdogTimerInterruptStatus);
    PRINT_VAL(Master.PFIStaticDigitalInput);
    PRINT_VAL(Master.DioPortsStaticDigitalInput);
    PRINT_VAL(Master.CSDIChangeDetectionStatus);
    PRINT_VAL(Master.PFIChangeDetectionLatched);
    PRINT_VAL(Master.CSIntForwardingControlStatus);
    PRINT_VAL(Master.CSIntForwardingDestination);

    fprintf(stderr,"----------------------------------------------\n");
    PRINT_VAL(Slave.CSScratchPad);
    PRINT_VAL(Slave.CSSignature);
    PRINT_VAL(Slave.CSTimeSincePowerUp);
    PRINT_VAL(Slave.CSWatchdogStatus);
    PRINT_VAL(Slave.CSGlobalInterruptStatus);
    PRINT_VAL(Slave.CSDIInterruptStatus);
    PRINT_VAL(Slave.CSWatchdogTimerInterruptStatus);
    PRINT_VAL(Slave.PFIStaticDigitalInput);
    PRINT_VAL(Slave.DioPortsStaticDigitalInput);
    PRINT_VAL(Slave.CSDIChangeDetectionStatus);
    PRINT_VAL(Slave.PFIChangeDetectionLatched);
    PRINT_VAL(Slave.CSIntForwardingControlStatus);
    PRINT_VAL(Slave.CSIntForwardingDestination);
    fprintf(stderr,"----------------------------------------------\n");

    fprintf(stderr,"\n======= BOARD MASTER [SHADOW] WRITE REGISTERS =========\n");
    PRINT_VAL_SHADOW(Master.CSJointReset);
    PRINT_VAL_SHADOW(Master.CSWatchdogTimeout);
    PRINT_VAL_SHADOW(Master.CSWatchdogConfiguration);
    PRINT_VAL_SHADOW(Master.CSWatchdogControl);
    PRINT_VAL_SHADOW(Master.CSWatchdogTimerInterrupt1);
    PRINT_VAL_SHADOW(Master.CSWatchdogTimerInterrupt2);
    PRINT_VAL_SHADOW(Master.CSGlobalInterruptEnable);
    PRINT_VAL_SHADOW(Master.PFIDirection);
    PRINT_VAL_SHADOW(Master.CSRTSITrigDirection);

    PRINT_VAL_SHADOW(Master.CSRTSIOutputSelect[0]);
    PRINT_VAL_SHADOW(Master.CSRTSIOutputSelect[1]);
    PRINT_VAL_SHADOW(Master.CSRTSIOutputSelect[2]);
    PRINT_VAL_SHADOW(Master.CSRTSIOutputSelect[3]);
    PRINT_VAL_SHADOW(Master.CSRTSIOutputSelect[4]);
    PRINT_VAL_SHADOW(Master.CSRTSIOutputSelect[5]);
    PRINT_VAL_SHADOW(Master.CSRTSIOutputSelect[6]);
    PRINT_VAL_SHADOW(Master.CSRTSIOutputSelect[7]);
    
    PRINT_VAL_SHADOW(Master.PFIFilterPort0Low);
    PRINT_VAL_SHADOW(Master.PFIFilterPort0High);
    PRINT_VAL_SHADOW(Master.PFIFilterPort1Low);
    PRINT_VAL_SHADOW(Master.PFIFilterPort1High);
    
    PRINT_VAL_SHADOW(Master.PFIOutputSelectPort0[0]);
    PRINT_VAL_SHADOW(Master.PFIOutputSelectPort0[1]);
    PRINT_VAL_SHADOW(Master.PFIOutputSelectPort0[2]);
    PRINT_VAL_SHADOW(Master.PFIOutputSelectPort0[3]);
    PRINT_VAL_SHADOW(Master.PFIOutputSelectPort0[4]);
    PRINT_VAL_SHADOW(Master.PFIOutputSelectPort0[5]);
    PRINT_VAL_SHADOW(Master.PFIOutputSelectPort0[6]);
    PRINT_VAL_SHADOW(Master.PFIOutputSelectPort0[7]);
    
    PRINT_VAL_SHADOW(Master.PFIOutputSelectPort1[0]);
    PRINT_VAL_SHADOW(Master.PFIOutputSelectPort1[1]);
    PRINT_VAL_SHADOW(Master.PFIOutputSelectPort1[2]);
    PRINT_VAL_SHADOW(Master.PFIOutputSelectPort1[3]);
    PRINT_VAL_SHADOW(Master.PFIOutputSelectPort1[4]);
    PRINT_VAL_SHADOW(Master.PFIOutputSelectPort1[5]);
    PRINT_VAL_SHADOW(Master.PFIOutputSelectPort1[6]);
    PRINT_VAL_SHADOW(Master.PFIOutputSelectPort1[7]);
    
    PRINT_VAL_SHADOW(Master.PFIStaticDigitalOutput);
    PRINT_VAL_SHADOW(Master.PFIWDTSafeState);
    PRINT_VAL_SHADOW(Master.PFIWDTModeSelect);
    PRINT_VAL_SHADOW(Master.DioPortsStaticDigitalOutput);
    PRINT_VAL_SHADOW(Master.DioPortsDIODirection);
    PRINT_VAL_SHADOW(Master.DioPortsDOWDTSafeState);
    PRINT_VAL_SHADOW(Master.DioPortsDOWDTModeSelectP01);
    PRINT_VAL_SHADOW(Master.DioPortsDOWDTModeSelectP23);
    PRINT_VAL_SHADOW(Master.DioPortsDIChangeIrqRE);
    PRINT_VAL_SHADOW(Master.DioPortsDIChangeIrqFE);
    PRINT_VAL_SHADOW(Master.PFIChangeIrq);
    PRINT_VAL_SHADOW(Master.DioPortsDIFilterP01);
    PRINT_VAL_SHADOW(Master.DioPortsDIFilterP23);
    PRINT_VAL_SHADOW(Master.CSChangeDetectionIrq);

    fprintf(stderr,"\n======= BOARD SLAVE [SHADOW] WRITE REGISTERS =========\n");
    PRINT_VAL_SHADOW(Slave.CSJointReset);
    PRINT_VAL_SHADOW(Slave.CSWatchdogTimeout);
    PRINT_VAL_SHADOW(Slave.CSWatchdogConfiguration);
    PRINT_VAL_SHADOW(Slave.CSWatchdogControl);
    PRINT_VAL_SHADOW(Slave.CSWatchdogTimerInterrupt1);
    PRINT_VAL_SHADOW(Slave.CSWatchdogTimerInterrupt2);
    PRINT_VAL_SHADOW(Slave.CSGlobalInterruptEnable);
    PRINT_VAL_SHADOW(Slave.PFIDirection);
    PRINT_VAL_SHADOW(Slave.CSRTSITrigDirection);

    PRINT_VAL_SHADOW(Slave.CSRTSIOutputSelect[0]);
    PRINT_VAL_SHADOW(Slave.CSRTSIOutputSelect[1]);
    PRINT_VAL_SHADOW(Slave.CSRTSIOutputSelect[2]);
    PRINT_VAL_SHADOW(Slave.CSRTSIOutputSelect[3]);
    PRINT_VAL_SHADOW(Slave.CSRTSIOutputSelect[4]);
    PRINT_VAL_SHADOW(Slave.CSRTSIOutputSelect[5]);
    PRINT_VAL_SHADOW(Slave.CSRTSIOutputSelect[6]);
    PRINT_VAL_SHADOW(Slave.CSRTSIOutputSelect[7]);
    
    PRINT_VAL_SHADOW(Slave.PFIFilterPort0Low);
    PRINT_VAL_SHADOW(Slave.PFIFilterPort0High);
    PRINT_VAL_SHADOW(Slave.PFIFilterPort1Low);
    PRINT_VAL_SHADOW(Slave.PFIFilterPort1High);
    
    PRINT_VAL_SHADOW(Slave.PFIOutputSelectPort0[0]);
    PRINT_VAL_SHADOW(Slave.PFIOutputSelectPort0[1]);
    PRINT_VAL_SHADOW(Slave.PFIOutputSelectPort0[2]);
    PRINT_VAL_SHADOW(Slave.PFIOutputSelectPort0[3]);
    PRINT_VAL_SHADOW(Slave.PFIOutputSelectPort0[4]);
    PRINT_VAL_SHADOW(Slave.PFIOutputSelectPort0[5]);
    PRINT_VAL_SHADOW(Slave.PFIOutputSelectPort0[6]);
    PRINT_VAL_SHADOW(Slave.PFIOutputSelectPort0[7]);
    
    PRINT_VAL_SHADOW(Slave.PFIOutputSelectPort1[0]);
    PRINT_VAL_SHADOW(Slave.PFIOutputSelectPort1[1]);
    PRINT_VAL_SHADOW(Slave.PFIOutputSelectPort1[2]);
    PRINT_VAL_SHADOW(Slave.PFIOutputSelectPort1[3]);
    PRINT_VAL_SHADOW(Slave.PFIOutputSelectPort1[4]);
    PRINT_VAL_SHADOW(Slave.PFIOutputSelectPort1[5]);
    PRINT_VAL_SHADOW(Slave.PFIOutputSelectPort1[6]);
    PRINT_VAL_SHADOW(Slave.PFIOutputSelectPort1[7]);
    
    PRINT_VAL_SHADOW(Slave.PFIStaticDigitalOutput);
    PRINT_VAL_SHADOW(Slave.PFIWDTSafeState);
    PRINT_VAL_SHADOW(Slave.PFIWDTModeSelect);
    PRINT_VAL_SHADOW(Slave.DioPortsStaticDigitalOutput);
    PRINT_VAL_SHADOW(Slave.DioPortsDIODirection);
    PRINT_VAL_SHADOW(Slave.DioPortsDOWDTSafeState);
    PRINT_VAL_SHADOW(Slave.DioPortsDOWDTModeSelectP01);
    PRINT_VAL_SHADOW(Slave.DioPortsDOWDTModeSelectP23);
    PRINT_VAL_SHADOW(Slave.DioPortsDIChangeIrqRE);
    PRINT_VAL_SHADOW(Slave.DioPortsDIChangeIrqFE);
    PRINT_VAL_SHADOW(Slave.PFIChangeIrq);
    PRINT_VAL_SHADOW(Slave.DioPortsDIFilterP01);
    PRINT_VAL_SHADOW(Slave.DioPortsDIFilterP23);
    PRINT_VAL_SHADOW(Slave.CSChangeDetectionIrq);
}
/***********************************************************************/

/***********************************************************************/
void
pcie6509_get_digital_input()
{
    int         status;
    u_short     port;
    u_char      line_mask_data;
    u_char      line_mask_direction;

    for(port = 0; port < PCIE6509_MAX_PORTS; port ++) {
        if((status=PCIE6509_Get_DigitalInput(MyHandle, port, &line_mask_data))) {
            Display_Library_Error(MyHandle);
            return;
        }

        if((status=PCIE6509_Get_PortDirection(MyHandle, port, &line_mask_direction))) {
            Display_Library_Error(MyHandle);
            return;
        }

        fprintf(stderr,"INPUT: Port %2d, Line Mask: Signal/Direction(shadow) -> 0x%02x/0x%02x\n",
                port,line_mask_data,line_mask_direction);
    }

}
/***********************************************************************/

/***********************************************************************/
void
pcie6509_get_digital_input_pmask()
{
    int                 status;
    pcie6509_io_port_t  iop;
    pcie6509_io_port_t  iop_dir;
    u_short             port;

    /* get directions */
    iop_dir.port_mask = PCIE6509_ALL_PORTS_MASK;
    if((status=PCIE6509_Get_PortDirectionPmask(MyHandle, &iop_dir))) {
        Display_Library_Error(MyHandle);
        return;
    }

    /* get signals */
    iop.port_mask = PCIE6509_ALL_PORTS_MASK;
    if((status=PCIE6509_Get_DigitalInputPmask(MyHandle, &iop))) {
        Display_Library_Error(MyHandle);
        return;
    }

    for(port = 0; port < PCIE6509_MAX_PORTS; port ++) {
        fprintf(stderr,"INPUT: Port %2d, Line Mask: Signal/Direction(shadow) -> 0x%02x/0x%02x\n",
                port,iop.line_mask[port],iop_dir.line_mask[port]);
    }

}
/***********************************************************************/

/***********************************************************************/
void
pcie6509_get_digital_output()
{
    int         status;
    u_short     port;
    u_char      line_mask;

    for(port = 0; port < PCIE6509_MAX_PORTS; port ++) {
        if((status=PCIE6509_Get_DigitalOutput(MyHandle, port, &line_mask))) {
            Display_Library_Error(MyHandle);
            return;
        }

        fprintf(stderr,"    ShadowRegister:OUTPUT:Port %2d, Line Mask -> 0x%02x\n",
                port,line_mask);
    }
}
/***********************************************************************/

/***********************************************************************/
void
pcie6509_get_digital_output_pmask()
{
    int                 status;
    pcie6509_io_port_t  iop;
    u_short             port;

    iop.port_mask = PCIE6509_ALL_PORTS_MASK;

    if((status=PCIE6509_Get_DigitalOutputPmask(MyHandle, &iop))) {
        Display_Library_Error(MyHandle);
        return;
    }

    for(port = 0; port < PCIE6509_MAX_PORTS; port ++) {
        fprintf(stderr,"    ShadowRegister:OUTPUT:Port %2d, Line Mask -> 0x%02x\n",port,iop.line_mask[port]); 
    }
}
/***********************************************************************/

/***********************************************************************/
int
Display_Driver_Error(void *MyHandle)
{
    int status;
    pcie6509_user_error_t   driver_error;

    /*** GET DRIVER ERROR ***/
    if((status=PCIE6509_Get_Driver_Error(MyHandle, &driver_error))) {
        fprintf(stderr,"PCIE6509_Get_Driver_Error Failed=%d\n",status);
        return(status);
    }

    fprintf(stderr,"===============================================\n");
    fprintf(stderr,"driver error information:\n");
    fprintf(stderr,"   error:   %d\n",driver_error.error);
    fprintf(stderr,"    name:   %s\n",driver_error.name);
    fprintf(stderr,"    desc:   %s\n",driver_error.desc);
    fprintf(stderr,"===============================================\n");

    return(driver_error.error);
}
/***********************************************************************/

/***********************************************************************/
void
pcie6509_get_board_info()
{
    pcie6509_board_info_t  board_info;

    if((status=PCIE6509_Get_BoardInfo(MyHandle, &board_info))) {
        Display_Library_Error(MyHandle);
        return;
    }

    fprintf(stderr,"Board ID               =0x%02x\n",board_info.BoardId);
    fprintf(stderr,"Board Subsystem PID    =0x%02x\n",board_info.SubsystemPid);
    fprintf(stderr,"Board Subsystem VID    =0x%02x\n",board_info.SubsystemVid);
    fprintf(stderr,"Board Master Signature =0x%02x\n",board_info.MasterSignature);
    fprintf(stderr,"Board Slave Signature  =0x%02x\n",board_info.SlaveSignature);
}
/***********************************************************************/

/***********************************************************************/
void
pcie6509_get_change_detection_latched()
{
    u_short port;
    u_char  line_mask;

    for(port = 0; port < PCIE6509_MAX_PORTS; port ++) {
        if((status=PCIE6509_Get_ChangeDetectionLatched(MyHandle, port, &line_mask))) {
            Display_Library_Error(MyHandle);
            return;
        }
        fprintf(stderr,"Port %2d, Line Mask -> 0x%02x\n",port,line_mask); 
    }
}
/***********************************************************************/

/***********************************************************************/
void
pcie6509_get_change_detection_latched_pmask()
{
    pcie6509_io_port_t  iop;
    u_short             port;

    iop.port_mask = PCIE6509_ALL_PORTS_MASK;

    if((status=PCIE6509_Get_ChangeDetectionLatchedPmask(MyHandle, &iop))) {
        Display_Library_Error(MyHandle);
        return;
    }

    for(port = 0; port < PCIE6509_MAX_PORTS; port ++) {
        fprintf(stderr,"    Port %2d, Line Mask -> 0x%02x\n",port,iop.line_mask[port]); 
    }
}
/***********************************************************************/

/***********************************************************************/
what change_status[]= {
    { PCIE6509_CDS_NO_CHANGE, "", "No change detection event detected" },
    { PCIE6509_CDS_CHANGE_DETECTED, "", "Change detection event detected" },

    { 0, 0, 0, }    /* list terminator */
};

what change_error[]= {
    { PCIE6509_CDS_NO_ERROR, "", "No errors detected" },
    { PCIE6509_CDS_MULTI_CHANGE_DETECT, "", "Multiple change detection events detected since last ack" },

    { 0, 0, 0, }    /* list terminator */
};

/***********************************************************************/
void
pcie6509_get_change_detection_status()
{
    pcie6509_change_detection_status_t  master;
    pcie6509_change_detection_status_t  slave;

    if((status=PCIE6509_Get_ChangeDetectionStatus(MyHandle, &master, &slave))) {
        Display_Library_Error(MyHandle);
        return;
    }
    _pcie6509_get_change_detection_status("Master (port 0..5)", master.status, master.error);
    _pcie6509_get_change_detection_status("Slave (port 6..11)", slave.status, slave.error);
}
/***********************************************************************/

void
_pcie6509_get_change_detection_status(char *desc, int status, int error)
{
    int i;
    fprintf(stderr,"\n%s:\n",desc);

    i=0;
    while(change_status[i].desc) {
        if(change_status[i].cmd == status)
            break;
        i++;
    }

    if(change_status[i].desc)
        fprintf(stderr,"   Status: %s\n",change_status[i].desc);
    else
        fprintf(stderr,"   Status: %s\n","### Not Defined ###");

    i=0;
    while(change_error[i].desc) {
        if(change_error[i].cmd == error)
            break;
        i++;
    }

    if(change_status[i].desc)
        fprintf(stderr,"    Error: %s\n",change_error[i].desc);
    else
        fprintf(stderr,"    Error: %s\n","### Not Defined ###");
}

/***********************************************************************/
void
pcie6509_get_fall_edge_enable()
{
    u_short port;
    u_char  line_mask;

    for(port = 0; port < PCIE6509_MAX_PORTS; port ++) {
        if((status=PCIE6509_Get_FallEdgeEnable(MyHandle, port, &line_mask))) {
            Display_Library_Error(MyHandle);
            return;
        }
        fprintf(stderr,"    ShadowRegister::Port %2d, Line Mask -> 0x%02x\n",port,line_mask); 
    }
}
/***********************************************************************/

/***********************************************************************/
void
pcie6509_get_fall_edge_enable_pmask()
{
    pcie6509_io_port_t  iop;
    u_short             port;

    iop.port_mask = PCIE6509_ALL_PORTS_MASK;

    if((status=PCIE6509_Get_FallEdgeEnablePmask(MyHandle, &iop))) {
        Display_Library_Error(MyHandle);
        return;
    }

    for(port = 0; port < PCIE6509_MAX_PORTS; port ++) {
        fprintf(stderr,"    ShadowRegister::Port %2d, Line Mask -> 0x%02x\n",port,iop.line_mask[port]); 
    }
}
/***********************************************************************/

/***********************************************************************/
void
pcie6509_get_filter()
{
    int                         line;
    char                        str[40];
    u_short                     port;
    u_char                      line_mask;
    pcie6509_filter_option_t    option[PCIE6509_LINES_PER_PORT];

    fprintf(stderr,"   Enter Port (0..%d) ->", PCIE6509_MAX_PORTS-1);

    if(user_input(flush,&cp) == 0)  /* if break received, skip */
        return;

    port = strtoul(cp,NULL,0);

    line_mask = PCIE6509_ALL_LINES_MASK;

    if((status=PCIE6509_Get_Filter(MyHandle, port, line_mask, option))) {
        Display_Library_Error(MyHandle);
        return;
    }

    fprintf(stderr,"Port %d...\n",port);
    for(line=0; line < PCIE6509_LINES_PER_PORT; line++) {
        if(PCIE6509_LINE_GET(line_mask,line)) {
            switch(option[line]) {
                case PCIE6509_FILTER_NONE:
                    strcpy(str,"None");
                break;
                case PCIE6509_FILTER_SMALL:
                    strcpy(str,"Small");
                break;
                case PCIE6509_FILTER_MEDIUM:
                    strcpy(str,"Medium");
                break;
                case PCIE6509_FILTER_LARGE:
                    strcpy(str,"Large");
                break;
                default:
                    strcpy(str,"*** Not Defined ***");
                break;
            }
            fprintf(stderr,"    ShadowRegister::Line %d, Filter -> %s\n",line, str);
        }
    }
}
/***********************************************************************/

/***********************************************************************/
void
pcie6509_get_info()
{
    #define FMT "%17s: "
    int     i;
    char    str[15];

    fprintf(stderr,"\n");
    fprintf(stderr,FMT " %s\n",     "Version",    info.version);
    fprintf(stderr,FMT " %s\n",     "Build",      info.built);
    fprintf(stderr,FMT " %s\n",     "Module",     info.module_name);
    fprintf(stderr,FMT " %d (%s)\n","Board Type", info.board_type, 
                                                  info.board_desc);
    fprintf(stderr,FMT " 0x%x\n",   "Device",     info.device);
    fprintf(stderr,FMT " %d\n",     "Bus",        info.bus);
    fprintf(stderr,FMT " %d\n",     "Slot",       info.slot);
    fprintf(stderr,FMT " %d\n",     "Func",       info.func);
    fprintf(stderr,FMT " 0x%x\n",   "Vendor ID",  info.vendor_id);
    fprintf(stderr,FMT " 0x%x\n",   "Device ID",  info.device_id);
    fprintf(stderr,FMT " N%x\n",    "Device",     info.device);
    fprintf(stderr,FMT " 0x%x\n",   "Board ID",   info.board_id);

    fprintf(stderr,FMT " 0x%x\n",   "Master Signature",   info.MasterSignature);
    fprintf(stderr,FMT " 0x%x\n",   "Slave Signature",   info.SlaveSignature);
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

/***********************************************************************/
void
pcie6509_get_interrupt_counter()
{
    int         status;
    u_int64_t   counter;

    if((status=PCIE6509_Get_InterruptCounter(MyHandle, &counter))) {
        Display_Library_Error(MyHandle);
        return;
    }

    fprintf(stderr,"Interrupt Counter: %d\n",(int)counter);
}
/***********************************************************************/

/***********************************************************************/
int
Display_Library_Error(void *MyHandle)
{
    int status;
    pcie6509_lib_error_t    lib_error;

    /*** GET LIBRARY ERROR ***/
    status=PCIE6509_Get_Lib_Error(MyHandle,&lib_error);
    fprintf(stderr,"===============================================\n");
    fprintf(stderr,"library error information:\n");
    if(lib_error.error != PCIE6509_LIB_NO_ERROR) {
        fprintf(stderr,"lib function:   %s(line=%d)\n",
                lib_error.function, lib_error.line_number);
    }
    fprintf(stderr,"       error:   %d\n",lib_error.error);
    fprintf(stderr,"        name:   %s\n",lib_error.name);
    fprintf(stderr,"        desc:   %s\n",lib_error.desc);
    fprintf(stderr,"===============================================\n");

    return(lib_error.error);
}
/***********************************************************************/

/***********************************************************************/
void
pcie6509_get_line_state()
{
    u_short              line;
    pcie6509_line_hilo_t state;

again:
    fprintf(stderr,"   Enter Line Number [%d-%d] ->",0,PCIE6509_MAX_LINES-1);

    if(user_input(flush,&cp) == 0)  /* if break received, skip */
        return;

    line = strtoul(cp,NULL,0);

    if((line > (PCIE6509_MAX_LINES-1))) {
        fprintf(stderr,"### ERROR! Invalid Line %d ###\n",
                line);
        goto again;
    }

    if((status=PCIE6509_Get_LineState(MyHandle, line, &state))) {
        Display_Library_Error(MyHandle);
        return;
    }

    fprintf(stderr,"Line %d = %s\n",line,
            (state==PCIE6509_LINE_LOW)?"Low":"High");

    return;
}
/***********************************************************************/

/***********************************************************************/
void
pcie6509_get_mapped_local_ptr()
{
    local_ptr = NULL;
    if((status=PCIE6509_Get_Mapped_Local_Ptr(MyHandle, &local_ptr))) {
        Display_Library_Error(MyHandle);
        return;
    } else { 
        fprintf(stderr,"\nlocal_ptr: %p\n",local_ptr);
    }
}
/***********************************************************************/

/***********************************************************************/
void
pcie6509_get_physical_memory()
{
    pcie6509_phys_mem_t phys_mem;

    phys_mem.phys_mem = 0;
    phys_mem.phys_mem_size = 0;

    /*** GET PHYSICAL ADDRESS ***/
    if((status=PCIE6509_Get_Physical_Memory(MyHandle, &phys_mem))) {
        if(Display_Library_Error(MyHandle) == PCIE6509_LIB_IOCTL_FAILED) {
            Display_Driver_Error(MyHandle);
            return;
        }
    } else {
        fprintf(stderr,"Physical Memory: address = %p, size=%d (0x%x)\n",
                phys_mem.phys_mem, phys_mem.phys_mem_size,
                phys_mem.phys_mem_size);
    }
}
/***********************************************************************/

/***********************************************************************/
void
pcie6509_get_port_direction()
{
    u_short port;
    u_char  line_mask;

    for(port = 0; port < PCIE6509_MAX_PORTS; port ++) {
        if((status=PCIE6509_Get_PortDirection(MyHandle, port, &line_mask))) {
            Display_Library_Error(MyHandle);
            return;
        }
        fprintf(stderr,"    ShadowRegister::Port %2d, Line Mask -> 0x%02x\n",port,line_mask);
    }
}
/***********************************************************************/

/***********************************************************************/
void
pcie6509_get_port_direction_pmask()
{
    pcie6509_io_port_t  iop;
    u_short             port;

    iop.port_mask = PCIE6509_ALL_PORTS_MASK;

    if((status=PCIE6509_Get_PortDirectionPmask(MyHandle, &iop))) {
        Display_Library_Error(MyHandle);
        return;
    }

    for(port = 0; port < PCIE6509_MAX_PORTS; port ++) {
        fprintf(stderr,"    ShadowRegister::Port %2d, Line Mask -> 0x%02x\n",port,iop.line_mask[port]); 
    }
}
/***********************************************************************/

/***********************************************************************/
void
pcie6509_get_rise_edge_enable()
{
    u_short port;
    u_char  line_mask;

    for(port = 0; port < PCIE6509_MAX_PORTS; port ++) {
        if((status=PCIE6509_Get_RiseEdgeEnable(MyHandle, port, &line_mask))) {
            Display_Library_Error(MyHandle);
            return;
        }
        fprintf(stderr,"    ShadowRegister::Port %2d, Line Mask -> 0x%02x\n",port,line_mask); 
    }
}
/***********************************************************************/

/***********************************************************************/
void
pcie6509_get_rise_edge_enable_pmask()
{
    pcie6509_io_port_t  iop;
    u_short             port;

    iop.port_mask = PCIE6509_ALL_PORTS_MASK;

    if((status=PCIE6509_Get_RiseEdgeEnablePmask(MyHandle, &iop))) {
        Display_Library_Error(MyHandle);
        return;
    }

    for(port = 0; port < PCIE6509_MAX_PORTS; port ++) {
        fprintf(stderr,"    ShadowRegister::Port %2d, Line Mask -> 0x%02x\n",port,iop.line_mask[port]); 
    }
}
/***********************************************************************/

/***********************************************************************/
void
pcie6509_get_rtsi_configuration()
{
    pcie6509_rtsi_config_t  master;
    pcie6509_rtsi_config_t  slave;

    if((status=PCIE6509_Get_RTSIConfiguration(MyHandle, &master, &slave))) {
        Display_Library_Error(MyHandle);
        return;
    }

    _pcie6509_get_rtsi_configuration("Master (port 0..5)",&master);
    _pcie6509_get_rtsi_configuration("Slave (port 6..11)",&slave);
}

void
_pcie6509_get_rtsi_configuration(char *who, pcie6509_rtsi_config_t *config) 
{
    int     line;
    char    str[40];

    fprintf(stderr,"\n%s...\n",who);
    for(line=0; line < PCIE6509_LINES_PER_PORT; line++) {
        if(PCIE6509_LINE_GET(config->CSRTSITrigDirection,line)) {
            switch(config->CSRTSIOutputSelect[line]) {
                case PCIE6509_RTSICON_EXPORT_DIO_CHANGE:
                    strcpy(str,"Export DIO Change Detection Signal");
                break;
                case PCIE6509_RTSICON_EXPORT_WDT_EXPIRE:
                    strcpy(str,"Export Watchdog Timer Expired Signal");
                break;
                default:
                    strcpy(str,"*** Not Defined ***");
                break;
            }
        } else {
            str[0]=0;
        }
        if(str[0])
            fprintf(stderr,"    ShadowRegister::Line %d (OUTPUT), RTSI Configuration -> (%d) %s\n",
                                line, config->CSRTSIOutputSelect[line],str);
        else
            fprintf(stderr,"    ShadowRegister::Line %d (INPUT)\n", line);
    }
}

/***********************************************************************/

/***********************************************************************/
void
pcie6509_get_scrap_register()
{
    u_int   scrap;

    if((status=PCIE6509_Get_ScrapRegister(MyHandle, &scrap))) {
        Display_Library_Error(MyHandle);
        return;
    }

    fprintf(stderr,"Scrap Register: 0x%08x\n",scrap);
}
/***********************************************************************/

/***********************************************************************/
void
pcie6509_get_scratch_pad_register()
{
    u_int   master;
    u_int   slave;

    if((status=PCIE6509_Get_ScratchPadRegister(MyHandle, &master, &slave))) {
        Display_Library_Error(MyHandle);
        return;
    }

    fprintf(stderr,"Scratch Pad Register: Master -> 0x%08x, Slave -> 0x%08x\n",master,slave);
}
/***********************************************************************/

/***********************************************************************/
void
pcie6509_get_shadow_registers_ptr()
{
    shadow_reg_ptr = NULL;
    if((status=PCIE6509_Get_Shadow_Registers_Ptr(MyHandle, &shadow_reg_ptr))) {
        Display_Library_Error(MyHandle);
        return;
    } else { 
        fprintf(stderr,"\nshadow_reg_ptr: %p\n",shadow_reg_ptr);
    }
}
/***********************************************************************/

/***********************************************************************/
void
pcie6509_get_uptime()
{
    pcie6509_uptime_t master, slave;

    if((status=PCIE6509_Get_UpTime(MyHandle, &master, &slave))) {
        Display_Library_Error(MyHandle);
        return;
    } else { 
        fprintf(stderr,"\nUptime...\n");
        fprintf(stderr,"    Master: Rawtime 0x%08x\n",master.raw_time);
        fprintf(stderr,"            Days %d, Hours %d, Minutes %d, Seconds %d.%d\n",
                                    master.days,master.hours,master.minutes,master.seconds,
                                    master.milli_seconds);
        fprintf(stderr,"\n");
        fprintf(stderr,"    Slave:  Rawtime 0x%08x\n",slave.raw_time);
        fprintf(stderr,"            Days %d, Hours %d, Minutes %d, Seconds %d.%d\n",
                                    slave.days,slave.hours,slave.minutes,slave.seconds,
                                    slave.milli_seconds);
    }
}
/***********************************************************************/

/***********************************************************************/
what get_value[]= {
    { PCIE6509_CTRL_IDENTIFICATION, "PCIE6509_CTRL_IDENTIFICATION", "" },
    { PCIE6509_CTRL_INTERRUPT_MASK, "PCIE6509_CTRL_INTERRUPT_MASK", "" },
    { PCIE6509_CTRL_INTERRUPT_STATUS, "PCIE6509_CTRL_INTERRUPT_STATUS", "" },
    { PCIE6509_CTRL_VOLATILE_INTERRUPT_STATUS, "PCIE6509_CTRL_VOLATILE_INTERRUPT_STATUS", "" },
    { PCIE6509_CTRL_SCRAP, "PCIE6509_CTRL_SCRAP", "" },
    { PCIE6509_CTRL_PCI_SUBSYSTEM_ID, "PCIE6509_CTRL_PCI_SUBSYSTEM_ID", "" },
    { PCIE6509_CTRL_MASTER_CS_SCRATCHPAD, "PCIE6509_CTRL_MASTER_CS_SCRATCHPAD", "" },
    { PCIE6509_CTRL_MASTER_CS_SIGNATURE, "PCIE6509_CTRL_MASTER_CS_SIGNATURE", "" },
    { PCIE6509_CTRL_MASTER_CS_TIME_SINCE_POWERUP, "PCIE6509_CTRL_MASTER_CS_TIME_SINCE_POWERUP", "" },
    { PCIE6509_CTRL_MASTER_CS_WATCHDOG_STATUS, "PCIE6509_CTRL_MASTER_CS_WATCHDOG_STATUS", "" },
    { PCIE6509_CTRL_MASTER_CS_GLOBAL_INTERRUPT_STATUS, "PCIE6509_CTRL_MASTER_CS_GLOBAL_INTERRUPT_STATUS", "" },
    { PCIE6509_CTRL_MASTER_CS_DI_INTERRUPT_STATUS, "PCIE6509_CTRL_MASTER_CS_DI_INTERRUPT_STATUS", "" },
    { PCIE6509_CTRL_MASTER_CS_WATCHDOG_TIMER_INTERRUPT_STATUS, "PCIE6509_CTRL_MASTER_CS_WATCHDOG_TIMER_INTERRUPT_STATUS", "" },
    { PCIE6509_CTRL_MASTER_PFI_STATIC_DIGITAL_INPUT, "PCIE6509_CTRL_MASTER_PFI_STATIC_DIGITAL_INPUT", "" },
    { PCIE6509_CTRL_MASTER_DIO_PORTS_STATIC_DIGITAL_INPUT, "PCIE6509_CTRL_MASTER_DIO_PORTS_STATIC_DIGITAL_INPUT", "" },
    { PCIE6509_CTRL_MASTER_CS_DI_CHANGE_DETECTION_STATUS, "PCIE6509_CTRL_MASTER_CS_DI_CHANGE_DETECTION_STATUS", "" },
    { PCIE6509_CTRL_MASTER_DIO_PORTS_DI_CHANGE_DETECTION_LATCHED, "PCIE6509_CTRL_MASTER_DIO_PORTS_DI_CHANGE_DETECTION_LATCHED", "" },
    { PCIE6509_CTRL_MASTER_PFI_CHANGE_DETECTION_LATCHED, "PCIE6509_CTRL_MASTER_PFI_CHANGE_DETECTION_LATCHED", "" },
    { PCIE6509_CTRL_MASTER_CS_INT_FORWARDING_CONTROL_STATUS, "PCIE6509_CTRL_MASTER_CS_INT_FORWARDING_CONTROL_STATUS", "" },
    { PCIE6509_CTRL_MASTER_CS_INT_FORWARDING_DESTINATION, "PCIE6509_CTRL_MASTER_CS_INT_FORWARDING_DESTINATION", "" },
    { PCIE6509_CTRL_SLAVE_CS_SCRATCHPAD, "PCIE6509_CTRL_SLAVE_CS_SCRATCHPAD", "" },
    { PCIE6509_CTRL_SLAVE_CS_SIGNATURE, "PCIE6509_CTRL_SLAVE_CS_SIGNATURE", "" },
    { PCIE6509_CTRL_SLAVE_CS_TIME_SINCE_POWERUP, "PCIE6509_CTRL_SLAVE_CS_TIME_SINCE_POWERUP", "" },
    { PCIE6509_CTRL_SLAVE_CS_WATCHDOG_STATUS, "PCIE6509_CTRL_SLAVE_CS_WATCHDOG_STATUS", "" },
    { PCIE6509_CTRL_SLAVE_CS_GLOBAL_INTERRUPT_STATUS, "PCIE6509_CTRL_SLAVE_CS_GLOBAL_INTERRUPT_STATUS", "" },
    { PCIE6509_CTRL_SLAVE_CS_DI_INTERRUPT_STATUS, "PCIE6509_CTRL_SLAVE_CS_DI_INTERRUPT_STATUS", "" },
    { PCIE6509_CTRL_SLAVE_CS_WATCHDOG_TIMER_INTERRUPT_STATUS, "PCIE6509_CTRL_SLAVE_CS_WATCHDOG_TIMER_INTERRUPT_STATUS", "" },
    { PCIE6509_CTRL_SLAVE_PFI_STATIC_DIGITAL_INPUT, "PCIE6509_CTRL_SLAVE_PFI_STATIC_DIGITAL_INPUT", "" },
    { PCIE6509_CTRL_SLAVE_DIO_PORTS_STATIC_DIGITAL_INPUT, "PCIE6509_CTRL_SLAVE_DIO_PORTS_STATIC_DIGITAL_INPUT", "" },
    { PCIE6509_CTRL_SLAVE_CS_DI_CHANGE_DETECTION_STATUS, "PCIE6509_CTRL_SLAVE_CS_DI_CHANGE_DETECTION_STATUS", "" },
    { PCIE6509_CTRL_SLAVE_DIO_PORTS_DI_CHANGE_DETECTION_LATCHED, "PCIE6509_CTRL_SLAVE_DIO_PORTS_DI_CHANGE_DETECTION_LATCHED", "" },
    { PCIE6509_CTRL_SLAVE_PFI_CHANGE_DETECTION_LATCHED, "PCIE6509_CTRL_SLAVE_PFI_CHANGE_DETECTION_LATCHED", "" },
    { PCIE6509_CTRL_SLAVE_CS_INT_FORWARDING_CONTROL_STATUS, "PCIE6509_CTRL_SLAVE_CS_INT_FORWARDING_CONTROL_STATUS", "" },
    { PCIE6509_CTRL_SLAVE_CS_INT_FORWARDING_DESTINATION, "PCIE6509_CTRL_SLAVE_CS_INT_FORWARDING_DESTINATION", "" },

    { 0, 0, 0, }    /* list terminator */
};

/*--------------*/

void
pcie6509_get_value()
{
    int     value;
    char    *name;
    int     ret_parm;

    if(Table_Selection(get_value, "Get Register Value", &ret_parm, 
                       &name, NULL ) == 0)
        return;

    if((status=PCIE6509_Get_Value(MyHandle, ret_parm, &value))) {
        Display_Library_Error(MyHandle);
    } else {
        fprintf(stderr,"%s=%d (0x%x)\n",name, value, value);
    }
}
/***********************************************************************/

/***********************************************************************/
void _pcie6509_get_watchdog_configuration(char *who, pcie6509_wdt_config_t *ms);

void
pcie6509_get_watchdog_configuration()
{
    pcie6509_wdt_config_t   master;
    pcie6509_wdt_config_t   slave;

    if((status=PCIE6509_Get_WatchdogConfiguration(MyHandle, &master, &slave))) {
        Display_Library_Error(MyHandle);
        return;
    }
    
    _pcie6509_get_watchdog_configuration("Master (port 0..5)",&master);
    _pcie6509_get_watchdog_configuration("Slave (port 6..11)",&slave);
}

void
_pcie6509_get_watchdog_configuration(char *who, pcie6509_wdt_config_t *ms)
{
    fprintf(stderr,"\n%s...\n",who);
    fprintf(stderr,"    ShadowRegister::External Trigger Select   -> PCIE6509_WDTC_RTSI%d\n",
                                                    ms->ExtTrigSel-PCIE6509_WDTC_RTSI0); 
    fprintf(stderr,"    ShadowRegister::External Trigger Polarity -> %s\n",
                            (ms->ExtTrigPol==PCIE6509_WDTC_ACTIVE_HIGH)?
                            "Active High":"Active Low");
    fprintf(stderr,"    ShadowRegister::External Trigger Enable   -> %s\n",
                            (ms->ExtTrigEn==PCIE6509_WDTC_EXT_DISABLED)?
                            "Disabled":"Use External Trigger Specified by External Trigger Select");
    fprintf(stderr,"    ShadowRegister::Internal Trigger Enable   -> %s\n",
                            (ms->IntTrigEn==PCIE6509_WDTC_INT_DISABLED)?
                            "Disabled":"Use Internal Trigger Counter");
}

/***********************************************************************/
void _pcie6509_get_watchdog_control(char *who, pcie6509_wdt_control_t ms);

void
pcie6509_get_watchdog_control()
{
    pcie6509_wdt_control_t   master;
    pcie6509_wdt_control_t   slave;

    if((status=PCIE6509_Get_WatchdogControl(MyHandle, &master, &slave))) {
        Display_Library_Error(MyHandle);
        return;
    }
    
    _pcie6509_get_watchdog_control("Master (port 0..5)",master);
    _pcie6509_get_watchdog_control("Slave (port 6..11)",slave);
}

void
_pcie6509_get_watchdog_control(char *who, pcie6509_wdt_control_t ms)
{
    char    ctrl[50];
    fprintf(stderr,"\n%s...\n",who);
    switch(ms) {
        case PCIE6509_WDTCON_START:
            strcpy(ctrl,"Start watchdog");
        break;
        case PCIE6509_WDTCON_RESTART_FEED:
            strcpy(ctrl,"Restart watchdog with FEED sequence");
        break;
        case PCIE6509_WDTCON_RESTART_FOOD:
            strcpy(ctrl,"Restart watchdog with FOOD sequence");
        break;
        case PCIE6509_WDTCON_PAUSE:
            strcpy(ctrl,"Pause watchdog");
        break;
        case PCIE6509_WDTCON_TERMINATE:
            strcpy(ctrl,"Terminate watchdog");
        break;
        case PCIE6509_WDTCON_RESTART_ACED:
            strcpy(ctrl,"Restart watchdog");
        break;
        default:
            strcpy(ctrl,"### Undefined Watchdog Control ###");
        break;
    }
    fprintf(stderr,"    ShadowRegister::WDT Control -> (0x%04X) %s\n",ms,ctrl);
}

/***********************************************************************/

/***********************************************************************/
void
pcie6509_get_watchdog_mode()
{
    int                         line;
    char                        str[40];
    u_short                     port;
    u_char                      line_mask;
    pcie6509_watchdog_mode_t    mode[PCIE6509_LINES_PER_PORT];

    fprintf(stderr,"   Enter Port (0..%d) ->", PCIE6509_MAX_PORTS-1);

    if(user_input(flush,&cp) == 0)  /* if break received, skip */
        return;

    port = strtoul(cp,NULL,0);

    line_mask = PCIE6509_ALL_LINES_MASK;

    if((status=PCIE6509_Get_WatchdogMode(MyHandle, port, line_mask, mode))) {
        Display_Library_Error(MyHandle);
        return;
    }

    fprintf(stderr,"    ShadowRegister::Port %d...\n",port);
    for(line=0; line < PCIE6509_LINES_PER_PORT; line++) {
        if(PCIE6509_LINE_GET(line_mask,line)) {
            switch(mode[line]) {
                case PCIE6509_WDT_MODE_DISABLE:
                    strcpy(str,"Disabled");
                break;
                case PCIE6509_WDT_MODE_FREEZE:
                    strcpy(str,"Freeze");
                break;
                case PCIE6509_WDT_MODE_TRISTATE:
                    strcpy(str,"Tristate");
                break;
                case PCIE6509_WDT_MODE_SAFE_VALUE:
                    strcpy(str,"Save Value");
                break;
                default:
                    strcpy(str,"*** Not Defined ***");
                break;
            }
            fprintf(stderr,"  Line %d, Mode -> %s\n",line, str);
        }
    }
}
/***********************************************************************/

/***********************************************************************/
void
pcie6509_get_watchdog_safe_state()
{
    u_short port;
    u_char  line_mask;

    for(port = 0; port < PCIE6509_MAX_PORTS; port ++) {
        if((status=PCIE6509_Get_WatchdogSafeState(MyHandle, port, &line_mask))) {
            Display_Library_Error(MyHandle);
            return;
        }
        fprintf(stderr,"    ShadowRegister::Port %2d, Line Mask -> 0x%02x\n",port,line_mask); 
    }
}
/***********************************************************************/

/***********************************************************************/
void
pcie6509_get_watchdog_safe_state_pmask()
{
    pcie6509_io_port_t  iop;
    u_short             port;

    iop.port_mask = PCIE6509_ALL_PORTS_MASK;

    if((status=PCIE6509_Get_WatchdogSafeStatePmask(MyHandle, &iop))) {
        Display_Library_Error(MyHandle);
        return;
    }

    for(port = 0; port < PCIE6509_MAX_PORTS; port ++) {
        fprintf(stderr,"    ShadowRegister::Port %2d, Line Mask -> 0x%02x\n",port,iop.line_mask[port]); 
    }
}
/***********************************************************************/

/***********************************************************************/
void _pcie6509_get_watchdog_status(char *who, pci6509_watchdog_status_t *ms);

void
pcie6509_get_watchdog_status()
{
    pci6509_watchdog_status_t   master;
    pci6509_watchdog_status_t   slave;

    if((status=PCIE6509_Get_WatchdogStatus(MyHandle, &master, &slave))) {
        Display_Library_Error(MyHandle);
        return;
    }
    
    _pcie6509_get_watchdog_status("Master (port 0..5)",&master);
    _pcie6509_get_watchdog_status("Slave (port 6..11)",&slave);
}

void
_pcie6509_get_watchdog_status(char *who, pci6509_watchdog_status_t *ms)
{
    char    ctrl[50];
    fprintf(stderr,"\n%s...\n",who);

    fprintf(stderr,"    Expiration Count: %d\n",ms->expiration_count);
    switch(ms->state) {
        case PCIE6509_WDTS_STATE_SYNCH_RESET:
            strcpy(ctrl,"Synch Reset");
        break;
        case PCIE6509_WDTS_STATE_COUNT_DOWN_FEED:
            strcpy(ctrl,"Countdown FEED");
        break;
        case PCIE6509_WDTS_STATE_COUNT_DOWN_FOOD:
            strcpy(ctrl,"Countdown FOOD");
        break;
        case PCIE6509_WDTS_STATE_SLEEPING:
            strcpy(ctrl,"Sleeping");
        break;
        case PCIE6509_WDTS_STATE_EXPIRED_PULSE:
            strcpy(ctrl,"Expired Pulse");
        break;
        case PCIE6509_WDTS_STATE_EXPIRED:
            strcpy(ctrl,"Expired");
        break;
        default:
            strcpy(ctrl,"### Undefined Watchdog Status ###");
        break;
    }
    fprintf(stderr,"    WDT Status: (0x%04x) %s\n",ms->state,ctrl);
}
/***********************************************************************/

void
pcie6509_get_watchdog_timeout()
{
    double  master_timeout;
    double  slave_timeout;
    u_int   raw_master_timeout;
    u_int   raw_slave_timeout;

    if((status=PCIE6509_Get_WatchdogTimeout(MyHandle,
                &master_timeout, &slave_timeout, 
                &raw_master_timeout, &raw_slave_timeout))) {
        Display_Library_Error(MyHandle);
        return;
    }

    fprintf(stderr,"Watchdog Timeout...\n");
    fprintf(stderr,
        "    ShadowRegister::Master (port0..5) = %f milli-secs, (0x%08x) bus clock\n",
            master_timeout,raw_master_timeout);

    fprintf(stderr,
        "    ShadowRegister::Slave (port6..11) = %f milli-secs, (0x%08x) bus clock\n",
            slave_timeout,raw_slave_timeout);
}

/***********************************************************************/
void
pcie6509_initialize_board()
{
    if((status=PCIE6509_Initialize_Board(MyHandle))) {
        Display_Library_Error(MyHandle);
        return;
    }
}
/***********************************************************************/

/***********************************************************************/
void
pcie6509_joint_reset()
{
    u_char  who;

    fprintf(stderr," %d - PCIE6509_SLAVE_STC\n",PCIE6509_SLAVE_STC);
    fprintf(stderr," %d - PCIE6509_MASTER_STC\n",PCIE6509_MASTER_STC);
    fprintf(stderr,"     Reset ->");

    if(user_input(flush,&cp) == 0)  /* if break received, skip */
        return;

    who = strtoul(cp, NULL,0);

    if((status=PCIE6509_JointReset(MyHandle, who))) {
        Display_Library_Error(MyHandle);
        return;
    }
}
/***********************************************************************/

/***********************************************************************/
void
pcie6509_mmap_physical_memory()
{
    void    *mapped_phys_mem_ptr;
    int     physical_size;

    fprintf(stderr,"   Enter physical memory size in bytes ->");

    if(user_input(flush,&cp) == 0)  /* if break received, skip */
        return;

    physical_size = strtoul(cp,NULL,0);

    if(physical_size == 0) {
        fprintf(stderr,"### ERROR! Length of zero is invalid ###\n");
        return;
    }

    mapped_phys_mem_ptr = NULL;

    if((status=PCIE6509_MMap_Physical_Memory(MyHandle, physical_size, 
                                                &mapped_phys_mem_ptr))) {
        if(Display_Library_Error(MyHandle) == PCIE6509_LIB_MMAP_FAILED) {
            Display_Driver_Error(MyHandle);
            return;
        }
    }

    fprintf(stderr,"\nMapped Physical_Memory_Ptr=%p\n", mapped_phys_mem_ptr);

    PCIE6509_Snap(stderr, "#### PHYSICAL MEMORY (RAW DATA) ####", 
                (char *)mapped_phys_mem_ptr, 
                64, 0, "+DBG+ ", 'c');
}
/***********************************************************************/

/***********************************************************************/
void
pcie6509_munmap_physical_memory()
{
    if((status=PCIE6509_Munmap_Physical_Memory(MyHandle))) {
        if(Display_Library_Error(MyHandle) == PCIE6509_LIB_MUNMAP_FAILED) {
            Display_Driver_Error(MyHandle);
        }
    }
}
/***********************************************************************/

/***********************************************************************/
void
pcie6509_read_operation()
{
    pcie6509_io_port_t rp;
    int             bytes_read;
    int             error;
    int             port;

    bzero(&rp, sizeof(rp));

    rp.port_mask = PCIE6509_ALL_PORTS_MASK;

    if((status=PCIE6509_Read(MyHandle, &rp, sizeof(rp), &bytes_read, 
                                                            &error))) {
        fprintf(stderr,"Read Failed: %s\n",strerror(error));
        Display_Library_Error(MyHandle);
        return;
    }

    fprintf(stderr,"Port Mask: 0x%03x (bytes read=%d)\n",rp.port_mask,bytes_read);

    for(port = 0; port < PCIE6509_MAX_PORTS; port++) {
        if(!(port % 4))
            fprintf(stderr,"\nPort");
        fprintf(stderr,"  P%-2d = 0x%02x",port,rp.line_mask[port]);
    }
    fprintf(stderr,"\n");
}
/***********************************************************************/

/***********************************************************************/
void
pcie6509_set_all_outputs()
{
    pcie6509_io_port_t  iop;
    u_short             port;

    bzero(&iop,sizeof(iop));

    fprintf(stderr,"   Enter Line Mask (0..0x%02x) ->",PCIE6509_ALL_LINES_MASK);

    if(user_input(flush,&cp) == 0)  /* if break received, skip */
         return;

    /* Set all ports to output */
    iop.port_mask = PCIE6509_ALL_PORTS_MASK;
    for(port=0; port < PCIE6509_MAX_PORTS; port++) {
        iop.line_mask[port] = 0xFF;     /* set all lines to output */
    }

    if((status=PCIE6509_Set_PortDirectionPmask(MyHandle, &iop))) {
        Display_Library_Error(MyHandle);
        return;
    }

    /* Set all ports to output */
    iop.port_mask = PCIE6509_ALL_PORTS_MASK;
    for(port=0; port < PCIE6509_MAX_PORTS; port++) {
        iop.line_mask[port] = strtoul(cp,NULL,16);     /* set all lines to output */
    }

    if((status=PCIE6509_Set_DigitalOutputPmask(MyHandle, &iop))) {
        Display_Library_Error(MyHandle);
        return;
    }
}
/***********************************************************************/

/***********************************************************************/
void
pcie6509_set_digital_output()
{
    u_short                     port;
    u_char                      line_mask;

    fprintf(stderr,"   Enter Port (0..%d) ->", PCIE6509_MAX_PORTS-1);

    if(user_input(flush,&cp) == 0)  /* if break received, skip */
        return;

    port = strtoul(cp,NULL,0);

    fprintf(stderr,"   Enter Line Mask (0..0x%02x) ->",PCIE6509_ALL_LINES_MASK);

    if(user_input(flush,&cp) == 0)  /* if break received, skip */
        return;

    line_mask = strtoul(cp,NULL,16);

    /* Set all lines for selected port as output */
    if((status=PCIE6509_Set_PortDirection(MyHandle, port, 0xFF))) {
        Display_Library_Error(MyHandle);
        return;
    }

    if((status=PCIE6509_Set_DigitalOutput(MyHandle, port, line_mask))) {
        Display_Library_Error(MyHandle);
        return;
    }
}
/***********************************************************************/

/***********************************************************************/
void
pcie6509_set_digital_output_pmask()
{
    pcie6509_io_port_t  iop;
    u_short             port;

    bzero(&iop,sizeof(iop));

    fprintf(stderr,"   Enter Port Mask (0..0x%03x) ->", PCIE6509_ALL_PORTS_MASK);

    if(user_input(flush,&cp) == 0)  /* if break received, skip */
        return;

    iop.port_mask = strtoul(cp,NULL,16);

    /* Set selected ports to output */
    for(port=0; port < PCIE6509_MAX_PORTS; port++) {
        if(PCIE6509_PORT_GET(iop.port_mask,port)) {
            iop.line_mask[port] = 0xFF;     /* set all lines to output */
        }
    }

    if((status=PCIE6509_Set_PortDirectionPmask(MyHandle, &iop))) {
        Display_Library_Error(MyHandle);
        return;
    }

    /* clear line mask */
    bzero(&iop.line_mask,sizeof(iop.line_mask));

    for(port=0; port < PCIE6509_MAX_PORTS; port++) {
        if(PCIE6509_PORT_GET(iop.port_mask,port)) {
            fprintf(stderr,"   Enter Port%02d, Line Mask (0..0x%02x) ->",port,PCIE6509_ALL_LINES_MASK);

            if(user_input(flush,&cp) == 0)  /* if break received, skip */
                return;

            iop.line_mask[port] = strtoul(cp,NULL,16);
        }
    }

    if((status=PCIE6509_Set_DigitalOutputPmask(MyHandle, &iop))) {
        Display_Library_Error(MyHandle);
        return;
    }
}
/***********************************************************************/

/***********************************************************************/
void
pcie6509_set_fall_edge_enable()
{
    u_short         port;
    u_char          line_mask;

    fprintf(stderr,"   Enter Port (0..%d) ->", PCIE6509_MAX_PORTS-1);

    if(user_input(flush,&cp) == 0)  /* if break received, skip */
        return;

    port = strtoul(cp,NULL,0);

    fprintf(stderr,"   Enter Line Mask (0..0x%02x) ->",PCIE6509_ALL_LINES_MASK);

    if(user_input(flush,&cp) == 0)  /* if break received, skip */
        return;

    line_mask = strtoul(cp,NULL,16);

    if((status=PCIE6509_Set_FallEdgeEnable(MyHandle, port, line_mask))) {
        Display_Library_Error(MyHandle);
        return;
    }
}
/***********************************************************************/

/***********************************************************************/
void
pcie6509_set_fall_edge_enable_pmask()
{
    pcie6509_io_port_t  iop;
    u_short             port;

    bzero(&iop,sizeof(iop));

    fprintf(stderr,"   Enter Port Mask (0..0x%03x) ->", PCIE6509_ALL_PORTS_MASK);

    if(user_input(flush,&cp) == 0)  /* if break received, skip */
        return;

    iop.port_mask = strtoul(cp,NULL,16);

    for(port=0; port < PCIE6509_MAX_PORTS; port++) {
        if(PCIE6509_PORT_GET(iop.port_mask,port)) {
            fprintf(stderr,"   Enter Port%02d, Line Mask (0..0x%02x) ->",port,PCIE6509_ALL_LINES_MASK);

            if(user_input(flush,&cp) == 0)  /* if break received, skip */
                return;

            iop.line_mask[port] = strtoul(cp,NULL,16);
        }
    }

    if((status=PCIE6509_Set_FallEdgeEnablePmask(MyHandle, &iop))) {
        Display_Library_Error(MyHandle);
        return;
    }
}
/***********************************************************************/

/***********************************************************************/
void
pcie6509_set_filter()
{
    u_short                     port;
    u_char                      line_mask;
    pcie6509_filter_option_t    option;

    fprintf(stderr,"   Enter Port (0..%d) ->", PCIE6509_MAX_PORTS-1);

    if(user_input(flush,&cp) == 0)  /* if break received, skip */
        return;

    port = strtoul(cp,NULL,0);

    fprintf(stderr,"   Enter Line Mask (0..0x%02x) ->",PCIE6509_ALL_LINES_MASK);

    if(user_input(flush,&cp) == 0)  /* if break received, skip */
        return;

    line_mask = strtoul(cp,NULL,16);

    fprintf(stderr,"  %d - PCIE6509_FILTER_NONE\n",PCIE6509_FILTER_NONE);
    fprintf(stderr,"  %d - PCIE6509_FILTER_SMALL\n",PCIE6509_FILTER_SMALL);
    fprintf(stderr,"  %d - PCIE6509_FILTER_MEDIUM\n",PCIE6509_FILTER_MEDIUM);
    fprintf(stderr,"  %d - PCIE6509_FILTER_LARGE\n",PCIE6509_FILTER_LARGE);

    fprintf(stderr,"   Enter Filter Option ->");

    if(user_input(flush,&cp) == 0)  /* if break received, skip */
        return;

    option = strtoul(cp,NULL,0);

    if((status=PCIE6509_Set_Filter(MyHandle, port, line_mask, option))) {
        Display_Library_Error(MyHandle);
        return;
    }
}
/***********************************************************************/

/***********************************************************************/
void
pcie6509_set_filter_pmask()
{
    pcie6509_io_port_t          iop;
    u_short                     port;
    pcie6509_filter_option_t    option;

    bzero(&iop,sizeof(iop));

    fprintf(stderr,"  %d - PCIE6509_FILTER_NONE\n",PCIE6509_FILTER_NONE);
    fprintf(stderr,"  %d - PCIE6509_FILTER_SMALL\n",PCIE6509_FILTER_SMALL);
    fprintf(stderr,"  %d - PCIE6509_FILTER_MEDIUM\n",PCIE6509_FILTER_MEDIUM);
    fprintf(stderr,"  %d - PCIE6509_FILTER_LARGE\n",PCIE6509_FILTER_LARGE);

    fprintf(stderr,"   Enter Filter Option ->");

    if(user_input(flush,&cp) == 0)  /* if break received, skip */
        return;

    option = strtoul(cp,NULL,0);

    fprintf(stderr,"   Enter Port Mask (0..0x%03x) ->", PCIE6509_ALL_PORTS_MASK);

    if(user_input(flush,&cp) == 0)  /* if break received, skip */
        return;

    iop.port_mask = strtoul(cp,NULL,16);

    for(port=0; port < PCIE6509_MAX_PORTS; port++) {
        if(PCIE6509_PORT_GET(iop.port_mask,port)) {
            fprintf(stderr,"   Enter Port%02d, Line Mask (0..0x%02x) ->",port,PCIE6509_ALL_LINES_MASK);

            if(user_input(flush,&cp) == 0)  /* if break received, skip */
                return;

            iop.line_mask[port] = strtoul(cp,NULL,16);
        }
    }

    if((status=PCIE6509_Set_FilterPmask(MyHandle, &iop, option))) {
        Display_Library_Error(MyHandle);
        return;
    }
}
/***********************************************************************/

/***********************************************************************/
void
pcie6509_set_interrupt_counter()
{
    u_int64_t   counter;
    fprintf(stderr,"  Enter Interrupt Counter to Set ->");

    if(user_input(flush,&cp) == 0)  /* if break received, skip */
        return;

    counter = strtoul(cp,NULL,0);

    if((status=PCIE6509_Set_InterruptCounter(MyHandle, counter))) {
        Display_Library_Error(MyHandle);
        return;
    }
}
/***********************************************************************/

/***********************************************************************/
void
pcie6509_set_port_direction()
{
    u_short                     port;
    u_char                      line_mask;

    fprintf(stderr,"   Enter Port (0..%d) ->", PCIE6509_MAX_PORTS-1);

    if(user_input(flush,&cp) == 0)  /* if break received, skip */
        return;

    port = strtoul(cp,NULL,0);

    fprintf(stderr,"   Enter Line Mask (0..0x%02x) ->",PCIE6509_ALL_LINES_MASK);

    if(user_input(flush,&cp) == 0)  /* if break received, skip */
        return;

    line_mask = strtoul(cp,NULL,16);

    if((status=PCIE6509_Set_PortDirection(MyHandle, port, line_mask))) {
        Display_Library_Error(MyHandle);
        return;
    }
}
/***********************************************************************/

/***********************************************************************/
void
pcie6509_set_port_direction_pmask()
{
    pcie6509_io_port_t  iop;
    u_short             port;

    bzero(&iop,sizeof(iop));

    fprintf(stderr,"   Enter Port Mask (0..0x%03x) ->", PCIE6509_ALL_PORTS_MASK);

    if(user_input(flush,&cp) == 0)  /* if break received, skip */
        return;

    iop.port_mask = strtoul(cp,NULL,16);

    for(port=0; port < PCIE6509_MAX_PORTS; port++) {
        if(PCIE6509_PORT_GET(iop.port_mask,port)) {
            fprintf(stderr,"   Enter Port%02d, Line Mask (0..0x%02x) ->",port,PCIE6509_ALL_LINES_MASK);

            if(user_input(flush,&cp) == 0)  /* if break received, skip */
                return;

            iop.line_mask[port] = strtoul(cp,NULL,16);
        }
    }

    if((status=PCIE6509_Set_PortDirectionPmask(MyHandle, &iop))) {
        Display_Library_Error(MyHandle);
        return;
    }
}
/***********************************************************************/

/***********************************************************************/
void
pcie6509_set_rising_edge_enable()
{
    u_short                     port;
    u_char                      line_mask;

    fprintf(stderr,"   Enter Port (0..%d) ->", PCIE6509_MAX_PORTS-1);

    if(user_input(flush,&cp) == 0)  /* if break received, skip */
        return;

    port = strtoul(cp,NULL,0);

    fprintf(stderr,"   Enter Line Mask (0..0x%02x) ->",PCIE6509_ALL_LINES_MASK);

    if(user_input(flush,&cp) == 0)  /* if break received, skip */
        return;

    line_mask = strtoul(cp,NULL,16);

    if((status=PCIE6509_Set_RiseEdgeEnable(MyHandle, port, line_mask))) {
        Display_Library_Error(MyHandle);
        return;
    }
}
/***********************************************************************/

/***********************************************************************/
void
pcie6509_set_rising_edge_enable_pmask()
{
    pcie6509_io_port_t  iop;
    u_short             port;

    bzero(&iop,sizeof(iop));

    fprintf(stderr,"   Enter Port Mask (0..0x%03x) ->", PCIE6509_ALL_PORTS_MASK);

    if(user_input(flush,&cp) == 0)  /* if break received, skip */
        return;

    iop.port_mask = strtoul(cp,NULL,16);

    for(port=0; port < PCIE6509_MAX_PORTS; port++) {
        if(PCIE6509_PORT_GET(iop.port_mask,port)) {
            fprintf(stderr,"   Enter Port%02d, Line Mask (0..0x%02x) ->",port,PCIE6509_ALL_LINES_MASK);

            if(user_input(flush,&cp) == 0)  /* if break received, skip */
                return;

            iop.line_mask[port] = strtoul(cp,NULL,16);
        }
    }

    if((status=PCIE6509_Set_RiseEdgeEnablePmask(MyHandle, &iop))) {
        Display_Library_Error(MyHandle);
        return;
    }
}
/***********************************************************************/

/***********************************************************************/
void
pcie6509_set_rtsi_configuration()
{
    pcie6509_rtsi_config_t  master;
    pcie6509_rtsi_config_t  slave;
    int                     line;

    fprintf(stderr,"  Enter Master (port0..5) RTSI Direction Mask (0..0x%02x)->",
                PCIE6509_RTSI_DIRECTION_MASK);

    if(user_input(flush,&cp) == 0)  /* if break received, skip */
        return;

    master.CSRTSITrigDirection = strtoul(cp,NULL,16);

    for(line=0; line < PCIE6509_NUMBER_OF_RTSI_LINES; line++) {
        if(PCIE6509_LINE_GET(master.CSRTSITrigDirection,line)) {
            fprintf(stderr,"    %d - Export DIO Change Detection Signal\n",
                                PCIE6509_RTSICON_EXPORT_DIO_CHANGE);
            fprintf(stderr,"    %d - Export Watchdog Timer Expired Signal\n",
                                PCIE6509_RTSICON_EXPORT_WDT_EXPIRE);
            fprintf(stderr,"    RTSI Output Line %d ->",line);

            if(user_input(flush,&cp) == 0)  /* if break received, skip */
                return;
        
            master.CSRTSIOutputSelect[line] = strtoul(cp,NULL,0);
        }
    }

    fprintf(stderr,"  Enter Slave (port6..11) RTSI Direction Mask (0..0x%02x)->",
                PCIE6509_RTSI_DIRECTION_MASK);

    if(user_input(flush,&cp) == 0)  /* if break received, skip */
        return;

    slave.CSRTSITrigDirection = strtoul(cp,NULL,16);

    for(line=0; line < PCIE6509_NUMBER_OF_RTSI_LINES; line++) {
        if(PCIE6509_LINE_GET(slave.CSRTSITrigDirection,line)) {
            fprintf(stderr,"    %d - Export DIO Change Detection Signal\n",
                                PCIE6509_RTSICON_EXPORT_DIO_CHANGE);
            fprintf(stderr,"    %d - Export Watchdog Timer Expired Signal\n",
                                PCIE6509_RTSICON_EXPORT_WDT_EXPIRE);
            fprintf(stderr,"    RTSI Output Line %d ->",line);

            if(user_input(flush,&cp) == 0)  /* if break received, skip */
                return;
        
            slave.CSRTSIOutputSelect[line] = strtoul(cp,NULL,0);
        }
    }

    if((status=PCIE6509_Set_RTSIConfiguration(MyHandle, &master, &slave))) {
        Display_Library_Error(MyHandle);
        return;
    }
}
/***********************************************************************/

/***********************************************************************/
void
pcie6509_set_scrap_register()
{
    u_int   scrap_reg;

    fprintf(stderr,"   Enter Scrap Reg -> ");

    if(user_input(flush,&cp) == 0)  /* if break received, skip */
        return;

    scrap_reg = strtoul(cp,NULL,16);

    if((status=PCIE6509_Set_ScrapRegister(MyHandle, scrap_reg))) {
        Display_Library_Error(MyHandle);
        return;
    }
}
/***********************************************************************/

/***********************************************************************/
void
pcie6509_set_scratchpad_register()
{
    u_int   master_scratchpad_reg;
    u_int   slave_scratchpad_reg;

    fprintf(stderr,"   Enter Master (port0..5) Scratchpad Reg -> ");

    if(user_input(flush,&cp) == 0)  /* if break received, skip */
        return;

    master_scratchpad_reg = strtoul(cp,NULL,16);

    fprintf(stderr,"   Enter Slave (port6..11) Scratchpad Reg -> ");

    if(user_input(flush,&cp) == 0)  /* if break received, skip */
        return;

    slave_scratchpad_reg = strtoul(cp,NULL,16);

    if((status=PCIE6509_Set_ScratchPadRegister(MyHandle, 
                &master_scratchpad_reg, &slave_scratchpad_reg))) {
        Display_Library_Error(MyHandle);
        return;
    }
}
/***********************************************************************/

/***********************************************************************/

what set_value[]= {
    { PCIE6509_CTRL_INTERRUPT_MASK, "PCIE6509_CTRL_INTERRUPT_MASK", "" },
    { PCIE6509_CTRL_SCRAP, "PCIE6509_CTRL_SCRAP", "" },
    { PCIE6509_CTRL_MASTER_CS_SCRATCHPAD, "PCIE6509_CTRL_MASTER_CS_SCRATCHPAD", "" },
    { PCIE6509_CTRL_MASTER_CS_JOINT_RESET, "PCIE6509_CTRL_MASTER_CS_JOINT_RESET", "" },
    { PCIE6509_CTRL_MASTER_CS_WATCHDOG_TIMEOUT, "PCIE6509_CTRL_MASTER_CS_WATCHDOG_TIMEOUT", "" },
    { PCIE6509_CTRL_MASTER_CS_WATCHDOG_CONFIGURATION, "PCIE6509_CTRL_MASTER_CS_WATCHDOG_CONFIGURATION", "" },
    { PCIE6509_CTRL_MASTER_CS_WATCHDOG_CONTROL, "PCIE6509_CTRL_MASTER_CS_WATCHDOG_CONTROL", "" },
    { PCIE6509_CTRL_MASTER_CS_WATCHDOG_TIMER_INTERRUPT1, "PCIE6509_CTRL_MASTER_CS_WATCHDOG_TIMER_INTERRUPT1", "" },
    { PCIE6509_CTRL_MASTER_CS_WATCHDOG_TIMER_INTERRUPT2, "PCIE6509_CTRL_MASTER_CS_WATCHDOG_TIMER_INTERRUPT2", "" },
    { PCIE6509_CTRL_MASTER_CS_GLOBAL_INTERRUPT_ENABLE, "PCIE6509_CTRL_MASTER_CS_GLOBAL_INTERRUPT_ENABLE", "" },
    { PCIE6509_CTRL_MASTER_PFI_DIRECTION, "PCIE6509_CTRL_MASTER_PFI_DIRECTION", "" },
    { PCIE6509_CTRL_MASTER_CS_RTS_TRIG_DIRECTION, "PCIE6509_CTRL_MASTER_CS_RTS_TRIG_DIRECTION", "" },
    { PCIE6509_CTRL_MASTER_CS_RTS_OUTPUT_SELECT_LINE_0, "PCIE6509_CTRL_MASTER_CS_RTS_OUTPUT_SELECT_LINE_0", "" },
    { PCIE6509_CTRL_MASTER_CS_RTS_OUTPUT_SELECT_LINE_1, "PCIE6509_CTRL_MASTER_CS_RTS_OUTPUT_SELECT_LINE_1", "" },
    { PCIE6509_CTRL_MASTER_CS_RTS_OUTPUT_SELECT_LINE_2, "PCIE6509_CTRL_MASTER_CS_RTS_OUTPUT_SELECT_LINE_2", "" },
    { PCIE6509_CTRL_MASTER_CS_RTS_OUTPUT_SELECT_LINE_3, "PCIE6509_CTRL_MASTER_CS_RTS_OUTPUT_SELECT_LINE_3", "" },
    { PCIE6509_CTRL_MASTER_CS_RTS_OUTPUT_SELECT_LINE_4, "PCIE6509_CTRL_MASTER_CS_RTS_OUTPUT_SELECT_LINE_4", "" },
    { PCIE6509_CTRL_MASTER_CS_RTS_OUTPUT_SELECT_LINE_5, "PCIE6509_CTRL_MASTER_CS_RTS_OUTPUT_SELECT_LINE_5", "" },
    { PCIE6509_CTRL_MASTER_CS_RTS_OUTPUT_SELECT_LINE_6, "PCIE6509_CTRL_MASTER_CS_RTS_OUTPUT_SELECT_LINE_6", "" },
    { PCIE6509_CTRL_MASTER_CS_RTS_OUTPUT_SELECT_LINE_7, "PCIE6509_CTRL_MASTER_CS_RTS_OUTPUT_SELECT_LINE_7", "" },
    { PCIE6509_CTRL_MASTER_PFI_FILTER_PORT0_LOW, "PCIE6509_CTRL_MASTER_PFI_FILTER_PORT0_LOW", "" },
    { PCIE6509_CTRL_MASTER_PFI_FILTER_PORT0_HIGH, "PCIE6509_CTRL_MASTER_PFI_FILTER_PORT0_HIGH", "" },
    { PCIE6509_CTRL_MASTER_PFI_FILTER_PORT1_LOW, "PCIE6509_CTRL_MASTER_PFI_FILTER_PORT1_LOW", "" },
    { PCIE6509_CTRL_MASTER_PFI_FILTER_PORT1_HIGH, "PCIE6509_CTRL_MASTER_PFI_FILTER_PORT1_HIGH", "" },
    { PCIE6509_CTRL_MASTER_PFI_OUTPUT_SELECT_PORT0_LINE_0, "PCIE6509_CTRL_MASTER_PFI_OUTPUT_SELECT_PORT0_LINE_0", "" },
    { PCIE6509_CTRL_MASTER_PFI_OUTPUT_SELECT_PORT0_LINE_1, "PCIE6509_CTRL_MASTER_PFI_OUTPUT_SELECT_PORT0_LINE_1", "" },
    { PCIE6509_CTRL_MASTER_PFI_OUTPUT_SELECT_PORT0_LINE_2, "PCIE6509_CTRL_MASTER_PFI_OUTPUT_SELECT_PORT0_LINE_2", "" },
    { PCIE6509_CTRL_MASTER_PFI_OUTPUT_SELECT_PORT0_LINE_3, "PCIE6509_CTRL_MASTER_PFI_OUTPUT_SELECT_PORT0_LINE_3", "" },
    { PCIE6509_CTRL_MASTER_PFI_OUTPUT_SELECT_PORT0_LINE_4, "PCIE6509_CTRL_MASTER_PFI_OUTPUT_SELECT_PORT0_LINE_4", "" },
    { PCIE6509_CTRL_MASTER_PFI_OUTPUT_SELECT_PORT0_LINE_5, "PCIE6509_CTRL_MASTER_PFI_OUTPUT_SELECT_PORT0_LINE_5", "" },
    { PCIE6509_CTRL_MASTER_PFI_OUTPUT_SELECT_PORT0_LINE_6, "PCIE6509_CTRL_MASTER_PFI_OUTPUT_SELECT_PORT0_LINE_6", "" },
    { PCIE6509_CTRL_MASTER_PFI_OUTPUT_SELECT_PORT0_LINE_7, "PCIE6509_CTRL_MASTER_PFI_OUTPUT_SELECT_PORT0_LINE_7", "" },
    { PCIE6509_CTRL_MASTER_PFI_OUTPUT_SELECT_PORT1_LINE_0, "PCIE6509_CTRL_MASTER_PFI_OUTPUT_SELECT_PORT1_LINE_0", "" },
    { PCIE6509_CTRL_MASTER_PFI_OUTPUT_SELECT_PORT1_LINE_1, "PCIE6509_CTRL_MASTER_PFI_OUTPUT_SELECT_PORT1_LINE_1", "" },
    { PCIE6509_CTRL_MASTER_PFI_OUTPUT_SELECT_PORT1_LINE_2, "PCIE6509_CTRL_MASTER_PFI_OUTPUT_SELECT_PORT1_LINE_2", "" },
    { PCIE6509_CTRL_MASTER_PFI_OUTPUT_SELECT_PORT1_LINE_3, "PCIE6509_CTRL_MASTER_PFI_OUTPUT_SELECT_PORT1_LINE_3", "" },
    { PCIE6509_CTRL_MASTER_PFI_OUTPUT_SELECT_PORT1_LINE_4, "PCIE6509_CTRL_MASTER_PFI_OUTPUT_SELECT_PORT1_LINE_4", "" },
    { PCIE6509_CTRL_MASTER_PFI_OUTPUT_SELECT_PORT1_LINE_5, "PCIE6509_CTRL_MASTER_PFI_OUTPUT_SELECT_PORT1_LINE_5", "" },
    { PCIE6509_CTRL_MASTER_PFI_OUTPUT_SELECT_PORT1_LINE_6, "PCIE6509_CTRL_MASTER_PFI_OUTPUT_SELECT_PORT1_LINE_6", "" },
    { PCIE6509_CTRL_MASTER_PFI_OUTPUT_SELECT_PORT1_LINE_7, "PCIE6509_CTRL_MASTER_PFI_OUTPUT_SELECT_PORT1_LINE_7", "" },
    { PCIE6509_CTRL_MASTER_PFI_STATIC_DIGITAL_OUTPUT, "PCIE6509_CTRL_MASTER_PFI_STATIC_DIGITAL_OUTPUT", "" },
    { PCIE6509_CTRL_MASTER_PFI_WDT_SAFE_STATE, "PCIE6509_CTRL_MASTER_PFI_WDT_SAFE_STATE", "" },
    { PCIE6509_CTRL_MASTER_PFI_WDT_MODE_SELECT, "PCIE6509_CTRL_MASTER_PFI_WDT_MODE_SELECT", "" },
    { PCIE6509_CTRL_MASTER_DIO_PORTS_STATIC_DIGITAL_OUTPUT, "PCIE6509_CTRL_MASTER_DIO_PORTS_STATIC_DIGITAL_OUTPUT", "" },
    { PCIE6509_CTRL_MASTER_DIO_PORTS_DIO_DIRECTION, "PCIE6509_CTRL_MASTER_DIO_PORTS_DIO_DIRECTION", "" },
    { PCIE6509_CTRL_MASTER_DIO_PORTS_DO_WDT_SAFE_STATE, "PCIE6509_CTRL_MASTER_DIO_PORTS_DO_WDT_SAFE_STATE", "" },
    { PCIE6509_CTRL_MASTER_DIO_PORTS_DO_WDT_MODE_SELECT_P01, "PCIE6509_CTRL_MASTER_DIO_PORTS_DO_WDT_MODE_SELECT_P01", "" },
    { PCIE6509_CTRL_MASTER_DIO_PORTS_DO_WDT_MODE_SELECT_P23, "PCIE6509_CTRL_MASTER_DIO_PORTS_DO_WDT_MODE_SELECT_P23", "" },
    { PCIE6509_CTRL_MASTER_DIO_PORTS_DI_CHANGE_IRQ_RE, "PCIE6509_CTRL_MASTER_DIO_PORTS_DI_CHANGE_IRQ_RE", "" },
    { PCIE6509_CTRL_MASTER_DIO_PORTS_DI_CHANGE_IRQ_FE, "PCIE6509_CTRL_MASTER_DIO_PORTS_DI_CHANGE_IRQ_FE", "" },
    { PCIE6509_CTRL_MASTER_PFI_CHANGE_IRQ, "PCIE6509_CTRL_MASTER_PFI_CHANGE_IRQ", "" },
    { PCIE6509_CTRL_MASTER_DIO_PORTS_DI_FILTER_P01, "PCIE6509_CTRL_MASTER_DIO_PORTS_DI_FILTER_P01", "" },
    { PCIE6509_CTRL_MASTER_DIO_PORTS_DI_FILTER_P23, "PCIE6509_CTRL_MASTER_DIO_PORTS_DI_FILTER_P23", "" },
    { PCIE6509_CTRL_MASTER_CS_CHANGE_DETECTION_IRQ, "PCIE6509_CTRL_MASTER_CS_CHANGE_DETECTION_IRQ", "" },
    { PCIE6509_CTRL_MASTER_CS_INT_FORWARDING_CONTROL_STATUS, "PCIE6509_CTRL_MASTER_CS_INT_FORWARDING_CONTROL_STATUS", "" },
    { PCIE6509_CTRL_MASTER_CS_INT_FORWARDING_DESTINATION, "PCIE6509_CTRL_MASTER_CS_INT_FORWARDING_DESTINATION", "" },
    { PCIE6509_CTRL_SLAVE_CS_SCRATCHPAD, "PCIE6509_CTRL_SLAVE_CS_SCRATCHPAD", "" },
    { PCIE6509_CTRL_SLAVE_CS_JOINT_RESET, "PCIE6509_CTRL_SLAVE_CS_JOINT_RESET", "" },
    { PCIE6509_CTRL_SLAVE_CS_WATCHDOG_TIMEOUT, "PCIE6509_CTRL_SLAVE_CS_WATCHDOG_TIMEOUT", "" },
    { PCIE6509_CTRL_SLAVE_CS_WATCHDOG_CONFIGURATION, "PCIE6509_CTRL_SLAVE_CS_WATCHDOG_CONFIGURATION", "" },
    { PCIE6509_CTRL_SLAVE_CS_WATCHDOG_CONTROL, "PCIE6509_CTRL_SLAVE_CS_WATCHDOG_CONTROL", "" },
    { PCIE6509_CTRL_SLAVE_CS_WATCHDOG_TIMER_INTERRUPT1, "PCIE6509_CTRL_SLAVE_CS_WATCHDOG_TIMER_INTERRUPT1", "" },
    { PCIE6509_CTRL_SLAVE_CS_WATCHDOG_TIMER_INTERRUPT2, "PCIE6509_CTRL_SLAVE_CS_WATCHDOG_TIMER_INTERRUPT2", "" },
    { PCIE6509_CTRL_SLAVE_CS_GLOBAL_INTERRUPT_ENABLE, "PCIE6509_CTRL_SLAVE_CS_GLOBAL_INTERRUPT_ENABLE", "" },
    { PCIE6509_CTRL_SLAVE_PFI_DIRECTION, "PCIE6509_CTRL_SLAVE_PFI_DIRECTION", "" },
    { PCIE6509_CTRL_SLAVE_CS_RTS_TRIG_DIRECTION, "PCIE6509_CTRL_SLAVE_CS_RTS_TRIG_DIRECTION", "" },
    { PCIE6509_CTRL_SLAVE_CS_RTS_OUTPUT_SELECT_LINE_0, "PCIE6509_CTRL_SLAVE_CS_RTS_OUTPUT_SELECT_LINE_0", "" },
    { PCIE6509_CTRL_SLAVE_CS_RTS_OUTPUT_SELECT_LINE_1, "PCIE6509_CTRL_SLAVE_CS_RTS_OUTPUT_SELECT_LINE_1", "" },
    { PCIE6509_CTRL_SLAVE_CS_RTS_OUTPUT_SELECT_LINE_2, "PCIE6509_CTRL_SLAVE_CS_RTS_OUTPUT_SELECT_LINE_2", "" },
    { PCIE6509_CTRL_SLAVE_CS_RTS_OUTPUT_SELECT_LINE_3, "PCIE6509_CTRL_SLAVE_CS_RTS_OUTPUT_SELECT_LINE_3", "" },
    { PCIE6509_CTRL_SLAVE_CS_RTS_OUTPUT_SELECT_LINE_4, "PCIE6509_CTRL_SLAVE_CS_RTS_OUTPUT_SELECT_LINE_4", "" },
    { PCIE6509_CTRL_SLAVE_CS_RTS_OUTPUT_SELECT_LINE_5, "PCIE6509_CTRL_SLAVE_CS_RTS_OUTPUT_SELECT_LINE_5", "" },
    { PCIE6509_CTRL_SLAVE_CS_RTS_OUTPUT_SELECT_LINE_6, "PCIE6509_CTRL_SLAVE_CS_RTS_OUTPUT_SELECT_LINE_6", "" },
    { PCIE6509_CTRL_SLAVE_CS_RTS_OUTPUT_SELECT_LINE_7, "PCIE6509_CTRL_SLAVE_CS_RTS_OUTPUT_SELECT_LINE_7", "" },
    { PCIE6509_CTRL_SLAVE_PFI_FILTER_PORT0_LOW, "PCIE6509_CTRL_SLAVE_PFI_FILTER_PORT0_LOW", "" },
    { PCIE6509_CTRL_SLAVE_PFI_FILTER_PORT0_HIGH, "PCIE6509_CTRL_SLAVE_PFI_FILTER_PORT0_HIGH", "" },
    { PCIE6509_CTRL_SLAVE_PFI_FILTER_PORT1_LOW, "PCIE6509_CTRL_SLAVE_PFI_FILTER_PORT1_LOW", "" },
    { PCIE6509_CTRL_SLAVE_PFI_FILTER_PORT1_HIGH, "PCIE6509_CTRL_SLAVE_PFI_FILTER_PORT1_HIGH", "" },
    { PCIE6509_CTRL_SLAVE_PFI_OUTPUT_SELECT_PORT0_LINE_0, "PCIE6509_CTRL_SLAVE_PFI_OUTPUT_SELECT_PORT0_LINE_0", "" },
    { PCIE6509_CTRL_SLAVE_PFI_OUTPUT_SELECT_PORT0_LINE_1, "PCIE6509_CTRL_SLAVE_PFI_OUTPUT_SELECT_PORT0_LINE_1", "" },
    { PCIE6509_CTRL_SLAVE_PFI_OUTPUT_SELECT_PORT0_LINE_2, "PCIE6509_CTRL_SLAVE_PFI_OUTPUT_SELECT_PORT0_LINE_2", "" },
    { PCIE6509_CTRL_SLAVE_PFI_OUTPUT_SELECT_PORT0_LINE_3, "PCIE6509_CTRL_SLAVE_PFI_OUTPUT_SELECT_PORT0_LINE_3", "" },
    { PCIE6509_CTRL_SLAVE_PFI_OUTPUT_SELECT_PORT0_LINE_4, "PCIE6509_CTRL_SLAVE_PFI_OUTPUT_SELECT_PORT0_LINE_4", "" },
    { PCIE6509_CTRL_SLAVE_PFI_OUTPUT_SELECT_PORT0_LINE_5, "PCIE6509_CTRL_SLAVE_PFI_OUTPUT_SELECT_PORT0_LINE_5", "" },
    { PCIE6509_CTRL_SLAVE_PFI_OUTPUT_SELECT_PORT0_LINE_6, "PCIE6509_CTRL_SLAVE_PFI_OUTPUT_SELECT_PORT0_LINE_6", "" },
    { PCIE6509_CTRL_SLAVE_PFI_OUTPUT_SELECT_PORT0_LINE_7, "PCIE6509_CTRL_SLAVE_PFI_OUTPUT_SELECT_PORT0_LINE_7", "" },
    { PCIE6509_CTRL_SLAVE_PFI_OUTPUT_SELECT_PORT1_LINE_0, "PCIE6509_CTRL_SLAVE_PFI_OUTPUT_SELECT_PORT1_LINE_0", "" },
    { PCIE6509_CTRL_SLAVE_PFI_OUTPUT_SELECT_PORT1_LINE_1, "PCIE6509_CTRL_SLAVE_PFI_OUTPUT_SELECT_PORT1_LINE_1", "" },
    { PCIE6509_CTRL_SLAVE_PFI_OUTPUT_SELECT_PORT1_LINE_2, "PCIE6509_CTRL_SLAVE_PFI_OUTPUT_SELECT_PORT1_LINE_2", "" },
    { PCIE6509_CTRL_SLAVE_PFI_OUTPUT_SELECT_PORT1_LINE_3, "PCIE6509_CTRL_SLAVE_PFI_OUTPUT_SELECT_PORT1_LINE_3", "" },
    { PCIE6509_CTRL_SLAVE_PFI_OUTPUT_SELECT_PORT1_LINE_4, "PCIE6509_CTRL_SLAVE_PFI_OUTPUT_SELECT_PORT1_LINE_4", "" },
    { PCIE6509_CTRL_SLAVE_PFI_OUTPUT_SELECT_PORT1_LINE_5, "PCIE6509_CTRL_SLAVE_PFI_OUTPUT_SELECT_PORT1_LINE_5", "" },
    { PCIE6509_CTRL_SLAVE_PFI_OUTPUT_SELECT_PORT1_LINE_6, "PCIE6509_CTRL_SLAVE_PFI_OUTPUT_SELECT_PORT1_LINE_6", "" },
    { PCIE6509_CTRL_SLAVE_PFI_OUTPUT_SELECT_PORT1_LINE_7, "PCIE6509_CTRL_SLAVE_PFI_OUTPUT_SELECT_PORT1_LINE_7", "" },
    { PCIE6509_CTRL_SLAVE_PFI_STATIC_DIGITAL_OUTPUT, "PCIE6509_CTRL_SLAVE_PFI_STATIC_DIGITAL_OUTPUT", "" },
    { PCIE6509_CTRL_SLAVE_PFI_WDT_SAFE_STATE, "PCIE6509_CTRL_SLAVE_PFI_WDT_SAFE_STATE", "" },
    { PCIE6509_CTRL_SLAVE_PFI_WDT_MODE_SELECT, "PCIE6509_CTRL_SLAVE_PFI_WDT_MODE_SELECT", "" },
    { PCIE6509_CTRL_SLAVE_DIO_PORTS_STATIC_DIGITAL_OUTPUT, "PCIE6509_CTRL_SLAVE_DIO_PORTS_STATIC_DIGITAL_OUTPUT", "" },
    { PCIE6509_CTRL_SLAVE_DIO_PORTS_DIO_DIRECTION, "PCIE6509_CTRL_SLAVE_DIO_PORTS_DIO_DIRECTION", "" },
    { PCIE6509_CTRL_SLAVE_DIO_PORTS_DO_WDT_SAFE_STATE, "PCIE6509_CTRL_SLAVE_DIO_PORTS_DO_WDT_SAFE_STATE", "" },
    { PCIE6509_CTRL_SLAVE_DIO_PORTS_DO_WDT_MODE_SELECT_P01, "PCIE6509_CTRL_SLAVE_DIO_PORTS_DO_WDT_MODE_SELECT_P01", "" },
    { PCIE6509_CTRL_SLAVE_DIO_PORTS_DO_WDT_MODE_SELECT_P23, "PCIE6509_CTRL_SLAVE_DIO_PORTS_DO_WDT_MODE_SELECT_P23", "" },
    { PCIE6509_CTRL_SLAVE_DIO_PORTS_DI_CHANGE_IRQ_RE, "PCIE6509_CTRL_SLAVE_DIO_PORTS_DI_CHANGE_IRQ_RE", "" },
    { PCIE6509_CTRL_SLAVE_DIO_PORTS_DI_CHANGE_IRQ_FE, "PCIE6509_CTRL_SLAVE_DIO_PORTS_DI_CHANGE_IRQ_FE", "" },
    { PCIE6509_CTRL_SLAVE_PFI_CHANGE_IRQ, "PCIE6509_CTRL_SLAVE_PFI_CHANGE_IRQ", "" },
    { PCIE6509_CTRL_SLAVE_DIO_PORTS_DI_FILTER_P01, "PCIE6509_CTRL_SLAVE_DIO_PORTS_DI_FILTER_P01", "" },
    { PCIE6509_CTRL_SLAVE_DIO_PORTS_DI_FILTER_P23, "PCIE6509_CTRL_SLAVE_DIO_PORTS_DI_FILTER_P23", "" },
    { PCIE6509_CTRL_SLAVE_CS_CHANGE_DETECTION_IRQ, "PCIE6509_CTRL_SLAVE_CS_CHANGE_DETECTION_IRQ", "" },
    { PCIE6509_CTRL_SLAVE_CS_INT_FORWARDING_CONTROL_STATUS, "PCIE6509_CTRL_SLAVE_CS_INT_FORWARDING_CONTROL_STATUS", "" },
    { PCIE6509_CTRL_SLAVE_CS_INT_FORWARDING_DESTINATION, "PCIE6509_CTRL_SLAVE_CS_INT_FORWARDING_DESTINATION", "" },

    { 0, 0, 0, }    /* list terminator */
};

/*--------------*/

void
pcie6509_set_value()
{
    int     value;
    char    *name;
    int     ret_parm;

    if(Table_Selection(set_value, "Set Register Value", &ret_parm,
                       &name, NULL ) == 0)
        return;

    fprintf(stderr,"   Enter %s Value ->", name);

    if(user_input(flush,&cp) == 0)  /* if break received, skip */
        return;

    value = strtoul(cp,NULL,0);

    if((status=PCIE6509_Set_Value(MyHandle, ret_parm, value))) {
        Display_Library_Error(MyHandle);
        return;
    }
}
/***********************************************************************/

void _pcie6509_set_watchdog_configuration(char *msdesc, pcie6509_wdt_config_t *ms);

/***********************************************************************/
void
pcie6509_set_watchdog_configuration()
{
    pcie6509_wdt_config_t   master;
    pcie6509_wdt_config_t   slave;

    _pcie6509_set_watchdog_configuration("Master (port0..5)", &master);
    _pcie6509_set_watchdog_configuration("Slave (port6..11)", &slave);

    if((status=PCIE6509_Set_WatchdogConfiguration(MyHandle, 
                &master, &slave))) {
        Display_Library_Error(MyHandle);
        return;
    }
}
/***********************************************************************/

void
_pcie6509_set_watchdog_configuration(char *msdesc, pcie6509_wdt_config_t *ms)
{
    fprintf(stderr,"  %d - PCIE6509_WDTC_RTSI0\n",PCIE6509_WDTC_RTSI0);
    fprintf(stderr,"  %d - PCIE6509_WDTC_RTSI1\n",PCIE6509_WDTC_RTSI1);
    fprintf(stderr,"  %d - PCIE6509_WDTC_RTSI2\n",PCIE6509_WDTC_RTSI2);
    fprintf(stderr,"  %d - PCIE6509_WDTC_RTSI3\n",PCIE6509_WDTC_RTSI3);
    fprintf(stderr,"  %d - PCIE6509_WDTC_RTSI4\n",PCIE6509_WDTC_RTSI4);
    fprintf(stderr,"  %d - PCIE6509_WDTC_RTSI5\n",PCIE6509_WDTC_RTSI5);
    fprintf(stderr,"  %d - PCIE6509_WDTC_RTSI6\n",PCIE6509_WDTC_RTSI6);
    fprintf(stderr,"  %d - PCIE6509_WDTC_RTSI7\n",PCIE6509_WDTC_RTSI7);

    fprintf(stderr,"   Enter %s External Trigger Source -> ",msdesc);

    if(user_input(flush,&cp) == 0)  /* if break received, skip */
        return;

    ms->ExtTrigSel = strtoul(cp,NULL,0);

    fprintf(stderr,"  %d - PCIE6509_WDTC_ACTIVE_HIGH\n",
                                        PCIE6509_WDTC_ACTIVE_HIGH);
    fprintf(stderr,"  %d - PCIE6509_WDTC_ACTIVE_LOW\n",
                                        PCIE6509_WDTC_ACTIVE_LOW);

    fprintf(stderr,"   Enter %s External Trigger Polarity -> ",msdesc);

    if(user_input(flush,&cp) == 0)  /* if break received, skip */
        return;

    ms->ExtTrigPol = strtoul(cp,NULL,0);

    fprintf(stderr,"  %d - PCIE6509_WDTC_EXT_DISABLED\n",
                                        PCIE6509_WDTC_EXT_DISABLED);
    fprintf(stderr,"  %d - PCIE6509_WDTC_EXT_USE_EXTERNAL_TRIGGER\n",
                                        PCIE6509_WDTC_EXT_USE_EXTERNAL_TRIGGER);

    fprintf(stderr,"   Enter %s External Trigger Enable -> ",msdesc);

    if(user_input(flush,&cp) == 0)  /* if break received, skip */
        return;

    ms->ExtTrigEn = strtoul(cp,NULL,0);

    fprintf(stderr,"  %d - PCIE6509_WDTC_INT_DISABLED\n",
                                        PCIE6509_WDTC_INT_DISABLED);
    fprintf(stderr,"  %d - PCIE6509_WDTC_INT_USE_INTERNAL_COUNTER\n",
                                        PCIE6509_WDTC_INT_USE_INTERNAL_COUNTER);

    fprintf(stderr,"   Enter %s Internal Trigger Enable -> ",msdesc);

    if(user_input(flush,&cp) == 0)  /* if break received, skip */
        return;

    ms->IntTrigEn = strtoul(cp,NULL,0);
}

/***********************************************************************/

void _pcie6509_set_watchdog_control(char *msdesc, pcie6509_wdt_control_t *ms);

/***********************************************************************/
void
pcie6509_set_watchdog_control()
{
    pcie6509_wdt_control_t   master;
    pcie6509_wdt_control_t   slave;

    _pcie6509_set_watchdog_control("Master (port0..5)", &master);
    _pcie6509_set_watchdog_control("Slave (port6..11)", &slave);

    if((status=PCIE6509_Set_WatchdogControl(MyHandle, 
                &master, &slave))) {
        Display_Library_Error(MyHandle);
        return;
    }
}
/***********************************************************************/

void
_pcie6509_set_watchdog_control(char *msdesc, pcie6509_wdt_control_t *ms)
{
    u_short control;

again:
    fprintf(stderr,"  0 - PCIE6509_WDTCON_START\n");
    fprintf(stderr,"  1 - PCIE6509_WDTCON_RESTART_FEED\n");
    fprintf(stderr,"  2 - PCIE6509_WDTCON_RESTART_FOOD\n");
    fprintf(stderr,"  3 - PCIE6509_WDTCON_RESTART_FEED_FOOD\n");
    fprintf(stderr,"  4 - PCIE6509_WDTCON_PAUSE\n");
    fprintf(stderr,"  5 - PCIE6509_WDTCON_TERMINATE\n");
    fprintf(stderr,"  6 - PCIE6509_WDTCON_RESTART_ACED\n");

    fprintf(stderr,"   Enter %s Watchdog Control -> ",msdesc);

    if(user_input(flush,&cp) == 0)  /* if break received, skip */
        return;

    control = strtoul(cp,NULL,0);

    switch(control) {
        case 0: *ms = PCIE6509_WDTCON_START; break;
        case 1: *ms = PCIE6509_WDTCON_RESTART_FEED; break;
        case 2: *ms = PCIE6509_WDTCON_RESTART_FOOD; break;
        case 3: *ms = PCIE6509_WDTCON_RESTART_FEED_FOOD; break;
        case 4: *ms = PCIE6509_WDTCON_PAUSE; break;
        case 5: *ms = PCIE6509_WDTCON_TERMINATE; break;
        case 6: *ms = PCIE6509_WDTCON_RESTART_ACED; break;
        default:
            fprintf(stderr,"Invalid Selection <%d>\n",control);
            goto again;
        break;
    }
}
/***********************************************************************/

/***********************************************************************/
void
pcie6509_set_watchdog_mode()
{
    u_short                     port;
    u_char                      line_mask;
    pcie6509_watchdog_mode_t    mode;

    fprintf(stderr,"   Enter Port (0..%d) ->", PCIE6509_MAX_PORTS-1);

    if(user_input(flush,&cp) == 0)  /* if break received, skip */
        return;

    port = strtoul(cp,NULL,0);

    fprintf(stderr,"   Enter Line Mask (0..0x%02x) ->",PCIE6509_ALL_LINES_MASK);

    if(user_input(flush,&cp) == 0)  /* if break received, skip */
        return;

    line_mask = strtoul(cp,NULL,16);

    fprintf(stderr,"  %d - PCIE6509_WDT_MODE_DISABLE\n",PCIE6509_WDT_MODE_DISABLE);
    fprintf(stderr,"  %d - PCIE6509_WDT_MODE_FREEZE\n",PCIE6509_WDT_MODE_FREEZE);
    fprintf(stderr,"  %d - PCIE6509_WDT_MODE_TRISTATE\n",PCIE6509_WDT_MODE_TRISTATE);
    fprintf(stderr,"  %d - PCIE6509_WDT_MODE_SAFE_VALUE\n",PCIE6509_WDT_MODE_SAFE_VALUE);

    fprintf(stderr,"   Enter Watchdog Mode -> ");

    if(user_input(flush,&cp) == 0)  /* if break received, skip */
        return;

    mode = strtoul(cp,NULL,0);

    if((status=PCIE6509_Set_WatchdogMode(MyHandle, port, line_mask, mode))) {
        Display_Library_Error(MyHandle);
        return;
    }
}
/***********************************************************************/

/***********************************************************************/
void
pcie6509_set_watchdog_mode_pmask()
{
    pcie6509_io_port_t          iop;
    u_short                     port;
    pcie6509_watchdog_mode_t    mode;

    bzero(&iop,sizeof(iop));

    fprintf(stderr,"  %d - PCIE6509_WDT_MODE_DISABLE\n",PCIE6509_WDT_MODE_DISABLE);
    fprintf(stderr,"  %d - PCIE6509_WDT_MODE_FREEZE\n",PCIE6509_WDT_MODE_FREEZE);
    fprintf(stderr,"  %d - PCIE6509_WDT_MODE_TRISTATE\n",PCIE6509_WDT_MODE_TRISTATE);
    fprintf(stderr,"  %d - PCIE6509_WDT_MODE_SAFE_VALUE\n",PCIE6509_WDT_MODE_SAFE_VALUE);

    fprintf(stderr,"   Enter Watchdog Mode -> ");

    if(user_input(flush,&cp) == 0)  /* if break received, skip */
        return;

    mode = strtoul(cp,NULL,0);

    fprintf(stderr,"   Enter Port Mask (0..0x%03x) ->", PCIE6509_ALL_PORTS_MASK);

    if(user_input(flush,&cp) == 0)  /* if break received, skip */
        return;

    iop.port_mask = strtoul(cp,NULL,16);

    for(port=0; port < PCIE6509_MAX_PORTS; port++) {
        if(PCIE6509_PORT_GET(iop.port_mask,port)) {
            fprintf(stderr,"   Enter Port%02d, Line Mask (0..0x%02x) ->",port,PCIE6509_ALL_LINES_MASK);

            if(user_input(flush,&cp) == 0)  /* if break received, skip */
                return;

            iop.line_mask[port] = strtoul(cp,NULL,16);
        }
    }

    if((status=PCIE6509_Set_WatchdogModePmask(MyHandle, &iop, mode))) {
        Display_Library_Error(MyHandle);
        return;
    }
}
/***********************************************************************/

/***********************************************************************/
void
pcie6509_set_watchdog_safe_state()
{
    u_short                     port;
    u_char                      line_mask;

    fprintf(stderr,"   Enter Port (0..%d) ->", PCIE6509_MAX_PORTS-1);

    if(user_input(flush,&cp) == 0)  /* if break received, skip */
        return;

    port = strtoul(cp,NULL,0);

    fprintf(stderr,"   Enter Line Mask (0..0x%02x) ->",PCIE6509_ALL_LINES_MASK);

    if(user_input(flush,&cp) == 0)  /* if break received, skip */
        return;

    line_mask = strtoul(cp,NULL,16);

    if((status=PCIE6509_Set_WatchdogSafeState(MyHandle, port, line_mask))) {
        Display_Library_Error(MyHandle);
        return;
    }
}
/***********************************************************************/

/***********************************************************************/
void
pcie6509_set_watchdog_safe_state_pmask()
{
    pcie6509_io_port_t  iop;
    u_short             port;

    bzero(&iop,sizeof(iop));

    fprintf(stderr,"   Enter Port Mask (0..0x%03x) ->", PCIE6509_ALL_PORTS_MASK);

    if(user_input(flush,&cp) == 0)  /* if break received, skip */
        return;

    iop.port_mask = strtoul(cp,NULL,16);

    for(port=0; port < PCIE6509_MAX_PORTS; port++) {
        if(PCIE6509_PORT_GET(iop.port_mask,port)) {
            fprintf(stderr,"   Enter Port%02d, Line Mask (0..0x%02x) ->",port,PCIE6509_ALL_LINES_MASK);

            if(user_input(flush,&cp) == 0)  /* if break received, skip */
                return;

            iop.line_mask[port] = strtoul(cp,NULL,16);
        }
    }

    if((status=PCIE6509_Set_WatchdogSafeStatePmask(MyHandle, &iop))) {
        Display_Library_Error(MyHandle);
        return;
    }
}
/***********************************************************************/

void _pcie6509_set_watchdog_timeout(char *ms, double *tout, 
                                    u_int *raw_tout, int format);
/***********************************************************************/
void
pcie6509_set_watchdog_timeout()
{
    int     format;
    double  master_timeout;
    double  slave_timeout;
    u_int   raw_master_timeout;
    u_int   raw_slave_timeout;

again:
    fprintf(stderr,"   0 - Timeout in Milli-Seconds\n");
    fprintf(stderr,"   1 - Timeout Raw)\n");
    fprintf(stderr,"Timeout Format? ->");

    if(user_input(flush,&cp) == 0)  /* if break received, skip */
        return;

    format = strtoul(cp,NULL,0);

    if((format < 0) || (format > 1)) {
        fprintf(stderr,"Error!!! Invalid Format '%d'\n",format);
        goto again;
    }

    _pcie6509_set_watchdog_timeout("Master (port0..5)", &master_timeout,
                                    &raw_master_timeout, format);

    _pcie6509_set_watchdog_timeout("Slave (port6..11)", &slave_timeout,
                                    &raw_slave_timeout, format);

    switch(format) {
        case 0: /* milli-seconds */
            if((status=PCIE6509_Set_WatchdogTimeout(MyHandle, 
                        &master_timeout, &slave_timeout, NULL, NULL))) {
                Display_Library_Error(MyHandle);
                return;
            }
        break;
        case 1: /* raw */
            if((status=PCIE6509_Set_WatchdogTimeout(MyHandle, 
                        NULL, NULL,&raw_master_timeout, &raw_slave_timeout))) {
                Display_Library_Error(MyHandle);
                return;
            }
        break;
    }
}
/***********************************************************************/

void
_pcie6509_set_watchdog_timeout(char *ms, double *tout, u_int *raw_tout, int format)
{
    fprintf(stderr,"%s....\n",ms);

    switch(format) {
        case 0: /* milli-seconds */
            fprintf(stderr,"   Timeout (milli-seconds)? ->");
            if(user_input(flush,&cp) == 0)  /* if break received, skip */
                return;
            *tout = strtod(cp, NULL);
        break;

        case 1: /* raw */
            fprintf(stderr,"   Timeout (raw - hex)? ->");
            if(user_input(flush,&cp) == 0)  /* if break received, skip */
                return;
            *raw_tout = strtoul(cp, NULL, 16);
        break;
    }
}

/***********************************************************************/
void
pcie6509_write_operation()
{
    pcie6509_io_port_t  iop;
    int                 bytes_written;
    u_short             port;
    int                 error;

    bzero(&iop,sizeof(iop));

    fprintf(stderr,"   Enter Port Mask (0..0x%03x) ->", PCIE6509_ALL_PORTS_MASK);

    if(user_input(flush,&cp) == 0)  /* if break received, skip */
        return;

    iop.port_mask = strtoul(cp,NULL,16);

    /* Set selected ports to output */
    for(port=0; port < PCIE6509_MAX_PORTS; port++) {
        if(PCIE6509_PORT_GET(iop.port_mask,port)) {
            iop.line_mask[port] = 0xFF;     /* set all lines to output */
        }
    }

    if((status=PCIE6509_Set_PortDirectionPmask(MyHandle, &iop))) {
        Display_Library_Error(MyHandle);
        return;
    }

    /* clear line mask */
    bzero(&iop.line_mask,sizeof(iop.line_mask));

    for(port=0; port < PCIE6509_MAX_PORTS; port++) {
        if(PCIE6509_PORT_GET(iop.port_mask,port)) {
            fprintf(stderr,"   Enter Port%02d, Line Mask (0..0x%02x) ->",port,PCIE6509_ALL_LINES_MASK);

            if(user_input(flush,&cp) == 0)  /* if break received, skip */
                return;

            iop.line_mask[port] = strtoul(cp,NULL,16);
        }
    }

    if((status=PCIE6509_Write(MyHandle, &iop, sizeof(iop), &bytes_written, 
                                                            &error))) {
        fprintf(stderr,"Write Failed: %s\n",strerror(error));
        Display_Library_Error(MyHandle);
        return;
    }

    fprintf(stderr,"   Bytes written = %d\n",bytes_written);
}
