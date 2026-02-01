savedcmd_/home/vortigern/git/quasar/linux-modules/dummy/dummy.o := x86_64-linux-gnu-ld -m elf_x86_64 -z noexecstack --no-warn-rwx-segments   -r -o /home/vortigern/git/quasar/linux-modules/dummy/dummy.o @/home/vortigern/git/quasar/linux-modules/dummy/dummy.mod  ; ./tools/objtool/objtool --hacks=jump_label --hacks=noinstr --hacks=skylake --ibt --orc --retpoline --rethunk --sls --static-call --uaccess --prefix=16  --link  --module /home/vortigern/git/quasar/linux-modules/dummy/dummy.o

/home/vortigern/git/quasar/linux-modules/dummy/dummy.o: $(wildcard ./tools/objtool/objtool)
