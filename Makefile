assemble:	assembler.c
		gcc -std=c99 -o assemble assembler.c

clean:
		rm assemble reformattedInFile