/*! \file d7ap_fs_data.c
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

// The cog section below does not generate code but defines some global variables and functions which are used in subsequent cog sections below,
// which do the actual code generation

/*[[[cog
import cog
from d7a.system_files.system_files import SystemFiles
from d7a.system_files.access_profile import AccessProfileFile
from d7a.system_files.dll_config import DllConfigFile
from d7a.system_files.firmware_version import FirmwareVersionFile
from d7a.system_files.system_file_ids import SystemFileIds
from d7a.system_files.not_implemented import NotImplementedFile
from d7a.system_files.security_key import SecurityKeyFile
from d7a.system_files.uid import UidFile
from d7a.fs.file_permissions import FilePermissions
from d7a.fs.file_properties import FileProperties
from d7a.fs.file_properties import ActionCondition, StorageClass, FileProperties
from d7a.fs.file_header import FileHeader
from d7a.dll.access_profile import AccessProfile, CsmaCaMode, SubBand
from d7a.dll.sub_profile import SubProfile
from d7a.phy.channel_header import ChannelHeader, ChannelBand, ChannelCoding, ChannelClass
from d7a.types.ct import CT

ap = AccessProfile(
  channel_header=ChannelHeader(
    channel_class=ChannelClass.LO_RATE,
    channel_coding=ChannelCoding.FEC_PN9,
    channel_band=ChannelBand.BAND_868
  ),
  sub_profiles=[SubProfile(subband_bitmap=0x01, scan_automation_period=CT(0))] * 4,
  sub_bands=[SubBand()] * 8
)

system_files = [
  UidFile(),
  NotImplementedFile(SystemFileIds.FACTORY_SETTINGS.value, 0),
  FirmwareVersionFile(),
  NotImplementedFile(SystemFileIds.DEVICE_CAPACITY.value, 19),
  NotImplementedFile(SystemFileIds.DEVICE_STATUS.value, 9),
  NotImplementedFile(SystemFileIds.ENGINEERING_MODE.value, 0),
  NotImplementedFile(SystemFileIds.VID.value, 3),
  NotImplementedFile(SystemFileIds.RFU_07.value, 0),
  NotImplementedFile(SystemFileIds.PHY_CONFIG.value, 9),
  NotImplementedFile(SystemFileIds.PHY_STATUS.value, 24),  # TODO assuming 3 channels for now
  DllConfigFile(active_access_class=0x21),
  NotImplementedFile(SystemFileIds.DLL_STATUS.value, 12),
  NotImplementedFile(SystemFileIds.NWL_ROUTING.value, 1),  # TODO variable routing table
  NotImplementedFile(SystemFileIds.NWL_SECURITY.value, 5),
  SecurityKeyFile(),
  NotImplementedFile(SystemFileIds.NWL_SSR.value, 4),  # TODO 0 recorded devices
  NotImplementedFile(SystemFileIds.NWL_STATUS.value, 20),
  NotImplementedFile(SystemFileIds.TRL_STATUS.value, 1),  # TODO 0 TRL records
  NotImplementedFile(SystemFileIds.SEL_CONFIG.value, 6),
  NotImplementedFile(SystemFileIds.FOF_STATUS.value, 10),
  NotImplementedFile(SystemFileIds.RFU_14.value, 0),
  NotImplementedFile(SystemFileIds.RFU_15.value, 0),
  NotImplementedFile(SystemFileIds.RFU_16.value, 0),
  NotImplementedFile(SystemFileIds.LOCATION_DATA.value, 1),  # TODO 0 recorded locations
  NotImplementedFile(SystemFileIds.D7AALP_RFU_18.value, 0),
  NotImplementedFile(SystemFileIds.D7AALP_RFU_19.value, 0),
  NotImplementedFile(SystemFileIds.D7AALP_RFU_1A.value, 0),
  NotImplementedFile(SystemFileIds.D7AALP_RFU_1B.value, 0),
  NotImplementedFile(SystemFileIds.D7AALP_RFU_1C.value, 0),
  NotImplementedFile(SystemFileIds.D7AALP_RFU_1D.value, 0),
  NotImplementedFile(SystemFileIds.D7AALP_RFU_1E.value, 0),
  NotImplementedFile(SystemFileIds.D7AALP_RFU_1F.value, 0),
  AccessProfileFile(0, ap),
  AccessProfileFile(1, ap),
  AccessProfileFile(2, ap),
  AccessProfileFile(3, ap),
  AccessProfileFile(4, ap),
  AccessProfileFile(5, ap),
  AccessProfileFile(6, ap),
  AccessProfileFile(7, ap),
  AccessProfileFile(8, ap),
  AccessProfileFile(9, ap),
  AccessProfileFile(10, ap),
  AccessProfileFile(11, ap),
  AccessProfileFile(12, ap),
  AccessProfileFile(13, ap),
  AccessProfileFile(14, ap)
]

sys_file_permission_default = FilePermissions(encrypted=False, executeable=False, user_readable=True, user_writeable=False, user_executeable=False,
                   guest_readable=True, guest_writeable=False, guest_executeable=False)
sys_file_permission_non_readable = FilePermissions(encrypted=False, executeable=False, user_readable=False, user_writeable=False, user_executeable=False,
                   guest_readable=False, guest_writeable=False, guest_executeable=False)
sys_file_prop_default = FileProperties(act_enabled=False, act_condition=ActionCondition.WRITE, storage_class=StorageClass.PERMANENT)

def output_file(file):
  file_type = SystemFileIds(file.id)
  cog.outl("\t// {} - {}".format(file_type.name, file_type.value))
  file_array_elements = "\t"
  for byte in bytearray(file):
    file_array_elements += "{}, ".format(byte)

  cog.outl(file_array_elements)

def output_fileheader(file):
  file_type = SystemFileIds(system_file.id)
  cog.outl("\t// {} - {}".format(file_type.name, file_type.value))
  file_header = FileHeader(permissions=file_permissions, properties=sys_file_prop_default, alp_command_file_id=0xFF, interface_file_id=0xFF, file_size=system_file.length, allocated_size=system_file.length)
  file_header_array_elements = "\t"
  for byte in bytearray(file_header):
    file_header_array_elements += "{}, ".format(byte)

  cog.outl(file_header_array_elements)

def output_system_file_offsets():
  current_offset = 0
  for system_file in system_files:
    file_type = SystemFileIds(system_file.id)
    cog.outl("\t{}, // {} - {} (length {}))".format(current_offset, file_type.name, file_type.value, system_file.length))
    current_offset += system_file.length
]]]*/
//[[[end]]] (checksum: d41d8cd98f00b204e9800998ecf8427e)


