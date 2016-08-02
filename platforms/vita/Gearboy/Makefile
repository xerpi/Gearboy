TITLE_ID = GEARBOY01
TARGET   = Gearboy
GEARBOY_SRC = ../../../src
OBJS     = main.o Sound_Queue_vita.o Audio_vita.o $(GEARBOY_SRC)/MBC2MemoryRule.o \
	$(GEARBOY_SRC)/MBC1MemoryRule.o $(GEARBOY_SRC)/IORegistersMemoryRule.o \
	$(GEARBOY_SRC)/audio/Gb_Apu.o $(GEARBOY_SRC)/MultiMBC1MemoryRule.o \
	$(GEARBOY_SRC)/GearboyCore.o $(GEARBOY_SRC)/audio/Multi_Buffer.o \
	$(GEARBOY_SRC)/audio/Effects_Buffer.o $(GEARBOY_SRC)/MBC5MemoryRule.o \
	$(GEARBOY_SRC)/audio/Gb_Apu_State.o $(GEARBOY_SRC)/audio/Blip_Buffer.o \
	$(GEARBOY_SRC)/MemoryRule.o $(GEARBOY_SRC)/Input.o $(GEARBOY_SRC)/Processor.o \
	$(GEARBOY_SRC)/Video.o $(GEARBOY_SRC)/Memory.o $(GEARBOY_SRC)/Cartridge.o \
	$(GEARBOY_SRC)/MBC3MemoryRule.o $(GEARBOY_SRC)/RomOnlyMemoryRule.o \
	$(GEARBOY_SRC)/CommonMemoryRule.o $(GEARBOY_SRC)/audio/Gb_Oscs.o \
	$(GEARBOY_SRC)/opcodes.o $(GEARBOY_SRC)/opcodes_cb.o

GEARBOY_FLAGS = -I$(GEARBOY_SRC) -I$(GEARBOY_SRC)/audio -DMINIZ_NO_TIME

LIBS = -lvita2d -lSceKernel_stub -lSceDisplay_stub -lSceCommonDialog_stub \
	-lSceGxm_stub -lSceSysmodule_stub -lSceCtrl_stub -lScePgf_stub \
	-lSceRtc_stub -lSceAudio_stub -lpng -ljpeg -lfreetype -lz -lm -lc -lstdc++

PREFIX    = arm-vita-eabi
CC        = $(PREFIX)-gcc
CXX       = $(PREFIX)-g++
CFLAGS    = -Wl,-q -Wall -O3 $(GEARBOY_FLAGS)
CXXFLAGS  = $(CFLAGS) -fno-rtti -fno-exceptions -fpermissive
ASFLAGS   = $(CFLAGS)

all: $(TARGET).vpk

%.vpk: eboot.bin param.sfo
	vita-pack-vpk -s param.sfo -b eboot.bin $@

param.sfo:
	vita-mksfoex -s TITLE_ID=$(TITLE_ID) "$(TARGET)" $@

eboot.bin: $(TARGET).velf
	vita-make-fself $< $@

%.velf: %.elf
	vita-elf-create $< $@

$(TARGET).elf: $(OBJS)
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@

clean:
	@rm -rf $(TARGET).vpk $(TARGET).velf $(TARGET).elf $(OBJS) \
		eboot.bin param.sfo

vpksend: $(TARGET).vpk
	curl -T $(TARGET).vpk ftp://$(PSVITAIP):1337/ux0:/
	@echo "Sent."

send: eboot.bin
	curl -T eboot.bin ftp://$(PSVITAIP):1337/ux0:/app/$(TITLE_ID)/
	@echo "Sent."
