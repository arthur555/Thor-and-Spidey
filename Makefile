CC=		gcc
CFLAGS=		-g -gdwarf-2 -Wall -Werror -std=gnu99
LD=		gcc
LDFLAGS=	-L.
AR=		ar
ARFLAGS=	rcs
TARGETS=	spidey

all:		$(TARGETS)

spidey: 	forking.o handler.o request.o single.o socket.o spidey.o utils.o
	$(LD) $(LDLAGS) -o $@ $^


%.o: 	%.c
	$(CC) $(CFLAGS) -c -o $@ $^

clean:
	@echo Cleaning...
	@rm -f $(TARGETS) *.o *.log *.input

.SUFFIXES:
.PHONY:		all test benchmark clean
