#!/usr/bin/python
import os
import sys
import re
import csv
import xlsxwriter
import operator

# Installing XlsxWriter:
# $ pip install XlsxWriter
# Or to a non system dir:
# $ pip install --user XlsxWriter

#compiler name
compiler_name = ''
#data compress
data_compress = ''

#excel formats
format_dict_title   = {'bold': True, 'border': 7, 'align': 'left', 'bg_color': '#f55066'}
format_dict_entry_A = {'border': 7, 'align': 'left', 'bg_color': '#dcff93'}
format_dict_entry_B = {'border': 7, 'align': 'left', 'bg_color': '#b8f1cc'}
format_dict_entry_C = {'border': 7, 'align': 'left', 'bg_color': '#b8f1ed'}
format_dict_entry_D = {'border': 7, 'align': 'left', 'bg_color': '#e7dac9'}
format_dict_entry_E = {'border': 7, 'align': 'left', 'bg_color': '#ffe543'}

format_dict_entry_Z = {'border': 7, 'align': 'left', 'bg_color': '#e1622f'}


#alios-things libraries
libname_aos = ['lwip.a', 'mbedtls.a', 'netmgr.a', 'osal_aos.a', 'osal_posix.a', 'rhino.a', 'sntp.a', 'spiffs.a', 'ulog.a', 'websocket.a',
               'vfs.a', 'yloop.a', 'kernel_init.a', 'cplusplus.a', 'cli.a', 'bluetooth.a', 'debug.a', 'http.a', 'httpc.a', 'kv.a', 'arch_armv7a.a']

#toolchain libraries
libname_tch = ['libstdc++.a', 'libm.a', 'libgcc.a', 'libc.a', 'libnosys.a' ]

#realtek libraries
libname_rtk = ['libwlan_lib.a', 'libbtstack.a', 'rtl_bt_stack.a']

#Allwinner libraries
libname_aw  = ['aw-alsa-lib.a', 'aw-alsa-plugins-awrate.a', 'aw-alsa-plugins-sona_audioaef.a', 'aw-alsa-plugins-speexrate.a', 'aw-command.a',
               'aw-iobox.a', 'aw-multiple-console.a', 'aw-rfkill.a', 'aw-sound-core.a', 'aw-usb.a', 'aw-wifi.a']
libname_aw += ['blkpart.a', 'env.a', 'libsbc.a', 'libsonaaef.a', 'mcu_r328.a', 'opus.a', 'ortp.a', 'sona_audioaef.a', 'upgrade_app.a', 'wifilib.a',
               'libaw_aacdec.a', 'libaw_mp3dec.a', 'libbtmanager.a', 'adecoder.a']

#Genie Smartbox libraries
libname_box = ['genie_smartbox.a', 'libaligenie.a', 'libartc_engine.a', 'libxengine_bundled.a', 'libev.a']

#library (*.a) parse
def find_lib_owner(libname):
    if libname in libname_aos:
        return 'Alios Things', format_dict_entry_A
    elif libname in libname_tch:
        return 'Toolchain', format_dict_entry_B
    elif libname in libname_rtk:
        return 'Realtek', format_dict_entry_C
    elif libname in libname_aw:
        return 'Allwinner', format_dict_entry_D
    elif libname in libname_box:
        return 'Genie Smartbox', format_dict_entry_E
    else :
        return 'Unknown', format_dict_entry_Z

