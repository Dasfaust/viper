# GNU Make workspace makefile autogenerated by Premake

ifndef config
  config=debug
endif

ifndef verbose
  SILENT = @
endif

ifeq ($(config),debug)
  Engine_config = debug
  Client_config = debug
endif
ifeq ($(config),release)
  Engine_config = release
  Client_config = release
endif
ifeq ($(config),dist)
  Engine_config = dist
  Client_config = dist
endif

PROJECTS := Engine Client

.PHONY: all clean help $(PROJECTS) 

all: $(PROJECTS)

Engine:
ifneq (,$(Engine_config))
	@echo "==== Building Engine ($(Engine_config)) ===="
	@${MAKE} --no-print-directory -C Engine -f Makefile config=$(Engine_config)
endif

Client: Engine
ifneq (,$(Client_config))
	@echo "==== Building Client ($(Client_config)) ===="
	@${MAKE} --no-print-directory -C Client -f Makefile config=$(Client_config)
endif

clean:
	@${MAKE} --no-print-directory -C Engine -f Makefile clean
	@${MAKE} --no-print-directory -C Client -f Makefile clean

help:
	@echo "Usage: make [config=name] [target]"
	@echo ""
	@echo "CONFIGURATIONS:"
	@echo "  debug"
	@echo "  release"
	@echo "  dist"
	@echo ""
	@echo "TARGETS:"
	@echo "   all (default)"
	@echo "   clean"
	@echo "   Engine"
	@echo "   Client"
	@echo ""
	@echo "For more information, see https://github.com/premake/premake-core/wiki"