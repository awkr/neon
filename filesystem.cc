#include "filesystem.h"
#include <cstdio>
#include <cstring>
#include <sys/stat.h>

bool filesystem_exists(const char *path) {
  struct stat st {};
  return stat(path, &st) == 0;
}

bool filesystem_open(File **file, const char *path, FileMode mode, bool binary) {
  const char *flags;
  if ((mode & FILE_MODE_READ) != 0 && (mode & FILE_MODE_WRITE) != 0) {
    flags = binary ? "w+b" : "w+";
  } else if ((mode & FILE_MODE_READ) != 0 && (mode & FILE_MODE_WRITE) == 0) {
    flags = binary ? "rb" : "r";
  } else if ((mode & FILE_MODE_READ) == 0 && (mode & FILE_MODE_WRITE) != 0) {
    flags = binary ? "wb" : "w";
  } else {
    fprintf(stderr, "invalid mode passed while opening file: '%s'\n", path);
    return false;
  }
  FILE *handle = fopen(path, flags);
  if (!handle) {
    fprintf(stderr, "error opening file: '%s'\n", path);
    return false;
  }

  auto f = new File();
  f->handle = handle;
  f->valid = true;
  *file = f;
  return true;
}

void filesystem_close(File **file) {
  if (auto f = *file; f->valid) {
    fclose((FILE *)f->handle);
    DELETE(f);
  }
}

bool filesystem_size(File *file, u64 *outSize) {
  if (file->valid) {
    fseek((FILE *)file->handle, 0, SEEK_END);
    *outSize = ftell((FILE *)file->handle);
    rewind((FILE *)file->handle);
    return true;
  }
  return false;
}

bool filesystem_read_line(File *file, u64 maxLength, char **outBuffer, u64 *outLength) {
  if (file->valid) {
    if (fgets(*outBuffer, maxLength, (FILE *)file->handle)) {
      *outLength = strlen(*outBuffer);
      return true;
    }
  }
  return false;
}

bool filesystem_write_line(File *file, const char *buffer) {
  if (file->valid) {
    auto result = fputs(buffer, (FILE *)file->handle);
    if (result != EOF) { result = fputc('\n', (FILE *)file->handle); }
    fflush((FILE *)file->handle);
    return result != EOF;
  }
  return false;
}

bool filesystem_read(File *file, u64 size, void **outBuffer, u64 *outBytesRead) {
  if (file->valid) {
    *outBytesRead = fread(outBuffer, 1, size, (FILE *)file->handle);
    return *outBytesRead == size;
  }
  return false;
}

bool filesystem_read(File *file, void *outBuffer, u64 *outBytesRead) {
  if (file->valid) {
    u64 size;
    if (!filesystem_size(file, &size)) { return false; }
    *outBytesRead = fread(outBuffer, 1, size, (FILE *)file->handle);
    return *outBytesRead == size;
  }
  return false;
}

bool filesystem_write(File *file, u64 size, const void *buffer, u64 *outBytesWritten) {
  if (file->valid) {
    *outBytesWritten = fwrite(buffer, 1, size, (FILE *)file->handle);
    if (*outBytesWritten != size) { return false; }
    fflush((FILE *)file->handle);
    return true;
  }
  return false;
}
