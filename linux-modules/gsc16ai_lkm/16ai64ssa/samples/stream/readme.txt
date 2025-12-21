; $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/16AI64SSA/samples/stream/readme.txt $
; $Rev: 53550 $
; $Date: 2023-08-07 14:24:04 -0500 (Mon, 07 Aug 2023) $

16AI64SSA: "stream" application: Examples


COMMAND LINE ARGUMENTS:

Continuous Testing:

  -c      Continue testing until an error occurs.
  -C      Continue testing even if errors occur.
  -dt     Delay for # seconds between continuous tests. (0 - 3600)
  -m#     Continue testing for up to # minutes.
  -n#     Continue testing for up to # iterations.

  The options -c and -C enable continuous, repetitive execution of the test
  cycle. The options -m# and -n# specify when continuous testing ends. The -dt
  option induces a delay between repeated tests.

Sample Rate:

  -sr#    Sample input data at # S/S (default: 10,000). (2000 - 250000)

Rx I/O Mode:

  -bmdma  Transfer data using Block Mode DMA.
  -dmdma  Transfer data using Demand Mode DMA (default).
  -pio    Transfer data using PIO.

Data Source Options:

  -rx0    Provide null data rather than read from the device.
  -rxd    Read data from the device (default).
  -rxm#   Provide Rx data for # minutes (default: -rxmb#). (1 - 1440)
  -rxmb#  Receive # megabytes of data (default: 5). (1 - 30000)

  The -rx0 and -rxd options specify the source of the data. The -rxm# and
  -rxmb# options specify that the data volume is limited either by duration
  or by volume, respectively.

Data Processing Options:

  -tx0    Do nothing with the received data (default).
  -txb    Write the receive data to a binary file (datast.bin).
  -txci   Add context information to the file name.
  -txct   Remove the channel tag from .txt file content.
  -txd    Save .txt file content in decimal format.
  -txfs#  Limit files to # million samples (in -r multiples). (0 - 30000)
  -txh    Save .txt file content as hexadecimal format (default).
  -txt    Write the receive data to a text file (datast.txt).
  -txv    Enable minor validation and reporting of Tx process data.

  The -tx0, -txb and -txt options specify what to do with received data. When
  the data is saved to disk, the -txci option includes processing and
  configuration information in the file name. The -txfs# options limits the
  volume of data in a given file, which may result in multiple data files
  being created. The -txd, -txh and -txct options format the data saved to
  .txt files. The -txv option enables minimal validation of data before being
  processed.

Other Options:

  -s      Show transfer statistics upon completion.



BASIC EXAMPLES:

The follow are basic examples of the application’s use.

1.	The following is a typical example in which data is read from the device
	while simultaneously being saved to disk. The throughput rates reported
	are a combination of both of these activities. Saving data to disk may
	typically be slower than reading from the device.

	./stream –rxmb100 –rxd –txb -sr100000 0

2.	The following is an example in which data is read from the device but is
	discarded rather than saved to disk. The Rx throughput rate reported is
	effectively the maximum sustainable rate at which the host is able to
	retrieve data from the device using the active sample rate.

	./stream –rxmb100 –rxd –tx0 -sr100000 0

3.	The following is an example in which null data is written to disk rather
	than valid data from the device. The throughput rate reported is effectively
	the maximum sustainable rate at which the host is able to write data to the
	disk.

	./stream –rxmb1000 –rx0 –txb 0



LARGE DATA VOLUME EXAMPLES:

Using command line arguments the application can be directed to read data from
a device for an extended period of time while saving all data to disk.

4.	The following is an example in which data is read from the device for 10
	minutes while simultaneously saving the received data to a binary file. The
	file name includes context information (see below).

	./stream -rxd -rxm10 -txb -txci 0

	Using a device with only four channels, the resulting file name is as
	follows and is 96,468,992 bytes.

	datast_st1293091.797_t#000_f#000_ts1293098.383_cq04_sr10000.000.bin

