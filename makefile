all:
	gcc main.c src/*.c sulfur_libs/blt_libs/*.c -o build/main
	build/main
	

clean:
	rm -f build/*.o