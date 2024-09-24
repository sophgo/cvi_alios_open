#!/usr/bin/env python

import os
from configdef import *
from ruamel.yaml import YAML


def __check_defconfig(config=None):
    if config in defconfig:
        return True
    elif sensor_def in config:
        return True
    elif chip_def in config:
        return True
    else:
        return False



def get_config():
    config = {}

    pathname = os.path.normpath(os.path.split(os.path.abspath(__file__))[0])
    print("enter include_config functions of " + __file__)
    config_path = os.path.join(pathname, "..", "..", "..", "build", ".config")
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
                elif not __check_defconfig(string.split("=")[0]):
                    # print(string + " is not in the support config list")
                    continue

                if string.split("=")[-1] == "y":
                    value = '1'
                elif string.split("=")[-1] == "n":
                    value = '0'
                elif string.split("=")[-1] == "m":
                    print("WARNING: unsurpport config value of M. The config:{} will be drop!".fromat(string))
                    continue

                # define cv180x/cv181x
                if "CONFIG_CHIP_ARCH_cv180x" in string.split("=")[0]:
                    config["ARCH_PHOBOS"] = "1"
                    config["__CV180X__"] = "1"
                    continue
                if "CONFIG_CHIP_ARCH_cv181x" in string.split("=")[0]:
                    config["ARCH_MARS"] = "1"
                    config["__CV181X__"] = "1"
                    continue

                config[string.split("=")[0]] = value
    except FileNotFoundError as fnf_error:
        print(fnf_error)
    return config


def get_yaml_define(yaml_file=None):
    if yaml_file == None:
        print("Error: yaml file is None, please check!")
        return None, None

    try:
        with open(yaml_file, mode="r") as f:
            yaml = YAML(typ='safe')
            data = yaml.load(f)
            return data["def_config"]
    except Exception as e:
        print("{0} in {1}.".format(str(e), yaml_file))

    return None


def dischange_define(config = None, define = None):
    if config == None or define == None:
        print("Error: config or define is None, please check!")
        return None
    for ck, cv in config.items():
        finded = 0
        for dk, dv in define.items():
            if ck == dk:
                finded = 1
                if cv != dv:
                    dv = cv
        if not finded:
            define[ck] = cv

def drop_yaml_define(yaml_file=None):
    if yaml_file == None:
        print("Error: yaml file is None, please check!")
        return None, None

    try:
        with open(yaml_file, mode="r") as f:
            lines = f.readlines()
            lineno = 0
            for line in lines:
                lineno = lineno + 1
                if line.strip().startswith('def_config'):
                    while lines[lineno][0] in [" ", "#"] or lines[lineno].strip() == "":
                        lines.pop(lineno)
                        # print(lines.pop(lineno), lineno, len(lines))
                        if lineno >= len(lines):
                            break
                    break
    except Exception as e:
        print("{0} in {1}.".format(str(e), yaml_file))

    return lines, lineno


def create_yaml(yaml_file = None, output = None):
    config = get_config()
    # print("config: ")
    # print(config)
    define = get_yaml_define(yaml_file)
    # print("define: ")
    # print(define)
    dischange_define(config, define)
    # print("dischange define: ")
    # print(define)
    file_data, lineno = drop_yaml_define(yaml_file)
    # print("yaml: " + str(lineno))
    # print(file_data)

    for dk in sorted(define, reverse = True):
        string = "  " + dk + ": " + str(define[dk]) + "\n"
        file_data.insert(lineno, string)

    try:
        with open(output, mode="w+") as f:
            f.write("".join(file_data))
    except Exception as e:
        print("{0} in {1}".format(str(e), output))

    print("[Defconfig] Create package.yaml finised.")

if __name__ == "__main__":
    create_yaml("package.yaml", "package.yaml")
