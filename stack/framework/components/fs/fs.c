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

#define FS_BLOCKDEVICES_COUNT       3 // metadata, permanent and volatile

static fs_file_t files[FRAMEWORK_FS_FILE_COUNT] = { 0 }; // TODO do not keep all file metadata in RAM but use smaller MRU cache to save RAM

static bool is_fs_init_completed = false;  //set in _d7a_verify_magic()

#define IS_SYSTEM_FILE(file_id)         (file_id <= 0x3F)

static fs_modified_file_callback_t file_modified_callbacks[FRAMEWORK_FS_FILE_COUNT] = { NULL }; // TODO limit to lower number so save RAM?

static uint32_t volatile_data_offset = 0;
static uint32_t permanent_data_offset = 0;

static blockdevice_t* bd[FS_BLOCKDEVICES_COUNT];

/* forward internal declarations */
static int _fs_init(void);
static int _fs_create_magic(void);
static int _fs_verify_magic(uint8_t* magic_number);
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

void fs_init()
{
    if (is_fs_init_completed)
        return /*0*/;

    memset(files,0,sizeof(files));

    // inject the mandatory blockdevice types from the platform
    // for now, only metadata, permanent and volatile storage are supported
    bd[FS_BLOCKDEVICE_TYPE_METADATA] = PLATFORM_METADATA_BLOCKDEVICE;
    bd[FS_BLOCKDEVICE_TYPE_PERMANENT] = PLATFORM_PERMANENT_BLOCKDEVICE;
    bd[FS_BLOCKDEVICE_TYPE_VOLATILE] = PLATFORM_VOLATILE_BLOCKDEVICE;

    _fs_init();

    is_fs_init_completed = true;
    DPRINT("fs_init OK");
}

int _fs_init()
{
    uint8_t expected_magic_number[FS_MAGIC_NUMBER_SIZE] = FS_MAGIC_NUMBER;
    if (_fs_verify_magic(expected_magic_number) < 0)
    {
        DPRINT("fs_init: no valid magic, recreating fs...");
        _fs_create_magic();
   }

    // initialise system file caching
    uint32_t number_of_files;
    blockdevice_read(bd[FS_BLOCKDEVICE_TYPE_METADATA], (uint8_t*)&number_of_files, FS_NUMBER_OF_FILES_ADDRESS, FS_NUMBER_OF_FILES_SIZE);
#if __BYTE_ORDER__ != __ORDER_BIG_ENDIAN__
    number_of_files = __builtin_bswap32(number_of_files);
#endif

    assert(number_of_files < FRAMEWORK_FS_FILE_COUNT);
    for(int file_id = 0; file_id < FRAMEWORK_FS_FILE_COUNT; file_id++)
    {
        blockdevice_read(bd[FS_BLOCKDEVICE_TYPE_METADATA], (uint8_t*)&files[file_id],
                         _get_file_header_address(file_id), FS_FILE_HEADER_SIZE);

#if __BYTE_ORDER__ != __ORDER_BIG_ENDIAN__
        // FS headers are stored in big endian
        files[file_id].addr = __builtin_bswap32(files[file_id].addr);
        files[file_id].length = __builtin_bswap32(files[file_id].length);
#endif

        if(_is_file_defined(file_id))
        {
            switch(files[file_id].blockdevice_index)
            {
                case FS_BLOCKDEVICE_TYPE_VOLATILE:
                {
                    //copy defaults from permanent storage to volatile
                    uint8_t data = 0x00;
                    for(int i=0; i < files[file_id].length; i++)
                    {
                        blockdevice_read(bd[FS_BLOCKDEVICE_TYPE_PERMANENT], &data, files[file_id].addr + i, 1);
                        blockdevice_program(bd[FS_BLOCKDEVICE_TYPE_VOLATILE], &data, volatile_data_offset + i, 1);
    
                    }
                    // update file header
                    files[file_id].addr = volatile_data_offset;
                    volatile_data_offset += files[file_id].length;
                    permanent_data_offset += files[file_id].length;
                    break;
                }
                case FS_BLOCKDEVICE_TYPE_PERMANENT:
                {
                    files[file_id].addr = permanent_data_offset;
                    permanent_data_offset += files[file_id].length;
                    break;
                }
                default:
                    assert(false);
            }
        }
    }
    return 0;
}

