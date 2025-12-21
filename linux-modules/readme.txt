Modules that have a name ending with _lkm are linux kernel modules.
Linux kernel modules needs to be specifically compiled and install using
linux kernel makefiles. Local makefiles make the bridge with linux kernel's
one and using make from whole application will trigger the compilation of
those modules.
But installation and packaging is not following the buildscript model, then
kernel modules will not be included into dist feature. Then those modules
should not be include in EXPMOD from local app.cfg.
Further improvements of buildscript may bring better integration in the future.
