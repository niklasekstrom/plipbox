# gnu makefile to trigger build via vamos

BIN_DIR=../bin
OBJ_DIR=../obj

# mapped dir in vamos
AMI_OBJ_DIR=base:obj
AMI_BIN_DIR=base:bin

# where do the amiga files reside?
# expects the following dirs:
# wb310          - HD install of workbench 3.1
# sc             - install directory of SAS C 6.58 compiler
# AmiTCP-SDK-4.3 - AmiTCP SDK installation
AMIGA_DIR?=$(HOME)/amiga/shared
export AMIGA_DIR

# config vamos
VAMOS=vamos
VAMOS_OPTS=-c vamosrc
RUN=$(VAMOS) $(VAMOS_OPTS) $(VAMOS_EXTRA)

# device file
include ../../version.mk
DEVICE_NAME = plipbox
DEVICE_VERSION = $(VERSION_MAJ)
DEVICE_REVISION = $(VERSION_MIN)
DEVICE_DATE = $(shell date '+%d.%m.%Y')
DEVICE_FILE = $(DEVICE_NAME).device_$(BUILD_TYPE)_$(CPUSUFFIX)

# SAS C tools
LD          = $(RUN) sc:c/slink
CC          = $(RUN) sc:c/sc
AS          = $(RUN) sc:c/sc

# SAS C flags
LIBS        = LIB lib:amiga.lib LIB lib:sc.lib LIB lib:debug.lib
CINCLUDES   = INCDIR="netinclude:" INCDIR="include:" INCDIR="$(DEVICE_NAME)"
ASMINCLUDES = $(CINCLUDES) INCDIR="$(AMI_OBJ_DIR)/$(BUILD_DIR)"
CPUSUFFIX   = 000
CPUCCOPT    = CPU=68$(CPUSUFFIX)
BUILD_TYPE  = DEBUG

# device constants available in source
COMMON_DEFINES := DEFINE DEVICE_NAME=$(DEVICE_NAME) 
COMMON_DEFINES += DEFINE DEVICE_VERSION=$(DEVICE_VERSION)
COMMON_DEFINES += DEFINE DEVICE_REVISION=$(DEVICE_REVISION)
COMMON_DEFINES += DEFINE DEVICE_DATE=$(DEVICE_DATE)

# shared flags
CFLAGS_COMMON = $(CINCLUDES) NOMULTIPLEINCLUDES ERRREXX COMMENTNEST \
                NOSTKCHK NOCHKABORT \
                SMALLCODE SMALLDATA VERBOSE NOICONS STRICT ANSI \
                $(CPUCCOPT) $(COMMON_DEFINES)

# build type flags
CFLAGS_DEBUG = $(CFLAGS_COMMON) 
CFLAGS_RELEASE = $(CFLAGS_COMMON) \
                 PARAMETERS=REGISTERS OPT OPTTIME OPTINLINE\
                 OPTSCHEDULE STRINGMERGE STRUCTUREEQUIVALENCE

# assembler
ASMFLAGS = $(ASMINCLUDES) VERBOSE
DEVINFO_INC = $(BUILD_PATH)/devinfo.i

# linker
LDFLAGS_DEBUG    = NOICONS SC  $(LIBS) TO
LDFLAGS_RELEASE  = NOICONS SC SD ND $(LIBS) TO

# output directory
BUILD_DIR = build_$(BUILD_TYPE)_$(CPUSUFFIX)
BUILD_PATH = $(OBJ_DIR)/$(BUILD_DIR)

# generic source files
CSRC=device.c server.c track.c
ASRC=rt.asm

# driver specific source files
CSRC_$(DRIVER_NAME) = hw.c
ASRC_$(DRIVER_NAME) = hwpar.asm

# object files
OBJS:=$(patsubst %.asm,$(BUILD_PATH)/%.o,$(ASRC) $(ASRC_$(DRIVER_NAME)))
OBJS+=$(patsubst %.c,$(BUILD_PATH)/%.o,$(CSRC) $(CSRC_$(DRIVER_NAME)))

# output device
OUTPUT = $(BIN_DIR)/$(DEVICE_FILE)

# tool to calculate lib size
LIBSIZE_TOOL = $(BUILD_PATH)/libsize
LIBSIZE_INC = $(BUILD_PATH)/libsize.i
LIBSIZE_OBJ = $(BUILD_PATH)/libsize.o

# the plipbox test tool
DEV_TEST_TOOL_NAME = dev_test_$(BUILD_TYPE)_$(CPUSUFFIX)
DEV_TEST_TOOL = $(BIN_DIR)/$(DEV_TEST_TOOL_NAME)
DEV_TEST_TOOL_OBJ = $(BUILD_PATH)/dev_test.o

# the udp test tool
UDP_TEST_TOOL_NAME = udp_test_$(BUILD_TYPE)_$(CPUSUFFIX)
UDP_TEST_TOOL = $(BIN_DIR)/$(UDP_TEST_TOOL_NAME)
UDP_TEST_TOOL_OBJ = $(BUILD_PATH)/udp_test.o

