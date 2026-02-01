#ifdef NIOS


#ifndef __KERNEL__
#  define __KERNEL__
#endif

#ifndef MODULE
#  define MODULE
#endif

#ifdef CONFIG_SMP
#define __SMP__
#endif

#include <linux/device.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/slab.h>
#include <linux/time.h>
#include <linux/interrupt.h>
#include <linux/cdev.h>
#include <linux/fs.h>

#include "kernel_compat.h"
#include "ddtsync.h"
#include "ddtpro.h"
#include "tsyncpci.h"
#include "tpro_func.h"
#include "os_wait.h"
#include "tsync_gpio_stamp_queue.h"
#include "tsync_hw_func.h"
#include "tsync_pps.h"
#include "tsync_ptp.h"
#include "tsync_ptp_host_reference.h"

#define VERSION		   "3.20"
#define TSYNC_BASE_ADDR (0)

char tsyncpci_copyright[]	  = TSYNC_PCI_DRV_COPYRIGHT;

static tsyncpci_dev_t* tsyncpci_find_device (struct inode *inode);
static void tsync_cleanup(void);
static int  tsync_close(struct inode *inode, struct file *filp);
static int  tsyncpci_init(void);
static int  tsync_probe(struct platform_device *pdev);
static long tsyncpci_ioctl(struct file   *filp, unsigned int cmd, unsigned long arg);
static int  tsyncpci_open(struct inode *inode, struct file *filp);
static int  tsync_remove(struct platform_device *pdev);

static struct of_device_id tsync_match[] =
{
	{ .compatible = "spectracom,tsync-nios" },
	{},
};
MODULE_DEVICE_TABLE(of, tsync_match);

static struct platform_driver tsyncpci_driver =
{
	.probe  = tsync_probe,
	.remove = tsync_remove,
	.driver = {
		.name			= TSYNC_PCI_DRV_NAME,
		.of_match_table = tsync_match,
		.owner			= THIS_MODULE,
	},
};

static struct file_operations tsyncpci_fops =
{
	.owner			= THIS_MODULE,
	.unlocked_ioctl = tsyncpci_ioctl,
	.open			= tsyncpci_open,
	.release		= tsync_close,
};

static struct class *tsyncpci_cs;
static struct device *tsyncpci_cd[TSYNC_PCI_MAX_NUM];
static struct cdev tsyncpci_cdev;
static dev_t tsyncpci_devNum;
static tsyncpci_dev_t *tsyncpci_devp[TSYNC_PCI_MAX_NUM];
static char NameStr[TSYNC_PCI_MAX_NUM][32];

extern FuncTable_t tsyncpci_tsync;


static void tsync_cleanup(void)
{
	platform_driver_unregister(&tsyncpci_driver);
	cdev_del(&tsyncpci_cdev);
	unregister_chrdev_region(tsyncpci_devNum, TSYNC_PCI_MAX_NUM);
	class_destroy(tsyncpci_cs);
}

static int tsync_close(struct inode *inode, struct file *filp)
{
	tsyncpci_user_t *user = NULL;
	tsyncpci_dev_t *hw = NULL;
	int status = 0;

	user = (tsyncpci_user_t *)(filp->private_data);

	if (!user) {
		status = -EFAULT;
		goto fail_exit;
	}

	hw = user->pDevice;
	if (!hw) {
		status = -EFAULT;
	}

	TPRO_SPIN_LOCK(&hw->userLock);

	if (user == hw->pFirstUser)
	{
		if (user == hw->pLastUser) {
			hw->pFirstUser = NULL;
			hw->pLastUser = NULL;
		}
		else {
			user->pPrev->pNext = hw->pFirstUser = user->pNext;
			user->pNext->pPrev = user->pPrev;
		}
	}
	else if (user == hw->pLastUser) {
		user->pNext->pPrev = hw->pLastUser = user->pPrev;
		user->pPrev->pNext = user->pNext;
	}
	else {
		user->pNext->pPrev = user->pPrev;
		user->pPrev->pNext = user->pNext;
	}
	TPRO_SPIN_UNLOCK(&hw->userLock);
	kfree(user);
fail_exit:
	return status;
}

static tsyncpci_dev_t* tsyncpci_find_device(struct inode *inode)
{
	tsyncpci_dev_t *pdev = NULL;
	unsigned int idx;

	for (idx = 0; idx < TSYNC_PCI_MAX_NUM; idx++) {
		if (tsyncpci_devp[idx]) {
			if (tsyncpci_devp[idx]->devNum == inode->i_rdev) {
				pdev = tsyncpci_devp[idx];
			}
		}
	}
	return pdev;
}

