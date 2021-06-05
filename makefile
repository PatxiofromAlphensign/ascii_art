ld=-lm
lib=ascii_art
MAIN=sample
include=-I./

compile: clean compile 
	gcc  $(include) $(MAIN).c $(lib).c -o $(MAIN) $(ld)
clean: 
	rm -rf $(MAIN)