# add driver directory
VPATH = .:$(DEVICE_NAME)

# --- rules ---

.PHONY: dev_test udp_test tools libsize all opt dist clean clean_obj clean_dist dirs

all: dirs $(OUTPUT) tools

opt:
	$(MAKE) BUILD_TYPE=RELEASE

dist: clean_dist dirs
	$(MAKE) BUILD_TYPE=RELEASE CPUSUFFIX=000
	$(MAKE) BUILD_TYPE=RELEASE CPUSUFFIX=020
	$(MAKE) BUILD_TYPE=RELEASE CPUSUFFIX=040
	@rm -rf $(OBJ_DIR)

clean: clean_obj
	rm -f $(OUTPUT) $(LIBSIZE_INC) $(LIBSIZE_TOOL) $(DEVINFO_INC)

clean_obj:
	rm -f $(OBJS)

clean_dist:
	rm -rf $(BIN_DIR) $(OBJ_DIR)

dirs: $(BUILD_PATH) $(BIN_DIR)

# --- compile ---
$(BUILD_PATH)/%.o: %.c
	$(CC) $(CFLAGS_$(BUILD_TYPE)) OBJECTNAME $(subst $(OBJ_DIR),$(AMI_OBJ_DIR),$@) $<

# --- assemble ---
$(BUILD_PATH)/%.o: %.asm
	$(AS) $(ASMFLAGS) OBJECTNAME $(subst $(OBJ_DIR),$(AMI_OBJ_DIR),$@) $<

# --- link ---
$(OUTPUT): $(OBJS)
	$(LD) $(subst $(OBJ_DIR),$(AMI_OBJ_DIR),$(OBJS)) $(LDFLAGS_$(BUILD_TYPE)) $(subst $(BIN_DIR),$(AMI_BIN_DIR),$@)

# --- dirs ---
$(BIN_DIR):
	mkdir $(BIN_DIR)

$(BUILD_PATH):
	mkdir -p $(BUILD_PATH)

# --- devinfo.i ---
$(DEVINFO_INC):
	@echo "DEVICE_NAME  MACRO" > $(DEVINFO_INC)
	@echo "  dc.b '$(DEVICE_NAME)'" >> $(DEVINFO_INC)
	@echo "	 ENDM" >> $(DEVINFO_INC)
	@echo "DEVICE_DATE  MACRO" >> $(DEVINFO_INC)
	@echo "  dc.b '$(DEVICE_DATE)'" >> $(DEVINFO_INC)
	@echo "	 ENDM" >> $(DEVINFO_INC)
	@echo "DEVICE_VER  MACRO" >> $(DEVINFO_INC)
	@echo "  dc.b '$(DEVICE_VERSION).$(DEVICE_REVISION)'" >> $(DEVINFO_INC)
	@echo "	 ENDM" >> $(DEVINFO_INC)
	@echo "DEVICE_VERSION equ $(DEVICE_VERSION)" >> $(DEVINFO_INC)
	@echo "DEVICE_REVISION equ $(DEVICE_REVISION)" >> $(DEVINFO_INC)

# --- libsize tool ---
gen: $(LIBSIZE_INC) $(DEVINFO_INC)
	@cat $(LIBSIZE_INC)
	@cat $(DEVINFO_INC)

$(LIBSIZE_INC): $(LIBSIZE_TOOL)
	$(RUN) $(LIBSIZE_TOOL) > $(LIBSIZE_INC)
	@cat $(LIBSIZE_INC)

# build libsize tool
libsize: $(LIBSIZE_TOOL)

$(LIBSIZE_TOOL): $(LIBSIZE_OBJ)
	$(LD) lib:c.o $(subst $(OBJ_DIR),$(AMI_OBJ_DIR),$<) $(LDFLAGS_$(BUILD_TYPE)) $(subst $(BIN_DIR),$(AMI_BIN_DIR),$@)

# ----- tools -----
tools: dev_test udp_test

# --- dev_test tool ---
dev_test: $(DEV_TEST_TOOL)

$(DEV_TEST_TOOL): $(DEV_TEST_TOOL_OBJ)
	$(LD) lib:c.o $(subst $(OBJ_DIR),$(AMI_OBJ_DIR),$<) $(LDFLAGS_$(BUILD_TYPE)) $(subst $(BIN_DIR),$(AMI_BIN_DIR),$@)

# --- udp test tool ---
udp_test: $(UDP_TEST_TOOL)

$(UDP_TEST_TOOL): $(UDP_TEST_TOOL_OBJ)
	$(LD) lib:c.o $(subst $(OBJ_DIR),$(AMI_OBJ_DIR),$<) $(LDFLAGS_$(BUILD_TYPE)) $(subst $(BIN_DIR),$(AMI_BIN_DIR),$@) LIB NETLIB:net.lib

# --- dependencies ---
rt.asm: $(LIBSIZE_INC) $(DEVINFO_INC)
$(LIBSIZE_INC): hw.h global.h hwbase.h
