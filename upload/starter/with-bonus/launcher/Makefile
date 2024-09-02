
LOADER_PATH = $(shell realpath ../bin)


all:

#   echo $(LOADER_PATH)
	gcc -m32 -c -fpic launch.c && gcc -m32 -L$(LOADER_PATH) -Wl,-rpath=$(LOADER_PATH) -o launch launch.o -l_simpleloader && mv launch ../bin/

clean :
		rm -f ../bin/launch
