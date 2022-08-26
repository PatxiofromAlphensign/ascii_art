LD=-lm
OBJ=ascii_art.o
MAIN=sample
INCLUDE=-I./
all: $(OBJ) $(MAIN)
$(OBJ): %.o : %.c
	gcc   -c $(INCLUDE) $(^) -o $@
$(MAIN): % : %.c
	gcc    $(INCLUDE) $(OBJ) $^ -o $(@) $(LD) 
clean: 
	rm -rf $(MAIN) $(OBJ)
