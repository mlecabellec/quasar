// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/OPTO16X16/OPTO16X16_Linux_1.x.x.x_GSC_DN/samples/regs/main.h $
// $Rev: 53723 $
// $Date: 2023-09-14 10:40:07 -0500 (Thu, 14 Sep 2023) $

// OPTO16X16: Sample Application: header file

#ifndef	__MAIN_H__
#define	__MAIN_H__

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "opto16x16_main.h"



// macros *********************************************************************

#define	SIZEOF_ARRAY(a)			(sizeof((a)) / sizeof((a)[0]))



// data types *****************************************************************

typedef struct
{
	// Application Settings

	s32	index;			// device index
	int	qty;			// Number of devices detected.

	int	fd;				// File descriptor for device to access.

	// Device Settings

} args_t;

typedef struct
{
	const char*	name;				// NULL terminates list.
	void		(*func)(int fd);	// NULL terminates list.
} menu_item_t;

typedef struct
{
	const char*			title;
	const menu_item_t*	list;
} menu_t;



// prototypes *****************************************************************

void	menu_call(int fd, const menu_t* menu);
int		menu_select(const menu_t* menu);

int		perform_tests(const args_t* args);

void	reg_mod_by_name(int fd);
void	reg_mod_by_offset(int fd);



#endif
