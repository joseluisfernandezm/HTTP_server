CC=gcc
CFLAGS=-c -Wall
LDFLAGS=
SOURCES=HTTPServidor.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=HTTPServidor
#Ficheros TXT a incluir en la entrega
TXT=contenidos.txt
#Ficheros .h a incluir en la entrega
HEADERS=

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	@echo -n compilando el Ejecutable \'$@\' ...
	@$(CC) $(LDFLAGS) $(OBJECTS) -o $@
	@echo [OK]

.c.o:
	@echo -n compilando Objeto \'$<\' ...
	@$(CC) $(CFLAGS) $< -o $@
	@echo [OK]

clean:
	@echo -n limpiando proyecto ...
	@rm -f $(wildcard *.o *.d core* *.P) $(EXECUTABLE)
	@echo [OK]
entrega:
	@echo Introduce tu grupo [3261,3262,3263]; \
	read g; \
	echo Introduce tu numero del alumno (dos digitos):; \
	read p;\
	echo  creando zip de entrega "P1_"$$g"_P"$$p".zip"...;\
	zip "P1_"$$g"_P"$$p".zip" $(SOURCES) $(TXT) $(HEADERS) makefile	

	
	@echo [OK]