#get symbol list from gcc map file
def get_sym_list_gcc(sym_all_list, map_file, mem_map_text):
    #1. get 'mem_map_text'
    # find memory map (without discard and debug sections)
    mem_map_list = re.findall(r'Linker script and memory map([\s\S]+?)OUTPUT', mem_map_text)
    mem_map_text = '' if not mem_map_list else mem_map_list[0]
    if not mem_map_text:
        print ('Can\'t parse memory info, memory info get fail!')
        return
    mem_map_text = mem_map_text.replace('\r', '')

    #2. find all object file (*.o) map info
    sym_all_list_a = re.findall(r' [\.\w]*\.(iram1|text|literal|rodata|rodata1|data|bss)(?:\.(\S+)\n? +| +)(0x\w+) +(0x\w+) +.+[/\\](.+\.a)\((.+\.(o|obj))\)\n', mem_map_text)
    sym_all_list_a = map(lambda arg : {'Type':arg[0], 'Sym':arg[1], 'Addr':int(arg[2], 16),
                     'Size':int(arg[3], 16), 'Lib':arg[4], 'File':arg[5]}, sym_all_list_a)

    sym_all_list_o = re.findall(r' [\.\w]*\.(iram1|text|literal|rodata|data|bss|mmu_tbl)(?:\.(\S+)\n? +| +)(0x\w+) +(0x\w+) +.+[/\\](.+\.(o|obj))\n', mem_map_text)
    sym_all_list_o = map(lambda arg : {'Type':arg[0], 'Sym':arg[1], 'Addr':int(arg[2], 16),
                     'Size':int(arg[3], 16), 'Lib':'null', 'File':arg[4]}, sym_all_list_o)

    sym_com_list_a = re.findall(r' (COMMON) +(0x\w+) +(0x\w+) +.+[/\\](.+\.a)\((.+\.(o|obj))\)\n +0x\w+ +(\w+)\n', mem_map_text)
    sym_com_list_a = map(lambda arg : {'Type':arg[0], 'Sym':arg[5], 'Addr':int(arg[1], 16),
                     'Size':int(arg[2], 16), 'Lib':arg[3], 'File':arg[4]}, sym_com_list_a)

    sym_com_list_o = re.findall(r' (COMMON) +(0x\w+) +(0x\w+) +.+[/\\](.+\.(o|obj))\n +0x\w+ +(\w+)\n', mem_map_text)
    sym_com_list_o = map(lambda arg : {'Type':arg[0], 'Sym':arg[4], 'Addr':int(arg[1], 16),
                     'Size':int(arg[2], 16), 'Lib':'null', 'File':arg[3]}, sym_com_list_o)

    sym_all_list.extend(sym_all_list_a)
    sym_all_list.extend(sym_all_list_o)
    sym_all_list.extend(sym_com_list_a)
    sym_all_list.extend(sym_com_list_o)


#get symbol list from gcc map file
def get_sym_list_armcc(sym_all_list, map_file, mem_map_text):
    #1. get 'mem_map_text'
    mem_map_text = mem_map_text.replace('\r', '')

    #2. find all object file (*.o) map info
    sym_all_list_o = re.findall(r'\s+(0x\w+)\s+(0x\w+)\s+(Zero|Data|Code)\s+(RW|RO)\s+\d+\s+(\S+)\s+(.+\.o)\n', mem_map_text)
    sym_all_list_o = map(lambda arg : {'Addr':arg[0], 'Size':int(arg[1], 16), 'Type':arg[2],
                         'Attr':arg[3], 'Sym':arg[4], 'Lib': 'null', 'File':arg[5]}, sym_all_list_o)
    sym_all_list.extend(sym_all_list_o)

    sym_all_list_a = re.findall(r'\s+(0x\w+)\s+(0x\w+)\s+(Zero|Data|Code)\s+(RW|RO)\s+\d+\s+(\S+)\s+(\w+\.ar?)\((.+\.o)\)\n', mem_map_text)
    sym_all_list_a = map(lambda arg : {'Addr':arg[0], 'Size':int(arg[1], 16), 'Type':arg[2],
                         'Attr':arg[3], 'Sym':arg[4], 'Lib': arg[5], 'File':arg[6]}, sym_all_list_a)
    sym_all_list.extend(sym_all_list_a)

    sym_all_list_l = re.findall(r'\s+(0x\w+)\s+(0x\w+)\s+(Zero|Data|Code)\s+(RW|RO)\s+\d+\s+(\S+)\s+(\w+\.l)\((.+\.o)\)\n', mem_map_text)
    sym_all_list_l = map(lambda arg : {'Addr':arg[0], 'Size':int(arg[1], 16), 'Type':arg[2],
                         'Attr':arg[3], 'Sym':arg[4], 'Lib': arg[5], 'File':arg[6]}, sym_all_list_l)
    sym_all_list.extend(sym_all_list_l)

