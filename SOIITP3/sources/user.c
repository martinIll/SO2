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

int callback_list_users (const struct _u_request * request, struct _u_response * response, void * user_data) {
  struct passwd* entry=getpwent();
  json_t *array=json_array();
  json_t *body;
  while(entry!=NULL){
    json_t* user=json_pack("{s:i,s:s}",
      "userid",entry->pw_uid,
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
  char buffer[TAM];
  json_t* body=ulfius_get_json_body_request(request,NULL);
  const char *username=json_string_value(json_object_get(body,"username")); 
   char password[TAM];
  sprintf(password,"%c%s%c",34,json_string_value(json_object_get(body,"password")),34);

  if(crypt(password,"A1")==NULL){
    perror("error encriptando contraseña");

    ulfius_set_json_body_response(response, 500, NULL);
    return U_CALLBACK_CONTINUE;
  }
  
  sprintf(buffer,"sudo useradd -p %s %s > /home/martin/salida",password,username);
  time_t tiempo = time(0);
  struct tm *tlocal = localtime(&tiempo);  
  strftime(buffer,TAM,"%d/%m/%y %H:%M:%S",tlocal);

  struct passwd* entry=getpwent();
  int32_t found=0;
  while(entry!=NULL){
    if(!(strcmp(entry->pw_name,username))){
      body=json_pack("{s:i,s:s,s:s}",
        "id",entry->pw_uid,
        "username",entry->pw_name,
        "created_at",buffer
      );
      found=1;
      break;
    }
    entry=getpwent();
  }

  if(!found){
    body=json_pack("{s:s}",
    "error","error creando usuario en el sistema");
    ulfius_set_json_body_response(response, 500, body);
  }

  endpwent();
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
  ulfius_add_endpoint_by_val(&instance, "GET", "/", NULL, 0, &callback_list_users, NULL);
  ulfius_add_endpoint_by_val(&instance, "POST", "/", NULL, 0, &callback_create_user, NULL);
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