// TODO platform dependent, move
const uint8_t fs_systemfiles_header_data[] __attribute__((used)) __attribute__((section(".d7ap_fs_systemfiles_header_data"))) = {
    /*[[[cog
    file_permissions = sys_file_permission_default
    for system_file in system_files:
      output_fileheader(system_file)
    ]]]*/
    // UID - 0
    36, 35, 255, 255, 0, 0, 0, 8, 0, 0, 0, 8, 
    // FACTORY_SETTINGS - 1
    36, 35, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 
    // FIRMWARE_VERSION - 2
    36, 35, 255, 255, 0, 0, 0, 15, 0, 0, 0, 15, 
    // DEVICE_CAPACITY - 3
    36, 35, 255, 255, 0, 0, 0, 19, 0, 0, 0, 19, 
    // DEVICE_STATUS - 4
    36, 35, 255, 255, 0, 0, 0, 9, 0, 0, 0, 9, 
    // ENGINEERING_MODE - 5
    36, 35, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 
    // VID - 6
    36, 35, 255, 255, 0, 0, 0, 3, 0, 0, 0, 3, 
    // RFU_07 - 7
    36, 35, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 
    // PHY_CONFIG - 8
    36, 35, 255, 255, 0, 0, 0, 9, 0, 0, 0, 9, 
    // PHY_STATUS - 9
    36, 35, 255, 255, 0, 0, 0, 24, 0, 0, 0, 24, 
    // DLL_CONFIG - 10
    36, 35, 255, 255, 0, 0, 0, 3, 0, 0, 0, 3, 
    // DLL_STATUS - 11
    36, 35, 255, 255, 0, 0, 0, 12, 0, 0, 0, 12, 
    // NWL_ROUTING - 12
    36, 35, 255, 255, 0, 0, 0, 1, 0, 0, 0, 1, 
    // NWL_SECURITY - 13
    36, 35, 255, 255, 0, 0, 0, 5, 0, 0, 0, 5, 
    // NWL_SECURITY_KEY - 14
    36, 35, 255, 255, 0, 0, 0, 16, 0, 0, 0, 16, 
    // NWL_SSR - 15
    36, 35, 255, 255, 0, 0, 0, 4, 0, 0, 0, 4, 
    // NWL_STATUS - 16
    36, 35, 255, 255, 0, 0, 0, 20, 0, 0, 0, 20, 
    // TRL_STATUS - 17
    36, 35, 255, 255, 0, 0, 0, 1, 0, 0, 0, 1, 
    // SEL_CONFIG - 18
    36, 35, 255, 255, 0, 0, 0, 6, 0, 0, 0, 6, 
    // FOF_STATUS - 19
    36, 35, 255, 255, 0, 0, 0, 10, 0, 0, 0, 10, 
    // RFU_14 - 20
    36, 35, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 
    // RFU_15 - 21
    36, 35, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 
    // RFU_16 - 22
    36, 35, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 
    // LOCATION_DATA - 23
    36, 35, 255, 255, 0, 0, 0, 1, 0, 0, 0, 1, 
    // D7AALP_RFU_18 - 24
    36, 35, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 
    // D7AALP_RFU_19 - 25
    36, 35, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 
    // D7AALP_RFU_1A - 26
    36, 35, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 
    // D7AALP_RFU_1B - 27
    36, 35, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 
    // D7AALP_RFU_1C - 28
    36, 35, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 
    // D7AALP_RFU_1D - 29
    36, 35, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 
    // D7AALP_RFU_1E - 30
    36, 35, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 
    // D7AALP_RFU_1F - 31
    36, 35, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 
    // ACCESS_PROFILE_0 - 32
    36, 35, 255, 255, 0, 0, 0, 65, 0, 0, 0, 65, 
    // ACCESS_PROFILE_1 - 33
    36, 35, 255, 255, 0, 0, 0, 65, 0, 0, 0, 65, 
    // ACCESS_PROFILE_2 - 34
    36, 35, 255, 255, 0, 0, 0, 65, 0, 0, 0, 65, 
    // ACCESS_PROFILE_3 - 35
    36, 35, 255, 255, 0, 0, 0, 65, 0, 0, 0, 65, 
    // ACCESS_PROFILE_4 - 36
    36, 35, 255, 255, 0, 0, 0, 65, 0, 0, 0, 65, 
    // ACCESS_PROFILE_5 - 37
    36, 35, 255, 255, 0, 0, 0, 65, 0, 0, 0, 65, 
    // ACCESS_PROFILE_6 - 38
    36, 35, 255, 255, 0, 0, 0, 65, 0, 0, 0, 65, 
    // ACCESS_PROFILE_7 - 39
    36, 35, 255, 255, 0, 0, 0, 65, 0, 0, 0, 65, 
    // ACCESS_PROFILE_8 - 40
    36, 35, 255, 255, 0, 0, 0, 65, 0, 0, 0, 65, 
    // ACCESS_PROFILE_9 - 41
    36, 35, 255, 255, 0, 0, 0, 65, 0, 0, 0, 65, 
    // ACCESS_PROFILE_10 - 42
    36, 35, 255, 255, 0, 0, 0, 65, 0, 0, 0, 65, 
    // ACCESS_PROFILE_11 - 43
    36, 35, 255, 255, 0, 0, 0, 65, 0, 0, 0, 65, 
    // ACCESS_PROFILE_12 - 44
    36, 35, 255, 255, 0, 0, 0, 65, 0, 0, 0, 65, 
    // ACCESS_PROFILE_13 - 45
    36, 35, 255, 255, 0, 0, 0, 65, 0, 0, 0, 65, 
    // ACCESS_PROFILE_14 - 46
    36, 35, 255, 255, 0, 0, 0, 65, 0, 0, 0, 65, 
    //[[[end]]] (checksum: 895c0b5ce2d7dfb9a3dd21225254693f)
};