#library (*.a) parse
def parse_library(sym_all_list, benchbook):
    lib_dic_list = []
    id_list = []

    #for each memmap info, classify by mem type
    for obj_dic in sym_all_list:
        id_str = obj_dic['Lib']
        if id_str not in id_list:
            idx = len(lib_dic_list)
            lib_dic_list.append({'Lib':obj_dic['Lib'], 'ROM':0, 'RAM':0, 'Text':0, 'Rodata':0, 'Data':0, 'Bss':0})
            id_list.append(id_str)
        else:
            idx = id_list.index(id_str)

        if compiler_name == 'gcc':
            if obj_dic['Type'] == 'text' or obj_dic['Type'] == 'literal' or obj_dic['Type'] == 'iram1':
                lib_dic_list[idx]['Text'] += obj_dic['Size']
            elif obj_dic['Type'] == 'rodata' or obj_dic['Type'] == 'rodata1':
                lib_dic_list[idx]['Rodata'] += obj_dic['Size']
            elif obj_dic['Type'] == 'data':
                lib_dic_list[idx]['Data'] += obj_dic['Size']
            elif obj_dic['Type'] == 'bss' or obj_dic['Type'] == 'COMMON' or obj_dic['Type'] == 'mmu_tbl':
                lib_dic_list[idx]['Bss'] += obj_dic['Size']
        elif compiler_name == 'armcc':
            if obj_dic['Type'] == 'Code':
                lib_dic_list[idx]['Text'] += obj_dic['Size']
            elif obj_dic['Type'] == 'Data' and obj_dic['Attr'] == 'RO':
                lib_dic_list[idx]['Rodata'] += obj_dic['Size']
            elif obj_dic['Type'] == 'Data' and obj_dic['Attr'] == 'RW':
                lib_dic_list[idx]['Data'] += obj_dic['Size']
            elif obj_dic['Type'] == 'Zero':
                lib_dic_list[idx]['Bss'] += obj_dic['Size']

    #sum ROM and RAM for each library file
    for lib_dic in lib_dic_list:
        lib_dic['ROM'] = lib_dic['Text'] + lib_dic['Rodata'] + lib_dic['Data']
        lib_dic['RAM'] = lib_dic['Text'] + lib_dic['Rodata'] + lib_dic['Data'] + lib_dic['Bss']

    title_format = benchbook.add_format(format_dict_title)

    #2. add obj_dic_list to excel table
    worksheet = benchbook.add_worksheet('Library')
    worksheet.set_column('A:B', 20)
    worksheet.set_column('C:H', 10)
    worksheet.set_column('I:J', 12)
    row = 0

    #set table title
    worksheet.write_row(row, 0, ['OWNER', 'MODULE', '', 'TEXT', 'RODATA', 'DATA', 'BSS', '','ROM TOTAL', 'RAM TOTAL'], title_format)
    row += 1

    #add table entry
    lib_dic_list = sorted(lib_dic_list, key=operator.itemgetter('RAM'), reverse=True)
    for lib_dic in lib_dic_list:
        if lib_dic['RAM'] == 0:
            continue
        (lib_owner, format_entry) = find_lib_owner(lib_dic['Lib'])
        entry_format = benchbook.add_format(format_entry)
        worksheet.write_row(row, 0, [lib_owner, lib_dic['Lib'], '', lib_dic['Text'], lib_dic['Rodata'],
                            lib_dic['Data'], lib_dic['Bss'], '',lib_dic['ROM'], lib_dic['RAM']], entry_format)
        row += 1

    #table ending, summary
    worksheet.write_row(row, 0, ['TOTAL (bytes)', '', '', '', '', '', '', '', '', ''], title_format)
    worksheet.write_formula(row, 3, '=SUM(D2:D' + str(row) + ')', title_format)
    worksheet.write_formula(row, 4, '=SUM(E2:E' + str(row) + ')', title_format)
    worksheet.write_formula(row, 5, '=SUM(F2:F' + str(row) + ')', title_format)
    worksheet.write_formula(row, 6, '=SUM(G2:G' + str(row) + ')', title_format)
    worksheet.write_formula(row, 8, '=SUM(I2:I' + str(row) + ')', title_format)    
    worksheet.write_formula(row, 9, '=SUM(J2:J' + str(row) + ')', title_format)

