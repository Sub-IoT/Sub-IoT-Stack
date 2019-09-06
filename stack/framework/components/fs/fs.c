/*! \file fs.c
 *

 *  \copyright (C) Copyright 2019 University of Antwerp and others (http://mosaic-lopow.github.io/dash7-ap-open-source-stack/)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

/*! \file fs.c
 * \addtogroup Fs
 * \ingroup framework
 * @{
 * \brief Filesystem APIs used as an abstraction level for underneath blockdevice driver
 * \author	philippe.nunes@cortus.com
 */

#include <string.h>

#include "framework_defs.h"
#include "debug.h"
#include "log.h"
#include "fs.h"
#include "errors.h"
#include "platform.h"
#include "hwblockdevice.h"

#if defined(FRAMEWORK_LOG_ENABLED) && defined(FRAMEWORK_FS_LOG_ENABLED)
  #define DPRINT(...) log_print_string( __VA_ARGS__)
#else
  #define DPRINT(...)
#endif

#define FS_STORAGE_CLASS_NUMOF       (FS_STORAGE_PERMANENT + 1)


static fs_file_t files[FRAMEWORK_FS_FILE_COUNT] = { 0 };

static bool is_fs_init_completed = false;  //set in _d7a_verify_magic()

#define IS_SYSTEM_FILE(file_id)         (file_id <= 0x3F)

static fs_modified_file_callback_t file_modified_callbacks[FRAMEWORK_FS_FILE_COUNT] = { NULL };

#ifdef VFS_MODULE
#include "vfs.h"

#define MAX_FILE_NAME           32          /* mount name included */

/* ALLOW_FORMAT:
 *  0: the fs must be mountable and valid, else error
 *     it reduces the code size by 3,2Kbytes, but might fail on boot
 *  1: the fs is recreated when not moutable or not valid.
 *
 * FORCE_REFORMAT: if (ALLOW_FORMAT==1)
 *  when the fs is moutable but the magic nok,
 *  0: just overwrite the files
 *  1: erase and reformat the whole fs.
 *     cleaner but old uninited files will be lost.
 */
#define ALLOW_FORMAT          1
#define FORCE_REFORMAT        1 && ALLOW_FORMAT

///////////////////////////////////////
// The d7a file header is stored in file.
// filename is $mount/d7f.$id
// filebody is [$header][$data]
// where $mount=/|/tmp
//       length(header) = 12
//       length($data) == $header.length == $header.allocated_length
///////////////////////////////////////
static const vfs_mount_t* const d7a_fs[] = {
        [FS_STORAGE_TRANSIENT]  = &ARCH_VFS_STORAGE_TRANSIENT,
        [FS_STORAGE_VOLATILE]   = &ARCH_VFS_STORAGE_VOLATILE,
        [FS_STORAGE_RESTORABLE] = &ARCH_VFS_STORAGE_RESTORABLE,
        [FS_STORAGE_PERMANENT]  = &ARCH_VFS_STORAGE_PERMANENT
};
#define FS_STORAGE_CLASS_NUMOF       (FS_STORAGE_PERMANENT+1)
#define FS_STORAGE_CLASS_NUMOF_CHECK (sizeof(d7a_fs)/sizeof(d7a_fs[0]))
#else

static uint32_t volatile_data_offset = 0;
static uint32_t permanent_data_offset = FS_FILES_DATA_OFFSET;

static blockdevice_t* bd[FS_STORAGE_CLASS_NUMOF];

#endif

/* forward internal declarations */
#ifdef VFS_MODULE
static const char * _file_mount_point(uint8_t file_id, const fs_file_header_t* file_header);
static int _get_file_name(char* file_name, uint8_t file_id, const fs_file_header_t* file_header);
static int _create_file_name(char* file_name, uint8_t file_id, fs_storage_class_t storage_class);
#endif

static int _fs_init_permanent_systemfiles(fs_filesystem_t* permanent_systemfiles);
static int _fs_create_magic(fs_storage_class_t storage_class);
static int _fs_verify_magic(fs_storage_class_t storage_class, uint8_t* magic_number);
static int _fs_create_file(uint8_t file_id, fs_storage_class_t storage_class, const uint8_t* initial_data, uint32_t length);

