import os
import json
import xml.dom.minidom
import yaml
import argparse

stroge_type = ['spinor','emmc','spinand']

def convert_yaml_2_xml(file_path,dst_path):
  defalut_storage_type=0
  doc = xml.dom.minidom.Document()
  with open(file_path, mode='r',encoding='utf-8') as f:
   yaml_json_array=yaml.load(stream=f,Loader=yaml.FullLoader)
   if isinstance(yaml_json_array['partitions'],list):
      root = doc.createElement('physical_partition')
      root.setAttribute('type', stroge_type[defalut_storage_type])
      for json_list in yaml_json_array['partitions']:
         if isinstance(json_list,dict):
            partition = doc.createElement('partition')
            for tmp_dict in json_list.items():
                tmp_str=tmp_dict[0]
                tmp_str_second=tmp_dict[1]
                if isinstance(tmp_dict[0],int):
                    tmp_str=str(hex(tmp_dict[0]))
                if isinstance(tmp_dict[1],int):
                    tmp_str_second=str(hex(tmp_dict[1]))
                partition.setAttribute(tmp_str,tmp_str_second)
            root.appendChild(partition)
         doc.appendChild(root)
         fp = open(dst_path, 'w')
         doc.writexml(fp, indent='\t', addindent='\t', newl='\n', encoding="utf-8")
  


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='partion_xml argparse')
    parser.add_argument('--file_path', type=str, default="./config.yaml")
    parser.add_argument('--dst_path', type=str, default="./config.xml")
    args = parser.parse_args()
    #args.file_path=os.path.split(os.path.realpath(__file__))[0]+args.file_path
    #args.dst_path=os.path.split(os.path.realpath(__file__))[0]+args.dst_path
    print("args.file_path is ",args.file_path)
    print("args.dst_path is ",args.dst_path)
    convert_yaml_2_xml(args.file_path,args.dst_path)
