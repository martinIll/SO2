#include <stdio.h>
#include <ulfius.h>
#include <jansson.h>
#include <unistd.h>
#define PORT 8081

int callback_hello_world (const struct _u_request * request, struct _u_response * response, void * user_data) {
  json_t *body = json_pack("{s:s, s:s, s:s, s:s, s:s, s:s, s:s, s:s, s:s}",
                          "kernelVersion","5.4.0-31-generic",
                          "processorName", "Intel (R) Core (TM) i7 7700K CPU @ 4. 20GHz",
                          "totalCPUCore", "2",
                          "totalMemory", "0.2",
                          "freeMemory", "0.1",
                          "diskTotal", "1,2",
                          "diskFree", "5.4",
                          "LoadAvg", "0",
                          "uptime", "74h 54m 7s"
                          );
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
  ulfius_add_endpoint_by_val(&instance, "GET", "/hardwareinfo", NULL, 0, &callback_hello_world, NULL);
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