static int tsyncpci_init(void)
{
	int ret;

	tsyncpci_cs = TSYNC_CLASS_CREATE(THIS_MODULE, TSYNC_PCI_DRV_NAME);

	if (IS_ERR(tsyncpci_cs)) {
		pr_err("Error creating sysfs class.\n");
		return PTR_ERR(tsyncpci_cs);
	}

	ret = alloc_chrdev_region(&tsyncpci_devNum, TSYNC_PCI_BASE_MINOR, TSYNC_PCI_MAX_NUM, TSYNC_PCI_DRV_NAME);

	if (ret) {
		pr_err("Failed to allocate char dev region\n");
		class_destroy(tsyncpci_cs);
		return ret;
	}

	cdev_init(&tsyncpci_cdev, &tsyncpci_fops);

	tsyncpci_cdev.owner = THIS_MODULE;
	tsyncpci_cdev.ops   = &tsyncpci_fops;

	ret = cdev_add(&tsyncpci_cdev, tsyncpci_devNum, TSYNC_PCI_MAX_NUM);

	if (ret) {
		pr_err("Unable to register a char device, err=%d", ret);
		unregister_chrdev_region(tsyncpci_devNum, TSYNC_PCI_MAX_NUM);
		class_destroy(tsyncpci_cs);
		return ret;
	}

	ret = platform_driver_register(&tsyncpci_driver);
	if (ret) {
		pr_err("Unable to register a device, err=%d", ret);
	}

	return ret;
}

