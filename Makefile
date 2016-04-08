APP                    ?= gateway
PLATFORM               ?= EFM32GG_STK3700
FRAMEWORK_LOG_ENABLED  ?= no
FRAMEWORK_LOG_BINARY   ?= no
TOOLCHAIN_DIR          ?= ../../gcc-arm-none-eabi-4_9-2015q3
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
PLATFORM_RADIO          = cc1101
APP_DIR                 = $(OSS)/apps/$(APP)
APP_UC                 := $(shell echo $(APP) | tr a-z A-Z)

JLINK                   = JLinkExe
DEVICE                  = EFM32GG230F1024
FLASH_SCRIPT            = script.gdb

SIZE                    = $(TOOLCHAIN_ABS_DIR)/bin/arm-none-eabi-size
OBJCOPY                 = $(TOOLCHAIN_ABS_DIR)/bin/arm-none-eabi-objcopy -O binary

-include $(APP_DIR)/Makefile

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
		 $(APP_SPECIFIC_VARIABLE_OVERRIDES) \
		 -DFRAMEWORK_DEBUG_ASSERT_MINIMAL=y \
		 -DFRAMEWORK_LOG_ENABLED=$(FRAMEWORK_LOG_ENABLED) \
		 -DFRAMEWORK_LOG_BINARY=$(FRAMEOWRK_LOG_BINARY) \
	)

$(ELF): $(BUILD_DIR)
	@echo "*** building $@"
	@(make -C $(BUILD_DIR))

size: $(ELF)
	@echo "*** sizing $<"
	@$(SIZE) $<

program: $(BIN)
	@echo "device $(DEVICE)" 	>  $(FLASH_SCRIPT)
	@echo "loadfile $<" 			>> $(FLASH_SCRIPT)
	@echo "verifybin $<, 0x0" >> $(FLASH_SCRIPT)
	@echo "r"									>> $(FLASH_SCRIPT)
	@echo "g"									>> $(FLASH_SCRIPT)
	@echo "exit"							>> $(FLASH_SCRIPT)
	@echo "*** programming..."
	@$(JLINK) < $(FLASH_SCRIPT)
	@rm -f $(FLASH_SCRIPT)

%.bin: %.elf
	@echo "*** creating $@"
	@$(OBJCOPY) $< $@

clean:
	@rm -rf $(BUILD_DIR) $(GDB_SCRIPT)

.PHONY: all program compile size clean