static inline bool _is_file_defined(uint8_t file_id)
{
    //return files[file_id].storage == FS_STORAGE_INVALID;
    return files[file_id].length != 0;
}

static inline uint32_t _get_file_header_address(uint8_t file_id)
{
    return FS_FILE_HEADERS_ADDRESS + (file_id * FS_FILE_HEADER_SIZE);
}

void fs_init(fs_filesystem_t* provisioning)
{
    if (is_fs_init_completed)
        return /*0*/;

    memset(files,0,sizeof(files));

#ifdef VFS_MODULE
    //mount permanent and transient storages
    int res=0;
    for (int i=0; i < FS_STORAGE_CLASS_NUMOF; i++) {
        res = vfs_mount((vfs_mount_t*)d7a_fs[i]);
        if (!(res == 0 || res==-EBUSY)) {
#if (ALLOW_FORMAT==1)
            DPRINT("Error: fs[%d] mount failed (%d). trying formatn",i,res);
            if ((res = vfs_format((vfs_mount_t*)d7a_fs[i]))!=0)
            {
                DPRINT("Error: fs[%d] format failed (%d)\n",i,res);
                /* no return. (re)try mount before failing */
            }
#endif
            if ((res = vfs_mount((vfs_mount_t*)d7a_fs[i])) != 0)
            {
                LOG_ERROR("D7A fs_init ERROR: fs[%d] mount failed (%d)",i,res);
                return /*-1*/;
            }
        }
        DPRINT("D7A fs[%d] mounted",i);
    }
#endif

#ifndef VFS_MODULE
    // initialise the blockdevice driver according platform specificities
    // for now, only permanent and volatile storage are supported
    bd[FS_STORAGE_PERMANENT] = PLATFORM_PERMANENT_BD;
    bd[FS_STORAGE_VOLATILE] = PLATFORM_VOLATILE_BD;
#endif

    if (provisioning)
        _fs_init_permanent_systemfiles(provisioning);

    is_fs_init_completed = true;
    DPRINT("fs_init OK");
}

int _fs_init_permanent_systemfiles(fs_filesystem_t* permanent_systemfiles)
{
    uint8_t expected_magic_number[FS_MAGIC_NUMBER_SIZE] = FS_MAGIC_NUMBER;
    if (_fs_verify_magic(FS_STORAGE_PERMANENT, expected_magic_number) < 0)
    {
        DPRINT("fs_init: no valid magic, recreating fs...");

#ifdef VFS_MODULE

#if (FORCE_REFORMAT == 1)
        DPRINT("init_permanent_systemfiles: force format\n");
        int res;
        if ((res = vfs_umount((vfs_mount_t*)d7a_fs[FS_STORAGE_PERMANENT])) != 0)
        {
            DPRINT("init_permanent_systemfiles: Oops, umount failed (%d)", res);
        }
        if ((res = vfs_format((vfs_mount_t*)d7a_fs[FS_STORAGE_PERMANENT])) != 0)
        {
            DPRINT("init_permanent_systemfiles: Oops, format failed (%d)", res);
            return res;
        }
        if ((res = vfs_mount((vfs_mount_t*)d7a_fs[FS_STORAGE_PERMANENT]))!=0)
        {
            DPRINT("init_permanent_systemfiles: Oops, remount failed (%d)",res);
            return res;
        }
        DPRINT("init_permanent_systemfiles: force format complete");
#endif

#endif

        _fs_create_magic(FS_STORAGE_PERMANENT);
   }

#ifdef VFS_MODULE
    for (int file_id = 0; file_id < permanent_systemfiles->metadata.nfiles; file_id++)
    {
        _fs_create_file(file_id, permanent_systemfiles->files_data + permanent_systemfiles->file_headers[file_id].addr,
                        permanent_systemfiles->file_headers[file_id].length);
    }
#else
    DPRINT("metadata addr: %x", permanent_systemfiles->metadata);
    DPRINT("metadata nfiles: %x", permanent_systemfiles->metadata.nfiles);
    // initialise system file caching
    size_t number_of_files;
    blockdevice_read(bd[FS_STORAGE_PERMANENT], (uint8_t*)&number_of_files, FS_NUMBER_OF_FILES_ADDRESS, FS_NUMBER_OF_FILES_SIZE);
    assert(number_of_files < FRAMEWORK_FS_FILE_COUNT);
    for(int file_id = 0; file_id < number_of_files; file_id++)
    {
        blockdevice_read(bd[FS_STORAGE_PERMANENT], (uint8_t*)&files[file_id],
                         _get_file_header_address(file_id), FS_FILE_HEADER_SIZE);
        if(_is_file_defined(file_id))
        {
            switch(files[file_id].storage)
            {
                case FS_STORAGE_VOLATILE:
                {
                    //copy defaults from permanent storage to volatile
                    uint8_t data = 0x00;
                    for(int i=0; i < files[file_id].length; i++)
                    {
                        blockdevice_read(bd[FS_STORAGE_PERMANENT], &data, files[file_id].addr + i, 1);
                        blockdevice_program(bd[FS_STORAGE_VOLATILE], &data, volatile_data_offset + i, 1);
    
                    }
                    // update file header
                    files[file_id].addr = volatile_data_offset;
                    volatile_data_offset += files[file_id].length;
                    break;
                }
                case FS_STORAGE_PERMANENT:
                {
                    permanent_data_offset += files[file_id].length;
                    break;
                }
            }
        }
    }
#endif
    return 0;
}

