# override sensible defaults in a local Makefile
-include Makefile.local

# if we're targetting the EZR-USB dongle, the RADIO and DEVICE are known
ifeq ($(PLATFORM),EZR32LG_USB01)
PLATFORM_RADIO          = si4460
DEVICE                  = EZR32LG330F256R60
endif

# these variable have sensible defaults if no prior value was assigned to them
APP                    ?= gateway
PLATFORM               ?= EFM32GG_STK3700
DEVICE                 ?= EFM32GG230F1024
PLATFORM_RADIO         ?= cc1101
FRAMEWORK_LOG_ENABLED  ?= no
FRAMEWORK_LOG_BINARY   ?= no
TOOLCHAIN_DIR          ?= 
BUILD                  ?= Debug

BUILD_DIR               = ../build/$(APP)

TARGET                  = $(BUILD_DIR)/apps/$(APP)/$(APP)
BIN                     = $(TARGET).bin
ELF                     = $(TARGET).elf

CMAKE                   = cmake

OSS                    := $(realpath stack)
TOOLCHAIN_ABS_DIR       = $(realpath $(TOOLCHAIN_DIR))
TOOLCHAIN_FILE          = $(OSS)/cmake/toolchains/gcc-arm-embedded.cmake
PLATFORM_UC            := $(shell echo $(PLATFORM) | tr a-z A-Z)
APP_DIR                 = $(OSS)/apps/$(APP)
APP_UC                 := $(shell echo $(APP) | tr a-z A-Z)

JLINK                   = JLinkExe
FLASH_SCRIPT            = script.gdb

SIZE                    = arm-none-eabi-size
OBJCOPY                 = arm-none-eabi-objcopy -O binary

# applications can specify aplication-specific variable overrides
-include $(APP_DIR)/Makefile

# these local Makefiles are ignored by git, but allow for local overrides
# via APP_SPECIFIC_VARIABLE_OVERRIDES
-include $(APP_DIR)/Makefile.local
-include $(EXTRA_APPS)/Makefile.local

all: $(ELF)

$(BUILD_DIR):
	@echo "*** preparing $@"
	@mkdir -p $@
	( cd $@; \
  	$(CMAKE) $(OSS) \
		 -DCMAKE_BUILD_TYPE=$(BUILD) \
		 -DTOOLCHAIN_DIR=$(TOOLCHAIN_ABS_DIR) \
		 -DCMAKE_TOOLCHAIN_FILE=$(TOOLCHAIN_FILE) \
		 -DPLATFORM=$(PLATFORM) \
		 -DPLATFORM_$(PLATFORM_UC)_RADIO=$(PLATFORM_RADIO) \
		 -DAPP_$(APP_UC)=on \
		 -DFRAMEWORK_DEBUG_ASSERT_MINIMAL=y \
		 -DFRAMEWORK_LOG_ENABLED=$(FRAMEWORK_LOG_ENABLED) \
		 -DFRAMEWORK_LOG_BINARY=$(FRAMEOWRK_LOG_BINARY) \
		 -DPLATFORM_USE_USB_CDC=$(PLATFORM_USE_USB_CDC) \
		 $(APP_SPECIFIC_VARIABLE_OVERRIDES) \
	)

$(ELF): $(BUILD_DIR)
	@echo "*** building $@"
	@(make -C $(BUILD_DIR))

size: $(ELF)
	@echo "*** sizing $<"
	@$(SIZE) $<

program: $(BIN)
	@echo "SelectEmuBySN $(PROGRAMMER)"  > $(FLASH_SCRIPT)
	@echo "si SWD"                      >> $(FLASH_SCRIPT)
	@echo "speed 4000"                  >> $(FLASH_SCRIPT)
	@echo "device $(DEVICE)"            >> $(FLASH_SCRIPT)
	@echo "loadfile $<"                 >> $(FLASH_SCRIPT)
	@echo "verifybin $<, 0x0"           >> $(FLASH_SCRIPT)
	@echo "r"                           >> $(FLASH_SCRIPT)
	@echo "g"                           >> $(FLASH_SCRIPT)
	@echo "exit"                        >> $(FLASH_SCRIPT)
	@echo "*** programming..."
	@$(JLINK) < $(FLASH_SCRIPT)
	@rm -f $(FLASH_SCRIPT)

%.bin: %.elf
	@echo "*** creating $@"
	@$(OBJCOPY) $< $@

clean:
	@rm -rf $(BUILD_DIR) $(GDB_SCRIPT)

.PHONY: all program compile size clean
