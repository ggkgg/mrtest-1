CC = gcc
OBJECTS = mr_test.o ../mr_common.o

mrtest: $(OBJECTS)
	$(CC) -o mrtest $(OBJECTS) -lpthread

mr_test.o: mr_test.c mr_test.h
	$(CC) -c mr_test.c

mr_commom.o: ../mr_common.c ../mr_common.h
	$(CC) -c ../mr_common.c

clean :
	rm $(OBJECTS) 
