# Tsync Driver

This is the extra kernel module for the Tsync card.

## Building

Be sure to have the kernel devel:

    apt install linux-headers linux-images

Then:

    $ make
    # make install

Using `make TSYNC_DRV_DEBUG=1` will add debug info to your build

## DKMS Build

If you want to use DKMS to build the kernel module use:


    # make dkms
