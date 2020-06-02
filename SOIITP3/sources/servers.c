#include <stdio.h>
#include <ulfius.h>
#include <jansson.h>
#include <unistd.h>
#include <string.h>
#include <sys/statfs.h>
#include <sys/utsname.h>
#define PORT 8081
#define TAM 1024

void getInfo(char* keywords[],char* searchBy[],const char *filename, int * index);
void getLoadAvg(char* keywords[],int index);
void getUptime(char* keywords[],int index);
void  getDiskInfo(char* keywords[],int index);
void getKernelVersion(char* keywords[],int index);

int callback_hardware_info (const struct _u_request * request, struct _u_response * response, void * user_data) {
 
  char * keywords[12];
  char* searchBy[2];
  int32_t index=0;
  for(int i=0;i<12;i++){
    keywords[i]=calloc(256,sizeof(char));
  }
  for(int i=0;i<2;i++){
    searchBy[i]=calloc(256,sizeof(char));
  }

  strcpy(searchBy[0],"model name");
  strcpy(searchBy[1],"cpu cores");
  getInfo(keywords, searchBy,"/proc/cpuinfo",&index);
  index+=2;
  strcpy(searchBy[0],"MemTotal");
  strcpy(searchBy[1],"MemFree");
  getInfo(keywords, searchBy,"/proc/meminfo",&index);
  index+=2;
  getDiskInfo(keywords,index);
  index+=2;
  getLoadAvg(keywords,index);
  index++;
  getUptime(keywords,index);
  index++;
  getKernelVersion(keywords,index);
  
  json_t *body = json_pack("{s:s, s:s, s:s, s:s, s:s, s:s, s:s, s:s, s:s}",
                          "kernelVersion",keywords[8],
                          "processorName",keywords[0],
                          "totalCPUCore",keywords[1],
                          "totalMemory", keywords[2],
                          "freeMemory", keywords[3],
                          "diskTotal", keywords[4],
                          "diskFree", keywords[5],
                          "LoadAvg", keywords[6],
                          "uptime",keywords[7]
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
  ulfius_add_endpoint_by_val(&instance, "GET", "/hardwareinfo", NULL, 0, &callback_hardware_info, NULL);
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

void getInfo(char* keywords[],char* searchBy[],const char *filename, int * index){
  char buffer[1024];

  FILE *fp=fopen(filename,"r");
  if (fp == NULL) {
    printf("Failed to run command\n" );
    exit(1);
  }

  //busca las palabras clave
  while (fgets(buffer, sizeof(buffer), fp) != NULL) {
    for(int i=*index;i<*index+2;i++){
      if(strstr (buffer, searchBy[0])){
        strcpy(keywords[*index],strtok(buffer+strlen(searchBy[0]),"\n")+3);
      }else if(strstr (buffer, searchBy[1])){
        strcpy(keywords[*index+1],strtok(buffer+strlen(searchBy[1]),"\n")+3);
      }
    }
  }
  fclose(fp);
}

void getLoadAvg(char* keywords[],int index){
  printf("%d",index);
  
  char buffer[TAM];
	FILE* fp;
	fp = fopen ("/proc/loadavg", "r");
	
  if(fgets (buffer, 5, fp)!=NULL){
	  sprintf (keywords[index],"%s", buffer);
  }

	fclose (fp);
}



void getUptime(char* keywords[],int index){
  FILE* fp;
	double uptime;
  printf("%d",index);
	fp = fopen ("/proc/uptime", "r");
  if(fscanf (fp, "%lf\n", &uptime)==EOF){
    fclose (fp);
    return;
  }
  fclose (fp);
	long time = (long) uptime; 
	const long minute = 60;
	const long hour = minute * 60;
	const long day = hour * 24;
	sprintf (keywords[index],"%ld días,%ld:%02ld:%02ld", 
					time / day, (time % day) / hour, (time % hour) / minute, time % minute);
}


void  getDiskInfo(char* keywords[],int index){
  struct statfs *buf=calloc(1,sizeof(struct statfs));
  
  if(statfs("/", buf)<0){
    perror("error al obtener estadisticas de fs");
    return;
  }
  printf("%ld %d\n",buf->f_blocks,buf->f_bsize);
  sprintf(keywords[index],"%ld GB",(buf->f_blocks*(unsigned)buf->f_bsize)/(1024*1024*1024));
  sprintf(keywords[index+1],"%ld GB",(buf->f_bfree*(unsigned)buf->f_bsize)/(1024*1024*1024));
}

void getKernelVersion(char* keywords[],int index){
  struct utsname *buf=calloc(1,sizeof(struct utsname));
 
  if(uname(buf)<0){
    perror("error al obtener estadisticas de fs");
    return;
  }

  sprintf(keywords[index],"%s",buf->release);
}