const uint8_t fs_systemfiles_file_data[] __attribute__((used)) __attribute__((section(".d7ap_fs_systemfiles_data"))) = {
    /*[[[cog
    for system_file in system_files:
      output_file(system_file)
    ]]]*/    
    // UID - 0
    0, 0, 0, 0, 0, 0, 0, 0, 
    // FACTORY_SETTINGS - 1

    // FIRMWARE_VERSION - 2
    0, 0, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 
    // DEVICE_CAPACITY - 3
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
    // DEVICE_STATUS - 4
    0, 0, 0, 0, 0, 0, 0, 0, 0, 
    // ENGINEERING_MODE - 5

    // VID - 6
    0, 0, 0, 
    // RFU_07 - 7

    // PHY_CONFIG - 8
    0, 0, 0, 0, 0, 0, 0, 0, 0, 
    // PHY_STATUS - 9
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
    // DLL_CONFIG - 10
    33, 255, 255, 
    // DLL_STATUS - 11
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
    // NWL_ROUTING - 12
    0, 
    // NWL_SECURITY - 13
    0, 0, 0, 0, 0, 
    // NWL_SECURITY_KEY - 14
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
    // NWL_SSR - 15
    0, 0, 0, 0, 
    // NWL_STATUS - 16
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
    // TRL_STATUS - 17
    0, 
    // SEL_CONFIG - 18
    0, 0, 0, 0, 0, 0, 
    // FOF_STATUS - 19
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
    // RFU_14 - 20

    // RFU_15 - 21

    // RFU_16 - 22

    // LOCATION_DATA - 23
    0, 
    // D7AALP_RFU_18 - 24

    // D7AALP_RFU_19 - 25

    // D7AALP_RFU_1A - 26

    // D7AALP_RFU_1B - 27

    // D7AALP_RFU_1C - 28

    // D7AALP_RFU_1D - 29

    // D7AALP_RFU_1E - 30

    // D7AALP_RFU_1F - 31

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
    //[[[end]]] (checksum: 04f1b5f8329a6d0a6d0403e3eaf47a72)
};

