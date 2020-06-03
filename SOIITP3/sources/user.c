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

#define PORT 8082
#define TAM 256

struct passwd* findUser(struct passwd* entry,const char* username);

int callback_list_users (const struct _u_request * request, struct _u_response * response, void * user_data) {
  struct passwd* entry=getpwent();
  json_t *array=json_array();
  json_t *body;
  while(entry!=NULL){
    json_t* user=json_pack("{s:i,s:s}",
      "user_id",entry->pw_uid,
      "username",entry->pw_name
    );
    json_array_append(array,user);
    entry=getpwent();
  }

  body=json_pack("{s:o}", "data", array);

  ulfius_set_json_body_response(response, 200, body);
  endpwent();
  return U_CALLBACK_CONTINUE;
}

int callback_create_user (const struct _u_request * request, struct _u_response * response, void * user_data) {
  json_t* body=ulfius_get_json_body_request(request,NULL);
  char buffer[TAM];
  
  if(json_object_get(body,"username")==NULL || json_object_get(body,"password")==NULL || !json_is_string(json_object_get(body,"username")) || !json_is_string(json_object_get(body,"password")) ){
    body=json_pack("{s:s}",
    "description","el nombre de usuario y la contraseña deben ser cadenas de texto.");
    ulfius_set_json_body_response(response, 400, body);
    return U_CALLBACK_COMPLETE;
  }

  const char *username=json_string_value(json_object_get(body,"username")); 
  char password[TAM];
  sprintf(password,"%c%s%c",34,json_string_value(json_object_get(body,"password")),34);
  
  struct passwd* entry=calloc(1,sizeof(entry));
  entry=findUser(entry,username);
  if(entry!=NULL){
    body=json_pack("{s:s}",
    "description","El nombre de usuario solicitado ya existe");
    ulfius_set_json_body_response(response, 409, body);
    endpwent();
    return U_CALLBACK_COMPLETE;
  }else{
     endpwent();
  }

  
  sprintf(buffer,"sudo useradd -p $(openssl passwd -1 %s) %s",password,username);
  system(buffer);
  time_t tiempo = time(0);
  struct tm *tlocal = localtime(&tiempo);  
  strftime(buffer,TAM,"%d/%m/%y %H:%M:%S",tlocal);

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
    endpwent();
  }
  ulfius_set_json_body_response(response, 200, body);

  return U_CALLBACK_CONTINUE;
}

/**
 * main function
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