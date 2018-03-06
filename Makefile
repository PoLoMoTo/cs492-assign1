# Name of your program:
NAME=assign1

# List of all .c source code files included in your program (separated by spaces):
SRC=main.c

SRCPATH=./
OBJ=$(addprefix $(SRCPATH), $(SRC:.c=.o))

RM=rm -f
INCPATH=includes
CFLAGS+=-g3 -std=c99 -std=gnu99
CC = gcc

all: $(OBJ)
	gcc $(OBJ) -o $(NAME) -lpthread

clean:
	-$(RM) *~
	-$(RM) *#*
	-$(RM) *swp
	-$(RM) *.core
	-$(RM) *.stackdump
	-$(RM) $(SRCPATH)*.o
	-$(RM) $(SRCPATH)*.obj
	-$(RM) $(SRCPATH)*~
	-$(RM) $(SRCPATH)*#*
	-$(RM) $(SRCPATH)*swp
	-$(RM) $(SRCPATH)*.core
	-$(RM) $(SRCPATH)*.stackdump

fclean: clean
	-$(RM) $(NAME)

re: fclean all

test: fclean all
	./$(NAME) 10 10 500 50 0 0 23492 > stat1fcfs.txt
	./$(NAME) 10 10 1000 50 0 0 23492 > stat2fcfs.txt
	./$(NAME) 10 10 1500 50 0 0 23492 > stat3fcfs.txt
	./$(NAME) 10 2 1000 50 0 0 23492 > stat4fcfs.txt
	./$(NAME) 10 20 1000 50 0 0 23492 > stat5fcfs.txt
	./$(NAME) 10 40 1000 50 0 0 23492 > stat6fcfs.txt
	./$(NAME) 2 10 1000 50 0 0 23492 > stat7fcfs.txt
	./$(NAME) 20 10 1000 50 0 0 23492 > stat8fcfs.txt
	./$(NAME) 40 10 1000 50 0 0 23492 > stat9fcfs.txt
	./$(NAME) 10 10 1000 0 0 0 23492 > stat10fcfs.txt
	./$(NAME) 10 10 1000 20 0 0 23492 > stat11fcfs.txt
	./$(NAME) 10 10 1000 500 0 0 23492 > stat11fcfs.txt
	./$(NAME) 10 10 500 50 1 100 23492 > stat1rr.txt
	./$(NAME) 10 10 1000 50 1 100 23492 > stat2rr.txt
	./$(NAME) 10 10 1500 50 1 100 23492 > stat3rr.txt
	./$(NAME) 10 2 1000 50 1 100 23492 > stat4rr.txt
	./$(NAME) 10 20 1000 50 1 100 23492 > stat5rr.txt
	./$(NAME) 10 40 1000 50 1 100 23492 > stat6rr.txt
	./$(NAME) 2 10 1000 50 1 100 23492 > stat7rr.txt
	./$(NAME) 20 10 1000 50 1 100 23492 > stat8rr.txt
	./$(NAME) 40 10 1000 50 1 100 23492 > stat9rr.txt
	./$(NAME) 10 10 1000 100 1 100 23492 > stat10rr.txt
	./$(NAME) 10 10 1000 20 1 100 23492 > stat11rr.txt
	./$(NAME) 10 10 1000 500 1 100 23492 > stat11rr.txt
	./$(NAME) 10 10 1000 50 1 50  23492 > stat2rr.txt
	./$(NAME) 10 10 1000 50 1 200 23492 > stat2rr.txt
	./$(NAME) 10 10 1000 50 1 700 23492 > stat2rr.txt
