#---------------------------------------------------------------------------------
# Switch App Store - Makefile
# A Nintendo Switch homebrew app store with Apple App Store style UI
#---------------------------------------------------------------------------------

#---------------------------------------------------------------------------------
# Environment setup
#---------------------------------------------------------------------------------
ifeq ($(strip $(DEVKITPRO)),)
$(error "Please set DEVKITPRO in your environment. export DEVKITPRO=<path to>/devkitpro")
endif

TOPDIR ?= $(CURDIR)
include $(DEVKITPRO)/libnx/switch_rules

#---------------------------------------------------------------------------------
# Application metadata
#---------------------------------------------------------------------------------
APP_TITLE	:=	Switch App Store
APP_AUTHOR	:=	yynickyeh
APP_VERSION	:=	0.1.0

#---------------------------------------------------------------------------------
# Target and build directories
#---------------------------------------------------------------------------------
TARGET		:=	switch-appstore
BUILD		:=	build
SOURCES		:=	source source/core source/ui source/ui/components source/ui/screens source/network source/store source/utils
DATA		:=	data
INCLUDES	:=	source include
ROMFS		:=	romfs

#---------------------------------------------------------------------------------
# Portlibs path
#---------------------------------------------------------------------------------
PORTLIBS	:=	$(DEVKITPRO)/portlibs/switch

#---------------------------------------------------------------------------------
# Compiler options
#---------------------------------------------------------------------------------
ARCH		:=	-march=armv8-a+crc+crypto -mtune=cortex-a57 -mtp=soft -fPIE

CFLAGS		:=	-g -Wall -O2 -ffunction-sections \
				$(ARCH) $(DEFINES)

CFLAGS		+=	$(INCLUDE) -D__SWITCH__ \
				-I$(PORTLIBS)/include \
				-I$(PORTLIBS)/include/SDL2

CXXFLAGS	:=	$(CFLAGS) -std=c++17 -fno-rtti -fno-exceptions

ASFLAGS		:=	-g $(ARCH)

LDFLAGS		:=	-specs=$(DEVKITPRO)/libnx/switch.specs -g $(ARCH) \
				-Wl,-Map,$(notdir $*.map)

#---------------------------------------------------------------------------------
# Libraries to link (explicit, no pkg-config)
#---------------------------------------------------------------------------------
LIBS		:=	-L$(PORTLIBS)/lib \
				-lSDL2_ttf -lSDL2_gfx -lSDL2_image -lSDL2 \
				-lEGL -lglapi -ldrm_nouveau \
				-lharfbuzz -lfreetype -lpng16 -ljpeg -lwebp \
				-lcurl -lmbedtls -lmbedcrypto -lmbedx509 \
				-lz -lbz2 \
				-lnx

#---------------------------------------------------------------------------------
# Library directories
#---------------------------------------------------------------------------------
LIBDIRS		:=	$(PORTLIBS) $(LIBNX)

#---------------------------------------------------------------------------------
# Build rules - do not modify below this line
#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))

export OUTPUT	:=	$(CURDIR)/$(TARGET)
export TOPDIR	:=	$(CURDIR)

export VPATH	:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
					$(foreach dir,$(DATA),$(CURDIR)/$(dir))

export DEPSDIR	:=	$(CURDIR)/$(BUILD)

CFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
SFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))
BINFILES	:=	$(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.*)))

ifeq ($(strip $(CPPFILES)),)
	export LD	:=	$(CC)
else
	export LD	:=	$(CXX)
endif

export OFILES_BIN	:=	$(addsuffix .o,$(BINFILES))
export OFILES_SRC	:=	$(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(SFILES:.s=.o)
export OFILES		:=	$(OFILES_BIN) $(OFILES_SRC)
export HFILES_BIN	:=	$(addsuffix .h,$(subst .,_,$(BINFILES)))

export INCLUDE	:=	$(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
					$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
					-I$(CURDIR)/$(BUILD)

export LIBPATHS	:=	$(foreach dir,$(LIBDIRS),-L$(dir)/lib)

ifeq ($(strip $(ICON)),)
	icons := $(wildcard *.jpg)
	ifneq (,$(findstring $(TARGET).jpg,$(icons)))
		export APP_ICON := $(TOPDIR)/$(TARGET).jpg
	else
		ifneq (,$(findstring icon.jpg,$(icons)))
			export APP_ICON := $(TOPDIR)/icon.jpg
		else
			ifneq (,$(wildcard icon.png))
				export APP_ICON := $(TOPDIR)/icon.png
			endif
		endif
	endif
else
	export APP_ICON := $(TOPDIR)/$(ICON)
endif

ifeq ($(strip $(NO_ICON)),)
	export NROFLAGS += --icon=$(APP_ICON)
endif

ifeq ($(strip $(NO_NACP)),)
	export NROFLAGS += --nacp=$(CURDIR)/$(TARGET).nacp
endif

ifneq ($(ROMFS),)
	export NROFLAGS += --romfsdir=$(CURDIR)/$(ROMFS)
endif

.PHONY: $(BUILD) clean

$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

clean:
	@echo Cleaning...
	@rm -rf $(BUILD) $(TARGET).nro $(TARGET).nacp $(TARGET).elf

else

DEPENDS	:=	$(OFILES:.o=.d)

.PHONY: all

all: $(OUTPUT).nro

$(OUTPUT).nro: $(OUTPUT).elf $(OUTPUT).nacp

$(OUTPUT).elf: $(OFILES)

$(OFILES_SRC): $(HFILES_BIN)

%.bin.o %_bin.h: %.bin
	@echo $(notdir $<)
	@$(bin2o)

-include $(DEPENDS)

endif