//TODO: CRC MAGIC
static int _fs_create_magic(fs_storage_class_t storage_class)
{
    assert(!is_fs_init_completed);
    uint8_t magic[] = FS_MAGIC_NUMBER;

#ifdef VFS_MODULE
    char fn[MAX_FILE_NAME];
    int rtc;
    int fd;

    snprintf(fn, MAX_FILE_NAME,   "%s/d7f.magic",
             (d7a_fs[storage_class])->mount_point);

    if ((fd = vfs_open(fn, O_CREAT | O_RDWR, 0)) < 0)
    {
        DPRINT("Error opening file magic for create (%s)",fn);
        return -ENOENT;
    }

    /* write the magic */
    rtc = vfs_write(fd, magic, FS_MAGIC_NUMBER_SIZE);
    if ( (rtc < 0) || ((unsigned)rtc < FS_MAGIC_NUMBER_SIZE) )
    {
        vfs_close(fd);
        DPRINT("Error writing file magic (%d)",rtc);
        return rtc;
    }
    vfs_close(fd);
#endif

    /* verify */
    return _fs_verify_magic(storage_class, magic);
}


/* The magic number allows to check filesystem integrity.*/
static int _fs_verify_magic(fs_storage_class_t storage_class, uint8_t* expected_magic_number)
{
    is_fs_init_completed = false;

#ifdef VFS_MODULE
    char fn[MAX_FILE_NAME];
    int rtc;
    int fd;

    snprintf( fn, MAX_FILE_NAME,   "%s/d7f.magic",
            (d7a_fs[storage_class])->mount_point);

    if ((fd = vfs_open(fn, O_RDONLY, 0)) < 0)
    {
        DPRINT("Error opening file magic for reading (%s)",fn);
        return -ENOENT;
    }

    /* read the magic */
    char magic[D7A_FS_MAGIC_LEN];
    memset(magic,0,D7A_FS_MAGIC_LEN);
    rtc = vfs_read(fd, magic, D7A_FS_MAGIC_LEN);
    if ( (rtc < 0) || ((unsigned)rtc < D7A_FS_MAGIC_LEN) )
    {
        vfs_close(fd);
        DPRINT("Error reading file magic (%d) exp:%ld",rtc, D7A_FS_MAGIC_LEN);
        return rtc;
    }
    vfs_close(fd);

    /* compare */
    for (int i = 0; i < (int)D7A_FS_MAGIC_LEN; i++) {
        if (magic[i] != D7A_FS_MAGIC[i]) {
            DPRINT("Error magic[%d] incorrect (%d)", i, magic[i]);
            return -EFAULT;
        }
    }
#else
    uint8_t magic_number[FS_MAGIC_NUMBER_SIZE];
    memset(magic_number,0,FS_MAGIC_NUMBER_SIZE);
    blockdevice_read(bd[storage_class], magic_number, 0, FS_MAGIC_NUMBER_SIZE);
    assert(memcmp(expected_magic_number, magic_number, FS_MAGIC_NUMBER_SIZE) == 0); // if not the FS on EEPROM is not compatible with the current code
#endif

    return 0;
}

