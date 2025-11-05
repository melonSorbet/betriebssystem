#
# Kurzanleitung
# =============
#
# +++ General Targets +++
#
# make                    -- siehe -> make all
#
# make all                -- Baut kernel.elf und generiert einen dump
#
# make install            -- Baut kernel.img und transferiert ihn auf den Server.
#                            Das Board holt sich diesen Kernel beim nächsten Reset.
#
# make qemu               -- Baut den Kernel und führt ihn unter QEMU aus
#
# make qemu_debug         -- Baut den Kernel und führt ihn unter QEMU mit debug
#                            Optionen aus. Zum debuggen in einem zweiten Terminal
#                            folgendes ausführen:
#                            $ arm-none-eabi-gdb build/kernel.elf
#                            $ target remote localhost:1234
# make qemu_trace         -- Wie qemu, gibt aber zusätzlich bei bestimmten MMIO-Zugriffen
#                            Meldungen aus
# make qemu_trace_mmu     -- Wie qemu_trace, gibt aber nur Meldungen zur MMU aus
#
# make gdb			      -- Startet gdb und verbindet sich mit qemu
#
# make gdb_window 	      -- Führt `make gdb` in einem neuen terminal aus
#
# make debug 		      -- Führt qemu_debug gdb_window aus
#
# make clean              -- Löscht alle erzeugten Dateien.
#
# make format             -- Formatiert alle .c und .h Dateien im Stil von kernel.org
#
# make submission         -- Verpackt alle Dateien die von git getrackt sind
#                            Hierbei wird immer der Branch genutzt, der unter
#                            SUBMISSION_BRANCH eingetragen ist.
#
# make home               -- kopiert das fertige image nach $TFTP_PATH, für die
#                            Arbeit zuhause einfach den Pfad eintragen
#
# +++ Single File Targets +++
#
# make kernel             -- Baut kernel.elf im build Ordner
#
# make kernel.bin         -- Baut kernel.bin im build Ordner
#
# make kernel.img         -- Baut kernel.img im build Ordner
#
# make kernel_only        -- Baut den kernel aber nur mit den Dateien aus SRC
#                            Zum testen, ob der Kernel unabhängig kompiliert
#
# make user_only          -- Baut nur die User Dateien aus USRC
#                            Zum testen, ob Userland unabhängig kompiliert
#
# +++ Sonstiges +++
#
# MODE=strict make TARGET -- Baut TARGET mit allen Warnings, die GCC
#                         -- unterstütz, sowie undefined behaviour sanitzers.
#                         -- Baut nicht alles neu, d.h. im Zweifel zuerst
#                         -- make clean ausführen.
#
# make qemu_record        -- Erstellt eine "Aufnahme" einer Ausführung des OS.
#                            Diese kann mit qemu_replay, qemu_debug_replay,
#                            und debug_replay zum debugging abgespielt werden.
#                            Dabei ist es dann in GDB möglich mit reverse-stepi
#                            und reverse-continue "rückwärts" zu debuggen.

# Binäre Lsg (falls verwendet, ansonsten leer lassen)
BIN_LSG = 

# Hier eure source files hinzufügen
SRC = arch/cpu/entry.S kernel/start.c arch/bsp/yellow_led.c lib/ubsan.c lib/mem.c

# Hier separate user source files hinzufügen
USRC = user/main.c

# Hier können eigene GCC flags mit angegeben werden.
# Die vorgegebenen Flags können weiter unten gefunden werden unter
# CFLAGS_ALL und dürfen nicht verändert werden
# Bsp: CFLAGS = -Wpedantic -Werror -O2
CFLAGS = -std=gnu23

# 'strict' Modus
ifeq ($(MODE), strict)
	CFLAGS_AVAIL = $(shell $(CC) -Q --help=warning | sed -e 's/^\s*\(\-\S*\)\s*\[\w*\]/\1 /gp;d' | tr -d '\n') $(CFLAGS_LAX)
	IGNORED_CFLAGS = -Wabi -Wc90-c99-compat -Wc99-c11-compat -Wc++-compat -Wtraditional \
					-Wtraditional-conversion -Wold-style-declaration -Wold-style-definition \
					-Wlong-long -Wdeclaration-after-statement -Wmain -Wswitch-default -Wstrict-flex-arrays \
					-Wsystem-headers
	CFLAGS_STRICT = $(filter-out $(IGNORED_CFLAGS), $(CFLAGS_AVAIL))

	CFLAGS += $(CFLAGS_STRICT) -Werror
	CFLAGS += -fsanitize=undefined
endif

# Gleiches gilt für Linkerflags, solltet ihr bspw. die libgcc mitlinken müssen
LDFLAGS =

# Hier den git branch angeben, der für die Abgabe benutzt werden soll
SUBMISSION_BRANCH = main

# Wenn ihr zuhause auf der Hardware arbeitet, hier das TFTP-Verzeichnis eintragen
TFTP_PATH = /srv/tftp

