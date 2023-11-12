/**
 * @file main.c
 * @brief Contient la fonction principale du programme
 * @note Le programme doit recevoir comme argument la Photo a comparer et comme second argument
 * @authors Elias, Deni, Luis Brunard
 * @date 2023-11-12
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/wait.h>

#include <semaphore.h>
#include <sys/mman.h>

#include "error_handling.h"

#define READ 0
#define WRITE 1
#define MAX_IMAGE_NAME_LENGTH 999
#define _GNU_SOURCE 
#define _POSIX_C_SOURCE 201710L

// Variables globales
int child_pids[2];
static volatile int keepRunning = 1;

// Semaphore
sem_t sem;

// Structure de la memoire partagee
struct shared_memory {
  int best_score;
  char best_path[MAX_IMAGE_NAME_LENGTH];

};

// Prototypes
int img_dist(char path_comp[] , char path_img[]);
static void child_process(int pipe[2], char *image_to_compare, struct shared_memory* shared_mem);
struct shared_memory* create_mem_share();
static void SIGINT_HANDLE ();
void SIGPIPE_HANDLE();

/**
*Doit recevoir comme argument la Photo a comparer et comme second argument
*le dossier conentant les photos avec lesquelles il doit comparer
*Le programme doit retourner la photo la plus similaire et le score de similarite
*Si le score est superieur a 64 alors aucune comparaison n'a pu etre effectuee
* @brief Fonction principale du programme
* @param argc le nombre d'arguments
* @param argv les arguments
*@return L'image la plus similaire (ex. Most similar image found: 'img/22.bmp' with a distance of 12.)
*/
int main(int argc, char* argv[]) {
   char *image_to_compare = malloc(MAX_IMAGE_NAME_LENGTH);
   char *database_image = malloc(MAX_IMAGE_NAME_LENGTH);
   int son_to_compute = 1;

   if(argc != 2){
      // Do nothing for now
   }
   image_to_compare = argv[1];
   // printf("Image to compare is: %s \n", image_to_compare);

   // Communication inter process
   pid_t first_son;
   pid_t second_son;
   static int fd1[2],fd2[2];

  //semaphore
  sem_init(&sem, 0, 1);

  struct shared_memory* shared_mem = create_mem_share();
  shared_mem->best_score = 255;

  checked(pipe(fd1));
  checked(pipe(fd2));


   // Process Fils 1
   first_son = fork();
   checked(first_son);

   if(first_son != 0){
      // Process Fils 2
      second_son = fork();
      checked(second_son);
      if (second_son != 0){
         // PARENT PROCESS
         checked(close(fd1[READ]));
         checked(close(fd2[READ]));

         signal(SIGINT, SIGINT_HANDLE);
         signal(SIGPIPE, SIGPIPE_HANDLE);
         child_pids[0] = first_son;
         child_pids[1] = second_son;
         while(fgets(database_image, MAX_IMAGE_NAME_LENGTH, stdin) != NULL && keepRunning){  
            size_t length = strlen(database_image);
            if (length > 0 && database_image[length - 1] == '\n') {
            database_image[length - 1] = '\0'; // Remplace le caractère de nouvelle ligne par un caractère nul
            }
            if (ferror(stdin)) {
               perror("Error reading from stdin");
            }

            if(!son_to_compute){
               checked_wr(write(fd1[WRITE], database_image, MAX_IMAGE_NAME_LENGTH));
            } 
            else{
               checked_wr(write(fd2[WRITE], database_image, MAX_IMAGE_NAME_LENGTH));
            }
            son_to_compute = (son_to_compute == 1) ? 0 : 1;
         }

         checked(close(fd1[WRITE]));
         checked(close(fd2[WRITE]));
         wait(NULL);
         wait(NULL); 
      }
      else{
         // SECOND CHILD PROCESS
         checked(close(fd1[READ]));
         checked(close(fd1[WRITE]));
         child_process(fd2, image_to_compare, shared_mem);
      }
   }
   else{
      // FIRST CHILD PROCESS
      checked(close(fd2[READ]));
      checked(close(fd2[WRITE]));
      child_process(fd1, image_to_compare, shared_mem);
   }

   if(shared_mem->best_score < 64){
      printf("Most similar image found: '%s' with a distance of %i.\n", shared_mem->best_path, shared_mem->best_score);
   }
   else{
      printf("No similar image found (no comparison could be performed successfully).\n");
   }
   sem_destroy(&sem);

   return 0; 
}
/**
 * Fonction qui cree une memoire partagee et renvoie un pointeur vers celle-ci
 * @return un pointeur vers la memoire partagee
 * **/
