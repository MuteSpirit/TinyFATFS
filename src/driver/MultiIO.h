// SPDX-License-Identifier: MIT
#pragma once

#include "IO.h"
// fatfs.h lives at the src/ root, one level up from driver/ -- same
// reasoning as ff/ff.h's "../driver/IO.h": a plain "fatfs.h" only
// resolves via the -I fallback (this library's own src/ root on the
// include path), which isn't guaranteed if src/ gets vendored into
// another project instead of installed as a regular dependency.
#include "../fatfs.h"

namespace fatfs {

/**
 * @brief File system driver which supports multiple drives:
 * Add the drivers by calling add()
 * then call mount() to mount the drives.
 * @ingroup io
 */

class MultiIO : public IO {
 public:
  MultiIO() = default;

  void add(IO& io) { io_vector.push_back(&io); }

  /// mount all the added drivers, each on its own logical drive number
  /// matching its index (requires FF_VOLUMES >= io_vector.size())
  /// TODO: investigate why mount is being called only once by SDClass
  FRESULT mount(FatFs& fs, BYTE ldrv = 0) override {
      FRESULT rc = FR_OK;
      for (size_t i = 0, e = io_vector.size(); i < e; ++i) {
          // we have to say FatFs that each IO from io_vector is separate
          // logical drive and initiate FATFS creation for it
          rc = io_vector[i]->mount(fs, (BYTE)i);
          if (rc != FR_OK) {
              break;
          }
      }
      return rc;
  }

  /// unmount all drivers
  FRESULT un_mount(FatFs& fs, BYTE ldrv = 0) override {
    return io_vector[ldrv]->un_mount(fs, 0);
  }

  DSTATUS disk_initialize(BYTE pdrv) override {
    if (pdrv >= io_vector.size()) return STA_NODISK;
    return io_vector[pdrv]->disk_initialize(0);
  };
  DSTATUS disk_status(BYTE pdrv) override {
    if (pdrv >= io_vector.size()) return STA_NODISK;
    return io_vector[pdrv]->disk_status(0);
  }
  DRESULT disk_read(BYTE pdrv, BYTE* buff, LBA_t sector, UINT count) override {
    if (pdrv >= io_vector.size()) return RES_NOTRDY;
    return io_vector[pdrv]->disk_read(0, buff, sector, count);
  }
  DRESULT disk_write(BYTE pdrv, const BYTE* buff, LBA_t sector,
                     UINT count) override {
    if (pdrv >= io_vector.size()) return RES_NOTRDY;
    return io_vector[pdrv]->disk_write(0, buff, sector, count);
  }
  DRESULT disk_ioctl(BYTE pdrv, ioctl_cmd_t cmd, void* buff) override {
    if (pdrv >= io_vector.size()) return RES_NOTRDY;
    return io_vector[pdrv]->disk_ioctl(0, cmd, buff);
  }

 protected:
#ifdef ETL_NO_STL
  vector<IO*, FF_VOLUMES> io_vector;
#else
  vector<IO*> io_vector;
#endif
};

}  // namespace fatfs
