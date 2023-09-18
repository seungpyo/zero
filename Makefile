index_test:
	gcc -g -o index_test index.c index_test.c
	echo "index_test compiled, run ./index_test <num_tests> <max_string_length> to test"

clean_index_test:
	rm index_test

clean:
	make clean_index_test