#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <semaphore.h>


#define READ 0
#define WRITE 1
#define MAX_IMAGE_NAME_LENGTH 999


//CHECKERS

#define CHECK_FORKING(process) \
   if(process == -1){ \
      fprintf(stderr, "Erreur lors de la création du processus fils 1"); \
      exit(EXIT_FAILURE);\
   }

//int img_dist(char* path_comp[] , char* path_img[]);

/**
*Doit recevoir comme argument la Photo a compareet comme second 
*le dossier conentant les photos avec lesquelles il doit comparer
*@return L'image la plus similaire (ex. Most similar image found: 'img/22.bmp' with a distance of 12.)
**/
int main(int argc, char* argv[]) {
   char *image_to_compare = malloc(MAX_IMAGE_NAME_LENGTH);
   char *database_image = malloc(MAX_IMAGE_NAME_LENGTH);
   int son_to_compute = 1;

   if(argc != 2){
      // Do nothing for now
   }
   image_to_compare = argv[1];
   printf("Image to compare is: %s \n", image_to_compare);

   // Communication inter process
   pid_t first_son;
   pid_t second_son;
   int fd1[2],fd2[2];

   //Création des 2 pipes
   if (pipe(fd1) < 0 || pipe(fd2) < 0){
      fprintf(stderr, "Erreur lors de la création des pipes\n");
      exit(EXIT_FAILURE);
   }

   //Création process fils

   // Process Fils 1
   first_son = fork();
   CHECK_FORKING(first_son);

   if(first_son != 0){
      second_son = fork();
      CHECK_FORKING(second_son);
      if (second_son != 0){
         // PARENT PROCESS
         while(fgets(database_image, MAX_IMAGE_NAME_LENGTH, stdin) != NULL)
         {  
            if(!son_to_compute){
               if(write(fd1[WRITE], database_image, MAX_IMAGE_NAME_LENGTH) == -1) {
                     //Handle error
               }
            } 
            else{
               if(write(fd2[WRITE], database_image, MAX_IMAGE_NAME_LENGTH) == -1) {
                     //Handle error
               }
            }
            son_to_compute = (son_to_compute == 1) ? 0 : 1;
         }
      }
      else{
         // SECOND CHILD PROCESS
         close(fd2[WRITE]);      
         char buf[MAX_IMAGE_NAME_LENGTH];
         
         while(read(fd2[READ], &buf, sizeof(buf))){
            printf("Fils 2, id: %d, parent %d: %s",getpid(), getppid(), buf);
         }
         exit(EXIT_SUCCESS);
      }
   }
   else{
      // FIRST CHILD PROCESS
      close(fd1[WRITE]);
      char buf[MAX_IMAGE_NAME_LENGTH];
      
      while(read(fd1[READ], &buf, sizeof(buf))){
            printf("Fils 1 id: %d, parent %d: %s",getpid(), getppid(), buf);
      }
      exit(EXIT_SUCCESS);
   }
   return 0;
   // printf("Process :\npid :%d\nppid:%d\n",getpid(),getppid());
   // if(first_son == 0){
   //    close(fd1[WRITE]);

   //    char buf[MAX_IMAGE_NAME_LENGTH];
      
   //    while(read(fd1[READ], &buf, sizeof(buf))){
   //          printf("Fils 1 : %s", buf);
   //       }

   //    exit(EXIT_SUCCESS);
   // } else{
   //    //Process Fils 2
      
   //    second_son = fork();
   //    CHECK_FORKING(second_son);
   //    if(second_son == 0){
   //       close(fd2[WRITE]);
   //       char buf[MAX_IMAGE_NAME_LENGTH];    
   //       while(read(fd2[READ], &buf, sizeof(buf))){
   //          printf("Fils 2 : %s", buf);
   //       }
         
   //       exit(EXIT_SUCCESS);

   //    }else{
   //       //Process Pere
   //       char new_file_to_compare[999];

   //       if(strcmp(bash_mode, "interactiv") == 0){
   //          printf("%s", "Entrez les fichier que vous voulez comparer un par un. Une fois que vous avez fini envoyer end\n");
   //          while (fgets(new_file_to_compare, sizeof(new_file_to_compare)-1, stdin) != NULL){
   //             if(!son_to_compute){
   //                write(fd1[WRITE], new_file_to_compare, MAX_IMAGE_NAME_LENGTH);
   //             } else{
   //                write(fd2[WRITE], new_file_to_compare, MAX_IMAGE_NAME_LENGTH);
   //             }
   //             son_to_compute = (son_to_compute == 1) ? 0 : 1;
   //          }
   //       }else{    
   //          FILE *fp;
   //          char command[999];
   //          sprintf(command, "./list-file.sh %s", database_path);
   //          fp = popen(command, "r");
   //          if(fp == NULL){
   //             printf("%s", "FAILED OPEN ./list-file\n");
   //             exit(1);
   //          }
   //           while (fgets(new_file_to_compare, sizeof(new_file_to_compare)-1, fp) != NULL){
   //             if(!son_to_compute){
   //                write(fd1[WRITE], new_file_to_compare, MAX_IMAGE_NAME_LENGTH);
   //             } else{
   //                write(fd2[WRITE], new_file_to_compare, MAX_IMAGE_NAME_LENGTH);
   //             }
   //             son_to_compute = (son_to_compute == 1) ? 0 : 1;
   //          }
   //          if (!feof(fp)) {
   //             printf("list-file.sh has finished\n");
   //          }
   //          pclose(fp);
   //       }

   //       //FIN PROCESS
   //       close(fd1[READ]);
   //       close(fd2[READ]);
   //       close(fd1[WRITE]);
   //       close(fd2[WRITE]);

   //       exit(EXIT_SUCCESS);
      
      
   //    }

   // free(bash_mode);
   // free(database_path);
   // free(image_to_compare);
   // return 0;

   // }
}
