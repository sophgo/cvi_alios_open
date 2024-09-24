# *
# * Copyright (C) 2024 Cvitek Co., Ltd.  All rights reserved.
# *
# * File Name: common/base.mk
# * Description:
# * Author: wangliang.wang@sophgo.com
# *
#
MPI_PARAM_DIR:=$(shell dirname $(realpath $(PARAM_FILE)))
#
CROSS_COMPILE ?= $(MPI_PARAM_DIR)/../../host-tools/Xuantie-900-gcc-elf-newlib-x86_64-V2.6.1/bin/riscv64-unknown-elf-
#
CC		:= $(CROSS_COMPILE)gcc
CXX	   := $(CROSS_COMPILE)g++
AS		:= $(CROSS_COMPILE)as
LD		:= $(CROSS_COMPILE)ld
CPP	   := $(CC) -E
AR		:= $(CROSS_COMPILE)ar
NM		:= $(CROSS_COMPILE)nm
STRIP	 := $(CROSS_COMPILE)strip
OBJCOPY   := $(CROSS_COMPILE)objcopy
OBJDUMP   := $(CROSS_COMPILE)objdump
AWK	   := awk
PERL	  := perl
PYTHON	:= python
CHECK	 := sparse
MAKE	  := make
# riscv64-unknown-linux-musl, riscv64-unknown-linux-gnu, aarch64-linux-gnu, arm-linux-gnueabihf gcc
TARGET_MACHINE  := $(shell ${CROSS_COMPILE}gcc -dumpmachine)
#
ifeq ($(notdir $(CROSS_COMPILE)), riscv64-unknown-linux-musl-)
CFLAGS		+= -mno-ldd -mcpu=c906fdv -march=rv64imafdcv0p7xthead -mcmodel=medany -mabi=lp64d
CXXFLAGS	+= -mno-ldd -mcpu=c906fdv -march=rv64imafdcv0p7xthead -mcmodel=medany -mabi=lp64d
ELFFLAGS	 += -mcpu=c906fdv -march=rv64imafdcv0p7xthead -mcmodel=medany -mabi=lp64d
else ifeq ($(notdir $(CROSS_COMPILE)), riscv64-unknown-linux-gnu-)
CFLAGS		+= -mno-ldd -mcpu=c906fdv -march=rv64imafdcv0p7xthead -mcmodel=medany -mabi=lp64d
CXXFLAGS	+= -mno-ldd -mcpu=c906fdv -march=rv64imafdcv0p7xthead -mcmodel=medany -mabi=lp64d
ELFFLAGS	 += -mcpu=c906fdv -march=rv64imafdcv0p7xthead -mcmodel=medany -mabi=lp64d
endif
#
ifeq ("$(origin V)", "command line")
  BUILD_VERBOSE = $(V)
endif
ifeq ($(BUILD_VERBOSE),1)
  Q =
else
  Q = @
endif
## COLOR ##
BLACK   = "\e[30;1m"
RED	 =  "\e[31;1m"
GREEN   = "\e[32;1m"
YELLOW  = "\e[33;1m"
BLUE	= "\e[34;1m"
PURPLE  = "\e[35;1m"
CYAN	= "\e[36;1m"
WHITE   = "\e[37;1m"
END	 = "\e[0m"
# Compile flags
OPT_CFLAGS	:= -O3
OPT_CFLAGS	+= -MMD
# for code debug
#OPT_CFLAGS += -save-temps=obj
CFLAGS		+= -g -fPIC -Wall -Wextra -Werror -std=gnu11 -D_GNU_SOURCE -Wno-stringop-truncation \
				-Wno-format-truncation -Wno-type-limits -Wno-enum-conversion -Wno-implicit-fallthrough \
				-Wno-stringop-overflow -Wno-sizeof-pointer-memaccess
CXX_STD		?= -std=gnu++11
CXXFLAGS	+= -g -fPIC -Wall -Wextra -Werror $(CXX_STD) -D_GNU_SOURCE -Wno-stringop-truncation
CFLAGS		+= $(OPT_CFLAGS) -D__FILENAME__=\"$(notdir $<)\"
CXXFLAGS	+= $(OPT_CFLAGS) -D__FILENAME__=\"$(notdir $<)\"
#

# assumptions
# - SEXT is defined (c or cpp)
# - SDIR is defined
# - SRCS is optional
# - if INCS is defined, will be add to CFLAGS or CXXFLAGS
# - if LIBS is defined, will be add to LDFLAGS
ifeq ($(SRCS_C), )
ifeq ($(SRCS_CPP), )
$(error "no source code file, please check!!")
endif
endif

BOOT_SOLUTION	?=normboot
CHIP_ARCH_L    := $(shell echo $(CHIP_ARCH) | tr A-Z a-z)

# Customer set
ODIR	:= $(CURDIR)/obj
OBJS	:= $(SRCS_C:$(SDIR)/%.c=$(ODIR)/%.$(BOOT_SOLUTION).$(CHIP_ARCH_L).o)
OBJS	+= $(SRCS_CPP:$(SDIR)/%.cpp=$(ODIR)/%.$(BOOT_SOLUTION).$(CHIP_ARCH_L).o)
#
ARFLAGS		:= rcs
CFLAGS		+= $(INCS)
CXXFLAGS	+= $(INCS)
#
.PHONY : all clean clean_all install $(TARGET_DIR)
 
all : $(TARGET_A) $(TARGET_DIR)

$(ODIR)/%.$(BOOT_SOLUTION).$(CHIP_ARCH_L).o : $(SDIR)/%.c
	$(Q)mkdir -p $(dir $@)
	$(Q)$(CC) $(CFLAGS) -o $@ -c $<
	$(Q)echo [$(notdir $(CC))] $(notdir $@)
 
$(ODIR)/%.$(BOOT_SOLUTION).$(CHIP_ARCH_L).o : $(SDIR)/%.cpp
	$(Q)mkdir -p $(dir $@)
	$(Q)$(CXX) $(CXXFLAGS) -o $@ -c $<
	$(Q)echo [$(notdir $(CXX))] $(notdir $@)
 
$(TARGET_DIR) :
	$(Q)mkdir -p $@
 
$(TARGET_A) : $(OBJS) | $(TARGET_DIR)
	$(Q)$(AR) $(ARFLAGS) $@ $^
	$(Q)echo $(YELLOW)[LINK]$(END)[$(notdir $(AR))] $(notdir $(TARGET_A))
 
#$(TARGET) : $(OBJS) | $(TARGET_DIR)
#	$(Q)$(CC) $(ELFFLAGS) -o $@.satic $^ -Wl,-Bstatic $(LIBS) -Wl,-Bdynamic $(DYN_LIBS)
#	$(Q)$(CC) $(ELFFLAGS) -o $@ $^ $(LIBS) $(DYN_LIBS)

clean:
	rm -rf $(TARGET_DIR)
 
clean_all: clean
	$(Q)if [ -d "$(SDIR)" ]; then \
		echo "rm -rf $(ODIR)"; \
		rm -rf $(ODIR); \
	fi
 
$(INSTALL_PATH)/lib :
	$(Q)mkdir -p $@
 
install: $(TARGET_A) $(TARGET_SO) | $(INSTALL_PATH)/lib
	$(Q)cp $^ $(INSTALL_PATH)/lib