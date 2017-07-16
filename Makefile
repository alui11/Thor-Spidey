CC=		gcc
CFLAGS=		-g -gdwarf-2 -Wall -std=gnu99
LD=		gcc
LDFLAGS=	-L.
TARGETS=	spidey

all:		$(TARGETS)

spidey: forking.o handler.o request.o single.o socket.o spidey.o utils.o
	@echo "Linking $@..."
	@$(LD) $(LDFLAGS) -o $@ $^

%.o: %.c spidey.h
	@echo "Compiling $@..."
	@$(CC) $(CFLAGS) -c -o $@ $<

clean:
	@echo Cleaning...
	@rm -f $(TARGETS) *.o *.log *.input

.PHONY:		all clean
