CFLAGS+=-Wno-implicit-function-declaration

sender: sender.c
	gcc $(CFLAGS) sender.c util.c -o sender -lpthread

receiver: receiver_pre_pre.c 
	gcc $(CFLAGS) receiver_pre_pre.c util.c -o receiver -lpthread
	
clean:
	rm -rf sender receiver