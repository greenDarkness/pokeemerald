# This controls building executables in the `tools` folder.
# Can be invoked through the `Makefile` or standalone.

MAKEFLAGS += --no-print-directory

# Inclusive list. If you don't want a tool to be built, don't add it here.

TOOLS_DIR := tools
TOOL_NAMES := aif2pcm bin2c gbafix gbagfx jsonproc mapjson mid2agb preproc ramscrgen rsfont scaninc
TOOLDIRS := $(TOOL_NAMES:%=$(TOOLS_DIR)/%)

# Tool making doesnt require a pokeemerald dependency scan.
RULES_NO_SCAN += tools check-tools clean-tools $(TOOLDIRS)
.PHONY: $(RULES_NO_SCAN)

tools: agbcc $(TOOLDIRS)

$(TOOLDIRS):
	@$(MAKE) -C $@

clean-tools:
	@$(foreach tooldir,$(TOOLDIRS),$(MAKE) clean -C $(tooldir);)

# Clone and build agbcc if not present
.PHONY: agbcc
agbcc:
ifeq (,$(wildcard agbcc))
	git clone https://github.com/pret/agbcc || true
endif

ifeq (,$(wildcard agbcc/.rebuilt))
	cd agbcc && git clean -fX && ./build.sh && ./install.sh ../../pokeemerald
	touch agbcc/.rebuilt
else
ifeq (,$(wildcard tools/agbcc/bin/agbcc$(EXE)))
	cd agbcc && ./install.sh ../../pokeemerald
endif
endif
