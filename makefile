CC=gcc
CFLAGS= -pthread -Wall -lm

ifeq ($(DEBUG), 1)
	CFLAGS += -g -fsanitize=address
else
	CFLAGS += -O3
endif 

TARGET=hello return_stack_ptr show_stack show_tid detach kway_merge_sort bind_affinity vec_sum shared_data shared_data_mutex deadlock bank exercise/hw1 exercise/hw2 exercise/hw3 exercise/hw3d
ALL: $(TARGET)

$(TARGET): %: %.c
	$(CC) -o $@ $< $(CFLAGS)

clean:
	rm -rf *.o $(TARGET) *.s
