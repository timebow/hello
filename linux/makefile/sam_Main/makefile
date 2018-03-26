TARGET := ./target/test2
OBJ    := ./obj/main.o
SRC    := ./main.c

# it's better to declare all fake targets(NOT real files), to avoid confict
# with those files with same name. 
.PHONY: test1 test2 test3 test4 test5 clean

test1:
    # directly run "gcc -o test main.c" to product target "test"
	gcc -o test main.c

test2:
    # $@=test2, here. $@ is the full name(include path) of target
	gcc -o $@ main.c

test3:
    # use $(TARGET) to define full name of target
	gcc -o $(TARGET) main.c

test4:
    # compile and output file as $(OBJ)
	gcc -c $(SRC) -o $(OBJ)
    # link objs to produce $(TARGET)
	gcc -o $(TARGET) $(OBJ)

test5:$(TARGET)
#$(TARGET) need $(OBJ), and do "gcc -o $@ $(OBJ)" to produce $(TARGET)
$(TARGET):$(OBJ)
	gcc -o $@ $(OBJ)
#$(OBJ) need $(SRC), and do "gcc -c $(SRC) -o $@" to produce $(OBJ)
$(OBJ):$(SRC)
	gcc -c $(SRC) -o $@

clean:
	rm test test2 $(TARGET) $(OBJ)