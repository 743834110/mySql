OBJS = dataTab.o IOUtil.o dataSear.o mysql.tab.o lex.o

test: $(OBJS)
	gcc -o test $(OBJS)
	
lex.o: IOUtil.h mysql.tab.h lex.h lex.c 
	gcc -c lex.c

dataTab.o: dataTab.h dataTab.c
	gcc -c dataTab.c			
			
mysql.tab.c mysql.tab.h: mysql.y
	bison -d mysql.y
	
IOUtil.o: IOUtil.h IOUtil.c
	gcc -c IOUtil.c

dataSear.o:	dataSear.h dataSear.c
	gcc -c dataSear.c

clean:
	rm -r *.o test
