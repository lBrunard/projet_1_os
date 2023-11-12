/**
 * @file main.c
 * @brief Contient la fonction principale du programme
 * @note Le programme doit recevoir comme argument la Photo a comparer et comme second argument
 * @authors Elias, Deni, Luis Brunard
 * @date 2023-11-12
*/

// Importation des modules
#define _GNU_SOURCE 
#define _POSIX_C_SOURCE 201710L

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <sys/mman.h>
#include "utils.h"

// Variables constantes
#define READ 0
#define WRITE 1
#define MAX_IMAGE_NAME_LENGTH 999

// Variables globales
static int child_pids[2]; /// Stocke les ID des processus fils
static volatile int keep_running = 1; // Contrôle la boucle du programme

// Semaphore
sem_t sem;


// Prototypes de fonctions
int img_dist(char path_comp[] , char path_img[]);
static void child_process(int pipe[2], char *image_to_compare, struct shared_memory* shared_mem);
static struct shared_memory* create_mem_share();

// Gestionnaires de signaux
static void SIGINT_HANDLE ();
static void SIGPIPE_HANDLE();

/**
*Doit recevoir comme argument la Photo a comparer avec avec les entrées stdin
*Le programme doit afficher la photo la plus similaire et le score de similarite
*Si le score est superieur a 64 alors aucune comparaison n'a pu etre effectuée
* @brief Fonction principale du programme
* @param argc le nombre d'arguments
* @param argv les arguments
*@return Code de sortie 0.
*/
int main(int argc, char* argv[]) {

   if(argc != 2){
      keep_running = 0;
   }

   char *image_to_compare = malloc(MAX_IMAGE_NAME_LENGTH);
   char *database_image = malloc(MAX_IMAGE_NAME_LENGTH);
   int son_to_compute = 1;

   image_to_compare = argv[1];

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


   // Création du processus Fils 1
   first_son = fork();
   checked(first_son);

   if(first_son != 0){
      // Création du processus Fils 2
      second_son = fork();
      checked(second_son);
      if (second_son != 0){
         // Processus parent
         checked(close(fd1[READ]));
         checked(close(fd2[READ]));

         signal(SIGINT, SIGINT_HANDLE);
         signal(SIGPIPE, SIGPIPE_HANDLE);
         child_pids[0] = first_son;
         child_pids[1] = second_son;
         while(fgets(database_image, MAX_IMAGE_NAME_LENGTH, stdin) != NULL && keep_running){  
            
            // Remplace le caractère de nouvelle ligne par un caractère nul
            size_t length = strlen(database_image);
            if (length > 0 && database_image[length - 1] == '\n') {
            database_image[length - 1] = '\0'; 
            }

            if (ferror(stdin)) {
               perror("Error reading from stdin");
            }

            // Envoie l'image aux processus enfants pour comparaison, en alternance
            if(!son_to_compute){
               checked(write(fd1[WRITE], database_image, MAX_IMAGE_NAME_LENGTH));
            } 
            else{
               checked(write(fd2[WRITE], database_image, MAX_IMAGE_NAME_LENGTH));
            }
            son_to_compute = (son_to_compute == 1) ? 0 : 1;
         }

         // Ferme les pipes et attend la fin des processus enfants
         checked(close(fd1[WRITE]));
         checked(close(fd2[WRITE]));
         wait(NULL);
         wait(NULL); 
      }
      else{
         // Processus du deuxième enfant
         checked(close(fd1[READ]));
         checked(close(fd1[WRITE]));
         child_process(fd2, image_to_compare, shared_mem);
      }
   }
   else{
      // Processus du premier enfant
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
 * Fonction qui cree un processus fils qui va lire dans le pipe et calculer la distance entre les images
 * @param pipe le pipe dans lequel le fils va lire
 * @param image_to_compare l'image a comparer
 * @param shared_mem la memoire partagee
 * **/
static void child_process(int pipe[2], char *image_to_compare, struct shared_memory* shared_mem){
   checked(close(pipe[WRITE]));
   char buf[MAX_IMAGE_NAME_LENGTH];
   ssize_t bytesRead;
   // Boucle alors que les pipes restent ouverts
   while ((bytesRead = read(pipe[READ], buf, sizeof(buf))) > 0) {
      int score = img_dist(image_to_compare, buf);\
      sem_wait(&sem);
      if (shared_mem->best_score > score){
         shared_mem->best_score =score;
         memcpy(shared_mem->best_path, buf, sizeof(buf));
      }
      sem_post(&sem);  
   }
   checked(close(pipe[READ]));
   exit(EXIT_SUCCESS);   
}

/**
 * Gestion du signal SIGINT qui arrête le programme après avoir attendu que les enfants aient fini
*/
static void SIGINT_HANDLE (){
   keep_running = 0;
}

/**
 * Gestion du signal SIGPIPE qui entraîne l’arrêt de img-search et de ses fils sans générer un crash
*/
static void SIGPIPE_HANDLE(){
   // Arrête les processus fils 
   kill(child_pids[0], SIGTERM);
   kill(child_pids[1], SIGTERM);
   keep_running = 0;
}

