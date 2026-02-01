// $URL: http://subversion:8080/svn/gsc/trunk/drivers/gsc_common/driver/linux/os_metrics.c $
// $Rev: 51748 $
// $Date: 2022-10-12 10:14:48 -0500 (Wed, 12 Oct 2022) $

// Linux: Device Driver: source file: This software is covered by the GNU GENERAL PUBLIC LICENSE (GPL).

#include "main.h"



// macros *********************************************************************

#if (DEV_METRICS_SHOW)

	#ifndef	OS_METRICS_MX_LOOPS
	#define	OS_METRICS_MX_LOOPS		5000000
	#endif

	#ifndef	OS_METRICS_RX_LOOPS
	#define	OS_METRICS_RX_LOOPS		5000000
	#endif

	#ifndef	OS_METRICS_TX_LOOPS
	#define	OS_METRICS_TX_LOOPS		20000000
	#endif

#endif



//*****************************************************************************
#if (DEV_METRICS_SHOW)
static void _reg_mx_metrics(dev_data_t* dev, VADDR_T va)
{
	struct timespec	t1;
	struct timespec	t2;
	int				i;
	s64				ns;
	s64				rate;

	getnstimeofday(&t1);

	for (i = 0; i < OS_METRICS_MX_LOOPS; i++)
		os_reg_mem_mx_u32(NULL, va, 0x0, 0x0);

	getnstimeofday(&t2);

	ns		= ((s64) t2.tv_sec * 1000000000LL + t2.tv_nsec)
			- ((s64) t1.tv_sec * 1000000000LL + t1.tv_nsec);
	rate	= ns * 1000 / i;
	printf(	"%s.%d: Reg Mx, %8i iterations, %11lld ns, %6ld.%03ld ns/iteration\n",
			DEV_NAME,
			dev->board_index,
			i,
			(long long) ns,
			(long) (rate / 1000),
			(long) (rate % 1000));
}
#endif



//*****************************************************************************
#if (DEV_METRICS_SHOW)
static void _reg_rx_metrics(dev_data_t* dev, VADDR_T va)
{
	struct timespec	t1;
	struct timespec	t2;
	int				i;
	s64				ns;
	s64				rate;

	getnstimeofday(&t1);

	for (i = 0; i < OS_METRICS_RX_LOOPS; i++)
		os_reg_mem_rx_u32(NULL, va);

	getnstimeofday(&t2);

	ns		= ((s64) t2.tv_sec * 1000000000LL + t2.tv_nsec)
			- ((s64) t1.tv_sec * 1000000000LL + t1.tv_nsec);
	rate	= ns * 1000 / i;
	printf(	"%s.%d: Reg Rx, %8i iterations, %11lld ns, %6ld.%03ld ns/iteration\n",
			DEV_NAME,
			dev->board_index,
			i,
			(long long) ns,
			(long) (rate / 1000),
			(long) (rate % 1000));
}
#endif



//*****************************************************************************
#if (DEV_METRICS_SHOW)
static void _reg_tx_metrics(dev_data_t* dev, VADDR_T va)
{
	struct timespec	t1;
	struct timespec	t2;
	int				i;
	s64				ns;
	s64				rate;

	getnstimeofday(&t1);

	for (i = 0; i < 20000000; i++)
		os_reg_mem_tx_u32(NULL, va, 0x0);

	getnstimeofday(&t2);

	ns		= ((s64) t2.tv_sec * 1000000000LL + t2.tv_nsec)
			- ((s64) t1.tv_sec * 1000000000LL + t1.tv_nsec);
	rate	= ns * 1000 / i;
	printf(	"%s.%d: Reg Tx, %8i iterations, %11lld ns, %6ld.%03ld ns/iteration\n",
			DEV_NAME,
			dev->board_index,
			i,
			(long long) ns,
			(long) (rate / 1000),
			(long) (rate % 1000));
}
#endif



//*****************************************************************************
void os_metrics(dev_data_t* dev, VADDR_T va)
{
#if (DEV_METRICS_SHOW)
	_reg_rx_metrics(dev, va);
	_reg_tx_metrics(dev, va);
	_reg_mx_metrics(dev, va);
#endif
}


