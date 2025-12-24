obj-m += kernelsp.o

all user.c:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	gcc -o user user.c

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm user
