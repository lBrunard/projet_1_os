#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <errno.h>
#include <sys/mman.h>
#ifndef MAP_ANONYMOUS
#define MAP_ANONYMOUS 0x20
#endif


#define READ 0
#define WRITE 1
#define MAX_IMAGE_NAME_LENGTH 999

sem_t sem;

struct shared_memory {
  int best_score;
  char best_path[MAX_IMAGE_NAME_LENGTH];

};

//CHECKERS

#define CHECK_FORKING(process) \
   if(process == -1){ \
      fprintf(stderr, "Erreur lors de la création du processus fils 1"); \
      exit(EXIT_FAILURE);\
   }

int img_dist(char path_comp[] , char path_img[]);

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

  //semaphore
  sem_init(&sem, 0, 1);

  //Mem_share
  const int protection = PROT_READ | PROT_WRITE ;
  const int visibility = MAP_SHARED | MAP_ANONYMOUS ;
  const int fd = -1;
  const int offset = 0;
  struct shared_memory* shared_mem = mmap(NULL , sizeof(struct shared_memory) , 
                                          protection , visibility , fd , offset );
  if(shared_mem == MAP_FAILED){
    perror("mmap");
    exit(1);
  }
  shared_mem->best_score = 255;



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
            size_t length = strlen(database_image);
            if (length > 0 && database_image[length - 1] == '\n') {
            database_image[length - 1] = '\0'; // Remplace le caractère de nouvelle ligne par un caractère nul
            }
            if(!son_to_compute){
               if(write(fd1[WRITE], database_image, MAX_IMAGE_NAME_LENGTH) == -1) {
                  perror("write son 1");
                  exit(1);
               }
            } 
            else{
               if(write(fd2[WRITE], database_image, MAX_IMAGE_NAME_LENGTH) == -1) {
                    perror("write son 2");
                    exit(1);    
               }
            }
            son_to_compute = (son_to_compute == 1) ? 0 : 1;
         }
        kill(first_son, SIGUSR1);
        kill(second_son, SIGUSR1);
        write(fd1[WRITE], "", 1);
        write(fd2[WRITE], "", 1);
        close(fd1[WRITE]);
        close(fd2[WRITE]);
         
         wait(NULL);
         wait(NULL);         
         
      }
      else{
         // SECOND CHILD PROCESS
        close(fd2[WRITE]);      
        char buf[MAX_IMAGE_NAME_LENGTH];
        ssize_t bytesRead;
        while((bytesRead = read(fd2[READ], buf, sizeof(buf))) > 0){
          if (bytesRead == 1 && buf[0] == '\0') {
            break; // Fin de fichier rencontrée, sortez de la boucle
          }
          printf("Fils 2, id: %d, parent %d: %s \n",getpid(), getppid(), buf);
          int score = img_dist(image_to_compare, buf);
          printf("Son 2 score : %i on %s\n", score, buf);
          sem_wait(&sem);
          printf("%s\n", "FILS 2 EN SECTION CRITIQUE");
          if (shared_mem->best_score > score){
            shared_mem->best_score =score;
            memcpy(shared_mem->best_path, buf, sizeof(buf));
            printf("Fils 2 , Nouveau meilleur score : %i, à : %s\n", shared_mem->best_score, shared_mem->best_path);
          }
          sem_post(&sem);
          printf("%s\n", "FILS 2 HORS SECTION CRITIQUE");
        }
         close(fd2[READ]);
         exit(EXIT_SUCCESS);
      }
   }
   else{
      // FIRST CHILD PROCESS
      close(fd1[WRITE]);
      char buf[MAX_IMAGE_NAME_LENGTH];
      ssize_t bytesRead;
      while ((bytesRead = read(fd1[READ], buf, sizeof(buf))) > 0) {
        if (bytesRead == 1 && buf[0] == '\0') {
          break; // Fin de fichier rencontrée, sortez de la boucle
        }
        printf("Fils 1 id: %d, parent %d: %s \n",getpid(), getppid(), buf);
        int score = img_dist(image_to_compare, buf);
        printf("Son 1 score : %i on %s\n", score, buf);
        sem_wait(&sem);
        printf("%s\n", "FILS 1 EN SECTION CRITIQUE");
        if (shared_mem->best_score > score){
          shared_mem->best_score =score;
          memcpy(shared_mem->best_path, buf, sizeof(buf));
          printf("Fils 1 , Nouveau meilleur score : %i, à : %s\n", shared_mem->best_score, shared_mem->best_path);
        }
        sem_post(&sem);
        printf("%s\n", "FILS 1 HORS SECTION CRITIQUE");
         
      }
      close(fd1[READ]);
      exit(EXIT_SUCCESS);
   }
    
  printf("best file is %s with score of %i\n", shared_mem->best_path, shared_mem->best_score);

  sem_destroy(&sem);
  return 0;
}

int img_dist(char path_comp[] , char path_img[]){
    char command[2048];
    /* verification des fichier donner a la fonction , en cas d'erreur , renvois -1 et affiche la raison*/
    if (access(path_comp,F_OK)==-1){

        printf("%s n'est pas un fichier valide \n" , path_comp);
        return -1;
    }
    
    snprintf(command, sizeof(command),"img-dist/img-dist %s %s",path_comp , path_img);
    /* utilisation de system pour utiliser le programe img-dist,  en cas d'erreur le retour est superieur a 64 */
    int retour = system(command);

    return (retour/256);

}
