CC = gcc
OBJECTS = mr_test.o mr_common.o mr_timer.o

mrtest: $(OBJECTS)
	$(CC) -o mrtest $(OBJECTS) -lpthread

mr_test.o: mr_test.c mr_test.h
	$(CC) -c mr_test.c

mr_commom.o: mr_common.c mr_common.h
	$(CC) -c mr_common.c

mr_timer.o: mr_timer.c mr_timer.h
	$(CC) -c mr_timer.c

clean :
	rm $(OBJECTS)
