tensor_test:
	gcc -g0 -o tensor_test tensor_test.c tensor.c

clean_tensor_test:
	rm -f tensor_test

disk_object_test:
	gcc -g0 -o disk_object_test disk_object_test.c disk_object.c tensor.c

clean_disk_object_test:
	rm -f disk_object_test
	rm -f test.zero

disk_object_so:
	gcc -shared -o zero_disk_object.so disk_object.c tensor.c

clean_disk_object_so:
	rm -f zero_disk_object.so

capi_test:
	make disk_object_so
	python zero.py
	gcc -g0 -o zero_api_test zero_api_test.c zero_disk_object.so
	./zero_api_test
	rm -f zero_api_test

clean_capi_test:
	make clean_disk_object_so
	rm -f capi.zero

clean:
	make clean_disk_object_test
	make clean_disk_object_so
	make clean_tensor_test
	make clean_capi_test