#ifdef VFS_MODULE
static int _create_file_name(char* file_name, uint8_t file_id, fs_storage_class_t storage_class)
{
    memset(file_name, 0, MAX_FILE_NAME);

    if (_is_file_defined(file_id)) {
        if (files[file_id].storage != storage_class) {
            //FIXME: this should not happen.
            DPRINT("Oops: somebody's trying to change the storage class.... mv m1 m2");
            assert(false);
        }
    } else {
        files[file_id].storage = storage_class;
    }

    return snprintf( file_name, MAX_FILE_NAME,   "%s/d7f.%u",
                     (d7a_fs[files[file_id].storage])->mount_point,
                     (unsigned)file_id );
}

static int _get_file_name(char* file_name, uint8_t file_id)
{
    memset(file_name, 0, MAX_FILE_NAME);

    if (!_is_file_defined(file_id))
    {
        files[file_id].storage = FS_STORAGE_PERMANENT;
    }

    return snprintf( file_name, MAX_FILE_NAME,   "%s/d7f.%u",
                     (d7a_fs[files[file_id].storage])->mount_point,
                     (unsigned)file_id );
}
#endif

int _fs_create_file(uint8_t file_id, fs_storage_class_t storage_class, const uint8_t* initial_data, uint32_t length)
{
    assert(file_id < FRAMEWORK_FS_FILE_COUNT);

    if (_is_file_defined(file_id))
        return -EEXIST;

    // update file caching for stat lookup
    files[file_id].storage = storage_class;
    files[file_id].length = length;

#ifdef VFS_MODULE
    char fn[MAX_FILE_NAME];
    int rtc;
    int fd;
    if ((rtc = _create_file_name(fn, file_id, storage_class)) <=0) {
        DPRINT("Error creating fileid=%d name (%d)",file_id, rtc);
        return -ENOENT;
    }

    if ((fd = vfs_open(fn, O_CREAT | O_RDWR, 0)) < 0) {
        DPRINT("Error creating fileid=%d (%s)",file_id, fn);
        return -ENOENT;
    }

    if(initial_data != NULL) {
        if ( (rtc = vfs_write(fd, initial_data, length)) != length ) {
            vfs_close(fd);
            DPRINT("Error writing fileid=%d header (%d)",file_id, rtc);
            return rtc;
        }
    }

    vfs_close(fd);
#else
    // only user files can be created
    assert(file_id >= 0x40);

    if (storage_class == FS_STORAGE_PERMANENT)
    {
        files[file_id].addr = permanent_data_offset;
        blockdevice_program(bd[FS_STORAGE_PERMANENT], (uint8_t*)&files[file_id], _get_file_header_address(file_id), FS_FILE_HEADER_SIZE);
        permanent_data_offset += length;
    }
    else
    {
        files[file_id].addr = volatile_data_offset;
        volatile_data_offset += length;
    }

    if(initial_data != NULL) {
        blockdevice_program(bd[storage_class], initial_data, files[file_id].addr, length);
    }
    else{
        // do not use variable length array to limit stack usage, do in chunks instead
        uint8_t default_data[64];
        memset(default_data, 0xff, 64);
        uint32_t remaining_length = length;
        int i = 0;
        while(remaining_length > 64) {
          blockdevice_program(bd[storage_class], default_data, files[file_id].addr + (i * 64), 64);
          remaining_length -= 64;
          i++;
        }

        blockdevice_program(bd[storage_class], default_data, files[file_id].addr  + (i * 64), remaining_length);
    }
#endif
    
    DPRINT("fs init file(file_id %d, storage %d, addr %p, length %d)",file_id, storage_class, files[file_id].addr, length);
    return 0;
}

int fs_init_file(uint8_t file_id, fs_storage_class_t storage_class, const uint8_t* initial_data, uint32_t length)
{
    assert(is_fs_init_completed);
    assert(file_id < FRAMEWORK_FS_FILE_COUNT);
    assert(file_id >= 0x40); // system files may not be inited
    assert(storage_class == FS_STORAGE_VOLATILE || storage_class == FS_STORAGE_PERMANENT); // other options not implemented

    return (_fs_create_file(file_id, storage_class, initial_data, length));
}

