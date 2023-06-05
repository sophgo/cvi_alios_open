#ifndef __VNET_REG__H__
#define __VNET_REG__H__

typedef struct
{
   volatile unsigned int status;
   volatile unsigned int intFlag;
   volatile unsigned int intMask;
   volatile unsigned int intClr;
   volatile unsigned int ipv4Addr;
   volatile unsigned int ipv4Mask;
   volatile unsigned int macAddr0;
   volatile unsigned int macAddr1;
   volatile unsigned int gw0;
   volatile unsigned int gw1;
   volatile unsigned int gw2;
   volatile unsigned int gw3;
   volatile unsigned int dns0;
   volatile unsigned int dns1;
   volatile unsigned int dns2;
   volatile unsigned int dns3;
   volatile unsigned int fwVersion;
   volatile unsigned int powerOff;
}vnet_reg_t;

vnet_reg_t *vnet_reg_get_addr(void);
void vnet_reg_init(void);

#endif
