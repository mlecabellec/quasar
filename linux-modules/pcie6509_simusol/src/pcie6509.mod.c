#define INCLUDE_VERMAGIC
#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
 .name = KBUILD_MODNAME,
 .init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
 .exit = cleanup_module,
#endif
 .arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0x206161c5, "struct_module" },
	{ 0xd6ee688f, "vmalloc" },
	{ 0xa5423cc4, "param_get_int" },
	{ 0xd0d8621b, "strlen" },
	{ 0x753e5d99, "pci_disable_device" },
	{ 0x973873ab, "_spin_lock" },
	{ 0x999e8297, "vfree" },
	{ 0xcb32da10, "param_set_int" },
	{ 0xb72397d5, "printk" },
	{ 0x2da418b5, "copy_to_user" },
	{ 0xa7046549, "vprintk" },
	{ 0x435b566d, "_spin_unlock" },
	{ 0x61651be, "strcat" },
	{ 0x42c8de35, "ioremap_nocache" },
	{ 0x179868c7, "pci_bus_read_config_dword" },
	{ 0x42d7e03a, "register_chrdev" },
	{ 0xedc03953, "iounmap" },
	{ 0x9ef749e2, "unregister_chrdev" },
	{ 0x46ef7288, "pci_get_device" },
	{ 0x818914dd, "pci_enable_device" },
	{ 0xf2a644fb, "copy_from_user" },
	{ 0xe914e41e, "strcpy" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "07C7CEFAD55B5D5FB3CAF2C");
