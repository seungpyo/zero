disk_object_test:
	gcc -g -o disk_object_test disk_object_test.c disk_object.c tensor.c

clean_disk_object_test:
	rm -f disk_object_test
	rm -f test.zero

clean:
	make clean_disk_object_test