static irqreturn_t tsync_interrupt_handler(int irq, void *dev_id)
{
	tsyncpci_dev_t *hw = (tsyncpci_dev_t *) dev_id;
	uint16_t status;
	int irq_count;
	struct timespec ts = {0};

	const int pps_irq = 0;
	const int timestamp_irq_idx = 10;
	const int heartbeat_irq_idx = 11;
	const int match_irq_idx = 12;
	getnstimeofday(&ts);

	status = tsync_read_interrupt_status(hw);

	for (irq_count = 0; irq_count < TSYNC_INTERRUPT_COUNT; irq_count++) {
		uint16_t intMask = 1 << irq_count;
		if (status & intMask) {

			if (irq_count == pps_irq) {
				tsync_pps_isr_handler((int)hw->slotPosition);
			}

			hw->intCounter[irq_count]++;
			hw->intTime[irq_count].seconds = ts.tv_sec;
			hw->intTime[irq_count].ns	  = ts.tv_nsec;

			if (atomic_read(&hw->tsyncInterrupts[irq_count].flag) == 0) {
				atomic_inc(&hw->tsyncInterrupts[irq_count].flag);
				os_waitPost(&hw->tsyncInterrupts[irq_count].waitQueue);
			}

			if (irq_count == heartbeat_irq_idx) {
				if (atomic_read(&hw->heartFlag) == 0) {
					atomic_inc(&hw->heartFlag);
					os_waitPost(&hw->heart_wait);
				}
			}
			else if (irq_count == match_irq_idx) {
				if (atomic_read(&hw->matchFlag) == 0) {
					atomic_inc(&hw->matchFlag);
					os_waitPost(&hw->match_wait);
				}
			}
			else if (irq_count == timestamp_irq_idx) {
				hw->eventCnt++;

				gpioQueueDrainBoardFIFO(hw);

				if (atomic_read(&hw->eventFlag) == 0) {
					atomic_inc(&hw->eventFlag);
					os_waitPost(&hw->event_wait);
				}
			}

		}
	}

	hw->intCnt++;
	return IRQ_RETVAL(status);

}
static int tsync_probe(struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;
	tsyncpci_dev_t *new;
	unsigned long irq_flags = 0;
	int ret, idx = 0;
	struct resource res;

	for (idx = 0; idx < ARRAY_SIZE(tsyncpci_devp); idx++) {
		if (!tsyncpci_devp[idx]) {
			break;
		}
	}

	if (idx == ARRAY_SIZE(tsyncpci_devp)) {
		printk(KERN_ERR "%s: no more available devices in array\n",
		       __func__);
		return -EBUSY;
	}

	new = kzalloc(sizeof(tsyncpci_dev_t), GFP_KERNEL);

	if (!new) {
		printk(KERN_ERR "Can't allocate memory for driver\n");
		return -ENOMEM;
	}

	TPRO_MUTEX_INIT(&new->deviceLock);
	TPRO_SPIN_LOCK_INIT(&new->userLock);

	ret = of_address_to_resource(np, TSYNC_BASE_ADDR, &res);
	if (ret) {
		printk(KERN_ERR "Can't get registers address\n");
		goto error_free;
	}

	if (!request_mem_region(res.start, resource_size(&res), "tsyncpci")) {
		printk(KERN_ERR "Unable to obtain I/O memory address 0x%08llX\n", (long long)res.start);
		ret = -EBUSY;
		goto error_free;
	}

	new->ioAddr = (unsigned long) ioremap(res.start, resource_size(&res));
	new->device  = pdev->id;
	new->irq  = irq_of_parse_and_map(np, 0);

	new->devNum  = MKDEV(MAJOR(tsyncpci_devNum), idx);

	sprintf(NameStr[idx], "tsyncpci%d", idx);

	printk(KERN_INFO "TSYNC NIOS version of board: %s\n", NameStr[idx]);
	new->pFunctionTable = &tsyncpci_tsync;

	tsyncpci_cd[idx] = device_create(tsyncpci_cs, NULL,
					 MKDEV(MAJOR(tsyncpci_devNum), idx),
					 NULL, NameStr[idx]);
	if (IS_ERR(tsyncpci_cd[idx])) {
		printk(KERN_ERR "Device creation failed\n");
		ret = PTR_ERR_OR_ZERO(tsyncpci_cd[idx]);
		goto  error_dev_unregister;
	}

	sprintf(NameStr[idx], "tsyncpci%d", MINOR(new->devNum));
	irq_flags |= IRQF_SHARED;

	ret = request_irq(new->irq, tsync_interrupt_handler, irq_flags,
			  NameStr[idx],new);

	if (ret < 0) {
		printk(KERN_ERR "Failed to request IRQ \n");
		ret = -EIO;
		goto  error_release_irq;
	}

	ret = new->pFunctionTable->InitializeBoard(new);
	if (ret) {
		ret = -EIO;
		printk(KERN_ERR "Board initialization failed.\n");
		goto  error_release_irq;
	}


	new->slotPosition = idx;

	ret = tsync_pps_register(new);
	if (ret) {
		printk(KERN_ERR "Failed to register PPS device.\n");
		goto error_release_irq;
	}

	ret = tsync_ptp_host_reference_register(new, &(pdev->dev));
	if (ret) {
		printk(KERN_ERR "Failed to register Host Reference PHC device.\n");
		goto error_unregister_pps;
	}

	ret = tsync_ptp_register(new);
	if(ret) {
		printk(KERN_ERR "Failed to register ptp clock.\n");
		goto error_unregister_phc;
	}

	tsyncpci_devp[idx] = new;
	return 0;

error_unregister_phc:
	tsync_ptp_host_reference_unregister(&(pdev->dev));
error_unregister_pps:
	tsync_pps_unregister(new);
error_release_irq:
	free_irq(new->irq, new);
error_dev_unregister:
	device_destroy(tsyncpci_cs, MKDEV(MAJOR(tsyncpci_devNum), idx));
	release_mem_region(res.start, resource_size(&res));
error_free:
	kfree(new);
	return ret;
}
static long tsyncpci_ioctl(struct file *filp, unsigned int  cmd, unsigned long arg) {
	tsyncpci_ioctl_cmd_t *cmdHnd = (tsyncpci_ioctl_cmd_t*)arg;
	tsyncpci_dev_t *hw = NULL;
	tsyncpci_user_t *user = NULL;
	tsyncpci_user_t *currUser = NULL;
	long status = 0;

	if (_IOC_NR(cmd) >= IOCTL_TPRO_CMD_COUNT)
	{
		status = -EFAULT;
		pr_err("Invalid ioctl command.\n");
		goto fail_exit;
	}

	user = (tsyncpci_user_t *)(filp->private_data);
	if (!user) {
		status = -EFAULT;
		TPRO_LOG_ERR;
		goto fail_exit;
	}

	hw = user->pDevice;

	if (!hw) {
		status = -EFAULT;
		goto fail_exit;
	}

	if (!TSYNC_ACCESS_OK(VERIFY_WRITE, (void*)cmdHnd, sizeof(tsyncpci_ioctl_cmd_t))) {
		status = -EFAULT;
		goto fail_exit;
	}

	if ((cmd == IOCTL_TPRO_OPEN) || (cmd == IOCTL_TSYNC_WAIT)) {
		/* These commands are always available. */
	}
	else {
		bool is_usued = false;

		TPRO_SPIN_LOCK(&hw->userLock);
		currUser = (currUser == NULL) ? user : currUser;
		is_usued = (currUser == user) ? 1 : 0;
		TPRO_SPIN_UNLOCK(&hw->userLock);
		if (!is_usued) {
			status = -EFAULT;
			pr_err("Multiuser conflict.\n");
			goto fail_exit;
		}
	}

	switch (cmd)
	{
	case IOCTL_TPRO_OPEN:
	{
		BoardObj board;

		if (!TSYNC_ACCESS_OK(VERIFY_WRITE, (void*)&cmdHnd->argVec[0], sizeof(BoardObj))) {
			status = -EFAULT;
			goto fail_exit;
		}

		if (copy_from_user(&board, &cmdHnd->argVec[0], sizeof (BoardObj))) {
			status = -EFAULT;
			goto fail_exit;
		}

		board.options = hw->options;

		if (copy_to_user(&cmdHnd->argVec[0], &board, sizeof (BoardObj))) {
			status = -EFAULT;
			goto fail_exit;
		}
		break;
	}

	case IOCTL_TPRO_GET_ALTITUDE:
	{
		unsigned char getAltResult;
		AltObj drvAlt;

		if (!TSYNC_ACCESS_OK(VERIFY_WRITE, (void*)&cmdHnd->argVec[0], sizeof(AltObj))) {
			status = -EFAULT;
			goto fail_exit;
		}

		TPRO_MUTEX_LOCK(&hw->deviceLock);
		getAltResult = hw->pFunctionTable->getAltitude(hw, &drvAlt);
		TPRO_MUTEX_UNLOCK(&hw->deviceLock);

		if (getAltResult != 0) {
			status = -EINVAL;
			goto fail_exit;
		}

		if (copy_to_user(&cmdHnd->argVec[0], &drvAlt, sizeof (AltObj))) {
			status = -EFAULT;
			goto fail_exit;
		}
		break;
	}

	case IOCTL_TPRO_GET_DATE:
	{
		DateObj drvDate;
		unsigned char getDateResult;

		if (!TSYNC_ACCESS_OK(VERIFY_WRITE, (void*)&cmdHnd->argVec[0], sizeof(DateObj))) {
			status = -EFAULT;
			goto fail_exit;
		}

		TPRO_MUTEX_LOCK(&hw->deviceLock);
		getDateResult = hw->pFunctionTable->getDate(hw, &drvDate);
		TPRO_MUTEX_UNLOCK(&hw->deviceLock);

		if (getDateResult != 0) {
			status = -EINVAL;
			goto fail_exit;
		}

		if (copy_to_user(&cmdHnd->argVec[0], &drvDate, sizeof(DateObj))) {
			status = -EFAULT;
			goto fail_exit;
		}
		break;
	}

	case IOCTL_TPRO_GET_DRIVER:
	{
		char version[7];

		if (!TSYNC_ACCESS_OK(VERIFY_WRITE, (void*)&cmdHnd->argVec[0], sizeof(version))) {
			status = -EFAULT;
			goto fail_exit;
		}

		strcpy(version, VERSION);

		if (copy_to_user(&cmdHnd->argVec[0], version, sizeof (version))) {
			status = -EFAULT;
			goto fail_exit;
		}
		break;
	}

	case IOCTL_TPRO_GET_FIRMWARE:
	{
		unsigned char firmware[5];
		unsigned char getFirmwareResult;

		if (!TSYNC_ACCESS_OK(VERIFY_WRITE, (void*)&cmdHnd->argVec[0], sizeof(firmware))) {
			status = -EFAULT;
			goto fail_exit;
		}

		TPRO_MUTEX_LOCK(&hw->deviceLock);
		getFirmwareResult = hw->pFunctionTable->getFirmware(hw, firmware);
		TPRO_MUTEX_UNLOCK(&hw->deviceLock);

		if (getFirmwareResult != 0) {
			status = -EINVAL;
			goto fail_exit;
		}

		if (copy_to_user(&cmdHnd->argVec[0], firmware, sizeof (firmware))) {
			status = -EFAULT;
			goto fail_exit;
		}
		break;
	}

	case IOCTL_TPRO_GET_LATITUDE:
	{
		LatObj drvLat;
		unsigned char getLatitudeResult;

		if (!TSYNC_ACCESS_OK(VERIFY_WRITE, (void*)&cmdHnd->argVec[0], sizeof(LatObj))) {
			status = -EFAULT;
			goto fail_exit;
		}

		TPRO_MUTEX_LOCK(&hw->deviceLock);
		getLatitudeResult = hw->pFunctionTable->getLatitude(hw, &drvLat);
		TPRO_MUTEX_UNLOCK(&hw->deviceLock);

		if (getLatitudeResult != 0) {
			status = -EINVAL;
			goto fail_exit;
		}

		/* copy object to user space */
		if (copy_to_user(&cmdHnd->argVec[0], &drvLat, sizeof (LatObj))) {
			status = -EFAULT;
			goto fail_exit;
		}
		break;
	}

	case IOCTL_TPRO_GET_LONGITUDE:
	{
		LongObj drvLong;
		unsigned char getLongitudeResult;

		if (!TSYNC_ACCESS_OK(VERIFY_WRITE, (void*)&cmdHnd->argVec[0], sizeof (LongObj))) {
			status = -EFAULT;
			goto fail_exit;
		}

		TPRO_MUTEX_LOCK(&hw->deviceLock);
		getLongitudeResult = hw->pFunctionTable->getLongitude(hw, &drvLong);
		TPRO_MUTEX_UNLOCK(&hw->deviceLock);

		if (getLongitudeResult != 0) {
			status = -EINVAL;
			goto fail_exit;
		}

		if (copy_to_user(&cmdHnd->argVec[0], &drvLong, sizeof (LongObj))) {
			status = -EFAULT;
			goto fail_exit;
		}
		break;
	}

	case IOCTL_TPRO_GET_SAT_INFO:
	{
		SatObj drvSat;
		unsigned char getSatInfoResult;

		if (!TSYNC_ACCESS_OK(VERIFY_WRITE, (void*)&cmdHnd->argVec[0], sizeof (SatObj))) {
			status = -EFAULT;
			goto fail_exit;
		}

		TPRO_MUTEX_LOCK(&hw->deviceLock);
		getSatInfoResult = hw->pFunctionTable->getSatInfo(hw, &drvSat);
		TPRO_MUTEX_UNLOCK(&hw->deviceLock);

		if (getSatInfoResult != 0) {
			status = -EINVAL;
			goto fail_exit;
		}

		if (copy_to_user(&cmdHnd->argVec[0], &drvSat, sizeof (SatObj))) {
			status = -EFAULT;
			goto fail_exit;
		}
		break;
	}

	case IOCTL_TPRO_GET_TIME:
	{
		TimeObj drvTime;

		if (!TSYNC_ACCESS_OK(VERIFY_WRITE, (void*)&cmdHnd->argVec[0], sizeof (TimeObj)))
		{
			status = -EFAULT;
			goto fail_exit;
		}

		TPRO_MUTEX_LOCK(&hw->deviceLock);
		(void)hw->pFunctionTable->getTime(hw, &drvTime);
		TPRO_MUTEX_UNLOCK(&hw->deviceLock);

		if (copy_to_user(&cmdHnd->argVec[0], &drvTime, sizeof (TimeObj))) {
			status = -EFAULT;
			goto fail_exit;
		}
		break;
	}

	case IOCTL_TPRO_RESET_FIRMWARE:
	{
		unsigned char resetResult;
		TPRO_MUTEX_LOCK(&hw->deviceLock);
		resetResult = hw->pFunctionTable->resetFirmware(hw);
		TPRO_MUTEX_UNLOCK(&hw->deviceLock);

		if (resetResult != 0) {
			status = -EINVAL;
			goto fail_exit;
		}
		break;
	}

	case IOCTL_TPRO_SET_HEARTBEAT:
	{
		HeartObj drvHeart;
		unsigned char setHeartResult;

		if (copy_from_user(&drvHeart, &cmdHnd->argVec [0], sizeof (HeartObj))) {
			status = -EFAULT;
			goto fail_exit;
		}

		TPRO_MUTEX_LOCK(&hw->deviceLock);
		setHeartResult = hw->pFunctionTable->setHeartbeat(hw, &drvHeart);
		TPRO_MUTEX_UNLOCK(&hw->deviceLock);

		if (setHeartResult != 0) {
			status = -EINVAL;
			goto fail_exit;
		}

		break;
	}

	case IOCTL_TPRO_SET_MATCHTIME:
	{
		MatchObj drvMatch;
		unsigned char setMatchTimeResult;

		if (copy_from_user(&drvMatch, &cmdHnd->argVec[0], sizeof (MatchObj))) {
			status = -EFAULT;
			goto fail_exit;
		}

		TPRO_MUTEX_LOCK(&hw->deviceLock);
		setMatchTimeResult = hw->pFunctionTable->setMatchTime(hw, &drvMatch);
		TPRO_MUTEX_UNLOCK(&hw->deviceLock);

		if (setMatchTimeResult != 0) {
			status = -EINVAL;
			goto fail_exit;
		}
		break;
	}

	case IOCTL_TPRO_SET_PROP_DELAY_CORR:
	{
		int microseconds;
		unsigned char setPropResult;

		if (!cmdHnd) {
			break;
		}

		__get_user(microseconds, (int *) &cmdHnd->argVec[0]);

		TPRO_MUTEX_LOCK(&hw->deviceLock);
		setPropResult = hw->pFunctionTable->setPropDelayCorr(hw, &microseconds);
		TPRO_MUTEX_UNLOCK(&hw->deviceLock);

		if (setPropResult != 0) {
			status = -EINVAL;
			goto fail_exit;
		}
		break;
	}

	case IOCTL_TPRO_SET_TIME:
	{
		TimeObj drvTime;
		unsigned char setTimeStatus;

		if (copy_from_user(&drvTime, &cmdHnd->argVec [0], sizeof (TimeObj))) {
			status = -EFAULT;
			goto fail_exit;
		}

		TPRO_MUTEX_LOCK(&hw->deviceLock);
		setTimeStatus = hw->pFunctionTable->setTime(hw, &drvTime);
		TPRO_MUTEX_UNLOCK(&hw->deviceLock);

		if (setTimeStatus != 0) {
			status = -EINVAL;
			goto fail_exit;
		}
		break;
	}

	case IOCTL_TPRO_SET_YEAR:
	{
		unsigned short year;
		unsigned char setYearStatus;

		/* get year value from user */
		__get_user(year, (unsigned short *) &cmdHnd->argVec[0]);

		/* set year to tpro card */
		TPRO_MUTEX_LOCK(&hw->deviceLock);
		setYearStatus = hw->pFunctionTable->setYear(hw, &year);
		TPRO_MUTEX_UNLOCK(&hw->deviceLock);

		if (setYearStatus != 0) {
			status = -EINVAL;
			goto fail_exit;
		}
		break;
	}

	case IOCTL_TPRO_SIM_EVENT:
	{
		TPRO_MUTEX_LOCK(&hw->deviceLock);
		(void)hw->pFunctionTable->simEvent(hw);
		TPRO_MUTEX_UNLOCK(&hw->deviceLock);
		break;
	}

	case IOCTL_TPRO_SYNCH_CONTROL:
	{
		int ctrl;
		unsigned char synchControlResult;

		__get_user(ctrl, (int *) &cmdHnd->argVec[0]);

		TPRO_MUTEX_LOCK(&hw->deviceLock);
		synchControlResult =
			hw->pFunctionTable->synchControl(hw, (unsigned char*)&ctrl);
		TPRO_MUTEX_UNLOCK(&hw->deviceLock);

		if (synchControlResult != 0) {
			status = -EINVAL;
			goto fail_exit;
		}
		break;
	}

	case IOCTL_TPRO_SYNCH_STATUS:
	{
		unsigned char stat = 0xFF;
		unsigned char synchControlResult;

		/* verify access to synch status character (write) */
		if (!TSYNC_ACCESS_OK(VERIFY_WRITE, (void*)&cmdHnd->argVec[0], sizeof (unsigned char)))
		{
			status = -EFAULT;
			goto fail_exit;
		}

		/* get synchronization status from tpro card */
		TPRO_MUTEX_LOCK(&hw->deviceLock);
		synchControlResult =
			hw->pFunctionTable->synchStatus(hw, &stat);
		TPRO_MUTEX_UNLOCK(&hw->deviceLock);

		if (synchControlResult != 0) {
			status = -EINVAL;
			goto fail_exit;
		}

		/* copy status to user-space */
		if (copy_to_user(&cmdHnd->argVec[0], &stat, sizeof (unsigned char))) {
			status = -EFAULT;
			goto fail_exit;
		}
		break;
	}

	case IOCTL_TPRO_WAIT_EVENT:
	{
		WaitObj drvWaitEvent;

		unsigned int waitRequired = 0;
		unsigned char getEventResult;

		if (!TSYNC_ACCESS_OK(VERIFY_WRITE, (void*)&cmdHnd->argVec[0], sizeof (WaitObj))) {
			status = -EFAULT;
			goto fail_exit;
		}

		if (copy_from_user(&drvWaitEvent, &cmdHnd->argVec[0], sizeof (WaitObj))) {
			status = -EFAULT;
			goto fail_exit;
		}

		TPRO_MUTEX_LOCK(&hw->deviceLock);
		(void)hw->pFunctionTable->toggleInterrupt(hw, FIFO_IRQ_ENABLE);
		TPRO_MUTEX_UNLOCK(&hw->deviceLock);

		if (hw->device == TSYNC_DEVID) {
			unsigned int count;
			gpioQueueCount(hw, 0, &count);

			if (count == 0) {
				waitRequired = 1;
			}
		}
		else {
			waitRequired = (hw->headIndex == hw->tailIndex);
		}

		if (waitRequired) {
			if (hw->headIndex == hw->tailIndex)
				{
					atomic_set (&hw->eventFlag, 0);

					status = os_waitPend(&hw->event_wait,
										 drvWaitEvent.jiffies,
										 &hw->eventFlag);

					if (status == OS_WAIT_STATUS_TIMEOUT) {
							TPRO_MUTEX_LOCK(&hw->deviceLock);
							(void)hw->pFunctionTable->toggleInterrupt(hw, DISABLE_INTERRUPTS);
							TPRO_MUTEX_UNLOCK(&hw->deviceLock);
							return 1;
						}

					if (status == OS_WAIT_STATUS_NG) {
							TPRO_MUTEX_LOCK(&hw->deviceLock);
							(void)hw->pFunctionTable->toggleInterrupt(hw, DISABLE_INTERRUPTS);
							TPRO_MUTEX_UNLOCK(&hw->deviceLock);
							return -1;
						}
				}
		}

		/* read event buffer into object */
		TPRO_MUTEX_LOCK(&hw->deviceLock);
		(void)hw->pFunctionTable->toggleInterrupt(hw, DISABLE_INTERRUPTS);
		getEventResult =
			hw->pFunctionTable->getEvent(hw, &drvWaitEvent);
		TPRO_MUTEX_UNLOCK(&hw->deviceLock);

		if (getEventResult != 0) {
			status = -EINVAL;
			goto fail_exit;
		}

		/* copy object back to user space */
		if (copy_to_user(&cmdHnd->argVec[0], &drvWaitEvent, sizeof (WaitObj))) {
			status = -EFAULT;
			goto fail_exit;
		}
		break;
	}

	case IOCTL_TPRO_WAIT_HEART:
	{
		int ticks;

		/* get microsecond value from user */
		__get_user(ticks, (int *) &cmdHnd->argVec[0]);

		TPRO_MUTEX_LOCK(&hw->deviceLock);
		(void)hw->pFunctionTable->toggleInterrupt(hw, HEART_IRQ_ENABLE);
		TPRO_MUTEX_UNLOCK(&hw->deviceLock);

		/* clear flag condition */
		atomic_set(&hw->heartFlag, 0);

		/* wait on semaphore */
		status = os_waitPend(&hw->heart_wait, ticks, &hw->heartFlag);

		if (status == OS_WAIT_STATUS_TIMEOUT) status = 1;
		if (status == OS_WAIT_STATUS_NG) status = -1;

		TPRO_MUTEX_LOCK(&hw->deviceLock);
		(void)hw->pFunctionTable->toggleInterrupt(hw, DISABLE_INTERRUPTS);
		TPRO_MUTEX_UNLOCK(&hw->deviceLock);
		break;
	}

	case IOCTL_TPRO_WAIT_MATCH:
	{
		int ticks;

		/* get microsecond value from user */
		__get_user(ticks, (int *) &cmdHnd->argVec[0]);

		TPRO_MUTEX_LOCK(&hw->deviceLock);
		(void)hw->pFunctionTable->toggleInterrupt(hw, MATCH_IRQ_ENABLE);
		TPRO_MUTEX_UNLOCK(&hw->deviceLock);

		/* clear flag condition */
		atomic_set(&hw->matchFlag, 0);

		/* wait on semaphore */
		status = os_waitPend(&hw->match_wait, ticks, &hw->matchFlag);

		if (status == OS_WAIT_STATUS_TIMEOUT) status = 1;
		if (status == OS_WAIT_STATUS_NG) status = -1;

		TPRO_MUTEX_LOCK(&hw->deviceLock);
		(void)hw->pFunctionTable->toggleInterrupt(hw, DISABLE_INTERRUPTS);
		TPRO_MUTEX_UNLOCK(&hw->deviceLock);
		break;
	}

	case IOCTL_TPRO_PEEK:
	{
		MemObj Reg;

		if (!TSYNC_ACCESS_OK(VERIFY_WRITE, (void*)&cmdHnd->argVec[0], sizeof(MemObj))) {
			status = -EFAULT;
			goto fail_exit;
		}

		if (copy_from_user(&Reg, &cmdHnd->argVec [0], sizeof (MemObj)))
		{
			status = -EFAULT;
			goto fail_exit;
		}

		TPRO_MUTEX_LOCK(&hw->deviceLock);
		(void)hw->pFunctionTable->peek(hw, &Reg);
		TPRO_MUTEX_UNLOCK(&hw->deviceLock);

		/* copy object back to user space */
		if (copy_to_user(&cmdHnd->argVec[0], &Reg, sizeof (MemObj))) {
			status = -EFAULT;
			goto fail_exit;
		}
		break;
	}

	case IOCTL_TPRO_POKE:
	{
		MemObj Reg;

		if (copy_from_user(&Reg, &cmdHnd->argVec [0], sizeof (MemObj))) {
			status = -EFAULT;
			goto fail_exit;
		}

		TPRO_MUTEX_LOCK(&hw->deviceLock);
		(void)hw->pFunctionTable->poke(hw, &Reg);
		TPRO_MUTEX_UNLOCK(&hw->deviceLock);
		break;
	}

	case IOCTL_TPRO_GET_NTP_TIME:
	{
		NtpTimeObj ntpTime;
		unsigned char getNTPResult;

		if (!TSYNC_ACCESS_OK(VERIFY_WRITE,(void*)&cmdHnd->argVec[0], sizeof (NtpTimeObj))) {
			status = -EFAULT;
			goto fail_exit;
		}

		TPRO_MUTEX_LOCK(&hw->deviceLock);
		getNTPResult = hw->pFunctionTable->getNtpTime(hw, &ntpTime);
		TPRO_MUTEX_UNLOCK(&hw->deviceLock);

		if (getNTPResult != 0) {
			status = -EINVAL;
			goto fail_exit;
		}

		if (hw->options < TSAT_PCI)	{
			memcpy(&ntpTime.refId, "IRIG", 4);
		}
		else {
			memcpy(&ntpTime.refId, "GPS ", 4);
		}

		if (copy_to_user(&cmdHnd->argVec[0], &ntpTime, sizeof(NtpTimeObj))) {
			status = -EFAULT;
			goto fail_exit;
		}
		break;
	}

	case IOCTL_TPRO_GET_FPGA:
	{
		unsigned char fpgaVer[6];
		unsigned char getFPGAResult;

		/* verify access to firmware bytes (write) */
		if (!TSYNC_ACCESS_OK(VERIFY_WRITE, (void*)&cmdHnd->argVec[0], sizeof(fpgaVer))) {
			status = -EFAULT;
			goto fail_exit;
		}

		TPRO_MUTEX_LOCK(&hw->deviceLock);
		getFPGAResult = hw->pFunctionTable->getFpgaVersion(hw, fpgaVer);
		TPRO_MUTEX_UNLOCK(&hw->deviceLock);

		if (getFPGAResult != 0) {
			status = -EINVAL;
			goto fail_exit;
		}

		if (copy_to_user(&cmdHnd->argVec[0], fpgaVer, sizeof (fpgaVer))) {
			status = -EFAULT;
			goto fail_exit;
		}
		break;
	}

	case IOCTL_TSYNC_GET:
	{
		int getStatus;

		TPRO_MUTEX_LOCK(&hw->deviceLock);
		getStatus = hw->pFunctionTable->get(hw, &cmdHnd->argVec[0]);
		TPRO_MUTEX_UNLOCK(&hw->deviceLock);

		if (getStatus != 0) {
			status = -EFAULT;
			goto fail_exit;
		}

	}
	break;

	case IOCTL_TSYNC_SET:
	{
		int setStatus;

		TPRO_MUTEX_LOCK(&hw->deviceLock);
		setStatus = hw->pFunctionTable->set(hw, &cmdHnd->argVec[0]);
		TPRO_MUTEX_UNLOCK(&hw->deviceLock);

		if (setStatus != 0) {
			status = -EFAULT;
			goto fail_exit;
		}
	}
	break;

	case IOCTL_TSYNC_WAIT:
	{
		ioctl_trans_di_wait transaction;

		if (!TSYNC_ACCESS_OK(VERIFY_WRITE, (void*)&cmdHnd->argVec[0], sizeof(ioctl_trans_di_wait))) {
			status = -EFAULT;
			goto fail_exit;
		}

		if (copy_from_user(&transaction, &cmdHnd->argVec[0], sizeof (ioctl_trans_di_wait))) {
			status = -EFAULT;
			goto fail_exit;
		}

		status = hw->pFunctionTable->wait(hw, &transaction);

		if (copy_to_user(&cmdHnd->argVec[0], &transaction, sizeof (ioctl_trans_di_wait))) {
			status = -EFAULT;
			goto fail_exit;
		}

	}
	break;

	default:
	{
		pr_err("Unhandled command\n");
		status = 1;
		goto fail_exit;
	}
	}

	fail_exit:
		TPRO_SPIN_LOCK(&hw->userLock);
		currUser = (currUser == user) ? NULL : currUser;
		TPRO_SPIN_UNLOCK(&hw->userLock);
		return status;

}

