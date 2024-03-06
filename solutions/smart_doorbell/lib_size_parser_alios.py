
import os
import pandas as pd

path="yoc_sdk/lib/"
output_name="lib_alios.csv"
output_total="lib_alios_total.csv"
tmp_name="lib_alios.csv.tmp"

# print(os.system("size yoc_sdk/lib/libvfs.a"))
cmd=f'rm {output_name}'
os.system(cmd)
cmd=f'rm {tmp_name}'
os.system(cmd)
cmd=f'rm {output_total}'
os.system(cmd)

for root,dirs,files in os.walk(path):
    # print("root:", root)
    # print("dirs:", dirs)
    # print("files:", files)
    for f in files:
        # print(f)
        file_path=os.path.join(root, f)
        # print(file_path)
        cmd=f'echo {file_path} >> {tmp_name}; size -t {file_path} >> {tmp_name}'
        # print(cmd)
        # print(os.system(cmd))
        os.system(cmd)


with open(tmp_name, 'r') as file:
    lines = file.readlines()
with open(output_name, 'w') as tmp_file:
    for i,line in enumerate(lines):
        if (i==1) or (("text" not in line) and ("TOTALS" not in line) and (not line.startswith(path))):
            tmp_file.write(line)
with open(output_total, 'w') as total_file:
    tmp_filename=""
    for i,line in enumerate(lines):
        if (line.startswith(path)):
            tmp_filename=line
        if (i==1):
            total_file.write(line)
        if ("TOTALS" in line):
            # print(line.splitlines()[0]+tmp_filename)
            total_file.write(line.splitlines()[0]+tmp_filename)

file.close()
tmp_file.close()
total_file.close()
