tensor_test:
	gcc -g -o tensor_test tensor.c tensor_test.c
	echo "index_test compiled, run ./tensor_test to test"

clean_tensor_test:
	rm -f tensor_test

index_test:
	gcc -g -o index_test index.c index_test.c tensor.c
	echo "index_test compiled, run ./index_test <num_tests> <max_string_length> to test"

clean_index_test:
	rm -f index_test

clean:
	make clean_tensor_test
	make clean_index_test
