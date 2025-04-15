TARGET := scummvm

APP_TITLE       := ScummVM
APP_DESCRIPTION := Point-and-click adventure game engines
APP_AUTHOR      := ScummVM Team
APP_ICON        := $(srcdir)/backends/platform/3ds/app/icon.png

APP_RSF         := $(srcdir)/backends/platform/3ds/app/scummvm.rsf
APP_BANNER_IMAGE:= $(srcdir)/backends/platform/3ds/app/banner.png
APP_BANNER_AUDIO:= $(srcdir)/backends/platform/3ds/app/banner.wav

BANNERTOOL       ?= bannertool
MAKEROM          ?= makerom

SHADER_GENFILES := \
	$(wildcard backends/platform/3ds/shaders/*/*_shbin.h) \
	$(wildcard backends/platform/3ds/shaders/*/*.shbin) \
	$(wildcard backends/platform/3ds/shaders/*/*.shbin.d)

.PHONY: clean_3ds dist_3ds

clean: clean_3ds

clean_3ds:
	$(RM) $(SHADER_GENFILES)
#TEMPLATE:
#	$(RM_REC) backends/platform/3ds/shaders/[ENGINE]/$(DEPDIR)
	$(RM) $(TARGET).smdh
	$(RM) $(TARGET).3dsx
	$(RM) $(TARGET).bnr
	$(RM) $(TARGET).cia
	$(RM) -rf romfs
	$(RM) -rf dist_3ds

romfs: $(DIST_FILES_THEMES) $(DIST_FILES_ENGINEDATA) $(DIST_FILES_NETWORKING) $(DIST_FILES_VKEYBD) $(DIST_3DS_EXTRA_FILES) $(PLUGINS)
	@rm -rf romfs
	@mkdir -p romfs
	@cp $(DIST_FILES_THEMES) romfs/
ifdef DIST_FILES_ENGINEDATA
	@cp $(DIST_FILES_ENGINEDATA) romfs/
endif
ifdef DIST_FILES_NETWORKING
	@cp $(DIST_FILES_NETWORKING) romfs/
endif
ifdef DIST_FILES_VKEYBD
	@cp $(DIST_FILES_VKEYBD) romfs/
endif
ifdef DIST_3DS_EXTRA_FILES
	@cp -a $(DIST_3DS_EXTRA_FILES) romfs/
endif
ifeq ($(DYNAMIC_MODULES),1)
	@mkdir -p romfs/plugins
	@for i in $(PLUGINS); do $(STRIP) --strip-debug $$i -o romfs/plugins/`basename $$i`; done
endif

$(TARGET).smdh: $(APP_ICON)
	@$(DEVKITPRO)/tools/bin/smdhtool --create "$(APP_TITLE)" "$(APP_DESCRIPTION)" "$(APP_AUTHOR)" $(APP_ICON) $@
	@echo built ... $(notdir $@)

$(TARGET).3dsx: $(EXECUTABLE) $(TARGET).smdh romfs
	@$(DEVKITPRO)/tools/bin/3dsxtool $< $@ --smdh=$(TARGET).smdh --romfs=romfs
	@echo built ... $(notdir $@)

$(TARGET).bnr: $(APP_BANNER_IMAGE) $(APP_BANNER_AUDIO)
	@$(BANNERTOOL) makebanner -o $@ -i $(APP_BANNER_IMAGE) -a $(APP_BANNER_AUDIO)
	@echo built ... $(notdir $@)

$(TARGET).cia: $(EXECUTABLE) $(APP_RSF) $(TARGET).smdh $(TARGET).bnr romfs
	@$(MAKEROM) -ver $(shell echo $$(($(VER_MAJOR)*1024+$(VER_MINOR)*16+$(VER_PATCH)))) -f cia -target t -exefslogo -o $@ -elf $(EXECUTABLE) -rsf $(APP_RSF) -banner $(TARGET).bnr -icon $(TARGET).smdh -DAPP_ROMFS=romfs/
	@echo built ... $(notdir $@)

dist_3ds: $(TARGET).cia $(TARGET).3dsx $(DIST_FILES_DOCS)
	@rm -rf dist_3ds
	@mkdir -p dist_3ds
	@cp $(TARGET).3dsx $(TARGET).cia dist_3ds/
	@cp $(DIST_FILES_DOCS) dist_3ds/
	@cp $(srcdir)/backends/platform/3ds/README.md dist_3ds/README-3DS.md
	@echo built ... $(notdir $@)

#---------------------------------------------------------------------------------
# rules for assembling GPU shaders
#---------------------------------------------------------------------------------
define shader-as
	$(eval FILEPATH := $(patsubst %.shbin.o,%.shbin,$@))
	$(eval FILE := $(patsubst %.shbin.o,%.shbin,$(notdir $@)))
	$(QUIET)$(MKDIR) $(*D)/$(DEPDIR)
	echo "$(FILEPATH): $< $1" > $(*D)/$(DEPDIR)/$(FILE).d
	$(DEVKITPRO)/tools/bin/picasso -o $(FILEPATH) $1
	$(eval _tmpasm := $(shell mktemp))
	$(DEVKITPRO)/tools/bin/bin2s -a 4 -H `(echo $(FILEPATH) | tr . _)`.h $(FILEPATH) > $(_tmpasm)
	$(CC) -x assembler-with-cpp $(CPPFLAGS) -g -march=armv6k -mtune=mpcore -mfloat-abi=hard -mtp=soft -c $(_tmpasm) -o $(FILEPATH).o
	@rm $(_tmpasm)
endef

vpath %.v.pica $(srcdir)
vpath %.g.pica $(srcdir)
vpath %.shlist $(srcdir)

%.shbin.o : %.v.pica %.g.pica
	@echo $(notdir $^)
	@$(call shader-as,$^)

%.shbin.o : %.v.pica
	@echo $(notdir $<)
	@$(call shader-as,$<)

%.shbin.o : %.shlist
	@echo $(notdir $<)
	@$(call shader-as,$(foreach file,$(shell cat $<),$(dir $<)/$(file)))
