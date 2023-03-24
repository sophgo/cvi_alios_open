CROSS_COMPILE   ?= riscv64-unknown-linux-musl-
CC        	 = $(CROSS_COMPILE)gcc

OBJFILE		 = ./affine.o

INCLUDE      = -I../include


LIB_CV_DIR  = -L../lib 
LIB_CV_NAME = -Wl,--whole-archive -lcsi_cv -Wl,--no-whole-archive
CCFLAGS	 	= -march=rv64imafdcv0p7_zfh_xtheadc -mabi=lp64d -O2 -static -g3
LDFLAGS	    = -march=rv64imafdcv0p7_zfh_xtheadc -mabi=lp64d -static -lm

.PHONY  : all

all: affine_demo

%.o: %.c
	@echo "  CC   $@"
	$(CC) $(CCFLAGS) -c $< -o $@ $(INCLUDE) 

affine_demo: $(OBJFILE)
	$(CC) $(LIB_CV_DIR) $(LIB_CV_NAME) $(OBJFILE) $(LDFLAGS) -o $@

clean:
	rm $(OBJFILE) affine_demo -rf
