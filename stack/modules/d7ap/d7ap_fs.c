/*! \file d7ap_fs.c
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
 *  \author glenn.ergeerts@uantwerpen.be
 *
 */

#include "d7ap_fs.h"

// TODO platform dependent, move
static const uint8_t fs_headers[] __attribute__((used)) __attribute__((section(".d7ap_fs_headers"))) = {
    /*[[[cog
    import cog
    from d7a.system_files.system_files import SystemFiles
    from d7a.system_files.access_profile import AccessProfileFile
    from d7a.system_files.dll_config import DllConfigFile
    from d7a.system_files.firmware_version import FirmwareVersionFile
    from d7a.system_files.system_file_ids import SystemFileIds
    from d7a.system_files.uid import UidFile
    from d7a.fs.file_permissions import FilePermissions
    from d7a.fs.file_properties import FileProperties
    from d7a.fs.file_properties import ActionCondition, StorageClass, FileProperties
    from d7a.fs.file_header import FileHeader


    sys_file_permission_default = FilePermissions(encrypted=False, executeable=False, user_readable=True, user_writeable=False, user_executeable=False,
                       guest_readable=True, guest_writeable=False, guest_executeable=False)
    sys_file_permission_non_readable = FilePermissions(encrypted=False, executeable=False, user_readable=False, user_writeable=False, user_executeable=False,
                       guest_readable=False, guest_writeable=False, guest_executeable=False)
    sys_file_prop_default = FileProperties(act_enabled=False, act_condition=ActionCondition.WRITE, storage_class=StorageClass.PERMANENT)
    for sys_file_idx in range(0x2F):
        file_len = 0
        file_name = "not implemented"
        file_permissions = sys_file_permission_default
        implemented_files = [file.value for file in SystemFileIds]
        if sys_file_idx in implemented_files:
          file_type = SystemFileIds(sys_file_idx)
          if file_type == SystemFileIds.SECURITY_KEY:
              file_permissions = sys_file_permission_non_readable

          file_name = file_type.name
          if file_type in SystemFiles().files:
              file_len = SystemFiles().files[file_type].length.value

        cog.outl("\t// {} - {}".format(sys_file_idx, file_name))
        file_header = FileHeader(permissions=file_permissions, properties=sys_file_prop_default, alp_command_file_id=0xFF, interface_file_id=0xFF, file_size=file_len, allocated_size=file_len)
        file_header_array_elements = "\t"
        for byte in bytearray(file_header):
          file_header_array_elements += "{}, ".format(byte)

        cog.outl(file_header_array_elements)
    ]]]*/
    // 0 - UID
    36, 35, 255, 255, 0, 0, 0, 8, 0, 0, 0, 8, 
    // 1 - not implemented
    36, 35, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 
    // 2 - FIRMWARE_VERSION
    36, 35, 255, 255, 0, 0, 0, 15, 0, 0, 0, 15, 
    // 3 - not implemented
    36, 35, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 
    // 4 - not implemented
    36, 35, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 
    // 5 - not implemented
    36, 35, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 
    // 6 - not implemented
    36, 35, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 
    // 7 - not implemented
    36, 35, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 
    // 8 - not implemented
    36, 35, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 
    // 9 - not implemented
    36, 35, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 
    // 10 - DLL_CONFIG
    36, 35, 255, 255, 0, 0, 0, 6, 0, 0, 0, 6, 
    // 11 - not implemented
    36, 35, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 
    // 12 - not implemented
    36, 35, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 
    // 13 - not implemented
    36, 35, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 
    // 14 - SECURITY_KEY
    0, 35, 255, 255, 0, 0, 0, 16, 0, 0, 0, 16, 
    // 15 - not implemented
    36, 35, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 
    // 16 - not implemented
    36, 35, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 
    // 17 - not implemented
    36, 35, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 
    // 18 - not implemented
    36, 35, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 
    // 19 - not implemented
    36, 35, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 
    // 20 - not implemented
    36, 35, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 
    // 21 - not implemented
    36, 35, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 
    // 22 - not implemented
    36, 35, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 
    // 23 - not implemented
    36, 35, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 
    // 24 - not implemented
    36, 35, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 
    // 25 - not implemented
    36, 35, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 
    // 26 - not implemented
    36, 35, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 
    // 27 - not implemented
    36, 35, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 
    // 28 - not implemented
    36, 35, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 
    // 29 - not implemented
    36, 35, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 
    // 30 - not implemented
    36, 35, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 
    // 31 - not implemented
    36, 35, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 
    // 32 - ACCESS_PROFILE_0
    36, 35, 255, 255, 0, 0, 0, 65, 0, 0, 0, 65, 
    // 33 - ACCESS_PROFILE_1
    36, 35, 255, 255, 0, 0, 0, 65, 0, 0, 0, 65, 
    // 34 - ACCESS_PROFILE_2
    36, 35, 255, 255, 0, 0, 0, 65, 0, 0, 0, 65, 
    // 35 - ACCESS_PROFILE_3
    36, 35, 255, 255, 0, 0, 0, 65, 0, 0, 0, 65, 
    // 36 - ACCESS_PROFILE_4
    36, 35, 255, 255, 0, 0, 0, 65, 0, 0, 0, 65, 
    // 37 - ACCESS_PROFILE_5
    36, 35, 255, 255, 0, 0, 0, 65, 0, 0, 0, 65, 
    // 38 - ACCESS_PROFILE_6
    36, 35, 255, 255, 0, 0, 0, 65, 0, 0, 0, 65, 
    // 39 - ACCESS_PROFILE_7
    36, 35, 255, 255, 0, 0, 0, 65, 0, 0, 0, 65, 
    // 40 - ACCESS_PROFILE_8
    36, 35, 255, 255, 0, 0, 0, 65, 0, 0, 0, 65, 
    // 41 - ACCESS_PROFILE_9
    36, 35, 255, 255, 0, 0, 0, 65, 0, 0, 0, 65, 
    // 42 - ACCESS_PROFILE_10
    36, 35, 255, 255, 0, 0, 0, 65, 0, 0, 0, 65, 
    // 43 - ACCESS_PROFILE_11
    36, 35, 255, 255, 0, 0, 0, 65, 0, 0, 0, 65, 
    // 44 - ACCESS_PROFILE_12
    36, 35, 255, 255, 0, 0, 0, 65, 0, 0, 0, 65, 
    // 45 - ACCESS_PROFILE_13
    36, 35, 255, 255, 0, 0, 0, 65, 0, 0, 0, 65, 
    // 46 - ACCESS_PROFILE_14
    36, 35, 255, 255, 0, 0, 0, 65, 0, 0, 0, 65, 
    //[[[end]]]
};

