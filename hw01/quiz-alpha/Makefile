all: rbtree avltree stree

rbtree:
	cc rb/rbtree.c -o rbtree.out

avl_dir = avl
avl_objects = avl.o test.o avl-test.o
avltree: $(avl_objects)
	cc $(avl_objects) -o avltree.out

$(avl_objects): %.o: $(avl_dir)/%.c
%.o : $(avl_dir)/%.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@

stree:
	cc s/s-tree.c -o s-tree.out

clean:
	rm -f *.out *.o
