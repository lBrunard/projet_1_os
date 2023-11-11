#define _POSIX_C_SOURCE 201710L
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
#include <fcntl.h> // to delete
#ifndef MAP_ANONYMOUS
#define MAP_ANONYMOUS 0x20
#endif


#define READ 0
#define WRITE 1
#define MAX_IMAGE_NAME_LENGTH 999
#define _GNU_SOURCE 
int child_pids[2];
static volatile int keepRunning = 1;
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

void SIGPIPE_HANDLE(){
   keepRunning = 0;
   // tu relaye le SIGPIPE ou SIGTERM a img-dist 
   // fait rien pour le moment
}

static void SIGINT_HANDLE (){

   int children_finish_status;

   kill(child_pids[0], SIGTERM);
   kill(child_pids[1], SIGTERM);
   for (int i = 0; i < 2; ++i) {
      waitpid(-1, &children_finish_status, 0);
      printf("Child %d exited with status: %d\n", i+1, WEXITSTATUS(children_finish_status));
   }
   keepRunning = 0;
}

int img_dist(char path_comp[] , char path_img[]);

static void child_process(int pipe[2], char *image_to_compare, struct shared_memory* shared_mem){
   close(pipe[WRITE]);
   char buf[MAX_IMAGE_NAME_LENGTH];
   ssize_t bytesRead;
   while ((bytesRead = read(pipe[READ], buf, sizeof(buf))) > 0) {

      //printf("Fils id: %d, parent %d: %s \n",getpid(), getppid(), buf);
      int score = img_dist(image_to_compare, buf);
      printf("Son %d score : %i on %s\n",getpid(), score, buf);
      sem_wait(&sem);
      //printf("FILS 1 EN SECTION CRITIQUE. \n");
      if (shared_mem->best_score > score){
         shared_mem->best_score =score;
         memcpy(shared_mem->best_path, buf, sizeof(buf));
         //printf("Fils %d , Nouveau meilleur score : %i, à : %s\n",getpid(), shared_mem->best_score, shared_mem->best_path);
      }
      sem_post(&sem);
      //printf("FILS %d HORS SECTION CRITIQUE .\n", getpid());
         
   }
   close(pipe[READ]);
   exit(EXIT_SUCCESS);   
}


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
         close(fd1[READ]);
         close(fd2[READ]);

         signal(SIGINT, SIGINT_HANDLE);
         signal(SIGPIPE, SIGPIPE_HANDLE);
         child_pids[0] = first_son;
         child_pids[1] = second_son;
         while(fgets(database_image, MAX_IMAGE_NAME_LENGTH, stdin) != NULL && keepRunning)
         {  
            // kill(getpid(), SIGPIPE); a utiliser pour tester SIGPIPE_handler
            size_t length = strlen(database_image);
            if (length > 0 && database_image[length - 1] == '\n') {
            database_image[length - 1] = '\0'; // Remplace le caractère de nouvelle ligne par un caractère nul
            }
            if (ferror(stdin)) {
               perror("Error reading from stdin");
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

         close(fd1[WRITE]);
         close(fd2[WRITE]);
         // kill(child_pids[0], SIGTERM);
         // kill(child_pids[1], SIGTERM);
         printf("Exited Loop \n");
         wait(NULL);
         wait(NULL);
    
         
      }
      else{
         // SECOND CHILD PROCESS
         close(fd1[READ]);
         close(fd1[WRITE]);
         child_process(fd2, image_to_compare, shared_mem);
      }
   }
   else{
      // FIRST CHILD PROCESS
      close(fd2[READ]);
      close(fd2[WRITE]);
      child_process(fd1, image_to_compare, shared_mem);
   }
   printf("Most similar image found: '%s' with a distance of %i. \n", shared_mem->best_path, shared_mem->best_score);
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
    
    snprintf(command, sizeof(command),"./img-dist/img-dist %s %s",path_comp , path_img);
    /* utilisation de system pour utiliser le programe img-dist,  en cas d'erreur le retour est superieur a 64 */
    int retour = system(command);

    return (retour/256);

}
