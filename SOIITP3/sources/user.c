#include <stdio.h>
#include <ulfius.h>
#include <jansson.h>
#include <unistd.h>
#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>
#include <crypt.h>
#include<string.h>
#include <time.h>
#include <yder.h>

#define PORT 8082
#define TAM 256

struct passwd* findUser(struct passwd* entry,const char* username);
/**
 * @brief callback para obtener lista de usuarios obtiene todos los usuarios del sistema y los devuelve como un arreglo objetos json

 *@returns int32_t

*/

int callback_list_users (const struct _u_request * request, struct _u_response * response, void * user_data) {
  struct passwd* entry=getpwent();
  json_t *array=json_array();
  json_t *body;

  //tomo cada entrada de /etc/passwd y lo guardo en array
  while(entry!=NULL){
    json_t* user=json_pack("{s:i,s:s}",
      "user_id",entry->pw_uid,
      "username",entry->pw_name
    );
    json_array_append(array,user);
    entry=getpwent();
  }
  //guardo array bajo la key data
  body=json_pack("{s:o}", "data", array);
  y_log_message(Y_LOG_LEVEL_INFO, "cantidad de usuarios %d", json_array_size(array));

  ulfius_set_json_body_response(response, 200, body);
  //cierro el archivo de passwd
  endpwent();
  return U_CALLBACK_CONTINUE;
}

/**
 * @brief callback para post a server para crear usuario nuevo toma el usuario y contraseña y crea un usuario nuevo en el sistema, siempre que no exista el usuario, existan
 * usuario y contraseña y tanto usuario como contraseña sean strings

 *@returns int32_t

*/
int callback_create_user (const struct _u_request * request, struct _u_response * response, void * user_data) {
  json_t* body=ulfius_get_json_body_request(request,NULL);
  char buffer[TAM];
  
  //si usuario o contraseña no son texto o no existen entonces devuelvo estado 400
  if(json_object_get(body,"username")==NULL || json_object_get(body,"password")==NULL || !json_is_string(json_object_get(body,"username")) || !json_is_string(json_object_get(body,"password")) ){
    body=json_pack("{s:s}",
    "description","el nombre de usuario y la contraseña deben ser cadenas de texto.");
    ulfius_set_json_body_response(response, 400, body);
    return U_CALLBACK_COMPLETE;
  }

  const char *username=json_string_value(json_object_get(body,"username")); 
  char password[TAM];
  //encierro la contraseña entre comillas dobles para poder admitir espacios
  sprintf(password,"%c%s%c",34,json_string_value(json_object_get(body,"password")),34);
  
  struct passwd* entry=calloc(1,sizeof(entry));
  //busco el usuario si ya existe entonces devuelvo 409
  entry=findUser(entry,username);
  if(entry!=NULL){
    body=json_pack("{s:s}",
    "description","El nombre de usuario solicitado ya existe");
    ulfius_set_json_body_response(response, 409, body);
    endpwent();
    return U_CALLBACK_COMPLETE;
  }else{
    //cierro archivo de claves
     endpwent();
  }

  //guardo el comando en el buffer y luego lo corro con system
  sprintf(buffer,"sudo useradd -p $(openssl passwd -1 %s) %s",password,username);
  system(buffer);
  //parseo timestamp
  time_t tiempo = time(0);
  struct tm *tlocal = localtime(&tiempo);  
  strftime(buffer,TAM,"%d/%m/%y %H:%M:%S",tlocal);

  //busco el usuario creado si existe devuelvo 200 con los datos correspondientes y en caso contrario 500
  entry=findUser(entry,username);
  if(entry==NULL){
    body=json_pack("{s:s}",
    "error","error creando usuario en el sistema");
    ulfius_set_json_body_response(response, 500, body);
    endpwent();
    return U_CALLBACK_CONTINUE;
  }else{
    body=json_pack("{s:i,s:s,s:s}",
      "id",entry->pw_uid,
      "username",entry->pw_name,
      "created_at",buffer
    );
    y_log_message(Y_LOG_LEVEL_INFO, "usuario  %d creado", entry->pw_uid);
    endpwent();
  }
  ulfius_set_json_body_response(response, 200, body);

  return U_CALLBACK_CONTINUE;
}

/**
 * @brief funcion main se inicializa el servicio y el logger yder que luego se utilizara en los servicios acto seguido el servicioe spera request indefinidamente

 *@returns int32_t

*/
int main(void) {
  struct _u_instance instance;
  // Initialize instance with the port number
  if (ulfius_init_instance(&instance, PORT, NULL, NULL) != U_OK) {
    fprintf(stderr, "Error ulfius_init_instance, abort\n");
    return(1);
  }
  // Endpoint list declaration
  ulfius_add_endpoint_by_val(&instance, "POST", "", NULL, 0, &callback_create_user, NULL);
  ulfius_add_endpoint_by_val(&instance, "GET", "", NULL, 0, &callback_list_users, NULL);
  // Start the framework
  if (ulfius_start_framework(&instance) == U_OK) {
    printf("Start framework on port %d\n", instance.port);
    //inicializo los logs
    y_init_logs("users",Y_LOG_MODE_FILE,Y_LOG_LEVEL_INFO,"/var/log/tp3/tp3.log", "Initializing logs mode: console, logs level: info");
    // Wait for the user to press <enter> on the console to quit the application
    pause();
  } else {
    fprintf(stderr, "Error starting framework\n");
  }
  printf("End framework\n");
  ulfius_stop_framework(&instance);
  ulfius_clean_instance(&instance);
  return 0;
}


/**
 * @brief funcion main se inicializa el servicio y el logger yder que luego se utilizara en los servicios acto seguido el servicioe spera request indefinidamente
 * @param struct passwd* entry estructura donde se guardara la entrada de usuario buscada en caso de encontrarse
 * @param const char* username nombre de usuario que se busca
 *@returns struct passwd* en caso de encontrar un usuario,NULL en caso contrario

*/
struct passwd* findUser(struct passwd* entry,const char* username){
  entry=getpwent();
  while(entry!=NULL){
    if(!(strcmp(entry->pw_name,username))){
      return entry;
    }
    entry=getpwent();
  }
  endpwent();
  return NULL;
}