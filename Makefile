LD=-lm
OBJ=ascii_art.o
MAIN=sample
INCLUDE=-I./
all: $(MAIN)
$(OBJ): %.o : %.c
	gcc   -c $(INCLUDE) $(^) -o $@
$(MAIN): $(OBJ)
	gcc   $(@).c $(INCLUDE) -o $(@) $(LD) $^
clean: 
	rm -rf $(MAIN)