static const uint8_t eeprom_fs[] __attribute__((used)) __attribute__((section(".d7ap_fs_data"))) = {
    /*[[[cog
    import cog
    from d7a.system_files.system_files import SystemFiles
    from d7a.system_files.access_profile import AccessProfileFile
    from d7a.system_files.dll_config import DllConfigFile
    from d7a.system_files.firmware_version import FirmwareVersionFile
    from d7a.system_files.system_file_ids import SystemFileIds
    from d7a.system_files.uid import UidFile
    from d7a.dll.access_profile import AccessProfile, CsmaCaMode, SubBand
    from d7a.dll.sub_profile import SubProfile
    from d7a.phy.channel_header import ChannelHeader, ChannelBand, ChannelCoding, ChannelClass
    from d7a.types.ct import CT

    #for file in SystemFiles().get_all_system_files():
    #    cog.outl("{} - {}".format(file.name, file.value))

    def output_file(file):
      file_type = SystemFileIds(file.id)
      cog.outl("\t// {} - {}".format(file_type.name, file_type.value))
      file_array_elements = "\t"
      for byte in bytearray(file):
        file_array_elements += "{}, ".format(byte)

      cog.outl(file_array_elements)

    output_file(UidFile())
    output_file(FirmwareVersionFile())
    output_file(DllConfigFile())
    ap = AccessProfile(
      channel_header=ChannelHeader(
        channel_class=ChannelClass.LO_RATE,
        channel_coding=ChannelCoding.FEC_PN9,
        channel_band=ChannelBand.BAND_868
      ),
      sub_profiles=[SubProfile(subband_bitmap=0x01, scan_automation_period=CT(0))] * 4,
      sub_bands=[SubBand()] * 8
    )
    output_file(AccessProfileFile(0, ap))
    output_file(AccessProfileFile(1, ap))
    output_file(AccessProfileFile(2, ap))
    output_file(AccessProfileFile(3, ap))
    output_file(AccessProfileFile(4, ap))
    output_file(AccessProfileFile(5, ap))
    output_file(AccessProfileFile(6, ap))
    output_file(AccessProfileFile(7, ap))
    output_file(AccessProfileFile(8, ap))
    output_file(AccessProfileFile(9, ap))
    output_file(AccessProfileFile(10, ap))
    output_file(AccessProfileFile(11, ap))
    output_file(AccessProfileFile(12, ap))
    output_file(AccessProfileFile(13, ap))
    output_file(AccessProfileFile(14, ap))
    ]]]*/
    // UID - 0
    0, 0, 0, 0, 0, 0, 0, 0, 
    // FIRMWARE_VERSION - 2
    0, 0, 
    // DLL_CONFIG - 10
    0, 255, 255, 
    // ACCESS_PROFILE_0 - 32
    50, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 
    // ACCESS_PROFILE_1 - 33
    50, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 
    // ACCESS_PROFILE_2 - 34
    50, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 
    // ACCESS_PROFILE_3 - 35
    50, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 
    // ACCESS_PROFILE_4 - 36
    50, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 
    // ACCESS_PROFILE_5 - 37
    50, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 
    // ACCESS_PROFILE_6 - 38
    50, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 
    // ACCESS_PROFILE_7 - 39
    50, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 
    // ACCESS_PROFILE_8 - 40
    50, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 
    // ACCESS_PROFILE_9 - 41
    50, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 
    // ACCESS_PROFILE_10 - 42
    50, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 
    // ACCESS_PROFILE_11 - 43
    50, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 
    // ACCESS_PROFILE_12 - 44
    50, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 
    // ACCESS_PROFILE_13 - 45
    50, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 
    // ACCESS_PROFILE_14 - 46
    50, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 0, 0, 0, 0, 0, 86, 255, 
    //[[[end]]]

  // 0x01 - factory settings, not defined
  // 0x02 - firmware version
  // 0x03 - device capacity
  // 0x04 - device status
  // 0x05 - engineering mode
  // 0x06 - VID
  // 0x07 - RFU
  // 0x08 - PHY config
  // 0x09 - PHY status
  // 0x0A - DLL config
  // 0x0B - DLL status
  // 0x0C - NWL routing
  // 0x0E - NWL security key
  // 0x0F - NWL securty state register
  // 0x10 - NWL status
  // 0x11 - TRL status
  // 0x12 - SEL config
  // 0x13 - FOF status
  // 0x14 - RFU
  // 0x15 - RFU
  // 0x16 - RFU
  // 0x17 - location data
  // 0x18-0x1F - reserved for D7AALP
  // 0x20-0x2E - Access Profiles
  // 0x2F - RFU
};



void d7ap_fs_init() {

}
