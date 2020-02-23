objects = test.o record_lock.o v_db.o
origins = $(objects:%.o=%.cc)
g11 = g++ -std=c++11
target = test_output

$(target): $(objects)
	$(g11) -o $(target) $(objects)

$(objects):$(origins)
	$(g11) -g -c $(origins)

.PHONY:clean
clean:
	rm target $(objects)