#object file (*.o) parse
def parse_object(sym_all_list, benchbook):
    obj_dic_list = []
    id_list = []

    #for each memmap info, classify by mem type
    for obj_dic in sym_all_list:
        id_str = obj_dic['File'] + obj_dic['Lib']
        if id_str not in id_list:
            idx = len(obj_dic_list)
            obj_dic_list.append({'File':obj_dic['File'], 'Lib':obj_dic['Lib'], 'ROM':0, 'RAM':0, 'Text':0, 'Rodata':0, 'Data':0, 'Bss':0})
            id_list.append(id_str)
        else:
            idx = id_list.index(id_str)

        if compiler_name == 'gcc':
            if obj_dic['Type'] == 'text' or obj_dic['Type'] == 'literal' or obj_dic['Type'] == 'iram1':
                obj_dic_list[idx]['Text'] += obj_dic['Size']
            elif obj_dic['Type'] == 'rodata' or obj_dic['Type'] == 'rodata1':
                obj_dic_list[idx]['Rodata'] += obj_dic['Size']
            elif obj_dic['Type'] == 'data':
                obj_dic_list[idx]['Data'] += obj_dic['Size']
            elif obj_dic['Type'] == 'bss' or obj_dic['Type'] == 'COMMON' or obj_dic['Type'] == 'mmu_tbl':
                obj_dic_list[idx]['Bss'] += obj_dic['Size']
        elif compiler_name == 'armcc':
            if obj_dic['Type'] == 'Code':
                obj_dic_list[idx]['Text'] += obj_dic['Size']
            elif obj_dic['Type'] == 'Data' and obj_dic['Attr'] == 'RO':
                obj_dic_list[idx]['Rodata'] += obj_dic['Size']
            elif obj_dic['Type'] == 'Data' and obj_dic['Attr'] == 'RW':
                obj_dic_list[idx]['Data'] += obj_dic['Size']
            elif obj_dic['Type'] == 'Zero':
                obj_dic_list[idx]['Bss'] += obj_dic['Size']

    #sum ROM and RAM for each objrary file
    for obj_dic in obj_dic_list:
        obj_dic['ROM'] = obj_dic['Text'] + obj_dic['Rodata'] + obj_dic['Data']
        obj_dic['RAM'] = obj_dic['Text'] + obj_dic['Rodata'] + obj_dic['Data'] + obj_dic['Bss']

    title_format = benchbook.add_format(format_dict_title)

    #2. add obj_dic_list to excel table
    worksheet = benchbook.add_worksheet('Object')
    worksheet.set_column('A:C', 20)
    worksheet.set_column('D:I', 10)
    worksheet.set_column('J:K', 12)
    row = 0

    #set table title
    worksheet.write_row(row, 0, ['OWNER', 'C FILE', 'MODULE', '', 'TEXT', 'RODATA', 'DATA', 'BSS', '', 'ROM TOTAL', 'RAM TOTAL'], title_format)
    row += 1

    #add table entry
    obj_dic_list = sorted(obj_dic_list, key=operator.itemgetter('RAM'), reverse=True)
    for obj_dic in obj_dic_list:
        (lib_owner, format_entry) = find_lib_owner(obj_dic['Lib'])

        entry_format = benchbook.add_format(format_entry)
        worksheet.write_row(row, 0, [lib_owner, obj_dic['File'], obj_dic['Lib'], '', obj_dic['Text'], obj_dic['Rodata'], 
                            obj_dic['Data'], obj_dic['Bss'], '', obj_dic['ROM'], obj_dic['RAM']], entry_format)
        row += 1

    #table ending, summary
    worksheet.write_row(row, 0, ['TOTAL (bytes)', '', '', '', '', '', '', '', '', '', ''], title_format)
    worksheet.write_formula(row, 4, '=SUM(E2:E' + str(row) + ')', title_format)
    worksheet.write_formula(row, 5, '=SUM(F2:F' + str(row) + ')', title_format)
    worksheet.write_formula(row, 6, '=SUM(G2:G' + str(row) + ')', title_format)
    worksheet.write_formula(row, 7, '=SUM(H2:H' + str(row) + ')', title_format)
    worksheet.write_formula(row, 9, '=SUM(J2:J' + str(row) + ')', title_format)
    worksheet.write_formula(row, 10, '=SUM(K2:K' + str(row) + ')', title_format)    
    
