// $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/OPTO16X16/OPTO16X16_Linux_1.x.x.x_GSC_DN/samples/regs/menu.c $
// $Rev: 53723 $
// $Date: 2023-09-14 10:40:07 -0500 (Thu, 14 Sep 2023) $

// OPTO16X16: Sample Application: source file

#include "main.h"



//*****************************************************************************
static int _get_char(void)
{
	char	buf[1024];
	int		c;

	memset(buf, 0, sizeof(buf));
	fgets(buf, sizeof(buf), stdin);
	c	= buf[0];
	return(c);
}



//*****************************************************************************
static int _make_selection(const menu_t* menu, int qty)
{
	int	c;
	int	i;

	for (;;)
	{
		printf("    Make a menu selection from 'A' to '%c': ", 'A' + qty - 1);
		c	= _get_char();

		if ((c >= 'A') && (c <= ('A' + qty - 1)))
		{
			i	= c - 'A';
			break;
		}

		if ((c >= 'a') && (c <= ('a' + qty - 1)))
		{
			i	= c - 'a';
			break;
		}
	}

	return(i);
}



//*****************************************************************************
static int _menu_show(const menu_t* menu, int done)
{
	int	i;
	int	len	= 0;
	int	tmp;

	printf("\n");

	for (i = 0; i <= 25; i++)
	{
		if (menu->list[i].name == NULL)
			break;

		tmp	= strlen(menu->list[i].name);

		if (len < tmp)
			len	= tmp;
	}

	for (i = 0; i <= 25; i++)
	{
		if (menu->list[i].name == NULL)
			break;

		printf("    %c. %-*s\n", 'A' + i, len, menu->list[i].name);
	}

	if (done)
	{
		printf("    %c. %s\n", 'A' + i, "Done");
		i++;
	}

	printf("\n");
	return(i);
}



//*****************************************************************************
void menu_call(int fd, const menu_t* menu)
{
	int			i;
	const char*	psz;
	int			qty;
	struct tm*	stm;
	time_t		tt;

	for (;;)
	{
		time(&tt);
		stm	= localtime(&tt);
		psz	= asctime(stm);
		printf("\n");
		printf("  %s          %s", menu->title, psz);
		qty	= _menu_show(menu, 1);
		i	= _make_selection(menu, qty);

		if (i >= (qty - 1))
		{
			printf("    -> %c. %s\n", 'A' + i, "Done");
			break;
		}

		printf("    -> %c. %s\n\n", 'A' + i, menu->list[i].name);

		(menu->list[i].func)(fd);
		printf("\n");
		printf("  Press return to continue: ");
		_get_char();
	}
}



//*****************************************************************************
int menu_select(const menu_t* menu)
{
	int			i;
	const char*	psz;
	int			qty;
	struct tm*	stm;
	time_t		tt;

	time(&tt);
	stm	= localtime(&tt);
	psz	= asctime(stm);
	printf("\n");
	printf("  %s          %s", menu->title, psz);
	qty	= _menu_show(menu, 0);
	i	= _make_selection(menu, qty);

	printf("    -> %c. %s\n\n", 'A' + i, menu->list[i].name);
	return(i);
}