//TODO: CRC MAGIC
static int _fs_create_magic()
{
    assert(!is_fs_init_completed);
    uint8_t magic[] = FS_MAGIC_NUMBER;

    /* verify */
    return _fs_verify_magic(magic);
}


/* The magic number allows to check filesystem integrity.*/
static int _fs_verify_magic(uint8_t* expected_magic_number)
{
    is_fs_init_completed = false;

    uint8_t magic_number[FS_MAGIC_NUMBER_SIZE];
    memset(magic_number,0,FS_MAGIC_NUMBER_SIZE);
    blockdevice_read(bd[FS_BLOCKDEVICE_TYPE_METADATA], magic_number, 0, FS_MAGIC_NUMBER_SIZE);
    assert(memcmp(expected_magic_number, magic_number, FS_MAGIC_NUMBER_SIZE) == 0); // if not the FS on EEPROM is not compatible with the current code

    return 0;
}


int _fs_create_file(uint8_t file_id, fs_storage_class_t storage_class, const uint8_t* initial_data, uint32_t length)
{
    assert(file_id < FRAMEWORK_FS_FILE_COUNT);

    if (_is_file_defined(file_id))
        return -EEXIST;

    // update file caching for stat lookup
    uint8_t bd_type = FS_BLOCKDEVICE_TYPE_PERMANENT;
    if(storage_class == FS_STORAGE_VOLATILE)
        bd_type = FS_BLOCKDEVICE_TYPE_VOLATILE;

    files[file_id].blockdevice_index = (uint8_t)bd_type;
    files[file_id].length = length;

    // only user files can be created
    assert(file_id >= 0x40);

    if (bd_type == FS_BLOCKDEVICE_TYPE_PERMANENT)
    {
        files[file_id].addr = permanent_data_offset;

#if __BYTE_ORDER__ != __ORDER_BIG_ENDIAN__
        fs_file_t file_header_big_endian;
        memcpy(&file_header_big_endian, (void*)&files[file_id], sizeof (fs_file_t));
        file_header_big_endian.length = __builtin_bswap32(file_header_big_endian.length);
        file_header_big_endian.addr = __builtin_bswap32(file_header_big_endian.addr);
        blockdevice_program(bd[FS_BLOCKDEVICE_TYPE_METADATA], (uint8_t*)&file_header_big_endian, _get_file_header_address(file_id), FS_FILE_HEADER_SIZE);
#else
        blockdevice_program(bd[FS_BLOCKDEVICE_TYPE_METADATA], (uint8_t*)&files[file_id], _get_file_header_address(file_id), FS_FILE_HEADER_SIZE);
#endif

        permanent_data_offset += length;
    }
    else
    {
        files[file_id].addr = volatile_data_offset;
        volatile_data_offset += length;
    }

    if(initial_data != NULL) {
        blockdevice_program(bd[bd_type], initial_data, files[file_id].addr, length);
    }
    else{
        // do not use variable length array to limit stack usage, do in chunks instead
        uint8_t default_data[64];
        memset(default_data, 0xff, 64);
        uint32_t remaining_length = length;
        int i = 0;
        while(remaining_length > 64) {
          blockdevice_program(bd[bd_type], default_data, files[file_id].addr + (i * 64), 64);
          remaining_length -= 64;
          i++;
        }

        blockdevice_program(bd[bd_type], default_data, files[file_id].addr  + (i * 64), remaining_length);
    }

    DPRINT("fs init file(file_id %d, storage %d, addr %p, length %d)\n",file_id, storage_class, files[file_id].addr, length);
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

    error_t e = blockdevice_read(bd[files[file_id].blockdevice_index], buffer, files[file_id].addr + offset, length);
    if(e != SUCCESS) {
        log_print_string("reading from blockdevice failed, you probably didn't allocate enough memory (volatile or permanent)");
        assert(false);
    }

    DPRINT("fs read_file(file_id %d, offset %d, addr %p, length %d)\n",file_id, offset, files[file_id].addr, length);
    return 0;
}

int fs_write_file(uint8_t file_id, uint32_t offset, const uint8_t* buffer, uint32_t length)
{
    if(!_is_file_defined(file_id)) return -ENOENT;

    if(files[file_id].length < offset + length) return -ENOBUFS;

    blockdevice_program(bd[files[file_id].blockdevice_index], buffer, files[file_id].addr + offset, length);

    DPRINT("fs write_file (file_id %d, offset %d, addr %p, length %d)\n",
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
