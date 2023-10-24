#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define READ 0
#define WRITE 1
#define MAX_IMAGE_NAME_LENGTH 999
#define SHARED_MEM_SIZE sizeof(char) * 10

#define CHECK_SHARED_MEMORY(ptr) \
   if ((ptr) == (char *) -1) { \
      fprintf(stderr, "Erreur lors de l'attachement de la zone de mémoire partagée\n"); \
      exit(EXIT_FAILURE); \
   }

#define CHECK_FORKING(process) \
   if(process == -1){ \
      fprintf(stderr, "Erreur lors de la création du processus fils 1"); \
      exit(EXIT_FAILURE);\
   }

#define CHECK_SHARED_MEM_DETACH(ptr) \
   if (shmdt(shared_memory) == -1) { \
            fprintf(stderr,"Erreur lors du détachement de la zone de mémoire partagée du processus fils 1"); \
            exit(EXIT_FAILURE); \
         }


void Process_args(char* img_path, int argc, char* argv[]);

/**
*Doit recevoir comme argument la Photo a compareet comme second 
*le dossier conentant les photos avec lesquelles il doit comparer
*@return L'image la plus similaire (ex. Most similar image found: 'img/22.bmp' with a distance of 12.)
**/
int main(int argc, char* argv[]) {
   char *image_to_compare;
   int son_to_compute = 1;
   image_to_compare= (char *)malloc(sizeof(char)*MAX_IMAGE_NAME_LENGTH);
   if (image_to_compare == NULL) {
      fprintf(stderr, "Erreur d'allocation mémoire\n");
      exit(EXIT_FAILURE);
   }
   Process_args(image_to_compare, argc, argv);
   
   // Communication inter process
   pid_t first_son;
   pid_t second_son;
   int fd1[2],fd2[2];
   int shmid;


   //Création des 2 pipes
   if (pipe(fd1) < 0 || pipe(fd2) < 0){
      fprintf(stderr, "Erreur lors de la création des pipes\n");
      exit(EXIT_FAILURE);
   }
   //Création de la zone de mémoire partagée
   shmid = shmget(IPC_PRIVATE, SHARED_MEM_SIZE, IPC_CREAT | 0666);
   if(shmid == -1){
      fprintf(stderr, "Erreur lors de la création de la zone de mémoire partagée");
      exit(EXIT_FAILURE);
   }
   //Attachement de la zone memoire partagée au process pere
   char *shared_memory = (char*) shmat(shmid, NULL, 0);
   CHECK_SHARED_MEMORY(shared_memory);
   //Création process fils
   first_son = fork();
   CHECK_FORKING(first_son);
   //Process Fils 1
   if(first_son == 0){
      close(fd1[WRITE]);
      //Attatchement mémoire partagée au fils 1
      char *shared_memory = (char *) shmat(shmid, NULL, 0);
      CHECK_SHARED_MEMORY(shared_memory);

      char buf[15];
      
      read(fd1[READ], &buf, sizeof(buf));
      printf("Fils 1 : %s", buf);


      ////Détatchement mémoire partagée au fils 1
      CHECK_SHARED_MEM_DETACH(shared_memory);
      exit(EXIT_SUCCESS);
   } else{
      //Process Fils 2
      close(fd2[WRITE]);
      second_son = fork();
      CHECK_FORKING(first_son);
      if(second_son == 0){
         char buf[15];
         //Attatchement mémoire partagée au fils 1
         char *shared_memory = (char *) shmat(shmid, NULL, 0);
         CHECK_SHARED_MEMORY(shared_memory);

         read(fd2[READ], &buf, sizeof(buf));
         printf("Fils 2 : %s", buf);
         
         CHECK_SHARED_MEM_DETACH(shared_memory);
         exit(EXIT_SUCCESS);

      }else{
         //Process Pere

         fd_set read_fds; //ensemble de descripteur de fichiers
         FD_ZERO(&read_fds); //Met l'ensemble a zer0
         FD_SET(fd1[READ], &read_fds); //Attribue des valeur dans le set
         FD_SET(fd2[READ], &read_fds); //Attribue des valeur dans le set

         if(son_to_compute == 1){
            struct timeval timeout = {0, 0};
            int pipe_clear = select(fd1[0] + 1, &read_fds, NULL, NULL, &timeout);
            //Select : renvoie une valeur [-1 = ERREUR, 0 = FD READY, 1+ = FD NOT READY]
            if(pipe_clear == -1){
               fprintf(stderr, "Erreur lors de la vérification de l'état du pipe du fils 1\n");
               exit(EXIT_FAILURE);
            }else if (pipe_clear == 0){
               write(fd1[WRITE], "MESSAGE FILS 1", 15);
            }else {
               write(fd2[WRITE], "MESSAGE FILS 2", 15);
            }
         }else if(son_to_compute == 2){
            struct timeval timeout = {0, 0};
            int pipe_clear = select(fd2[0] + 1, &read_fds, NULL, NULL, &timeout);
            if(pipe_clear == -1){
               fprintf(stderr, "Erreur lors de la vérification de l'état du pipe du fils 2\n");
               exit(EXIT_FAILURE);
            }else if (pipe_clear == 0){
               write(fd2[WRITE], "MESSAGE FILS 2", 15);
            }else {
               write(fd1[WRITE], "MESSAGE FILS 1", 15);
            }
         }

      //FIN PROCESS
      CHECK_SHARED_MEM_DETACH(shared_memory);
      close(fd1[READ]);
      close(fd2[READ]);
      close(fd1[WRITE]);
      close(fd2[WRITE]);

      wait(NULL);
      wait(NULL);
      // Supprimer la zone de mémoire partagée
      if (shmctl(shmid, IPC_RMID, NULL) == -1) {
         perror("Erreur lors de la suppression de la zone de mémoire partagée");
         exit(EXIT_FAILURE);
      }

      exit(EXIT_SUCCESS);
      
      
   }



   free(image_to_compare);
   return 0;

}
}


void Process_args(char* image_to_compare, int argc, char* argv[]){
   if (argc!=2){
        printf("nombre d'argument incorecte");
        exit(1);        
   }  else {
        strcat(image_to_compare,argv[1]);
   }
}