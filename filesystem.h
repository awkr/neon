#pragma once

#include "defines.h"

struct File {
  void *handle;
  bool valid;
};

enum FileMode {
  FILE_MODE_READ = 0x1,
  FILE_MODE_WRITE = 0x2,
};

bool filesystem_exists(const char *path);
bool filesystem_open(const char *path, FileMode mode, bool binary, File *outFile);
void filesystem_close(File *file);
bool filesystem_size(File *file, u64 *outSize);
bool filesystem_read_line(File *file, u64 maxLength, char **outBuffer, u64 *outLength);
/**
 * Writes text to the provided file, appending a '\n' afterward.
 * @param file
 * @param buffer
 * @return
 */
bool filesystem_write_line(File *file, const char *buffer);
bool filesystem_read(File *file, u64 size, void **outBuffer, u64 *outBytesRead);
bool filesystem_read(File *file, void *outBuffer, u64 *outBytesRead);
bool filesystem_write(File *file, u64 size, const void *buffer, u64 *outBytesWritten);
