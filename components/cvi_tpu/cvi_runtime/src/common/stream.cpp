#include <iostream>
#include <sstream>
#include <runtime/debug.h>
#include <runtime/stream.hpp>

namespace cvi {
namespace runtime {

FileStream::FileStream(const std::string& file_name) {
#ifdef USE_AOS_FILE
  _fd = aos_open(file_name.c_str(), O_RDONLY);
  if (_fd < 0) {
    TPU_LOG_ERROR("Error, Failed to open %s\n", file_name.c_str());
    return;
  }
  struct aos_stat file_stat;
  if (0 != aos_stat(file_name.c_str(), &file_stat)) {
    TPU_LOG_ERROR("Error, Failed to get file stat, %s\n", file_name.c_str());
    return;
  }
  _length = file_stat.st_size;
#else
  _fstream = new std::ifstream(file_name, std::ifstream::binary);
  if (!_fstream->good()) {
    TPU_LOG_ERROR("Error, Failed to open %s\n", file_name.c_str());
    return;
  }
  _fstream->seekg(0, _fstream->end);
  _length = _fstream->tellg();
  _fstream->seekg(0, _fstream->beg);
#endif
}

FileStream::~FileStream() {
#ifdef USE_AOS_FILE
  if (_fd) {
    aos_close(_fd);
  }
#else
  if (_fstream)
    delete _fstream;
#endif
}

size_t FileStream::read(uint8_t *buf, size_t offset, size_t size) {
  TPU_ASSERT(offset + size <= _length, "model is incomplete or incorrect!");
#ifdef USE_AOS_FILE
  aos_lseek(_fd, offset, SEEK_SET);
  size_t ret = aos_read(_fd, buf, size);
  if (ret != size) {
    TPU_ASSERT(0, "Error, Failed to read file");
    return 0;
  }
#else
  _fstream->seekg(offset);
  _fstream->read((char *)buf, size);
#endif
  return size;
}

BufferStream::BufferStream(const int8_t *buf, size_t size)
  : buffer(buf) {
  _length = size;
}

size_t BufferStream::read(uint8_t *buf, size_t offset, size_t size) {
  TPU_ASSERT(offset + size <= _length, "model is incomplete or incorrect!");
  memcpy(buf, buffer + offset, size);
  return size;
}

} // namespace runtime
} // namespace cvi

