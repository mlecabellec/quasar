// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/OPTO16X16/OPTO16X16_Linux_1.x.x.x_GSC_DN/samples/id/perform.c $
// $Rev: 53722 $
// $Date: 2023-09-14 10:39:54 -0500 (Thu, 14 Sep 2023) $

// OPTO16X16: Sample Application: source file

#include "main.h"



//*****************************************************************************
static void _debounce_resolution(void)
{
	gsc_label("Debounce Resolution");
	printf("200ns\n");
}



//*****************************************************************************
static void _input_qty(void)
{
	gsc_label("Isolated Inputs");
	printf("16\n");
}



//*****************************************************************************
static void _output_qty(void)
{
	gsc_label("Isolated Outputs");
	printf("16\n");
}



//*****************************************************************************
static int _id_device_mailbox(const args_t* args)
{
	int	errs	= 0;
	u32	field;
	u32	mbox;

	gsc_label("PLD Revision Level");
	errs	+= opto16x16_reg_read(args->fd, -1, 0, GSC_PLX_9056_MBOX0, &mbox);
	field	= mbox & 0xFFFF;
	printf("0x%04lX\n", (long) field);

	gsc_label("EEPROM Revision Level");
	field	= mbox >> 16;
	printf("0x%04lX\n", (long) field);

	gsc_label("Board Assy Revision");
	errs	+= opto16x16_reg_read(args->fd, -1, 0, GSC_PLX_9056_MBOX1, &mbox);
	field	= mbox >> 16;
	printf("0x%04lX\n", (long) field);

	return(errs);
}



//*****************************************************************************
static int _register_map(const args_t* args)
{
	int	errs;

	printf("\n");
	errs	= opto16x16_reg_list(args->fd, args->detail);
	return(errs);
}



//*****************************************************************************
int perform_tests(const args_t* args)
{
	int	errs	= 0;
	int	i;

	gsc_label("Device Features");
	printf("\n");

	gsc_label_level_inc();

	_debounce_resolution();
	_input_qty();
	_output_qty();

	for (i = 0; i < OPTO16X16_IOCTL_QUERY_LAST; i++)
		errs	+= opto16x16_query(args->fd, -1, 1, i, NULL);

	gsc_label_level_dec();

	errs	+= _id_device_mailbox(args);
	errs	+= _register_map(args);
	printf("\n");

	return(errs);
}


