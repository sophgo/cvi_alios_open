#!/usr/bin/env python

import os
import re


def update_yaml(file_path, changes):
    current_dir = os.path.dirname(os.path.abspath(__file__))
    config_path = os.path.join(current_dir, "..", "..", "build", ".config")
    support_proc = 0

    try:
        with open(config_path) as f:
            lines = f.readlines()
            for line in lines:
                string = line.replace('\n', '')
                string = line.replace('\r', '')
                string = line.strip()

                if not string:
                    continue
                elif string[0] == "#":
                    continue
                elif string.isspace():
                    continue
                elif string.split("=")[0] == "CONFIG_MEDIA_SUPPORT_PROC" and string.split("=")[-1] == "y":
                    print("MEDIA_SUPPORT_PROC has been setted--" + string)
                    support_proc = 1
                    break
    except FileNotFoundError as fnf_error:
        print(fnf_error)

    # read YAML file
    with open(file_path, 'r') as file:
        content = file.read()

    # update config
    for key, value in changes.items():
        content = re.sub(
            r'({}\s*:\s*)\d'.format(key),
            r'\g<1>{}'.format(support_proc),
            content
        )

    # write back file
    with open(file_path, 'w') as file:
        file.write(content)


if __name__ == "__main__":
    # package.yaml的路径
    file_path = 'package.yaml'
    # 开启debug需要修改的配置项
    changes = {
        'CONFIG_CVI_LOG_TRACE_SUPPORT': 1,
        'CONFIG_SYS_SUPPORT_PROC': 1,
        'CONFIG_VB_SUPPORT_PROC': 1,
        'CONFIG_ICPM_SUPPORT_PROC': 1,
        'CONFIG_LDC_SUPPORT_PROC': 1,
        'CONFIG_RGN_SUPPORT_PROC': 1,
        'CONFIG_VC_SUPPORT_PROC': 1,
        'CONFIG_VI_SUPPORT_PROC': 1,
        'CONFIG_VO_SUPPORT_PROC': 1,
        'CONFIG_VPSS_SUPPORT_PROC': 1,
        'CONFIG_LOG_SUPPORT_PROC': 1
    }

    update_yaml(file_path, changes)

