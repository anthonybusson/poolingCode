pooling: main.o aleatoire.o header.h aleatoire.h simulator.o testUnitaire.o parserTrace.o simulator.h scheduling.o scheduling.h testUnitaire.h parserTrace.h 
	gcc -Wall -g main.o aleatoire.o simulator.o scheduling.o testUnitaire.o parserTrace.o -lm -o pooling

main.o: main.c header.h aleatoire.h
	gcc -c -g -Wall main.c -lm -o main.o

scheduling.o: scheduling.c scheduling.h header.h simulator.h
	gcc -c -g -Wall scheduling.c -lm -o scheduling.o 

simulator.o: simulator.c simulator.h header.h scheduling.h parserTrace.h
	gcc -c -g -Wall simulator.c -lm -o simulator.o 

parserTrace.o: parserTrace.c parserTrace.h header.h
	gcc -c -g -Wall parserTrace.c -lm -o parserTrace.o 

aleatoire.o: aleatoire.c aleatoire.h
	gcc -c -g -Wall aleatoire.c -lm -o aleatoire.o 

testUnitaire.o: testUnitaire.c testUnitaire.h
	gcc -c -g -Wall testUnitaire.c -lm -o testUnitaire.o 

clean: 
	rm *.o pooling