static int tsyncpci_open(struct inode *inode, struct file *filp)
{
	tsyncpci_dev_t * hw;
	tsyncpci_user_t *user;

	hw = tsyncpci_find_device(inode);

	if (!hw) {
		printk(KERN_ERR "%s: no corresponding hw device available, maybe not initialized\n",
		       __func__);
		return -ENODEV;
	}

	user = kmalloc (sizeof(tsyncpci_user_t), GFP_KERNEL);

	if (!user) {
		return -ENOMEM;
	}

	user->pDevice = hw;

	TPRO_SPIN_LOCK(&hw->userLock);

	if (hw->pFirstUser) {
		user->pNext = hw->pFirstUser;
		user->pPrev = hw->pLastUser;
		user->pNext->pPrev = user->pPrev->pNext = hw->pLastUser = user;
	}
	else {
		hw->intCnt = 0;
		hw->eventCnt = 0;
		hw->headIndex = hw->tailIndex = 0;
		user->pNext = user->pPrev = hw->pFirstUser = hw->pLastUser = user;
	}

	TPRO_SPIN_UNLOCK(&hw->userLock);
	filp->private_data = user;

	return 0;
}

static int tsync_remove(struct platform_device *pdev)
{
	tsyncpci_dev_t *hw = NULL;
	struct device *cd = NULL;
	unsigned int idx, irq;

	irq = irq_of_parse_and_map(pdev->dev.of_node, 0);

	for (idx = 0; idx < TSYNC_PCI_MAX_NUM; idx++) {
		if (tsyncpci_devp[idx]) {
			if (irq == tsyncpci_devp[idx]->irq)
			{
				hw = tsyncpci_devp[idx];
				cd = tsyncpci_cd[idx];
				tsyncpci_devp[idx] = NULL;
				tsyncpci_cd[idx]   = NULL;
				break;
			}
		}
	}

	if (hw) {
		tsync_ptp_host_reference_unregister(cd);
		tsync_pps_unregister(hw);

		(void)hw->pFunctionTable->InitializeBoard(hw);
		free_irq(hw->irq, hw);

		TPRO_SPIN_LOCK_DESTROY(&hw->userLock);
		TPRO_MUTEX_DESTROY(&hw->deviceLock);
		kfree(hw);
	}

	if (cd) {
		device_destroy(tsyncpci_cs, MKDEV(MAJOR(tsyncpci_devNum), idx));
	}
	return 0;
}

module_init(tsyncpci_init);
module_exit(tsync_cleanup);

MODULE_AUTHOR("Spectracom Corporation (spectracom.com)");
MODULE_DESCRIPTION("Spectracom TSync NiosII Timing Board");
MODULE_LICENSE("GPL");
MODULE_VERSION(VERSION);


#endif/* #ifdef NIOS */
