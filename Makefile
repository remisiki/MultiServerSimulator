CC      = gcc

CFLAGS  = -std=c11 -Wconversion -Wall -Werror -Wextra -pedantic -mrdrnd -O3

LDFLAGS = 

TARGET  = sim

SRCDIR  = ./src

SRCS    = $(wildcard $(SRCDIR)/*.c)

OBJDIR  = ./obj

OBJS    = $(addprefix $(OBJDIR)/, $(notdir $(SRCS:.c=.o)))

INCDIR  = -I./inc -I/usr/include

LIBDIR  = 

LIBS    = -lgslcblas -lgsl -lm

$(TARGET): $(OBJS) $(LIBS)
	$(CC) -o $@ $^ $(LDFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	-mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) $(INCDIR) -o $@ -c $<

all: clean $(OBJS) $(TARGET)

clean:
	-rm -f $(OBJS) $(TARGET) *.d
