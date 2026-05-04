# -----------------------------------------------------------------------------
#.SILENT:
VERIF = $(WORKINGDIR)/verif

SRC := src/program.c 

# Ensure local include path for test_define.c
CFLAGS += -Iinclude

include $(VERIF)/C_ENV/SOC_ENV/make.include.c_env

clean:
	\rm -rf *.bin *.o *.map *.lst image.htm *.out *.mem *.axf *.so objects temp.hex