struct shared_memory* create_mem_share(){
   #ifndef MAP_ANONYMOUS // MAP_ANONYMOUS peut ne pas etre defini sur certains systemes.
   #define MAP_ANONYMOUS 0x20
   #endif
   const int protection = PROT_READ | PROT_WRITE ; // Lecture et ecriture
   const int visibility = MAP_SHARED | MAP_ANONYMOUS ; // Memoire partagee et anonyme
   const int fd = -1; // Pas de fichier
   const int offset = 0; // Pas de decalage
   struct shared_memory* shared_mem = mmap(NULL , sizeof(struct shared_memory) , 
                                          protection , visibility , fd , offset );
   if(shared_mem == MAP_FAILED){
    perror("mmap");
    exit(1);
  } 
   return shared_mem;
}

/**
 * Fonction qui cree un processus fils qui va lire dans le pipe et calculer la distance entre les images
 * @param pipe le pipe dans lequel le fils va lire
 * @param image_to_compare l'image a comparer
 * @param shared_mem la memoire partagee
 * **/
static void child_process(int pipe[2], char *image_to_compare, struct shared_memory* shared_mem){
   checked(close(pipe[WRITE]));
   char buf[MAX_IMAGE_NAME_LENGTH];
   ssize_t bytesRead;
   while ((bytesRead = read(pipe[READ], buf, sizeof(buf))) > 0) {

      //printf("Fils id: %d, parent %d: %s \n",getpid(), getppid(), buf);
      int score = img_dist(image_to_compare, buf);
      //printf("Son %d score : %i on %s\n",getpid(), score, buf);
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
   checked(close(pipe[READ]));
   exit(EXIT_SUCCESS);   
}

/**
 * Fonction qui gere le signal SIGINT et qui tue les processus fils proprement 
*/
static void SIGINT_HANDLE (){
   int children_finish_status;
   kill(child_pids[0], SIGTERM);
   kill(child_pids[1], SIGTERM);
   for (int i = 0; i < 2; ++i) {
      waitpid(-1, &children_finish_status, 0);   }
   keepRunning = 0;
}
/**
 * Fonction qui gere le signal SIGPIPE et qui tue les processus fils proprement
 * en cas de SIGPIPE la variable keepRunning est mise a 0 pour que le programme principal s'arrete
*/
void SIGPIPE_HANDLE(){
   keepRunning = 0;
   // tu relaye le SIGPIPE ou SIGTERM a img-dist 
   // fait rien pour le moment
}

/**
 * Fonction qui execute le programme img-dist avec les arguments path_comp et path_img
 * @param path_comp le chemin vers l'image a comparer
 * @param path_img le chemin vers l'image avec laquelle il faut comparer
 * @return le retour de la fonction system qui execute le programme img-dist, le score de similarite
*/
int img_dist(char path_comp[] , char path_img[]){
    char command[2048];
    checked(access(path_img,F_OK));
    
    snprintf(command, sizeof(command),"./img-dist/img-dist %s %s",path_comp , path_img);
    /* utilisation de system pour utiliser le programe img-dist,  en cas d'erreur le retour est superieur a 64 */
    int retour = system(command);

    return (retour/256);

}