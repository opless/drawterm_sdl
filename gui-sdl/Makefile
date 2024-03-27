ROOT=..
include ../Make.config
LIB=libgui.a

OFILES=\
	dt.$O\
	sdl.$O\
	sdl_audio.$O

default: $(LIB)
$(LIB): $(OFILES)
	$(AR) r $(LIB) $(OFILES)
	$(RANLIB) $(LIB)

