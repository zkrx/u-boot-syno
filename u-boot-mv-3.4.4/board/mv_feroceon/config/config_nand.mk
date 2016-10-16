
BOOT_TEXT_BASE = 0x00020000

NAND_LDSCRIPT = ./board/$(BOARDDIR)/u-boot-nand.lds

NAND_LDFLAGS += -Bdynamic -T $(NAND_LDSCRIPT) -Ttext $(BOOT_TEXT_BASE) $(PLATFORM_LDFLAGS)


ifeq ($(NAND_LARGE_PAGE), y)
NAND_OBJS        = ./board/mv_feroceon/USP/nBootstrap_LP.o
else
NAND_OBJS        = ./board/mv_feroceon/USP/nBootstrap.o
endif

NAND_OBJS +=   ./board/mv_feroceon/USP/nBootloader.o
