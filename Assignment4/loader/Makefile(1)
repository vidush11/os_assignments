
all:
	gcc -m32 -o loader.o -c -fpic loader.c && gcc -m32 -shared -o lib_simpleloader.so loader.o && mv lib_simpleloader.so ../bin/

clean :
		rm -f ../loader/lib_simpleloader.so
