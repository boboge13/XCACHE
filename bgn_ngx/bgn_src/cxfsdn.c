/******************************************************************************
*
* Copyright (C) Chaoyong Zhou
* Email: bgnvendor@163.com
* QQ: 2796796
*
*******************************************************************************/
#ifdef __cplusplus
extern "C"{
#endif/*__cplusplus*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include <ctype.h>
#include <errno.h>

#include "type.h"
#include "mm.h"
#include "log.h"
#include "cmpic.inc"
#include "cmutex.h"
#include "cmisc.h"

#include "real.h"

#include "clist.h"
#include "task.h"
#include "cepoll.h"

#include "crb.h"

#include "cxfsdn.h"

/*Random File System Data Node*/

EC_BOOL cxfsdn_node_write(CXFSDN *cxfsdn, const UINT32 node_id, const UINT32 data_max_len, const UINT8 *data_buff, UINT32 *offset)
{
    UINT32       offset_b; /*real base offset of block in physical file*/
    UINT32       offset_r; /*real offset in physical file*/

    offset_b = CXFSDN_OFFSET(cxfsdn) + CXFSDN_SIZE(cxfsdn) + (node_id << CXFSPGB_CACHE_BIT_SIZE);
    offset_r = offset_b + (*offset);

    if(SWITCH_ON == CXFSDN_CAMD_SWITCH)
    {
        ASSERT(NULL_PTR != CXFSDN_CAMD_MD(cxfsdn));

        if(EC_FALSE == camd_file_write(CXFSDN_CAMD_MD(cxfsdn),
                                       CXFSDN_SATA_DISK_FD(cxfsdn), &offset_r, data_max_len, data_buff))
        {
            dbg_log(SEC_0191_CXFSDN, 0)(LOGSTDOUT, "error:cxfsdn_node_write: "
                                "amd write %ld bytes to node %ld at offset %ld failed\n",
                                data_max_len, node_id, offset_r);
            return (EC_FALSE);
        }

        dbg_log(SEC_0191_CXFSDN, 9)(LOGSTDOUT, "[DEBUG] cxfsdn_node_write: "
                            "amd write %ld bytes to node %ld at offset %ld done\n",
                            data_max_len, node_id, offset_r);
    }

    if(SWITCH_OFF == CXFSDN_CAMD_SWITCH)
    {
        if(EC_FALSE == c_file_pwrite(CXFSDN_SATA_DISK_FD(cxfsdn), &offset_r, data_max_len, data_buff))
        {
            dbg_log(SEC_0191_CXFSDN, 0)(LOGSTDOUT, "error:cxfsdn_node_write: "
                                "flush %ld bytes to node %ld at offset %ld failed\n",
                                data_max_len, node_id, offset_r);
            return (EC_FALSE);
        }
    }

    (*offset) = (offset_r - offset_b);
    return (EC_TRUE);
}

EC_BOOL cxfsdn_node_read(CXFSDN *cxfsdn, const UINT32 node_id, const UINT32 data_max_len, UINT8 *data_buff, UINT32 *offset)
{
    UINT32       offset_b; /*real base offset of block in physical file*/
    UINT32       offset_r; /*real offset in physical file*/

    offset_b = CXFSDN_OFFSET(cxfsdn) + CXFSDN_SIZE(cxfsdn) + (node_id << CXFSPGB_CACHE_BIT_SIZE);
    offset_r = offset_b + (*offset);

    if(SWITCH_ON == CXFSDN_CAMD_SWITCH)
    {
        ASSERT(NULL_PTR != CXFSDN_CAMD_MD(cxfsdn));

        if(EC_FALSE == camd_file_read(CXFSDN_CAMD_MD(cxfsdn),
                                      CXFSDN_SATA_DISK_FD(cxfsdn), &offset_r, data_max_len, data_buff))
        {
            dbg_log(SEC_0191_CXFSDN, 0)(LOGSTDOUT, "error:cxfsdn_node_read: "
                                "amd read %ld bytes from node %ld at offset %ld failed\n",
                                data_max_len, node_id, offset_r);
            return (EC_FALSE);
        }

        dbg_log(SEC_0191_CXFSDN, 9)(LOGSTDOUT, "[DEBUG] cxfsdn_node_read: "
                            "amd read %ld bytes from node %ld at offset %ld done\n",
                            data_max_len, node_id, offset_r);
    }

    if(SWITCH_OFF == CXFSDN_CAMD_SWITCH)
    {
        if(EC_FALSE == c_file_pread(CXFSDN_SATA_DISK_FD(cxfsdn), &offset_r, data_max_len, data_buff))
        {
            dbg_log(SEC_0191_CXFSDN, 0)(LOGSTDOUT, "error:cxfsdn_node_read: "
                                "load %ld bytes from node %ld at offset %ld failed\n",
                                data_max_len, node_id, offset_r);
            return (EC_FALSE);
        }
    }

    (*offset) = (offset_r - offset_b);
    return (EC_TRUE);
}

CXFSDN *cxfsdn_create(const int cxfsdn_sata_fd, const UINT32 cxfsdn_sata_size, const UINT32 cxfsdn_sata_offset,
                         const UINT32 cxfsdn_mem_size,
                         const int cxfsdn_ssd_fd, const UINT32 cxfsdn_ssd_size, const UINT32 cxfsdn_ssd_offset)
{
    CXFSDN  *cxfsdn;

    UINT32   dn_size;
    UINT32   dn_mem_align;
    UINT8   *dn_mem_cache;

    UINT32   disk_size;
    uint16_t disk_max_num;

    if(ERR_FD == cxfsdn_sata_fd)
    {
        dbg_log(SEC_0191_CXFSDN, 0)(LOGSTDOUT, "error:cxfsdn_create: no fd\n");
        return (NULL_PTR);
    }

    disk_size     = (((UINT32)CXFSPGD_MAX_BLOCK_NUM) * ((UINT32)CXFSPGB_CACHE_MAX_BYTE_SIZE));
    disk_max_num  = (uint16_t)(cxfsdn_sata_size / disk_size);

    for(;0 < disk_max_num; disk_max_num --)
    {
        UINT32      dn_total_size;
        UINT32      mask;

        dn_size = cxfspgv_size(disk_max_num); /*data node meta data size*/
        mask    = (CXFSDN_MEM_ALIGNMENT - 1);
        dn_size = (dn_size + mask) & (~mask);

        dn_total_size = dn_size + disk_size * ((UINT32)disk_max_num);
        if(dn_total_size <= cxfsdn_sata_size)
        {
            break;
        }
    }

    if(0 == disk_max_num)
    {
        dbg_log(SEC_0191_CXFSDN, 0)(LOGSTDOUT, "error:cxfsdn_create: "
                                               "sata size %ld, disk size %ld => invalid disk max num %u\n",
                                               cxfsdn_sata_size, disk_size, disk_max_num);
        return (NULL_PTR);
    }

    if(CXFSPGV_MAX_DISK_NUM <= disk_max_num)
    {
        dbg_log(SEC_0191_CXFSDN, 0)(LOGSTDOUT, "error:cxfsdn_create: "
                                               "disk max num %u >= %u\n",
                                               disk_max_num, CXFSPGV_MAX_DISK_NUM);
        return (NULL_PTR);
    }

    dbg_log(SEC_0191_CXFSDN, 9)(LOGSTDOUT, "[DEBUG] cxfsdn_create: "
                                           "sata size %ld, disk size %ld => disk max num %u => dn size %ld\n",
                                           cxfsdn_sata_size, disk_size, disk_max_num, dn_size);

    cxfsdn = cxfsdn_new();
    if(NULL_PTR == cxfsdn)
    {
        dbg_log(SEC_0191_CXFSDN, 0)(LOGSTDOUT, "error:cxfsdn_create: new cxfsdn failed\n");
        return (NULL_PTR);
    }

    dn_mem_align = CXFSDN_MEM_ALIGNMENT;

    if(SWITCH_OFF == CXFS_DN_MMAP_SWITCH)
    {
        dn_mem_cache = c_memalign_new(dn_size, dn_mem_align);
        if(NULL_PTR == dn_mem_cache)
        {
            dbg_log(SEC_0191_CXFSDN, 0)(LOGSTDOUT, "error:cxfsdn_create: "
                                                   "alloc %ld bytes with alignment %ld failed\n",
                                                   dn_size, dn_mem_align);
            cxfsdn_free(cxfsdn);
            return (NULL_PTR);
        }

        dbg_log(SEC_0191_CXFSDN, 0)(LOGSTDOUT, "[DEBUG] cxfsdn_create: "
                                               "dn_size %ld, sata offset %ld\n",
                                               dn_size, cxfsdn_sata_offset);

        CXFSDN_SATA_DISK_FD(cxfsdn) = cxfsdn_sata_fd;
        CXFSDN_SIZE(cxfsdn)         = dn_size;
        CXFSDN_OFFSET(cxfsdn)       = cxfsdn_sata_offset;

        CXFSDN_CXFSPGV(cxfsdn) = cxfspgv_new(dn_mem_cache, dn_size, disk_max_num);
        if(NULL_PTR == CXFSDN_CXFSPGV(cxfsdn))
        {
            dbg_log(SEC_0191_CXFSDN, 0)(LOGSTDOUT, "error:cxfsdn_create: new vol failed\n");

            c_memalign_free(dn_mem_cache);

            cxfsdn_free(cxfsdn);
            return (NULL_PTR);
        }
    }

    if(SWITCH_ON == CXFS_DN_MMAP_SWITCH)
    {
        UINT8   *addr;

        addr = c_mmap_aligned_addr(dn_size, dn_mem_align);
        if(NULL_PTR == addr)
        {
            dbg_log(SEC_0191_CXFSDN, 0)(LOGSTDOUT, "error:cxfsdn_create: "
                                                   "fetch mmap aligned addr of size %ld align %ld failed\n",
                                                   dn_size, dn_mem_align);
            cxfsdn_free(cxfsdn);
            return (NULL_PTR);
        }

        dn_mem_cache = (UINT8 *)mmap(addr, dn_size,
                                     PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FIXED,
                                     cxfsdn_sata_fd, cxfsdn_sata_offset);
        if(MAP_FAILED == dn_mem_cache)
        {
            dbg_log(SEC_0191_CXFSDN, 0)(LOGSTDOUT, "error:cxfsdn_create: "
                               "mmap fd %d offset %ld size %ld, failed, errno = %d, errstr = %s\n",
                               cxfsdn_sata_fd, cxfsdn_sata_offset, dn_size, errno, strerror(errno));
            cxfsdn_free(cxfsdn);
            return (NULL_PTR);
        }

        CXFSDN_SATA_DISK_FD(cxfsdn) = cxfsdn_sata_fd;
        CXFSDN_SIZE(cxfsdn)         = dn_size;
        CXFSDN_OFFSET(cxfsdn)       = cxfsdn_sata_offset;

        CXFSDN_CXFSPGV(cxfsdn) = cxfspgv_new(dn_mem_cache, dn_size, disk_max_num);
        if(NULL_PTR == CXFSDN_CXFSPGV(cxfsdn))
        {
            dbg_log(SEC_0191_CXFSDN, 0)(LOGSTDOUT, "error:cxfsdn_create: new vol failed\n");

            munmap(dn_mem_cache, dn_size);

            cxfsdn_free(cxfsdn);
            return (NULL_PTR);
        }
    }

    CXFSDN_MEM_CACHE(cxfsdn) = dn_mem_cache;

    dbg_log(SEC_0191_CXFSDN, 9)(LOGSTDOUT, "[DEBUG] cxfsdn_create: create vol done\n");

    if(SWITCH_ON == CXFSDN_CAMD_SWITCH)
    {
        CAMD_MD    *camd_md;
        UINT32      sata_disk_size; /*in byte*/
        UINT32      mem_disk_size;  /*in byte*/
        UINT32      ssd_disk_offset;/*in byte*/
        UINT32      ssd_disk_size;  /*in byte*/
        int         sata_disk_fd;
        int         ssd_disk_fd;

        sata_disk_fd    = cxfsdn_sata_fd;
        sata_disk_size  = cxfsdn_sata_size - cxfsdn_sata_offset;

        mem_disk_size   = cxfsdn_mem_size;

        ssd_disk_fd     = cxfsdn_ssd_fd;
        ssd_disk_offset = cxfsdn_ssd_offset;
        ssd_disk_size   = cxfsdn_ssd_size;

        camd_md = camd_start(sata_disk_fd, sata_disk_size,
                             mem_disk_size,
                             ssd_disk_fd, ssd_disk_offset, ssd_disk_size);
        if(NULL_PTR == camd_md)
        {
            dbg_log(SEC_0191_CXFSDN, 0)(LOGSTDOUT, "error:cxfsdn_create:start camd failed\n");
            cxfsdn_free(cxfsdn);
            return (NULL_PTR);
        }

        if(EC_FALSE == camd_create(camd_md))
        {
            dbg_log(SEC_0191_CXFSDN, 0)(LOGSTDOUT, "error:cxfsdn_create:create cdc failed\n");
            camd_end(camd_md);
            cxfsdn_free(cxfsdn);
            return (NULL_PTR);
        }

        if(ERR_FD != camd_get_eventfd(camd_md)
        && NULL_PTR != task_brd_default_get_cepoll())
        {
            task_brd_process_add(task_brd_default_get(),
                                (TASK_BRD_CALLBACK)camd_process,
                                (void *)camd_md);

            cepoll_set_event(task_brd_default_get_cepoll(),
                              camd_get_eventfd(camd_md),
                              CEPOLL_RD_EVENT,
                              (const char *)"camd_event_handler",
                              (CEPOLL_EVENT_HANDLER)camd_event_handler,
                              (void *)camd_md);
        }

        CXFSDN_CAMD_MD(cxfsdn)     = camd_md;
        CXFSDN_SSD_DISK_FD(cxfsdn) = cxfsdn_ssd_fd;

        dbg_log(SEC_0191_CXFSDN, 0)(LOGSTDOUT, "[DEBUG] cxfsdn_create:create cdc done\n");
    }

    return (cxfsdn);
}

EC_BOOL cxfsdn_add_disk(CXFSDN *cxfsdn, const uint16_t disk_no)
{
    if(EC_FALSE == cxfspgv_add_disk(CXFSDN_CXFSPGV(cxfsdn), disk_no))
    {
        dbg_log(SEC_0191_CXFSDN, 0)(LOGSTDOUT, "error:cxfsdn_add_disk: cxfspgv add disk %u failed\n", disk_no);
        return (EC_FALSE);
    }

    return (EC_TRUE);
}

EC_BOOL cxfsdn_del_disk(CXFSDN *cxfsdn, const uint16_t disk_no)
{
    if(EC_FALSE == cxfspgv_del_disk(CXFSDN_CXFSPGV(cxfsdn), disk_no))
    {
        dbg_log(SEC_0191_CXFSDN, 0)(LOGSTDOUT, "error:cxfsdn_del_disk: cxfspgv del disk %u failed\n", disk_no);
        return (EC_FALSE);
    }
    return (EC_TRUE);
}

EC_BOOL cxfsdn_mount_disk(CXFSDN *cxfsdn, const uint16_t disk_no)
{
    if(EC_FALSE == cxfspgv_mount_disk(CXFSDN_CXFSPGV(cxfsdn), disk_no))
    {
        dbg_log(SEC_0191_CXFSDN, 0)(LOGSTDOUT, "error:cxfsdn_mount_disk: cxfspgv mount disk %u failed\n", disk_no);
        return (EC_FALSE);
    }

    return (EC_TRUE);
}

EC_BOOL cxfsdn_umount_disk(CXFSDN *cxfsdn, const uint16_t disk_no)
{
    if(EC_FALSE == cxfspgv_umount_disk(CXFSDN_CXFSPGV(cxfsdn), disk_no))
    {
        dbg_log(SEC_0191_CXFSDN, 0)(LOGSTDOUT, "error:cxfsdn_umount_disk: cxfspgv umount disk %u failed\n", disk_no);
        return (EC_FALSE);
    }

    return (EC_TRUE);
}


CXFSDN *cxfsdn_new()
{
    CXFSDN *cxfsdn;

    alloc_static_mem(MM_CXFSDN, &cxfsdn, LOC_CXFSDN_0001);
    if(NULL_PTR != cxfsdn)
    {
        cxfsdn_init(cxfsdn);
        return (cxfsdn);
    }
    return (cxfsdn);
}

EC_BOOL cxfsdn_init(CXFSDN *cxfsdn)
{
    CXFSDN_CXFSPGV(cxfsdn)      = NULL_PTR;

    CXFSDN_SSD_DISK_FD(cxfsdn)  = ERR_FD;

    CXFSDN_SATA_DISK_FD(cxfsdn) = ERR_FD;
    CXFSDN_OFFSET(cxfsdn)       = ERR_OFFSET;
    CXFSDN_SIZE(cxfsdn)         = 0;

    CXFSDN_MEM_CACHE(cxfsdn)    = NULL_PTR;

    CXFSDN_CAMD_MD(cxfsdn)      = NULL_PTR;

    return (EC_TRUE);
}

EC_BOOL cxfsdn_clean(CXFSDN *cxfsdn)
{
    if(NULL_PTR != CXFSDN_CXFSPGV(cxfsdn))
    {
        cxfspgv_close(CXFSDN_CXFSPGV(cxfsdn));
        CXFSDN_CXFSPGV(cxfsdn) = NULL_PTR;
    }

    if(SWITCH_OFF == CXFS_DN_MMAP_SWITCH
    && NULL_PTR != CXFSDN_MEM_CACHE(cxfsdn))
    {
        c_memalign_free(CXFSDN_MEM_CACHE(cxfsdn));
        CXFSDN_MEM_CACHE(cxfsdn) = NULL_PTR;
    }

    if(SWITCH_ON== CXFS_DN_MMAP_SWITCH
    && NULL_PTR != CXFSDN_MEM_CACHE(cxfsdn))
    {
        UINT32      wsize;
        UINT8      *mem_cache;

        wsize     = CXFSDN_SIZE(cxfsdn);
        mem_cache = CXFSDN_MEM_CACHE(cxfsdn);

        if(0 != munmap(mem_cache, wsize))
        {
            dbg_log(SEC_0191_CXFSDN, 0)(LOGSTDOUT, "warn:cxfsdn_clean: "
                                                   "munmap size %ld failed\n",
                                                   wsize);
        }
        else
        {
            dbg_log(SEC_0191_CXFSDN, 0)(LOGSTDOUT, "[DEBUG] cxfsdn_clean: "
                                                   "munmap size %ld done\n",
                                                   wsize);
        }

        CXFSDN_MEM_CACHE(cxfsdn) = NULL_PTR;
    }

    if(NULL_PTR != CXFSDN_CAMD_MD(cxfsdn))
    {
        camd_end(CXFSDN_CAMD_MD(cxfsdn));
        CXFSDN_CAMD_MD(cxfsdn) = NULL_PTR;
    }

    CXFSDN_SSD_DISK_FD(cxfsdn)  = ERR_FD;

    CXFSDN_SATA_DISK_FD(cxfsdn) = ERR_FD;
    CXFSDN_OFFSET(cxfsdn)       = ERR_OFFSET;
    CXFSDN_SIZE(cxfsdn)         = 0;

    return (EC_TRUE);
}

EC_BOOL cxfsdn_free(CXFSDN *cxfsdn)
{
    if(NULL_PTR != cxfsdn)
    {
        cxfsdn_clean(cxfsdn);
        free_static_mem(MM_CXFSDN, cxfsdn, LOC_CXFSDN_0002);
    }
    return (EC_TRUE);
}

void cxfsdn_print(LOG *log, const CXFSDN *cxfsdn)
{
    if(NULL_PTR != cxfsdn && NULL_PTR != CXFSDN_CXFSPGV(cxfsdn))
    {
        cxfspgv_print(log, CXFSDN_CXFSPGV(cxfsdn));
    }
    return;
}

EC_BOOL cxfsdn_is_full(CXFSDN *cxfsdn)
{
    return cxfspgv_is_full(CXFSDN_CXFSPGV(cxfsdn));
}

EC_BOOL cxfsdn_flush(CXFSDN *cxfsdn)
{
    if(SWITCH_OFF == CXFS_DN_MMAP_SWITCH
    && NULL_PTR != cxfsdn
    && ERR_FD != CXFSDN_SATA_DISK_FD(cxfsdn))
    {
        UINT32      offset;
        UINT32      wsize;
        UINT8      *mem_cache;

        offset    = CXFSDN_OFFSET(cxfsdn);
        wsize     = CXFSDN_SIZE(cxfsdn);
        mem_cache = CXFSDN_MEM_CACHE(cxfsdn);

        dbg_log(SEC_0191_CXFSDN, 0)(LOGSTDOUT, "[DEBUG] cxfsdn_flush: [1]"
                                               "disk num %u, disk max num %u\n",
                                               CXFSPGV_DISK_NUM(CXFSDN_CXFSPGV(cxfsdn)),
                                               CXFSPGV_DISK_MAX_NUM(CXFSDN_CXFSPGV(cxfsdn)));

        if(EC_FALSE == c_file_pwrite(CXFSDN_SATA_DISK_FD(cxfsdn), &offset, wsize, mem_cache))
        {
            dbg_log(SEC_0191_CXFSDN, 0)(LOGSTDOUT, "error:cxfsdn_flush: "
                                                   "flush %ld bytes to offset %ld failed\n",
                                                   wsize, CXFSDN_OFFSET(cxfsdn));
            return (EC_FALSE);
        }

        dbg_log(SEC_0191_CXFSDN, 9)(LOGSTDOUT, "[DEBUG] cxfsdn_flush: "
                                               "flush %ld bytes to offset %ld done\n",
                                               wsize, CXFSDN_OFFSET(cxfsdn));
    }

    if(SWITCH_ON == CXFS_DN_MMAP_SWITCH
    && NULL_PTR != cxfsdn
    && ERR_FD != CXFSDN_SATA_DISK_FD(cxfsdn))
    {
        UINT32      wsize;
        UINT8      *mem_cache;

        wsize     = CXFSDN_SIZE(cxfsdn);
        mem_cache = CXFSDN_MEM_CACHE(cxfsdn);

        dbg_log(SEC_0191_CXFSDN, 0)(LOGSTDOUT, "[DEBUG] cxfsdn_flush: [1]"
                                               "disk num %u, disk max num %u\n",
                                               CXFSPGV_DISK_NUM(CXFSDN_CXFSPGV(cxfsdn)),
                                               CXFSPGV_DISK_MAX_NUM(CXFSDN_CXFSPGV(cxfsdn)));

        if(0 != msync(mem_cache, wsize, MS_SYNC))
        {
            dbg_log(SEC_0191_CXFSDN, 0)(LOGSTDOUT, "warn:cxfsdn_flush: "
                                                   "sync dn with size %ld failed\n",
                                                   wsize);
        }
        else
        {
            dbg_log(SEC_0191_CXFSDN, 0)(LOGSTDOUT, "[DEBUG] cxfsdn_flush: "
                                                   "sync dn with size %ld done\n",
                                                   wsize);
        }
    }
    return (EC_TRUE);
}

EC_BOOL cxfsdn_load(CXFSDN *cxfsdn, const CXFSCFG *cxfscfg,
                       const int cxfsdn_sata_fd,
                       const UINT32 cxfsdn_mem_size,
                       const int cxfsdn_ssd_fd)
{
    CXFSPGV    *cxfspgv;

    UINT32      dn_offset;
    UINT32      dn_mem_size;
    UINT32      dn_mem_align;
    UINT8      *dn_mem_cache;

    if(NULL_PTR == cxfsdn)
    {
        dbg_log(SEC_0191_CXFSDN, 0)(LOGSTDOUT, "error:cxfsdn_load: cxfsdn is null\n");
        return (EC_FALSE);
    }

    if(NULL_PTR != CXFSDN_CXFSPGV(cxfsdn))
    {
        dbg_log(SEC_0191_CXFSDN, 0)(LOGSTDOUT, "error:cxfsdn_load: vol is not null\n");
        return (EC_FALSE);
    }

    dn_mem_size  = CXFSCFG_DN_E_OFFSET(cxfscfg) - CXFSCFG_DN_S_OFFSET(cxfscfg);
    dn_mem_align = CXFSDN_MEM_ALIGNMENT;
    ASSERT(0 == (dn_mem_size & (CXFSDN_MEM_ALIGNMENT - 1)));

    if(SWITCH_OFF == CXFS_DN_MMAP_SWITCH)
    {
        dn_mem_cache = c_memalign_new(dn_mem_size, dn_mem_align);
        if(NULL_PTR == dn_mem_cache)
        {
            dbg_log(SEC_0191_CXFSDN, 0)(LOGSTDOUT, "error:cxfsdn_load: "
                                                   "alloc %ld bytes with alignment %ld failed\n",
                                                   dn_mem_size, dn_mem_align);
            return (EC_FALSE);
        }

        dn_offset = CXFSCFG_DN_S_OFFSET(cxfscfg);
        if(EC_FALSE == c_file_pread(cxfsdn_sata_fd, &dn_offset, dn_mem_size, dn_mem_cache))
        {
            dbg_log(SEC_0191_CXFSDN, 0)(LOGSTDOUT, "error:cxfsdn_load: "
                                                   "load %ld bytes from offset %ld failed\n",
                                                   dn_mem_size,
                                                   CXFSCFG_DN_S_OFFSET(cxfscfg));
            c_memalign_free(dn_mem_cache);
            return (EC_FALSE);
        }

        cxfspgv = cxfspgv_open(dn_mem_cache, cxfscfg);
        if(NULL_PTR == cxfspgv)
        {
            dbg_log(SEC_0191_CXFSDN, 0)(LOGSTDOUT, "error:cxfsdn_load: load/open vol failed\n");
            c_memalign_free(dn_mem_cache);
            return (EC_FALSE);
        }
    }

    if(SWITCH_ON == CXFS_DN_MMAP_SWITCH)
    {
        UINT8   *addr;

        addr = c_mmap_aligned_addr(dn_mem_size, dn_mem_align);
        if(NULL_PTR == addr)
        {
            dbg_log(SEC_0191_CXFSDN, 0)(LOGSTDOUT, "error:cxfsdn_load: "
                                                   "fetch mmap aligned addr of size %ld align %ld failed\n",
                                                   dn_mem_size, dn_mem_align);
            return (EC_FALSE);
        }

        dn_offset = CXFSCFG_DN_S_OFFSET(cxfscfg);
        dn_mem_cache = (UINT8 *)mmap(addr, dn_mem_size,
                                     PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FIXED,
                                     cxfsdn_sata_fd, dn_offset);
        if(MAP_FAILED == dn_mem_cache)
        {
            dbg_log(SEC_0191_CXFSDN, 0)(LOGSTDOUT, "error:cxfsdn_load: "
                               "mmap fd %d offset %ld size %ld, failed, errno = %d, errstr = %s\n",
                               cxfsdn_sata_fd, dn_offset, dn_mem_size, errno, strerror(errno));
            return (EC_FALSE);
        }

        cxfspgv = cxfspgv_open(dn_mem_cache, cxfscfg);
        if(NULL_PTR == cxfspgv)
        {
            dbg_log(SEC_0191_CXFSDN, 0)(LOGSTDOUT, "error:cxfsdn_load: load/open vol failed\n");
            munmap(dn_mem_cache, dn_mem_size);
            return (EC_FALSE);
        }
    }

    CXFSDN_SATA_DISK_FD(cxfsdn) = cxfsdn_sata_fd;
    CXFSDN_OFFSET(cxfsdn)       = CXFSPGV_OFFSET(cxfspgv);
    CXFSDN_SIZE(cxfsdn)         = CXFSPGV_FSIZE(cxfspgv);
    CXFSDN_CXFSPGV(cxfsdn)      = cxfspgv;
    CXFSDN_MEM_CACHE(cxfsdn)    = dn_mem_cache;

    dbg_log(SEC_0191_CXFSDN, 9)(LOGSTDOUT, "[DEBUG] cxfsdn_load: load/open vol done\n");

    if(SWITCH_ON == CXFSDN_CAMD_SWITCH)
    {
        CAMD_MD     *camd_md;

        UINT32       sata_disk_size;
        UINT32       mem_disk_size;
        UINT32       ssd_disk_offset;
        UINT32       ssd_disk_size;
        int          ssd_disk_fd;
        int          sata_disk_fd;

        sata_disk_fd    = cxfsdn_sata_fd;
        sata_disk_size  = CXFSCFG_SATA_DISK_SIZE(cxfscfg) - CXFSCFG_DN_S_OFFSET(cxfscfg);

        mem_disk_size   = cxfsdn_mem_size;

        ssd_disk_fd     = cxfsdn_ssd_fd;
        ssd_disk_offset = CXFSCFG_SSD_DISK_OFFSET(cxfscfg);
        ssd_disk_size   = CXFSCFG_SSD_DISK_SIZE(cxfscfg);

        camd_md = camd_start(sata_disk_fd, sata_disk_size,
                             mem_disk_size,
                             ssd_disk_fd, ssd_disk_offset, ssd_disk_size);
        if(NULL_PTR == camd_md)
        {
            dbg_log(SEC_0191_CXFSDN, 0)(LOGSTDOUT, "error:cxfsdn_load:start camd failed\n");
            return (EC_FALSE);
        }

        if(EC_FALSE == camd_load(camd_md))
        {
            dbg_log(SEC_0191_CXFSDN, 0)(LOGSTDOUT, "error:cxfsdn_load:load cdc failed\n");
            return (EC_FALSE);
        }

        if(ERR_FD != camd_get_eventfd(camd_md)
        && NULL_PTR != task_brd_default_get_cepoll())
        {
            task_brd_process_add(task_brd_default_get(),
                                (TASK_BRD_CALLBACK)camd_process,
                                (void *)camd_md);

            cepoll_set_event(task_brd_default_get_cepoll(),
                              camd_get_eventfd(camd_md),
                              CEPOLL_RD_EVENT,
                              (const char *)"camd_event_handler",
                              (CEPOLL_EVENT_HANDLER)camd_event_handler,
                              (void *)camd_md);
        }

        CXFSDN_CAMD_MD(cxfsdn)     = camd_md;
        CXFSDN_SSD_DISK_FD(cxfsdn) = cxfsdn_ssd_fd;
    }

    return (EC_TRUE);
}

CXFSDN *cxfsdn_open(const CXFSCFG *cxfscfg, const int cxfsdn_sata_fd, const int cxfsdn_ssd_fd)
{
    CXFSDN *cxfsdn;

    cxfsdn = cxfsdn_new();
    if(NULL_PTR == cxfsdn)
    {
        dbg_log(SEC_0191_CXFSDN, 0)(LOGSTDOUT, "error:cxfsdn_open: new cxfsdn failed\n");
        return (NULL_PTR);
    }

    if(EC_FALSE == cxfsdn_load(cxfsdn, cxfscfg,
                               cxfsdn_sata_fd,
                               CXFSDN_CAMD_MEM_DISK_SIZE,
                               cxfsdn_ssd_fd))
    {
        dbg_log(SEC_0191_CXFSDN, 0)(LOGSTDOUT, "error:cxfsdn_open: load cxfsdn failed\n");
        cxfsdn_free(cxfsdn);
        return (NULL_PTR);
    }

    dbg_log(SEC_0191_CXFSDN, 9)(LOGSTDOUT, "[DEBUG] cxfsdn_open: load cxfsdn done\n");

    return (cxfsdn);
}

EC_BOOL cxfsdn_close(CXFSDN *cxfsdn)
{
    if(NULL_PTR != cxfsdn)
    {
        if(SWITCH_ON == CXFSDN_CAMD_SWITCH)
        {
            CAMD_MD     *camd_md;

            camd_md = CXFSDN_CAMD_MD(cxfsdn);

            if(NULL_PTR != camd_md
            && ERR_FD != camd_get_eventfd(camd_md)
            && NULL_PTR != task_brd_default_get_cepoll())
            {
                task_brd_process_del(task_brd_default_get(),
                                    (TASK_BRD_CALLBACK)camd_process,
                                    (void *)camd_md);

                cepoll_del_event(task_brd_default_get_cepoll(),
                                  camd_get_eventfd(camd_md),
                                  CEPOLL_RD_EVENT);
            }
        }

        cxfsdn_flush(cxfsdn);
        cxfsdn_free(cxfsdn);
    }
    return (EC_TRUE);
}

/*random access for reading, the offset is for the whole 64M page-block */
EC_BOOL cxfsdn_read_o(CXFSDN *cxfsdn, const uint16_t disk_no, const uint16_t block_no, const UINT32 offset, const UINT32 data_max_len, UINT8 *data_buff, UINT32 *data_len)
{
    UINT32 node_id;
    UINT32 offset_t;

    if(NULL_PTR == cxfsdn)
    {
        dbg_log(SEC_0191_CXFSDN, 0)(LOGSTDOUT, "error:cxfsdn_read_o: cxfsdn is null\n");
        return (EC_FALSE);
    }

    if(NULL_PTR == data_buff)
    {
        dbg_log(SEC_0191_CXFSDN, 0)(LOGSTDOUT, "error:cxfsdn_read_o: data_buff is null\n");
        return (EC_FALSE);
    }

    if(CXFSPGB_CACHE_MAX_BYTE_SIZE < data_max_len)
    {
        dbg_log(SEC_0191_CXFSDN, 0)(LOGSTDOUT, "error:cxfsdn_read_o: data max len %ld overflow\n", data_max_len);
        return (EC_FALSE);
    }

    if(CXFSPGB_CACHE_MAX_BYTE_SIZE <= offset)
    {
        dbg_log(SEC_0191_CXFSDN, 0)(LOGSTDOUT, "error:cxfsdn_read_o: offset %ld overflow\n", offset);
        return (EC_FALSE);
    }

    if(CXFSPGB_CACHE_MAX_BYTE_SIZE < offset + data_max_len)
    {
        dbg_log(SEC_0191_CXFSDN, 0)(LOGSTDOUT, "error:cxfsdn_read_o: offset %ld + data_max_len %ld = %ld overflow\n",
                            offset, data_max_len, offset + data_max_len);
        return (EC_FALSE);
    }

    node_id  = CXFSDN_NODE_ID_MAKE(disk_no, block_no);
    offset_t = offset;

    if(EC_FALSE == cxfsdn_node_read(cxfsdn, node_id, data_max_len, data_buff, &offset_t))
    {
        dbg_log(SEC_0191_CXFSDN, 0)(LOGSTDOUT, "error:cxfsdn_read_o: read %ld bytes at offset %ld from node %ld failed\n",
                           data_max_len, offset, node_id);
        return (EC_FALSE);
    }

    if(NULL_PTR != data_len)
    {
        (*data_len) = offset_t - offset;
    }

    return (EC_TRUE);
}

/*random access for writting */
/*offset: IN/OUT, the offset is for the whole 64M page-block*/
EC_BOOL cxfsdn_write_o(CXFSDN *cxfsdn, const UINT32 data_max_len, const UINT8 *data_buff, const uint16_t disk_no, const uint16_t block_no, UINT32 *offset)
{
    UINT32 node_id;
    UINT32 offset_t;

    if(NULL_PTR == cxfsdn)
    {
        dbg_log(SEC_0191_CXFSDN, 0)(LOGSTDOUT, "error:cxfsdn_write_o: cxfsdn is null\n");
        return (EC_FALSE);
    }

    if(NULL_PTR == data_buff)
    {
        dbg_log(SEC_0191_CXFSDN, 0)(LOGSTDOUT, "error:cxfsdn_write_o: data_buff is null\n");
        return (EC_FALSE);
    }

    if(CXFSPGB_CACHE_MAX_BYTE_SIZE < data_max_len)
    {
        dbg_log(SEC_0191_CXFSDN, 0)(LOGSTDOUT, "error:cxfsdn_write_o: data max len %ld overflow\n", data_max_len);
        return (EC_FALSE);
    }

    node_id  = CXFSDN_NODE_ID_MAKE(disk_no, block_no);
    offset_t = (*offset);

    if(EC_FALSE == cxfsdn_node_write(cxfsdn, node_id, data_max_len, data_buff, offset))
    {
        dbg_log(SEC_0191_CXFSDN, 0)(LOGSTDOUT, "error:cxfsdn_write_o: write %ld bytes to disk %u block %u offset %ld failed\n",
                            data_max_len, disk_no, block_no, offset_t);

        return (EC_FALSE);
    }

    //dbg_log(SEC_0191_CXFSDN, 9)(LOGSTDOUT, "[DEBUG] cxfsdn_write_o: write %ld bytes to disk %u block %u offset %ld done\n",
    //                    data_max_len, disk_no, block_no, offset_t);

    return (EC_TRUE);
}

EC_BOOL cxfsdn_read_b(CXFSDN *cxfsdn, const uint16_t disk_no, const uint16_t block_no, const uint16_t page_no, const UINT32 data_max_len, UINT8 *data_buff, UINT32 *data_len)
{
    UINT32 offset;

    if(NULL_PTR == cxfsdn)
    {
        dbg_log(SEC_0191_CXFSDN, 0)(LOGSTDOUT, "error:cxfsdn_read_b: cxfsdn is null\n");
        return (EC_FALSE);
    }

    if(NULL_PTR == data_buff)
    {
        dbg_log(SEC_0191_CXFSDN, 0)(LOGSTDOUT, "error:cxfsdn_read_b: data_buff is null\n");
        return (EC_FALSE);
    }

    if(CXFSPGB_CACHE_MAX_BYTE_SIZE < data_max_len)
    {
        dbg_log(SEC_0191_CXFSDN, 0)(LOGSTDOUT, "error:cxfsdn_read_b: data max len %ld overflow\n", data_max_len);
        return (EC_FALSE);
    }

    ASSERT(0 == page_no);

    offset  = (((UINT32)page_no) << (CXFSPGB_PAGE_BIT_SIZE));
    if(EC_FALSE == cxfsdn_read_o(cxfsdn, disk_no, block_no, offset, data_max_len, data_buff, data_len))
    {
        dbg_log(SEC_0191_CXFSDN, 0)(LOGSTDOUT, "error:cxfsdn_read_b: read %ld bytes from disk %u block %u page %u failed\n",
                           data_max_len, disk_no, block_no, page_no);
        return (EC_FALSE);
    }
    return (EC_TRUE);
}

EC_BOOL cxfsdn_write_b(CXFSDN *cxfsdn, const UINT32 data_max_len, const UINT8 *data_buff, uint16_t *disk_no, uint16_t *block_no, UINT32 *offset)
{
    uint32_t size;
    uint16_t page_no;

    if(NULL_PTR == cxfsdn)
    {
        dbg_log(SEC_0191_CXFSDN, 0)(LOGSTDOUT, "error:cxfsdn_write_b: cxfsdn is null\n");
        return (EC_FALSE);
    }

    if(NULL_PTR == data_buff)
    {
        dbg_log(SEC_0191_CXFSDN, 0)(LOGSTDOUT, "error:cxfsdn_write_b: data_buff is null\n");
        return (EC_FALSE);
    }

    if(CXFSPGB_CACHE_MAX_BYTE_SIZE < data_max_len + (*offset))
    {
        dbg_log(SEC_0191_CXFSDN, 0)(LOGSTDOUT, "error:cxfsdn_write_b: data max len %ld + offset %ld = %ld overflow\n",
                           data_max_len, (*offset), data_max_len + (*offset));
        return (EC_FALSE);
    }

    size = CXFSPGB_CACHE_MAX_BYTE_SIZE;

    if(EC_FALSE == cxfspgv_new_space(CXFSDN_CXFSPGV(cxfsdn), size, disk_no, block_no, &page_no))
    {
        dbg_log(SEC_0191_CXFSDN, 0)(LOGSTDOUT, "error:cxfsdn_write_b: new %ld bytes space from vol failed\n", data_max_len);
        return (EC_FALSE);
    }
    ASSERT(0 == page_no);

    if(EC_FALSE == cxfsdn_write_o(cxfsdn, data_max_len, data_buff, *disk_no, *block_no, offset))
    {
        dbg_log(SEC_0191_CXFSDN, 0)(LOGSTDOUT, "error:cxfsdn_write_b: write %ld bytes to disk %u block %u page %u failed\n",
                            data_max_len, (*disk_no), (*block_no), (page_no));

        cxfspgv_free_space(CXFSDN_CXFSPGV(cxfsdn), *disk_no, *block_no, page_no, size);
        return (EC_FALSE);
    }
    dbg_log(SEC_0191_CXFSDN, 9)(LOGSTDOUT, "[DEBUG] cxfsdn_write_b: write %ld bytes to disk %u block %u page %u done\n",
                        data_max_len, (*disk_no), (*block_no), (page_no));

    return (EC_TRUE);
}

EC_BOOL cxfsdn_update_b(CXFSDN *cxfsdn, const UINT32 data_max_len, const UINT8 *data_buff, const uint16_t disk_no, const uint16_t block_no, UINT32 *offset)
{
    if(NULL_PTR == cxfsdn)
    {
        dbg_log(SEC_0191_CXFSDN, 0)(LOGSTDOUT, "error:cxfsdn_update_b: cxfsdn is null\n");
        return (EC_FALSE);
    }

    if(NULL_PTR == data_buff)
    {
        dbg_log(SEC_0191_CXFSDN, 0)(LOGSTDOUT, "error:cxfsdn_update_b: data_buff is null\n");
        return (EC_FALSE);
    }

    if(CXFSPGB_CACHE_MAX_BYTE_SIZE < data_max_len + (*offset))
    {
        dbg_log(SEC_0191_CXFSDN, 0)(LOGSTDOUT, "error:cxfsdn_update_b: data max len %ld + offset %ld = %ld overflow\n",
                           data_max_len, (*offset), data_max_len + (*offset));
        return (EC_FALSE);
    }

    if(EC_FALSE == cxfsdn_write_o(cxfsdn, data_max_len, data_buff, disk_no, block_no, offset))
    {
        dbg_log(SEC_0191_CXFSDN, 0)(LOGSTDOUT, "error:cxfsdn_update_b: write %ld bytes to disk %u block %u failed\n",
                            data_max_len, disk_no, block_no);
        return (EC_FALSE);
    }

    dbg_log(SEC_0191_CXFSDN, 9)(LOGSTDOUT, "[DEBUG] cxfsdn_update_b: write %ld bytes to disk %u block %u done\n",
                        data_max_len, disk_no, block_no);

    return (EC_TRUE);
}

EC_BOOL cxfsdn_read_p(CXFSDN *cxfsdn, const uint16_t disk_no, const uint16_t block_no, const uint16_t page_no, const UINT32 data_max_len, UINT8 *data_buff, UINT32 *data_len)
{
    UINT32 offset;

    if(NULL_PTR == cxfsdn)
    {
        dbg_log(SEC_0191_CXFSDN, 0)(LOGSTDOUT, "error:cxfsdn_read_p: cxfsdn is null\n");
        return (EC_FALSE);
    }

    if(NULL_PTR == data_buff)
    {
        dbg_log(SEC_0191_CXFSDN, 0)(LOGSTDOUT, "error:cxfsdn_read_p: data_buff is null\n");
        return (EC_FALSE);
    }

    if(CXFSPGB_CACHE_MAX_BYTE_SIZE < data_max_len)
    {
        dbg_log(SEC_0191_CXFSDN, 0)(LOGSTDOUT, "error:cxfsdn_read_p: data max len %ld overflow\n", data_max_len);
        return (EC_FALSE);
    }

    offset  = (((UINT32)page_no) << (CXFSPGB_PAGE_BIT_SIZE));
    //dbg_log(SEC_0191_CXFSDN, 9)(LOGSTDOUT, "[DEBUG] cxfsdn_read_p: disk %u, block %u, page %u ==> offset %ld\n", disk_no, block_no, page_no, offset);
    if(EC_FALSE == cxfsdn_read_o(cxfsdn, disk_no, block_no, offset, data_max_len, data_buff, data_len))
    {
        dbg_log(SEC_0191_CXFSDN, 0)(LOGSTDOUT, "error:cxfsdn_read_p: read %ld bytes from disk %u block %u page %u failed\n",
                           data_max_len, disk_no, block_no, page_no);
        return (EC_FALSE);
    }
    return (EC_TRUE);
}

EC_BOOL cxfsdn_write_p(CXFSDN *cxfsdn, const UINT32 data_max_len, const UINT8 *data_buff, uint16_t *disk_no, uint16_t *block_no, uint16_t *page_no)
{
    UINT32   offset;
    uint32_t size;

    if(NULL_PTR == cxfsdn)
    {
        dbg_log(SEC_0191_CXFSDN, 0)(LOGSTDOUT, "error:cxfsdn_write_p: cxfsdn is null\n");
        return (EC_FALSE);
    }

    if(NULL_PTR == data_buff)
    {
        dbg_log(SEC_0191_CXFSDN, 0)(LOGSTDOUT, "error:cxfsdn_write_p: data_buff is null\n");
        return (EC_FALSE);
    }

    if(CXFSPGB_CACHE_MAX_BYTE_SIZE < data_max_len)
    {
        dbg_log(SEC_0191_CXFSDN, 0)(LOGSTDOUT, "error:cxfsdn_write_p: data max len %ld overflow\n", data_max_len);
        return (EC_FALSE);
    }

    size = (uint32_t)(data_max_len);

    if(EC_FALSE == cxfspgv_new_space(CXFSDN_CXFSPGV(cxfsdn), size, disk_no, block_no,  page_no))
    {
        dbg_log(SEC_0191_CXFSDN, 0)(LOGSTDOUT, "error:cxfsdn_write_p: new %ld bytes space from vol failed\n", data_max_len);
        return (EC_FALSE);
    }

    offset  = (((UINT32)(*page_no)) << (CXFSPGB_PAGE_BIT_SIZE));

    if(EC_FALSE == cxfsdn_write_o(cxfsdn, data_max_len, data_buff, *disk_no, *block_no, &offset))
    {
        dbg_log(SEC_0191_CXFSDN, 0)(LOGSTDOUT, "error:cxfsdn_write_p: write %ld bytes to disk %u block %u page %u failed\n",
                            data_max_len, (*disk_no), (*block_no), (*page_no));

        cxfspgv_free_space(CXFSDN_CXFSPGV(cxfsdn), *disk_no, *block_no, *page_no, size);
        return (EC_FALSE);
    }
    dbg_log(SEC_0191_CXFSDN, 9)(LOGSTDOUT, "[DEBUG] cxfsdn_write_p: write %ld bytes to disk %u block %u page %u done\n",
                        data_max_len, (*disk_no), (*block_no), (*page_no));

    return (EC_TRUE);
}

/*random access for reading, the offset is for the user file but not for the whole 64M page-block */
EC_BOOL cxfsdn_read_e(CXFSDN *cxfsdn, const uint16_t disk_no, const uint16_t block_no, const uint16_t page_no, const uint32_t offset, const UINT32 data_max_len, UINT8 *data_buff, UINT32 *data_len)
{
    UINT32 offset_t;

    offset_t  = (((UINT32)page_no) << (CXFSPGB_PAGE_BIT_SIZE)) + offset;
    if(EC_FALSE == cxfsdn_read_o(cxfsdn, disk_no, block_no, offset_t, data_max_len, data_buff, data_len))
    {
        dbg_log(SEC_0191_CXFSDN, 0)(LOGSTDOUT, "error:cxfsdn_read_e: read %ld bytes from disk %u block %u page %u offset %u failed\n",
                           data_max_len, disk_no, block_no, page_no, offset);
        return (EC_FALSE);
    }
    return (EC_TRUE);
}

/*random access for writting, the offset is for the user file but not for the whole 64M page-block */
EC_BOOL cxfsdn_write_e(CXFSDN *cxfsdn, const UINT32 data_max_len, const UINT8 *data_buff, const uint16_t disk_no, const uint16_t block_no, const uint16_t page_no, const uint32_t offset)
{
    UINT32 offset_t;

    offset_t  = (((UINT32)page_no) << (CXFSPGB_PAGE_BIT_SIZE)) + offset;
    if(EC_FALSE == cxfsdn_write_o(cxfsdn, data_max_len, data_buff, disk_no, block_no, &offset_t))
    {
        dbg_log(SEC_0191_CXFSDN, 0)(LOGSTDOUT, "error:cxfsdn_write_e: write %ld bytes to disk %u block %u page %u offset %ld failed\n",
                           data_max_len, disk_no, block_no, page_no, offset_t);
        return (EC_FALSE);
    }
    return (EC_TRUE);
}

EC_BOOL cxfsdn_remove(CXFSDN *cxfsdn, const uint16_t disk_no, const uint16_t block_no, const uint16_t page_no, const UINT32 data_max_len)
{
    uint32_t size;

    if(NULL_PTR == cxfsdn)
    {
        dbg_log(SEC_0191_CXFSDN, 0)(LOGSTDOUT, "error:cxfsdn_remove: cxfsdn is null\n");
        return (EC_FALSE);
    }

    if(CXFSPGB_CACHE_MAX_BYTE_SIZE < data_max_len)
    {
        dbg_log(SEC_0191_CXFSDN, 0)(LOGSTDOUT, "error:cxfsdn_remove: data max len %ld overflow\n", data_max_len);
        return (EC_FALSE);
    }

    size = (uint32_t)(data_max_len);

    if(EC_FALSE == cxfspgv_free_space(CXFSDN_CXFSPGV(cxfsdn), disk_no, block_no, page_no, size))
    {
        dbg_log(SEC_0191_CXFSDN, 0)(LOGSTDOUT, "error:cxfsdn_remove: free %ld bytes space to vol failed\n", data_max_len);
        return (EC_FALSE);
    }
    dbg_log(SEC_0191_CXFSDN, 9)(LOGSTDOUT, "[DEBUG] cxfsdn_remove: free %ld bytes to disk %u block %u page %u done\n",
                        data_max_len, disk_no, block_no, page_no);

    return (EC_TRUE);
}

#ifdef __cplusplus
}
#endif/*__cplusplus*/