# Standardinstallationspunkt der Toolchain
PREFIX ?= $(HOME)/arm

# +-----------------------------------------------------+
# |                                                     |
# |   Ab hier nichts mehr anpassen! Änderungen unter-   |
# |     halb dieses Banners führen zu Punktabzug!       |
# |                                                     |
# +-----------------------------------------------------+

# Source files, die fürs Testen einkompiliert werden.
TSRC =
SRC += $(TSRC)

# build directory
BUILD_DIR = build

# separate kernel files and add build prefix
KSRC_C = $(filter %.c, $(SRC))
KSRC_S = $(filter %.S, $(SRC))
KOBJ_C = $(addprefix $(BUILD_DIR)/,$(KSRC_C:%.c=%.o))
KOBJ_S = $(addprefix $(BUILD_DIR)/,$(KSRC_S:%.S=%.o))
KOBJ =  $(KOBJ_C) $(KOBJ_S)

# separate user files and add build prefix
USRC_C = $(filter %.c, $(USRC))
USRC_S = $(filter %.S, $(USRC))
UOBJ_C = $(addprefix $(BUILD_DIR)/,$(USRC_C:%.c=%.o))
UOBJ_S = $(addprefix $(BUILD_DIR)/,$(USRC_S:%.S=%.o))
UOBJ =  $(UOBJ_C) $(UOBJ_S)

# accumulate
OBJ_C = $(KOBJ_C) $(UOBJ_C)
OBJ_S = $(KOBJ_S) $(UOBJ_S)
OBJ = $(KOBJ) $(UOBJ)

# all header files
HEADER_FILES = $$(find . -name '*.h')

# auto generated dep files
DEP = $(OBJ:.o=.d)

# linker script
LSCRIPT = kernel.lds

# static lib location for binary solution
BIN_LSG_DIR = ./

# toolchain
CC = $(PREFIX)/bin/arm-none-eabi-gcc
LD = $(PREFIX)/bin/arm-none-eabi-ld
OBJCOPY = $(PREFIX)/bin/arm-none-eabi-objcopy
OBJDUMP = $(PREFIX)/bin/arm-none-eabi-objdump
IMG = $(PREFIX)/bin/mkimage
QEMU = $(PREFIX)/bin/qemu-system-arm
QEMU_IMG = $(PREFIX)/bin/qemu-img
GDB = $(PREFIX)/bin/arm-none-eabi-gdb
FORMAT = $(PREFIX)/bin/clang-format

# configuration
CFLAGS_ALL = -Wall -Wextra -ggdb -ffreestanding -mcpu=cortex-a7 -mfloat-abi=soft -mno-unaligned-access $(CFLAGS)
CPPFLAGS = -Iinclude
LDFLAGS += -T$(LSCRIPT) -nostdlib --fatal-warnings
ifneq ($(BIN_LSG), )
	LDFLAGS += -L$(BIN_LSG_DIR) $(addprefix -l,$(BIN_LSG))
endif
OBJCOPYFLAGS = -Obinary -S --set-section-flags .bss=contents,alloc,load,data
OBJDUMPFLAGS = -dfzh
IMGFLAGS = -A arm -T standalone -C none -a 0x8000
QEMUFLAGS = -M raspi2b -nographic -icount shift=9
QEMU_REPLAY_FLAGS = -icount rrfile=$(BUILD_DIR)/replay.bin

FORMATFLAGS = --style="file:.clang-format" -i

# Regeln
.PHONY: all

# default target
all: kernel dump

# include auto generated targets
-include $(DEP)

# single file targets
$(OBJ_S):$(BUILD_DIR)/%.o: %.S
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS_ALL) -MMD -MP -o $@ -c $<

$(OBJ_C):$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS_ALL) -MMD -MP -o $@ -c $<

$(BUILD_DIR)/kernel.elf: $(OBJ) $(LSCRIPT)
ifneq ($(words $(OBJ)), $(words $(SRC) $(USRC)))
	@echo The number of objects files given to the linker does not match the\
		number of kernel and user files specified. Check your file extensions.; exit 1
else
	$(LD) -o $@ $(wordlist 2,$(words $^),x $^) $(LDFLAGS)
endif

$(BUILD_DIR)/kernel.bin: $(BUILD_DIR)/kernel.elf
	$(OBJCOPY) $(OBJCOPYFLAGS) $< $@

$(BUILD_DIR)/kernel.img: $(BUILD_DIR)/kernel.bin
	$(IMG) $(IMGFLAGS) -d $< $@

$(BUILD_DIR)/kernel_only.elf: $(KOBJ) $(LSCRIPT)
	$(LD) -o $@ $(wordlist 2,$(words $^),x $^) $(LDFLAGS)

$(BUILD_DIR)/user_only.elf: $(UOBJ) $(LSCRIPT)
	$(LD) -o $@ $(wordlist 2,$(words $^),x $^) $(LDFLAGS) --entry=0

