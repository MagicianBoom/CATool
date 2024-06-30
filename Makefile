CC = gcc
LD = gcc

SRCS = $(wildcard *.c)
OBJS = $(patsubst %.c, obj/%.o, $(SRCS))
TARGET = obj/catool

.PHONY: all clean

# 要生成的目标文件
all: $(TARGET)

$(TARGET): $(OBJS)
	$(LD) -o $@ $^

obj/%.o: %.c
	mkdir -p obj
	$(CC) -c $< -o $@

clean:
	rm -f obj/*.o $(TARGET)
