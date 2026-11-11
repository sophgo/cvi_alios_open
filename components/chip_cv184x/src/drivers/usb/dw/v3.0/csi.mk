DWC3DIR = $(DRIVERDIR)/usb/dw/v3.0
USBDIR = $(LIBSDIR)/usb

INCDIR += -I$(DWC3DIR)/
INCDIR += -I$(USBDIR)/
INCDIR += -I$(USBDIR)/gadget
INCDIR += -I$(USBDIR)/include

DRIVER_CSRC += $(DWC3DIR)/core.c
DRIVER_CSRC += $(DWC3DIR)/ep0.c
DRIVER_CSRC += $(DWC3DIR)/gadget.c
DRIVER_CSRC += $(USBDIR)/lib/linux_compat.c
DRIVER_CSRC += $(USBDIR)/lib/panic.c
DRIVER_CSRC += $(USBDIR)/lib/strto.c
DRIVER_CSRC += $(USBDIR)/lib/ctype.c
DRIVER_CSRC += $(USBDIR)/fastboot/fb_command.c
DRIVER_CSRC += $(USBDIR)/fastboot/fb_common.c
DRIVER_CSRC += $(USBDIR)/fastboot/fb_getvar.c
#DRIVER_CSRC += $(USBDIR)/gadget/composite.c
DRIVER_CSRC += $(USBDIR)/gadget/config.c
DRIVER_CSRC += $(USBDIR)/gadget/epautoconf.c
DRIVER_CSRC += $(USBDIR)/gadget/f_fastboot.c
DRIVER_CSRC += $(USBDIR)/gadget/g_dnl.c
DRIVER_CSRC += $(USBDIR)/gadget/usbstring.c
DRIVER_CSRC += $(USBDIR)/gadget/udc/udc-core.c
