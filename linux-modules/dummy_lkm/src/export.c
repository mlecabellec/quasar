#include "export.h"
#include <linux/kernel.h>
#include <linux/module.h>

int dummy_export_service(int arg) {
	printk("dummy exported service [arg=%d]\n",arg);
	return arg*2;
}

EXPORT_SYMBOL(dummy_export_service);