5.	The following example is the same as the previous one except that saved
	files are limited to 10M samples each.

	./stream -rxd -rxm10 -txb -txci -txfs10 0

	This results in the following files, whose sizes are 41,943,040 bytes,
	41,943,040 bytes and 12,582,912 bytes, respectively.

	datast_st1293836.754_t#000_f#000_ts1293843.340_cq04_sr10000.000.bin
	datast_st1293836.754_t#000_f#001_ts1294105.487_cq04_sr10000.000.bin
	datast_st1293836.754_t#000_f#002_ts1294367.635_cq04_sr10000.000.bin

6.	The following command line uses a four channel device to perform a single
	test streaming data for five hours. In the test, file sizes are limited to
	10 minutes of data to help limit data loss in case of an error.

	./stream -sr40000 -rxd -rxm300 -txb -txci -txfs96 0

	10 minutes of data is as follows:
		     4 channels
	*    40000 S/S/channel
	*       60 seconds/minute
	*       10 minutes
	==========
	96,000,000 samples

	This results in a series of 29 files as identified below. The first 28 are
	402,653,184 bytes while the last is 246,415,360 bytes.

	datast_st1299198.862_t#000_f#000_ts1299200.516_cq04_sr40000.000.bin
	datast_st1299198.862_t#000_f#001_ts1299829.670_cq04_sr40000.000.bin
	...
	datast_st1299198.862_t#000_f#027_ts1316187.679_cq04_sr40000.000.bin
	datast_st1299198.862_t#000_f#028_ts1316816.833_cq04_sr40000.000.bin

7.	The following command line uses a four channel device to perform a
	series of six tests. Each test collects only 1MB of data, but there is a
	delay of 10 minutes between consecutive tests.

	./stream -sr40000 -rxd -rxmb1 -txb -txci -dt600 -C -n6 0

	This results in a series of six files as identified below. Each file is
	1,048,576 bytes.

	datast_st1360812.584_t#000_f#000_ts1360814.238_cq04_sr40000.000.bin
	datast_st1360812.584_t#001_f#000_ts1361435.531_cq04_sr40000.000.bin
	datast_st1360812.584_t#002_f#000_ts1362056.823_cq04_sr40000.000.bin
	datast_st1360812.584_t#003_f#000_ts1362678.111_cq04_sr40000.000.bin
	datast_st1360812.584_t#004_f#000_ts1363299.407_cq04_sr40000.000.bin
	datast_st1360812.584_t#005_f#000_ts1363920.696_cq04_sr40000.000.bin



CONTEXT INFORMATION:

If the -txb and -txci command flags are used, the file name created will
include information as follows.

Example: datast_st1281459.479_t#000_f#000_ts1281466.066_cq04_sr10000.000.bin

Start Time: stXXX.XXX: This is the time at which operations began. The time is
given with millisecond resolution. All files created during each execution of
the application will have the same exact Start Time. See TIME below.

Test Number: t#XXX: If the application is directed to run continuously using
the -C or -c command line arguments, then each test is numbered starting with
zero. All files created during the same test execution will have the same Test
Number.

File Number: f#XXX: If a test is configured to produce multiple files, then
each file created during each test is given a sequential File Number starting
at zero. Sequence numbering is reset to zero as each test begins. Multiple
files may be generated when file sized are limited via the -txfs# command line
argument.

Time Stamp: tsXXX.XXX: Each file is given a unique Time Stamp. This time is
computed to be the time at which the file's first scan of data was sampled
from the cable interface. The time is given with millisecond resolution. See
TIME below.

Channel Quantity: cqXX: This number reflects the number of input channels
installed on the accessed device.

Sample Rate: srXXX.XXX: This number reflects the sample rate produced by the
device's current configuration. This value may differ from the requested
sample rate as the hardware may not be able to precisely produce the rate
requested. For example, if the command line requests a rate of 119,300 S/S the
value reported here will be 119,299.688.



TIME:

The time values reported are obtained based upon available services and
options. In general, the time is obtained by calling clock_gettime(). If
unavailable the time will come from the gettimeofday() service.
