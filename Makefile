CC = clang
CFLAGS = -Wall -Wvla -Werror -gdwarf-4

.PHONY: asan msan nosan

asan: CFLAGS += -fsanitize=address,leak,undefined
asan: all

msan: CFLAGS += -fsanitize=memory,undefined -fsanitize-memory-track-origins
msan: all

nosan: all

.PHONY: all
all: lvc

testPoodle: lvc.c
	$(CC) $(CFLAGS) -o lvc lvc.c

.PHONY: clean
clean:
	rm -f lvc
