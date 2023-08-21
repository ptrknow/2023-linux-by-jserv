all: rbtree avltree

rbtree:
	cc rbtree.c -o rbtree.out

avl_objects = avl.o test.o avl-test.o
avltree: $(avl_objects)
	cc $(avl_objects) -o avltree.out

$(avl_objects): %.o: %.c
%.o : %.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@

clean:
	rm -f *.out
