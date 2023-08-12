all: normal sanitize

normal:
	cc qsort-mt.c -o qsort-mt-normal.out

sanitize:
	cc qsort-mt.c -o qsort-mt-san.out -fsanitize=thread -g -O1 -fPIE -pie

clean:
	rm -f *.out