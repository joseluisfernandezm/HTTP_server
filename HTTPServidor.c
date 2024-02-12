#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/stat.h>


#define MAX_CONNECTION 6


int sockfd;
int newsock;

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

//Mime, 23/10/2020
//Funcionalidad, esta funcion se encarga de buscar en el directorio en funcion del mime que esté pidiendo ( html,gif,pdf, png...). Pasandole como argumento la extension del recurso que han pedido
//y mediante la funcion popen se ejecuta el comando awk que nos muestra los archivos mime que tenemos disponibles. el mime encontrado a raiz de la extension dada se guardará en tipo y se usará en la funcion process request,
// el mime es guardado en faux una vez que la funcion popen haya ejecutado y leido el comando.
//Entrada, *tipo que es el que va a indicar a la http request el tipo de archivo que vamos a mandar, por ejemplo image/gif o application/pdf,
//         *extension recoge la extension del recurso solicitado
//Salida retorna -1 si hay fallos en popen 1 si la variable extension esta "vacia", vale 0, o tambien si no se encuentra el mime type correspondiente. Y 0 si se ha encontrado y pasado a la variable tipo de forma correcta.


int Mime(char *tipo, char* extension) {
  	FILE *faux;
	tipo[0]=0;
	char comando[1000]={0};
	if(strlen(extension)==0)
		return 1;
	sprintf(comando,"awk '{for(i=2;i<=NR;i++){if($i==\"%s\")print $1}}' /etc/mime.types",extension);
    faux=popen(comando,"r");
    if(faux==NULL){
		printf("ERROR abriendo fichero auxilar popen\n");
		return -1;
	}
    fscanf(faux,"%s",tipo);
	pclose(faux);
	if(strlen(tipo)==0)
		return 1;   //No encontrado!
	return 0;       //Encontrado!
}

//Busqueda, 23/10/2020
//Funcionalidad; es la de buscar el fichero solicitado por el cliente en el directorio para indicar si este se encuentra o no en el mismo. Para ello concatena a la ruta del fichero el nombre del mismo precedido por /
//como ya mencionaremos en el resto de las cabeceras de las siguientes funciones. Una vez concatenada la URL, con la funcion stat comprobaremos si de verdad existe o no el recurso.
//
//Entrada, *ruta_fichero, puntero que apunta a -> directorio_de_recursos:cadena de caracteres que guardará la ruta del fichero en el server, su contenido se encuentra en ARedes1.txt del directorio que leeremos en la funcion de lectura
//*nombre_fichero puntero que apunta a -> recurso:nombre del archivo que se quiere buscar, se recoge a partir de la peticion Get del cliente, si este no indica ninguno, la funcion process_request proporcionará el recurso pr defecto prueba.html
//struct stat *info, puntero al que pasamos una variable de la estructura stat necesaria para el empleo de la misma funcion stat.
// recurso_defecto: char que guardará el nombre del archivo por defecto recogido de ARedes1.txt// nombre_fichero: char que recoge el nombre del fichero que vamos a abrir para extraer la info
//Salida un entero que nos indica si no se ha podido encontrar el fichero -1 y si ha podido 0
int busqueda(char *ruta_fichero,char *nombre_fichero, struct stat * info) {
	char ruta_y_nombre_de_fichero[2000]={0};
  int ret3;
	strcat(ruta_y_nombre_de_fichero,ruta_fichero);
	strcat(ruta_y_nombre_de_fichero,"/");
	strcat(ruta_y_nombre_de_fichero,nombre_fichero);

	printf("\nA buscar --%s--\n",ruta_y_nombre_de_fichero);

  ret3=stat(ruta_y_nombre_de_fichero,info);


	if(ret3!=0){
    printf("No se encontro el fichero en el directorio\n");
		return -1;
  }
	else{
		return 0;
  }
}


