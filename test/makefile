g11 = g++ -std=c++11

generate_input: generate_input.cc
	$(g11) -g -o generate_input generate_input.cc libv_db.a

test_output: test_output.cc
	$(g11) -g -o test_output test_output.cc libv_db.a

.PHONY:clean
clean:
	rm generate_input test_output