int fs_read_file(uint8_t file_id, uint32_t offset, uint8_t* buffer, uint32_t length)
{
    if(!_is_file_defined(file_id)) return -ENOENT;

    if(files[file_id].length < offset + length) return -EINVAL;
#ifdef MODULE_VFS
    char fn[MAX_FILE_NAME];
    int rtc;
    int fd;

    if ((rtc = _get_file_name(fn, file_id, NULL)) <=0)
    {
        DPRINT("Error creating fileid=%d name (%d)",file_id, rtc);
        return rtc;
    }
    
    if ((fd = vfs_open(fn, O_RDONLY, 0)) < 0)
    {
        DPRINT("Error opening fileid=%d for reading (%s)",file_id, fn);
        return -ENOENT;
    }

    if (buffer && length)
    {
        if (offset)
        {
            if ( (rtc = vfs_lseek(fd, offset, SEEK_SET)) != 0 )
            {
                vfs_close(fd);
                DPRINT("Error seeking fileid=%d header (%d)",file_id, rtc);
                return rtc;
            }
        }

        memset(buffer,0,length);
        if ( (rtc = vfs_read(fd, buffer, length)) < 0 )
        {
            vfs_close(fd);
            DPRINT("Error reading fileid=%d (%d) data[%d]",file_id, rtc, length);
            return rtc;
        }
    }

    vfs_close(fd);
#else
    fs_storage_class_t storage = files[file_id].storage;
    blockdevice_read(bd[storage], buffer, files[file_id].addr + offset, length);
#endif

    DPRINT("fs read_file(file_id %d, offset %d, addr %p, length %d)",file_id, offset, files[file_id].addr, length);
    return 0;
}

int fs_write_file(uint8_t file_id, uint32_t offset, const uint8_t* buffer, uint32_t length)
{
    if(!_is_file_defined(file_id)) return -ENOENT;

#ifdef MODULE_VFS
    char fn[MAX_FILE_NAME];
    int rtc;
    int fd;
    if ((rtc = _get_file_name(fn, file_id, NULL)) <=0)
    {
        DPRINT("Error creating fileid=%d name (%d)\n",file_id, rtc);
        return -ENOENT;
    }

    if ((fd = vfs_open(fn, O_RDWR, 0)) < 0)
    {
        DPRINT("Error opening fileid=%d rdwr (%s)\n",file_id, fn);
        return -ENOENT;
    }

    if (buffer && length)
    {
        rtc = vfs_lseek(fd, offset, SEEK_SET);
        if ( (rtc < 0) || ((unsigned)rtc < offset) )
        {
            vfs_close(fd);
            DPRINT("Error seeking fileid=%d return (%d)\n",file_id, rtc);
            return rtc;
        }
        rtc = vfs_write(fd, buffer, length);
        if ( (rtc < 0) || ((unsigned)rtc < length) )
        {
            vfs_close(fd);
            DPRINT("Error writing fileid=%d (%d) data[%d]\n",file_id, rtc,length);
            return rtc;
        }
    }

    vfs_close(fd);
#else
    if(files[file_id].length < offset + length) return -ENOBUFS;

    fs_storage_class_t storage = files[file_id].storage;
    blockdevice_program(bd[storage], buffer, files[file_id].addr + offset, length);
#endif

    DPRINT("fs write_file (file_id %d, offset %d, addr %p, length %d)",
           file_id, offset, files[file_id].addr, length);

    if(file_modified_callbacks[file_id])
         file_modified_callbacks[file_id](file_id);

    return 0;
}

fs_file_stat_t *fs_file_stat(uint8_t file_id)
{
    assert(is_fs_init_completed);

    assert(file_id < FRAMEWORK_FS_FILE_COUNT);

    if (_is_file_defined(file_id))
        return (fs_file_stat_t*)&files[file_id];
    else
        return NULL;
}

bool fs_unregister_file_modified_callback(uint8_t file_id) {
    if(file_modified_callbacks[file_id]) {
        file_modified_callbacks[file_id] = NULL;
        return true;
    } else
        return false;
}

bool fs_register_file_modified_callback(uint8_t file_id, fs_modified_file_callback_t callback)
{
    assert(_is_file_defined(file_id));

    if(file_modified_callbacks[file_id])
        return false; // already registered

    file_modified_callbacks[file_id] = callback;
    return true;
}
