# Compilation

See https://doc.sophgo.com/cvitek-develop-docs/master/docs_latest_release/CV184x/zh/01.software/BSP/AliOS_Compilation_and_Usage_Instructions/build/html/index.html.


# Macro

Macro variables used in codes are defined in the package.yaml files of various levels of directories during the compilation process. 

During the compilation process, macro variables in the original package.yaml files will be updated by macro variables from the .config file generated in build project.
For example, 'def_config' in 'solutions/normboot/customization/cv1842hp_gc8613/package.yaml.turnkey' will be updated by '../build/.config', and then saved to a new file 'solutions/normboot/package.yaml'.

If need to add/delete macro variables from '../build/.config', they should be made in 'defconfig' variable in 'scripts/defconfig/configdef.py'.

Usage rules for some macro variables are as follows:
+ ARCH_MARS3: Used to destinguish project, but it's not used actually.
+ CONFIG_ARCH: The definition is chip architecture, but it's not used actually.
+ CONFIG_BOARD_CV184X: Used to destinguish different peripherals in boards. Macros at the same level include CONFIG_BOARD_CV181XC and CONFIG_BOARD_CV181XH, etc.
+ CVI_SOC_CV184X: Used to destinguish soc. Macros at the same level include CVI_SOC_CV181X and CVI_SOC_CV180X, etc.
+ CONFIG_CHIP: The definition is chip model, but it's not used actually.
+ CONFIG_CHIP_ARCH_cv184x: Used to destinguish chip series, but it's not used actually.
+ CONFIG_CHIP_BIG_ARM: Used to distinguish the big or small ends of the chip arm, but it's not used actually.
+ CONFIG_CHIP_cv1842cp: Used to distinguish the chip model. Macros at the same level include CONFIG_CHIP_cv1843cp and CONFIG_CHIP_cv180zb, etc.
+ \_\_CV184X\_\_: Used to distinguish DIE, it can be used in driver and application code. Macros at the same level include \_\_CV181X\_\_ and \_\_CV180X\_\_, etc.


# License

Project license: BSD 3-Clause (applies to code developed by Sophgo Technologies Inc.)
See LICENSE at repository root.

The licenses contained in release files are listed as follows:
1. the Apache License, Version 2.0：See http://www.apache.org/licenses/LICENSE-2.0.
2. The MIT License.
3. the GNU Lesser General Public License: See http://www.gnu.org/copyleft/lesser.html.
4. the GNU General Public License: See http://www.gnu.org/licenses/.
