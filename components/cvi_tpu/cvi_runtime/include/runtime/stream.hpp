#ifndef RUNTIME_CVISTREAM_H
#define RUNTIME_CVISTREAM_H

#define USE_AOS_FILE

#ifdef USE_AOS_FILE
#include <fcntl.h>
#include "vfs.h"
#else
#include <iostream>
#include <fstream>
#endif

namespace cvi {
namespace runtime {

class BaseStream {
public:
  BaseStream() {}
  virtual ~BaseStream() {}

  size_t length() {
    return _length;
  }

  virtual size_t read(uint8_t *buf, size_t offset, size_t size) = 0;

protected:
  size_t _length = 0;
};

class FileStream : public BaseStream {
public:
  FileStream(const std::string &file_name);
  ~FileStream();
  size_t read(uint8_t *buf, size_t offset, size_t size);
private:
#ifdef USE_AOS_FILE
  int _fd;
#else
  std::ifstream *_fstream;
#endif
};

class BufferStream : public BaseStream {
public:
  BufferStream(const int8_t *buf, size_t size);
  ~BufferStream() {}
  size_t read(uint8_t *buf, size_t offset, size_t size);

private:
  const int8_t *buffer;
};

} // namespace runtime
} // namespace cvi

#endif
