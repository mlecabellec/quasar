/*===========================================================================*
 * FILE:                       U S B _ S R V R . C
 *===========================================================================*
 *
 * COPYRIGHT (C) 2014 BY
 *          ABACO SYSTEMS, INC., GOLETA, CALIFORNIA
 *          ALL RIGHTS RESERVED.
 *
 *          THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND
 *          COPIED ONLY IN ACCORDANCE WITH THE TERMS OF SUCH LICENSE AND WITH
 *          THE INCLUSION OF THE ABOVE COPYRIGHT NOTICE.  THIS SOFTWARE OR ANY
 *          OTHER COPIES THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE
 *          AVAILABLE TO ANY OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF THE
 *          SOFTWARE IS HEREBY TRANSFERRED.
 *
 *          THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE WITHOUT
 *          NOTICE AND SHOULD NOT BE CONSTRUED AS A COMMITMENT BY ABACO SYSTEMS 
 *
 *============================================================================*
 *
 * FUNCTION:  example for using the USB server functionality  
 *
 *===========================================================================*/

/* $Revision:  1.00 Release $
   Date        Revision
  ----------   ---------------------------------------------------------------
  08/28/2014   initial release
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>


// this will enable the server to run in the background
#define FORK_PROCESS

int usb_srvr_init(int dev, int opt);
int usb_srvr_close(int dev);
void sig_handler(int sig);

int dev=0, opt=0, state=0;


int main(int argc, char **argv) {
  int status=0;
 #ifdef FORK_PROCESS
  pid_t pid;
 #endif

  // get options
  if(argc >= 2)
    dev = atoi(argv[1]);
  if(argc >= 3)
    opt = atoi(argv[2]);

  printf("USB_SERVER:  starting ...\n");

 #ifdef FORK_PROCESS
  if((status = fork()) < 0)
    return 1;
  else if(status > 0)
    return 0;  // parent has exited

  // create a new process group
  if((pid = setsid()) == -1) {
    printf("USB_SERVER:  errno - %d (%s)\n", errno, strerror(errno));
    return 1;
  }

  // set the file/directory permissions
  umask(077);
 #endif

  // configure the following POSIX signals to handle
  signal(SIGINT, sig_handler);
  signal(SIGTERM, sig_handler);
  signal(SIGUSR1, sig_handler);
  signal(SIGUSR2, sig_handler);

  // this could take some time to program the USB device if is has not already been programmed
  if((status = usb_srvr_init(dev, opt)) != 0)
    return -1;
  state = 1;

  printf("\nUSB_SERVER:  server operational\n");

  while(state) {
    sleep(30);
  }

  printf("\nUSB_SERVER:  server closed\n");

  return 0;
}

// POSIX signal handler
void sig_handler(int sig) {
  int status=0;
  sigset_t sigset;

  sigemptyset(&sigset);
  sigaddset(&sigset, SIGUSR1);
  sigprocmask(SIG_BLOCK, &sigset, NULL);

  switch(sig) {
  case SIGINT:
  case SIGTERM:
    printf("\nUSB_SERVER:  server closing from user request. This could take some time to close...\n");
    if((status = usb_srvr_close(dev)) != 0)
      printf("USB_SERVER:  close failed. status - %d\n", status);
    state = 0;
    break;
  case SIGUSR1:
    printf("\nUSB_SERVER:  critical error detected from the server, closing.\n");
    if((status = usb_srvr_close(dev)) != 0)
      printf("USB_SERVER:  close failed. status - %d\n", status);
    state = 0;
    break;
  case SIGUSR2:
    printf("\nUSB_SERVER:  restarting server from user request. This could take some time to restart...\n");
    if(usb_srvr_close(dev) != 0) {
      printf("USB_SERVER:  close failed. status - %d\n", status);
      state = 0;
    }
    // small delay to let the USB interface sync
    usleep(10000);
    if((status = usb_srvr_init(dev, opt)) != 0) {
      printf("USB_SERVER:  start failed. status - %d\n", status);
      state = 0;
    }
    printf("USB_SERVER:  server operational\n");
    break;
  };

  sigprocmask(SIG_UNBLOCK, &sigset, NULL);
}