void process_request(int newsock,char *directorio_de_recursos, char *recurso_defecto){
	char buffer[600000];
	int n, s;
  int ret2,retm;
  unsigned int ret3;
  char tipo[1000];
  char extension[64]={0};
  char metodo[1000];
  char recurso[4000];
  char version[1000];
  char cadena[4000];
  FILE *d;
  struct stat info;



	bzero(buffer,4096);
	//leemos del socket hasta 4096 bytes
	n = read(newsock,buffer,4096);
	if (n < 0)
		error("ERROR leyendo del socket");

	//Si no hay contenido que leer salimos
         if(strlen(buffer)==0)
         {
                 close(newsock);
                 return;
         }

	/*****PRACTICA IMPLEMENTAR AQUI***/
/*
Para la correcta realización de las prácticas se incluyen las siguientes tareas que (al menos) deben realizarse dentro de la función process_request:

1 Obtener método, recurso y versión de la petición HTTP almacenada en la variable buffer. Se recomienda el uso de la función sscanf.
2 Comprobar que el método de la petición sea GET y la versión HTTP/1.1 o HTTP/1.0
3 Comprobar si el recurso que se solicita es /.
        3a Si es /, simplemente el recurso solicitado es el recurso por defecto (tal como se leyó en el fichero de configuración).
4 Se debe comprobar la existencia del recurso solicitado por el navegador. Para ello se recomienda el uso de la función stat.
     4a Si el recurso existe se debe devolver una cabecera HTTP 200 OK junto con el contenido del recurso (por ejemplo, web solicitada) solicitado.
         4aa Para construir la cabecera de respuesta se recomienda el uso de la función sprintf. El contenido del campo   Content-Length se obtendrá de la función stat (ver código de ayuda) mientras que el contenido del campo Content-Type se obtendrá del fichero de mimes (ver código de ayuda).
         4ab Para la lectura de los recursos (ficheros a enviar como respuesta) se debe hacer uso de la funcion fread y NO de la funcion fgets
     4b Si el recurso no existe se debe devolver una cabecera HTTP 404 Not Found que no contenga Content-Type ni Content-Length (mirar ejemplos enunciado, o Wireshark!).

*/

  sscanf(buffer,"%s %s %s", metodo, recurso, version);

  printf("\nPeticion HTTP del cliente al server;\n%s\n",buffer );

  if(strcmp("GET", metodo)){
    printf("no se puede leer el metodo de la peticion");
    close(newsock);
    return;
  }
  if(strcmp(recurso,"/")==0){
    strcpy(recurso, recurso_defecto);
  }
  else{
    strcpy(recurso, recurso+1);
  }
//  printf("Recurso nuevo a buscar: %s\n", recurso);
  if(strcmp("HTTP/1.1", version) && strcmp("HTTP/1.0", version)){
    printf("ERROR: La version no corresponde con HTTP");
    close(newsock);
    return;
  }
  ret2=busqueda(directorio_de_recursos, recurso, &info); //es donde el servidor almacena todos los recursos (fotos, etc). c
  printf("El tamaño del recurso es %ld bytes", info.st_size);

  if(ret2==-1){

    sprintf(buffer, "%s 404 Not Found\r\n",version);
    s=send(newsock,buffer,strlen(buffer),0);
    printf("\nEl mensaje es:\n%s\n",buffer);

  close(newsock); // cerramos el socket nuevo que se ha creado en el otro sentido
  }


if(ret2==0){

char * extension_limit = NULL;
char * tipo_extension = (char*)malloc(150*sizeof(char));

if(tipo_extension == NULL){
  printf("ERROR AL RESERVAR MEMORIA\n");
  return;
}

extension_limit = strstr(recurso,".");
printf("\nLa extension del recurso es:%s\n", extension_limit);


if(extension_limit == NULL){
  strcpy(tipo,"application/octet-stream");
}

else{

	tipo_extension = extension_limit+1;
  retm=Mime(tipo,tipo_extension);

  if(retm== -1){
    strcpy(tipo,"application/octet-stream");
  }

  sprintf(buffer, "%s 200 OK\r\nContent-Type:%s\r\nContent-length:%ld\r\n\r\n",version,tipo,info.st_size);// se guarda la respuesta en buffer
  printf("\nRespuesta HTTP del server al cliente;\n%s\n", buffer);

  if (write(newsock,buffer,strlen(buffer)) < 0){
      error("ERROR writing to socket");
  }

  int i = 0;//variables auxiliares para bucles
  int j = 0;


//apertura del recurso
char search[2000]={0};
int ret3;
strcat(search,directorio_de_recursos);// concatenacion necesaria para indicar la ruta del recurso que vamos a abrir
strcat(search,"/");
strcat(search,recurso);



  d=fopen (search,"r");
    if(d == NULL){
    printf("Error al abrir el recurso a mandar (%s).\n",recurso);

  }

  while(i < info.st_size)
  {
      j = fread(cadena,1,1048,d);

      if (write(newsock,cadena,j) < 0)
      {
          error("ERROR writing to socket");
      }

      i = i + j;
  }


}


	//cerramos el socket nuevo que se ha creado en el otro sentido
	close(newsock);
}
}

