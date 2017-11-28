OBJS = dataTab.o IOUtil.o dataSear.o mysql.tab.o lex.o result.o extend.o

test: $(OBJS)
	gcc -o test $(OBJS)

mysql.tab.c mysql.tab.h: mysql.y
	bison -d mysql.y

lex.o: IOUtil.h mysql.tab.h lex.h lex.c 
	gcc -c lex.c

dataTab.o: dataTab.h dataTab.c
	gcc -c dataTab.c 			
			
IOUtil.o: IOUtil.h IOUtil.c
	gcc -c IOUtil.c

result.o: result.h result.c
	gcc -c result.c

dataSear.o:	dataSear.h dataSear.c
	gcc -c dataSear.c

extend.o: extend.h extend.c
	gcc -c extend.c

clean:
	rm -r *.o test
