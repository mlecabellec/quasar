# $URL: http://subversion:8080/svn/gsc/trunk/drivers/LINUX/16AI64SSA/api/makefile $
# $Rev: 54952 $
# $Date: 2024-08-07 15:23:29 -0500 (Wed, 07 Aug 2024) $

# 16AI64SSA: API Library: make file

default: all

CC			= gcc
CC_FLAGS	= -c -fPIC -Wall -I../include
DEP_FILE	= makefile.dep
DEVICE		= 16ai64ssa
A_NAME		= ${DEVICE}_api.a
SO_NAME		= lib${DEVICE}_api.so
TARGET		= ${TARGET_A} ${TARGET_SO}
TARGET_A	= ../lib/${A_NAME}
TARGET_SO	= ../lib/${SO_NAME}

.NOTPARALLEL: %.o

ECHO	:= ${shell ls /bin/echo 2>/dev/null | wc -l}
ifeq ("${ECHO}","1")
ECHO	:= /bin/echo
else
ECHO	:= echo
endif

ifeq ("${GSC_API_COMP_FLAGS}","")
GSC_API_COMP_FLAGS_MSG=
else
GSC_API_COMP_FLAGS_MSG="  (added '${GSC_API_COMP_FLAGS}')"
endif

ifeq ("${GSC_API_LINK_FLAGS}","")
GSC_API_LINK_FLAGS_MSG=
else
GSC_API_LINK_FLAGS_MSG="  (added '${GSC_API_LINK_FLAGS}')"
endif

OBJ_FILES	:=			\
			close.o		\
			init.o		\
			ioctl.o		\
			open.o		\
			read.o

include ./os_api_makefile.inc

# Sort object file list to compile most recentlty changed sources first.
OBJ_FILES:= $(shell ls -t $(patsubst %.o,%.c,${OBJ_FILES}))
OBJ_FILES:= $(patsubst %.c,%.o,${OBJ_FILES})

.c.o:
	@echo == Compiling: $<  ${GSC_API_COMP_FLAGS_MSG}
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
	@${CC} ${CC_FLAGS} $< -o $@ ${GSC_API_COMP_FLAGS}

all: ${TARGET}
	@echo ==== All Done

release: ${TARGET}
	@rm -f *.o
	@echo ==== release Done

clean::
	@echo ==== Cleaning ${TARGET_A} ${TARGET_SO} ...
	@rm -f *.a *.o ${TARGET_A} ${TARGET_SO}
	@-chmod +rw ${DEP_FILE}
	@echo > ${DEP_FILE}

${TARGET_A}: ${GSC_FILES} ${OBJ_FILES}

	@echo ==== Linking: $@  ${GSC_LIB_LINK_FLAGS_MSG}
	@ld -r -o $@ ${GSC_FILES} ${OBJ_FILES} ${GSC_LIB_LINK_FLAGS}

${TARGET_SO}: exports.def ${GSC_FILES} ${OBJ_FILES}
	@echo ==== Linking: $@  ${GSC_API_LINK_FLAGS_MSG}
	@ld -shared -soname ${SO_NAME} -Bsymbolic -x -X --no-undefined --retain-symbols-file exports.def -o ${TARGET_SO} ${OBJ_FILES} ${GSC_FILES} -lc ${GSC_API_LINK_FLAGS}
	@rm -f /usr/lib/${SO_NAME}
	@cp ${TARGET_SO} /usr/lib/
	@ldconfig

${DEP_FILE}:
	@echo ==== Creating: $@
	@echo > ${DEP_FILE}

include ${DEP_FILE}
