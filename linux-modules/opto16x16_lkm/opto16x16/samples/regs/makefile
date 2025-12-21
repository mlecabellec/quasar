# $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/OPTO16X16/OPTO16X16_Linux_1.x.x.x_GSC_DN/samples/regs/makefile $
# $Rev: 53723 $
# $Date: 2023-09-14 10:40:07 -0500 (Thu, 14 Sep 2023) $

# OPTO16X16: Sample Application: make file

default:	all

CC			= gcc
CC_FLAGS	= -c -Wall -I../../include -L/usr/lib
DEP_FILE	= makefile.dep
LIB_FILES	= ../../lib/opto16x16_main.a ../../lib/libopto16x16_api.so
TARGET		= regs

.NOTPARALLEL: %.o

ECHO	:= ${shell ls /bin/echo 2>/dev/null | wc -l}
ifeq ("${ECHO}","1")
ECHO	:= /bin/echo
else
ECHO	:= echo
endif

ifeq ("${GSC_APP_COMP_FLAGS}","")
GSC_APP_COMP_FLAGS_MSG=
else
GSC_APP_COMP_FLAGS_MSG="  (added '${GSC_APP_COMP_FLAGS}')"
endif

ifeq ("${GSC_APP_LINK_FLAGS}","")
GSC_APP_LINK_FLAGS_MSG=
else
GSC_APP_LINK_FLAGS_MSG="  (added '${GSC_APP_LINK_FLAGS}')"
endif

OBJ_FILES	:=			\
			main.o		\
			menu.o		\
			perform.o	\
			reg.o

# Sort object file list to compile most recentlty changed sources first.
OBJ_FILES:= $(shell ls -t $(patsubst %.o,%.c,${OBJ_FILES}))
OBJ_FILES:= $(patsubst %.c,%.o,${OBJ_FILES})

.c.o:
	@echo == Compiling: $<  ${GSC_APP_COMP_FLAGS_MSG}
	@-chmod +rw ${DEP_FILE}
	@# Get the dependency list for this module.
	@-${CC} -MM ${CC_FLAGS} $< > .tmp1
	@# Remove the trailing white space and backslash.
	@-sed -e "s/[ ]*[\\\\]//g" < .tmp1 > .tmp2
	@# Put everything on seperate lines.
	@-tr [:space:] \\n < .tmp2 > .tmp3
	@# Remove all of the system include files.
	@-grep -v "^[ ]*/" < .tmp3 > .tmp4
	@# Remove all empty lines.
	@-grep [[:alnum:]] < .tmp4 > .tmp5
	@# Put everything on the same line.
	@-tr '\n' '\040' < .tmp5 > .tmp6
	@-${ECHO} -e '\012' >> .tmp6
	@# Add all of the other dependencies to the end of this file.
	@-echo >> ${DEP_FILE}
	@-grep -v "^[ ]*$@" < ${DEP_FILE} >> .tmp6
	@# Remove blank lines from the list.
	@-grep "[[:alnum:]]" < .tmp6 > .tmp7
	@# Sort the list and put it in the dependency file.
	@-sort < .tmp7 > ${DEP_FILE}
	@# Cleanup.
	@rm -f ${DEP_FILE}.tmp* .tmp*
	@# Compile the module.
	@${CC} ${CC_FLAGS} $< -o $@ ${GSC_APP_COMP_FLAGS}

all: ${TARGET}
	@echo ==== All Done

release: ${TARGET}
	@rm -f *.a *.o
	@echo ==== Release Done

clean::
	@echo ==== Cleaning ${TARGET} ...
	@rm -f *.a *.o ${TARGET}
	@-chmod +rw ${DEP_FILE}
	@echo > ${DEP_FILE}

${TARGET}: makefile ${OBJ_FILES} ${LIB_FILES}
	@echo ==== Linking: $@  ${GSC_APP_LINK_FLAGS_MSG}
	@${CC} ${OBJ_FILES} ${LIB_FILES} -o $@ -lm -lpthread -lrt ${GSC_APP_LINK_FLAGS}

${DEP_FILE}:
	@echo ==== Creating: $@
	@echo > ${DEP_FILE}

include ${DEP_FILE}