$(BUILD_DIR)/kernel_dump.s: $(BUILD_DIR)/kernel.elf
	$(OBJDUMP) $(OBJDUMPFLAGS) $< > $@

# aliases
.PHONY: dump kernel kernel.bin kernel.img kernel_only user_only
dump: $(BUILD_DIR)/kernel_dump.s
kernel: $(BUILD_DIR)/kernel.elf
kernel.bin: $(BUILD_DIR)/kernel.bin
kernel.img: $(BUILD_DIR)/kernel.img
kernel_only: $(BUILD_DIR)/kernel_only.elf
user_only: $(BUILD_DIR)/user_only.elf

# general targets
.PHONY: install home qemu qemu_debug clean submission
install: $(BUILD_DIR)/kernel.img
	arm-install-image $(BUILD_DIR)/kernel.img

home: $(BUILD_DIR)/kernel.img
	cp -v $(BUILD_DIR)/kernel.img $(TFTP_PATH)

$(BUILD_DIR)/vmstate.qcow2: $(BUILD_DIR)/kernel.elf
	mkdir -p $(BUILD_DIR)
	$(QEMU_IMG) create -f qcow2 $@ 32M

qemu: $(BUILD_DIR)/kernel.elf
	$(QEMU) $(QEMUFLAGS) -kernel $(BUILD_DIR)/kernel.elf

qemu_record: $(BUILD_DIR)/kernel.elf
	$(QEMU) $(QEMUFLAGS) $(QEMU_REPLAY_FLAGS) -icount rr=record -kernel $(BUILD_DIR)/kernel.elf

qemu_replay: $(BUILD_DIR)/kernel.elf $(BUILD_DIR)/replay.bin
	$(QEMU) $(QEMUFLAGS) $(QEMU_REPLAY_FLAGS) -icount rr=replay -kernel $(BUILD_DIR)/kernel.elf

qemu_debug: $(BUILD_DIR)/kernel.elf
	$(QEMU) $(QEMUFLAGS) -s -S -kernel $(BUILD_DIR)/kernel.elf

qemu_debug_replay: $(BUILD_DIR)/kernel.elf $(BUILD_DIR)/vmstate.qcow2 $(BUILD_DIR)/replay.bin
	$(QEMU) $(QEMUFLAGS) $(QEMU_REPLAY_FLAGS) -drive file=$(BUILD_DIR)/vmstate.qcow2,if=none,snapshot=on,id=img-direct -icount rr=replay -s -S -kernel $(BUILD_DIR)/kernel.elf

qemu_trace: $(BUILD_DIR)/kernel.elf
	$(QEMU) $(QEMUFLAGS) -d "trace:bcm283*,trace:pl011*" -kernel $<

qemu_trace_mmu: $(BUILD_DIR)/kernel.elf
	$(QEMU) $(QEMUFLAGS) -d "mmu" -kernel $<

gdb: $(BUILD_DIR)/kernel.elf
	$(GDB) --ex="target remote localhost:1234" --ex "set scheduler-locking on" $<

gdb_window: $(BUILD_DIR)/kernel.elf
ifeq (, $(shell which x-terminal-emulator 2>/dev/null))
	@which $(TERM) || { echo "TERM not set properly, please do so, e.g. with 'export TERM=xterm'"; exit 1; }
	$(TERM) -e "bash -c 'make gdb; killall $(QEMU)'" &
else
	x-terminal-emulator -e "make gdb; killall $(QEMU)" &
endif

debug: gdb_window qemu_debug

debug_replay: gdb_window qemu_debug_replay

format:
	@$(FORMAT) $(FORMATFLAGS) $$(find . -name '*.c' -o -name '*.h')
	@echo "Finished reformatting code."

clean:
	rm -rf $(BUILD_DIR)

CURR_BRANCH = $(shell git branch --show-current 2> /dev/null)
STATUS_BRANCH = "$(shell git status --porcelain 2> /dev/null)"

submission:
ifneq ($(CURR_BRANCH), $(SUBMISSION_BRANCH))
	@echo "Refusing to create submission!"
	@echo "Current branch does not match submission branch."
	@echo " > Currently on branch: '$(CURR_BRANCH)'"
	@echo " > But registered submission branch is: '$(SUBMISSION_BRANCH)'"
	@echo "Please change branch or fix submission branch."
	@exit 1
endif
ifneq ($(STATUS_BRANCH), "")
	@echo "Refusing to create submission!"
	@echo "Submission branch is dirty or contains untracked files."
	@echo ""
	@echo "Please commit changes and add non essential files to '.gitignore'."
	@echo "Use 'git status' for a detailed list."
	@echo ""
	@exit 1
endif
	git archive --format=tar.gz -o submission.tar.gz $(SUBMISSION_BRANCH)
	@echo Successfully created submission.tar.gz
	@echo We recommend testing archive before submitting it.
