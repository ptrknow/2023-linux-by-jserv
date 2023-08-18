all: normal optimize sanitize optimize_sanitize

normal:
	cc qsort-mt.c -o qsort-mt-normal.out

optimize:
	cc qsort-mt.c -DUSE_4_BYTE_SWAP -o qsort-mt-opt.out

sanitize:
	cc qsort-mt.c -o qsort-mt-san.out -fsanitize=thread -g -O1 -fPIE -pie

optimize_sanitize:
	cc qsort-mt.c -DUSE_4_BYTE_SWAP -o qsort-mt-opt-san.out -fsanitize=thread -g -O1 -fPIE -pie

clean:
	rm -f *.out