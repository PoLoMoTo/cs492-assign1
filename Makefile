# Name of your program:
NAME=assign1

# List of all .c source code files included in your program (separated by spaces):
SRC=main.c

SRCPATH=./
OBJ=$(addprefix $(SRCPATH), $(SRC:.c=.o))

RM=rm -f
INCPATH=includes
CFLAGS+=-g3

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
	valgrind ./$(NAME) 3 3 50 5 0 0 23492
