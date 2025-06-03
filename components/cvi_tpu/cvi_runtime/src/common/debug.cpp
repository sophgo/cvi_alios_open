#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <fstream>
#include <iostream>
#include <runtime/debug.h>
#include <cvibuilder/cvimodel_generated.h>

const char *level_name[] = {"EMG", "ALT", "CRI", "ERR", "WRN", "NOT", "INF", "DBG"};

void showRuntimeVersion() {
  printf("Cvitek Runtime (%d.%d.%d)%s\n",
         cvi::model::MajorVersion_value,
         cvi::model::MinorVersion_value,
         cvi::model::SubMinorVersion_value,
         RUNTIME_VERSION);
}

void dumpSysfsDebugFile(const char *path) {
  std::string line;
  std::ifstream file(path);
  std::cout << "dump " << path << "\n";
  while (std::getline(file, line )) {
    std::cout << line << "\n";
  }
  file.close();
  std::cout << "=======\n";
}