void handle(int nsignal){
	printf("server esta apagandose\n");
	close(sockfd);
	close(newsock);
	exit(0);
}

//lectura_fichero_de_parametros, 23/10/2020
//Funcionalidad, recoge los parametros del archivo ARedes1.txt y lo guarda en las variables de entrada de la funcion.
//Entrada,portno:numero de puerto a asignar// directorio_de_recursos:cadena de caracteres que guardará la ruta del fichero en el server//
// recurso_defecto: char que guardará el nombre del archivo por defecto recogido de ARedes1.txt// nombre_fichero: char que recoge el nombre del fichero que vamos a abrir para extraer la info
//Salida un entero que nos indica si no se ha podido llevar a cabo la lectura -1 si no hay fallo de lectura 0, todo esta OK
int lectura_fichero_de_parametros(int *port2, char *directorio_de_recursos, char *recurso_defecto, char* nombre_fichero) {
	FILE *f;

	f=fopen (nombre_fichero,"r");
    if(f == NULL){
		printf("Error al abrir el fichero de configuración (%s).\n",nombre_fichero);
		return -1;
	}


    if(fscanf(f,"%d %s %s", port2, directorio_de_recursos,recurso_defecto)==3){
      printf("lectura correcta\n");
    }
    else {
		printf("Error2 al leer el fichero de configuración.\n");
		return -1;
	}

    fclose(f);
	return 0;
}




int main(int argc, char *argv[])
{
	int portn;
	struct sockaddr_in serv_addr,cli_addr;
	int n, newsock;



  int ret1, port2=0;
  char recurso_defecto[1000]={0}, directorio_de_recursos[1000]={0};// cadenas que guardaran el nombre del archivo por defecto y la direccion de la carpeta de recursos






	// Crear un socket para manejar conexiones TCP
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		error("ERROR abriendo socket");



	/*****PRACTICA IMPLEMENTAR AQUI***/

	/*Lectura de fichero de configuracion y asignacion de puerto a variable portn*/
	/* (ver código de ayuda) */
	/**********************************/
  ret1=lectura_fichero_de_parametros(&port2, directorio_de_recursos, recurso_defecto,"ARedes1.txt");
  printf("Se ha leido un puerto=%d, un directorio de recursos=%s, y un recurso por defecto=%s\n",port2,directorio_de_recursos,recurso_defecto);
	// Preparar una estructura con informacion sobre como vamos
	// a recibir las conexiones TCP
	// En este ejemplo, enlazaremos el socket a la IP local al puerto TCP PORT
	// Esto lo conseguiremos inicializando
	// la estructura a cero mediante, por ejemplo, 'bzero'.
	bzero((char *) &serv_addr, sizeof(serv_addr));
	portn = port2;// cambiamos el puerto por defecto con el pedido
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portn);


	// Activaremos una propiedad del socket que permitira que otros
	// sockets puedan reutilizar cualquier puerto al que nos enlacemos.
	// Esto permitira en protocolos como el TCP, poder ejecutar un
	// mismo programa varias veces seguidas y enlazarlo siempre al
	// mismo puerto. De lo contrario habria que esperar a que el puerto
	// quedase disponible (TIME_WAIT en el caso de TCP)
	if (setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&serv_addr,sizeof(struct sockaddr_in))==-1)
	{
 		error("ERROR: Fallo al reutilizar el puerto.\n");
	}

	if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
		error("ERROR on binding");

	/* Habilitar socket para recibir una conexion */
	if (listen(sockfd, MAX_CONNECTION) == -1){
		printf("Error en listen\n");
		close(sockfd);
		return -1;
	}

	/* Captura la senal SIGINT */
	if(signal(SIGINT,handle)==SIG_ERR)
	{
		error("ERROR: Fallo al capturar la senal SIGINT.\n");
	}

	/* While (true) */
	while (1){
		/* Espera de conexion */
		n = sizeof(struct sockaddr_in);
		if ((newsock=accept(sockfd,(struct sockaddr*)&cli_addr,(socklen_t *) &n)) == -1){
			printf("Error en accept\n");
			continue;
		}
		/* procesar peticion */
		process_request(newsock,directorio_de_recursos, recurso_defecto);
	}
	close(sockfd);
	return 0;
}
