# Smart Doorbell

该Solution适用于智慧门铃门锁场景

<br>

## 客制化参考Project说明

| Name          | Description         | Link                                                   |
| ------------- | ------------------- | ------------------------------------------------------ |
| turnkey       | CV180ZB双目参考方案 | [smart_doorbell_turnkey.md](smart_doorbell_turnkey.md) |
| turnkey_180xb | CV180XB单目EVB方案  | [[smdb]turnkey_180xb.md]([smdb]turnkey_180xb.md)       |
| turnkey_qfn   | CV181XC双目EVB方案  | [[smdb]turnkey_qfn.md]([smdb]turnkey_qfn.md)           |

<br>


## customization说明
  | Name               | Description                                |
  | ------------------ | ------------------------------------------ |
  | custom_sysparam.c  | vb配置                                     |
  | custom_viparam.c   | vi配置 sensor相关配置                      |
  | custom_voparam.c   | vo配置                                     |
  | custom_vpssparam.c | vpss配置                                   |
  | custom_vencparam.c | venc的配置                                 |
  | custom_platform.c  | 平台相关配置，如pinmux设定，GPIO拉高等操作 |

<br>
