# -----------------------------------------------------------------------------
#.SILENT:
VERIF = $(WORKINGDIR)/verif

SRC :=  program.c \


DEFINEFLAGS +=  DEBUG_RW_MSG \

include $(VERIF)/C_ENV/SOC_ENV/make.include.c_env
clean:
	\rm -rf *.bin *.o *.map *.lst image.htm *.out *.mem *.axf *.so objects  temp.hex
