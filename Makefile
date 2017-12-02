OBJS = sql.tab.o dataTab.o IOUtil.o symtab.o lex.o result.o extend.o dataUpd.o

sql: $(OBJS)
	gcc -o sql $(OBJS)& ./config/build_sql.sh config/sql_source.txt

sql.tab.c sql.tab.h: sql.y
	bison -d sql.y

sql.tab.o: sql.tab.h sql.tab.c
	gcc -c sql.tab.c

lex.o: IOUtil.h sql.tab.h lex.h lex.c 
	gcc -c lex.c

dataTab.o: dataTab.h dataTab.c
	gcc -c dataTab.c 			
			
IOUtil.o: IOUtil.h IOUtil.c
	gcc -c IOUtil.c

result.o: result.h result.c
	gcc -c result.c

symtab.o:	symtab.h symtab.c
	gcc -c symtab.c

dataUpd.o:	dataUpd.h dataUpd.c
	gcc -c dataUpd.c

extend.o: extend.h extend.c
	gcc -c extend.c

clean:
	rm -rf *.o sql sql.tab.[ch] *.sql