// Store the offsets of the start of each system file in the data section, for fast lookup
// This is stored only in RAM since it doesn't take much space
__attribute__((used)) const uint16_t fs_systemfiles_file_offsets[] = {
  /*[[[cog
  output_system_file_offsets()
  ]]]*/
  0, // UID - 0 (length 8))
  8, // FACTORY_SETTINGS - 1 (length 0))
  8, // FIRMWARE_VERSION - 2 (length 15))
  23, // DEVICE_CAPACITY - 3 (length 19))
  42, // DEVICE_STATUS - 4 (length 9))
  51, // ENGINEERING_MODE - 5 (length 0))
  51, // VID - 6 (length 3))
  54, // RFU_07 - 7 (length 0))
  54, // PHY_CONFIG - 8 (length 9))
  63, // PHY_STATUS - 9 (length 24))
  87, // DLL_CONFIG - 10 (length 3))
  90, // DLL_STATUS - 11 (length 12))
  102, // NWL_ROUTING - 12 (length 1))
  103, // NWL_SECURITY - 13 (length 5))
  108, // NWL_SECURITY_KEY - 14 (length 16))
  124, // NWL_SSR - 15 (length 4))
  128, // NWL_STATUS - 16 (length 20))
  148, // TRL_STATUS - 17 (length 1))
  149, // SEL_CONFIG - 18 (length 6))
  155, // FOF_STATUS - 19 (length 10))
  165, // RFU_14 - 20 (length 0))
  165, // RFU_15 - 21 (length 0))
  165, // RFU_16 - 22 (length 0))
  165, // LOCATION_DATA - 23 (length 1))
  166, // D7AALP_RFU_18 - 24 (length 0))
  166, // D7AALP_RFU_19 - 25 (length 0))
  166, // D7AALP_RFU_1A - 26 (length 0))
  166, // D7AALP_RFU_1B - 27 (length 0))
  166, // D7AALP_RFU_1C - 28 (length 0))
  166, // D7AALP_RFU_1D - 29 (length 0))
  166, // D7AALP_RFU_1E - 30 (length 0))
  166, // D7AALP_RFU_1F - 31 (length 0))
  166, // ACCESS_PROFILE_0 - 32 (length 65))
  231, // ACCESS_PROFILE_1 - 33 (length 65))
  296, // ACCESS_PROFILE_2 - 34 (length 65))
  361, // ACCESS_PROFILE_3 - 35 (length 65))
  426, // ACCESS_PROFILE_4 - 36 (length 65))
  491, // ACCESS_PROFILE_5 - 37 (length 65))
  556, // ACCESS_PROFILE_6 - 38 (length 65))
  621, // ACCESS_PROFILE_7 - 39 (length 65))
  686, // ACCESS_PROFILE_8 - 40 (length 65))
  751, // ACCESS_PROFILE_9 - 41 (length 65))
  816, // ACCESS_PROFILE_10 - 42 (length 65))
  881, // ACCESS_PROFILE_11 - 43 (length 65))
  946, // ACCESS_PROFILE_12 - 44 (length 65))
  1011, // ACCESS_PROFILE_13 - 45 (length 65))
  1076, // ACCESS_PROFILE_14 - 46 (length 65))
  //[[[end]]] (checksum: 01a082019e42bbc0dbf7f7f3ff409c24)
};