#symbol parse
def parse_symbol(sym_all_list, benchbook):
    func_dic_list = []
    rodt_dic_list = []
    data_dic_list = []
    bss_dic_list = []
    id_list = []

    #for each memmap info, classify by mem type, add table entry
    for sym_dic in sym_all_list:
        if compiler_name == 'gcc':
            if sym_dic['Type'] == 'text' or sym_dic['Type'] == 'literal' or sym_dic['Type'] == 'iram1':
                func_dic_list.append({'Sym':sym_dic['Sym'], 'File':sym_dic['File'],
                                      'Lib':sym_dic['Lib'], 'Size':sym_dic['Size']})
            elif sym_dic['Type'] == 'rodata':
                rodt_dic_list.append({'Sym':sym_dic['Sym'], 'File':sym_dic['File'],
                                      'Lib':sym_dic['Lib'], 'Size':sym_dic['Size']})
            elif sym_dic['Type'] == 'data':
                data_dic_list.append({'Sym':sym_dic['Sym'], 'File':sym_dic['File'],
                                      'Lib':sym_dic['Lib'], 'Size':sym_dic['Size']})
            elif sym_dic['Type'] == 'bss' or sym_dic['Type'] == 'COMMON':
                bss_dic_list.append({'Sym':sym_dic['Sym'], 'File':sym_dic['File'],
                                      'Lib':sym_dic['Lib'], 'Size':sym_dic['Size']})
        if compiler_name == 'armcc':
            if sym_dic['Type'] == 'Code':
                func_dic_list.append({'Sym':sym_dic['Sym'], 'File':sym_dic['File'],
                                      'Lib':sym_dic['Lib'], 'Size':sym_dic['Size']})
            elif sym_dic['Attr'] == 'RO':
                rodt_dic_list.append({'Sym':sym_dic['Sym'], 'File':sym_dic['File'],
                                      'Lib':sym_dic['Lib'], 'Size':sym_dic['Size']})
            else:
                data_dic_list.append({'Sym':sym_dic['Sym'], 'File':sym_dic['File'],
                                      'Lib':sym_dic['Lib'], 'Size':sym_dic['Size']})

    title_format = benchbook.add_format(format_dict_title)

    #2. add func_dic_list to excel table
    worksheet = benchbook.add_worksheet('Function')
    worksheet.set_column('A:D', 20)
    worksheet.set_column('E:E', 10)
    row = 0

    worksheet.write_row(row, 0, ['OWNER', 'FUNCTION', 'C FILE', 'MODULE', 'SIZE'], title_format)
    row += 1

    func_dic_list = sorted(func_dic_list, key=operator.itemgetter('Size'), reverse=True)
    for sym_dic in func_dic_list:
        (lib_owner, format_entry) = find_lib_owner(sym_dic['Lib'])
        entry_format = benchbook.add_format(format_entry)
        worksheet.write_row(row , 0, [lib_owner, sym_dic['Sym'], sym_dic['File'],
                            sym_dic['Lib'], sym_dic['Size']], entry_format)
        row += 1

    worksheet.write_row(row, 0, ['TOTAL', '', '', ''], title_format)
    worksheet.write_formula(row, 4, '=SUM(E2:E' + str(row) + ')', title_format)

    #3. add func_dic_list to excel table
    worksheet = benchbook.add_worksheet('Rodata')
    worksheet.set_column('A:D', 20)
    worksheet.set_column('E:E', 10)
    row = 0

    worksheet.write_row(row, 0, ['OWNER', 'Rodata', 'C FILE', 'MODULE', 'SIZE'], title_format)
    row += 1

    rodt_dic_list = sorted(rodt_dic_list, key=operator.itemgetter('Size'), reverse=True)
    for sym_dic in rodt_dic_list:
        (lib_owner, format_entry) = find_lib_owner(sym_dic['Lib'])
        entry_format = benchbook.add_format(format_entry)
        worksheet.write_row(row , 0, [lib_owner, sym_dic['Sym'], sym_dic['File'],
                            sym_dic['Lib'], sym_dic['Size']], entry_format)
        row += 1

    worksheet.write_row(row, 0, ['TOTAL', '', '', ''], title_format)
    worksheet.write_formula(row, 4, '=SUM(E2:E' + str(row) + ')', title_format)

    #4. add func_dic_list to excel table
    worksheet = benchbook.add_worksheet('Data')
    worksheet.set_column('A:D', 20)
    worksheet.set_column('E:E', 10)
    row = 0

    worksheet.write_row(row, 0, ['OWNER', 'DATA', 'C FILE', 'MODULE', 'SIZE'], title_format)
    row += 1

    data_dic_list = sorted(data_dic_list, key=operator.itemgetter('Size'), reverse=True)
    for sym_dic in data_dic_list:
        (lib_owner, format_entry) = find_lib_owner(sym_dic['Lib'])
        entry_format = benchbook.add_format(format_entry)
        worksheet.write_row(row , 0, [lib_owner, sym_dic['Sym'], sym_dic['File'],
                            sym_dic['Lib'], sym_dic['Size']], entry_format)
        row += 1

    worksheet.write_row(row, 0, ['TOTAL', '', '', ''], title_format)
    worksheet.write_formula(row, 4, '=SUM(E2:E' + str(row) + ')', title_format)

    #5. add func_dic_list to excel table
    worksheet = benchbook.add_worksheet('Bss')
    worksheet.set_column('A:D', 20)
    worksheet.set_column('E:E', 10)
    row = 0

    worksheet.write_row(row, 0, ['OWNER', 'BSS', 'C FILE', 'MODULE', 'SIZE'], title_format)
    row += 1

    bss_dic_list = sorted(bss_dic_list, key=operator.itemgetter('Size'), reverse=True)
    for sym_dic in bss_dic_list:
        (lib_owner, format_entry) = find_lib_owner(sym_dic['Lib'])
        entry_format = benchbook.add_format(format_entry)
        worksheet.write_row(row , 0, [lib_owner, sym_dic['Sym'], sym_dic['File'],
                            sym_dic['Lib'], sym_dic['Size']], entry_format)
        row += 1

    worksheet.write_row(row, 0, ['TOTAL', '', '', ''], title_format)
    worksheet.write_formula(row, 4, '=SUM(E2:E' + str(row) + ')', title_format)

