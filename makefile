all:	
	gcc -pthread -Wall mcsh5.c comProcessing.c memory.c mcshlist.c -o mcsh5

clean:
	rm -rf *o mcsh5
