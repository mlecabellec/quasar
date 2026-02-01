# Tsync

Regroups all the packages to use your Tsync Card.

This include:

* tsync-driver: A kernel module for Tsync

* libtsync: A library to access the Tsync Card

* tsync-utils: A group of programs to use your Tsync Card

* common: A common dir for tsync-driver & libtsync sources

## Building

    $ make
    # make install

### Variables

Variables can be set in env or as a Makefile variable.
```sh
# Both case are valid:
export VARIABLE
make

make VARIABLE=1
```

| name                    | effect                                                                            |
|-------------------------|-----------------------------------------------------------------------------------|
| TSYNC_DRV_DEBUG         | If set, compile TSYNC driver with debug messages enabled                          |
| TSYNC_DRV_DKMS_BUILD    | If set, use DKMS to build/install driver instead of compiling for a single kernel |

### Notes

* By default, libtsync will be installed in **/usr/lib** but you can change that
with the **LIBDIR** variable while running the install rule:
```sh
# make LIBDIR=/another/path install
```

## Troubleshoot

### Missing libtsync.so

#### Symptom

After the installation, the following message appears when I try to run a
command:

> error while loading shared libraries: libtsync.so: cannot open shared object
file: No such file or directory

#### Resolution

Verify the presence of the file in the system with `ls /usr/lib/libtsync*`,
the command should list `libtsync.so`.

If the file does not exist, be sure to execute `make libtsync-install`.

If the file exists, reload the runtime library cache with: `ldconfig`.

### modprobe: key was rejected by the service

#### Symptom

You can't load the `tsyncpci` module on a secured kernel.

#### Resolution

Even if links point to redhat documentation, this is applicable on most
distributions with **UEFI Secure Boot enable** & **CONFIG_MODULE_SIG** enable.

* Create a signing key ->
[RedHat: Generate a signing key](https://access.redhat.com/documentation/en-us/red_hat_enterprise_linux/8/html/managing_monitoring_and_updating_the_kernel/signing-kernel-modules-for-secure-boot_managing-monitoring-and-updating-the-kernel#generating-a-public-and-private-key-pair_signing-kernel-modules-for-secure-boot)

* Sign the module with your generated private key -> [RedHat: Signing kernel modules](https://access.redhat.com/documentation/en-us/red_hat_enterprise_linux/8/html/managing_monitoring_and_updating_the_kernel/signing-kernel-modules-for-secure-boot_managing-monitoring-and-updating-the-kernel#signing-kernel-modules-with-the-private-key_signing-kernel-modules-for-secure-boot)

* Enroll the key in MOK with `mokutil` -> [RedHat: Enrolling public key on system](https://access.redhat.com/documentation/en-us/red_hat_enterprise_linux/8/html/managing_monitoring_and_updating_the_kernel/signing-kernel-modules-for-secure-boot_managing-monitoring-and-updating-the-kernel#enrolling-public-key-on-target-system-by-adding-the-public-key-to-the-mok-list_signing-kernel-modules-for-secure-boot)