def get_mem_info(map_file):
    mem_map_text = ''
    sym_all_list = []
    global compiler_name
    global data_compress

    #1. get 'mem_map_text'
    with open(map_file, 'r') as f:
        mem_map_text = f.read()
        if not mem_map_text:
            print ('Can\'t parse map_file!')
            return
    map_flag = re.findall(r'Memory Map of the image', mem_map_text)
    if not map_flag:
        compiler_name = 'gcc'
        data_compress = 'no'
        get_sym_list_gcc(sym_all_list, map_file, mem_map_text)
    else:
        compiler_name = 'armcc'
        data_compress = 'yes'
        get_sym_list_armcc(sym_all_list, map_file, mem_map_text)

    #2. footprint.xlsx parse
    benchbook = xlsxwriter.Workbook('footprint.xlsx')
    # library (*.a) parse
    parse_library(sym_all_list, benchbook)
    # object file (*.o) parse
    parse_object(sym_all_list, benchbook)
    # symbol parse
    parse_symbol(sym_all_list, benchbook)
    benchbook.close()

def main():
    if len(sys.argv) > 1:
        map_file = sys.argv[1]
    else:
        file_list = os.listdir('.')
        for map_file in file_list:
            if r'.map' in map_file:
                break

    get_mem_info(map_file)

if __name__ == "__main__":
    main()
