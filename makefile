file_set = record_lock v_db
objects = $(file_set:%=%.o)
origins = $(objects:%.o=src/%.cc)
g11 = g++ -std=c++11
target = libv_db.a

$(target): $(objects)
	ar -crv $(target) $(objects)

$(objects):$(origins)
	$(g11) -g -c $(origins)

.PHONY:clean
clean:
	rm $(target) $(objects)

