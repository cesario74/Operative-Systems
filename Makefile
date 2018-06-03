OBJS = $(patsubst %.c, %.o, $(wildcard *.c))
EXEC=notebook

install:$(OBJS)
		gcc -o $(EXEC) $(OBJS)

clean:
		rm $(EXEC) *.o
