// SPDX-License-Identifier: MIT
#pragma once

#include <stdlib.h>
#include <cstring>

#include "../fatfs.h"

#include "IO.h"

#if defined(ESP32) || defined(ARDUINO_ARCH_RP2040)
#include "Arduino.h"
#endif

namespace fatfs {

/**
 * @brief The data is stored in RAM. On an ESP32 or RP2350 when PSRAM has been
 * activated we store it in PSRAM.
 * @ingroup io
 * @param sectorCount - amount of sectors to store
 * @param sectorSize - valid sector sizes are 512, 1024, 2048 and 4096
 */
template<size_t sectorCount, size_t sectorSize = FF_MAX_SS>
class RamIO : public IO {
 public:
  RamIO() {
    memset(work_buffer, 0, sizeof(work_buffer));
  }

  ~RamIO() {
    for (auto* ptr : sectors) {
      free(ptr);
    }
  }

  DSTATUS disk_initialize(BYTE pdrv) override {
    if (pdrv != 0) return STA_NODISK;

    // allocate sectors
    if (sectors.size() == 0) {
      for (size_t j = 0; j < sectorCount; j++) {
        uint8_t* ptr = nullptr;
#ifdef ESP32
        ptr = (uint8_t*)ps_malloc(sectorSize);
#elif defined(ARDUINO_ARCH_RP2040) && defined(RP2350_PSRAM_CS)
        // free() releases PSRAM blocks as well
        ptr = (uint8_t*)pmalloc(sectorSize);
#endif
        if (ptr == nullptr) ptr = (uint8_t*)malloc(sectorSize);
        if (ptr == nullptr) {
          // out of memory: release what we got and report the failure
          for (auto* p : sectors) free(p);
          sectors.clear();
          status = STA_NOINIT;
          return status;
        }
        memset(ptr, 0, sectorSize);
        sectors.push_back(ptr);
      }
    }
    status = STA_CLEAR;
    return status;
  }

  DSTATUS disk_status(BYTE pdrv) override {
    if (pdrv != 0) return STA_NODISK;
    return status;
  }

  DRESULT disk_read(BYTE pdrv, BYTE* buffer, LBA_t sectorNo, UINT nSectors) override {
    if (pdrv != 0) return RES_NOTRDY;
    if (status == STA_NOINIT) return RES_NOTRDY;
    if (sectors.size() == 0 || sectorNo + nSectors > sectorCount) return RES_ERROR;
    for (size_t j = sectorNo; j < sectorNo + nSectors; j++) {
      int idx = j - sectorNo;
      uint8_t* source = sectors[j];
      memcpy(buffer + (idx * sectorSize), source , sectorSize);
    }
    return RES_OK;
  }

  DRESULT disk_write(BYTE pdrv, const BYTE* buffer, LBA_t sectorNo,
                     UINT nSectors) override {
    if (pdrv != 0) return RES_NOTRDY;
    if (status == STA_NOINIT) return RES_NOTRDY;
    if (sectors.size() == 0 || sectorNo + nSectors > sectorCount) return RES_ERROR;
    for (size_t j = sectorNo; j < sectorNo + nSectors; j++) {
      size_t idx = j - sectorNo;
      uint8_t* target = sectors[j];
      memcpy(target, buffer + (idx * sectorSize), sectorSize);
    }
    return RES_OK;
  }

  DRESULT disk_ioctl(BYTE pdrv, ioctl_cmd_t cmd, void* buffer) override {
    DRESULT res;
    if (pdrv) return RES_PARERR; /* Check parameter */
    switch (cmd) {
      case CTRL_SYNC: /* Wait for end of internal write process of the drive */
        res = RES_OK;
        break;

      case GET_SECTOR_COUNT: { /* Get drive capacity in unit of sector (DWORD)
                                */
        DWORD result = sectorCount;
        memcpy(buffer, &result, (sizeof(result)));
        res = RES_OK;
      } break;

      case GET_BLOCK_SIZE: { /* Get erase block size in unit of sector (DWORD)
                              */
        DWORD result = 1;
        memcpy(buffer, &result, (sizeof(result)));
        res = RES_OK;
      } break;

      case CTRL_TRIM: { /* Erase a block of sectors (used when _USE_ERASE == 1)
                         */
        if (buffer == nullptr || status == STA_NOINIT) {
          res = RES_PARERR;
          break;
        }
        DWORD range[2];
        // determine range
        memcpy(&range, buffer, sizeof(range));
        // validate range: must be inside the drive and allocated
        if (range[0] > range[1] || range[1] >= sectorCount ||
            sectors.size() != sectorCount) {
          res = RES_PARERR;
          break;
        }
        // clear memory
        for (DWORD j = range[0]; j <= range[1]; j++) {
          memset(sectors[j], 0, sectorSize);
        }
        res = RES_OK; /* FatFs does not check result of this command */
      } break;

      default:
        res = RES_PARERR;
        break;
    }
    return res;
  }

  virtual FRESULT mount(FatFs& fs, BYTE pdrv) override
  {
      memset(work_buffer, 0, sizeof(work_buffer));

      // the file system is empty so we need to format it
      char path[6] = {0};
      snprintf(path, sizeof(path), "%d:", pdrv);
      FRESULT rc = fs.f_mkfs(path, nullptr, work_buffer, sizeof(work_buffer));
      if (FR_OK != rc) {
        return rc;
      }

      return fs.f_mount(&fatfs, path, 0);
  }

 protected:
#ifdef ETL_NO_STL
  vector<uint8_t*, sectorCount> sectors;
#else
  vector<uint8_t*> sectors;
#endif
  DSTATUS status = STA_NOINIT;
  uint8_t work_buffer[FF_MAX_SS];
};


}  // namespace fatfs
