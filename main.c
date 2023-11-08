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


#define READ 0
#define WRITE 1
#define MAX_IMAGE_NAME_LENGTH 999

sem_t sem;
int best_score = 255;
char best_path[MAX_IMAGE_NAME_LENGTH];






//CHECKERS

#define CHECK_FORKING(process) \
   if(process == -1){ \
      fprintf(stderr, "Erreur lors de la création du processus fils 1"); \
      exit(EXIT_FAILURE);\
   }

int img_dist(char path_comp[] , char path_img[]);
void son1_critic_sec(int score, char path_img[]);
void son2_critic_sec(int score, char path_img[]);


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
                  //Handle error
               }
            } 
            else{
               if(write(fd2[WRITE], database_image, MAX_IMAGE_NAME_LENGTH) == -1) {
                    perror("write son 2");
                    exit(1);    
                    //Handle error
               }
            }
            son_to_compute = (son_to_compute == 1) ? 0 : 1;
         }
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
            son2_critic_sec(score, buf);
        
            //printf("Score : %d \n",img_dist(image_to_compare,buf));
            
            
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
            son1_critic_sec(score, buf);
      
            //printf("Score : %d \n",img_dist(image_to_compare,buf));
         
      }
      close(fd1[READ]);
      exit(EXIT_SUCCESS);
   }
    
  printf("best file is %s with score of %i\n", best_path, best_score);


   free(database_image);
   free(image_to_compare);
   return 0;
}


void son1_critic_sec(int score, char path_img[]){
  sem_wait(&sem);
  printf("%s\n", "SON 1 IN CRITIC SECTION");
printf("score : %i, best_score : %i, path_img : %s\n", score, best_score, path_img);

  if (score < best_score){
    best_score = score;
    memcpy(best_path, path_img, MAX_IMAGE_NAME_LENGTH);
    printf("SON 1 FOUND NEW BETTER IMAGE NOW, best_score : %i and best_path : %s\n", best_score, best_path);

  }
  sem_post(&sem);
  printf("%s\n", "SON 1 OUT CRITIC SECTION");
  printf("\n");
  
}
void son2_critic_sec(int score, char path_img[]){
  sem_wait(&sem);
  printf("%s\n", "SON 2 IN CRITIC SECTION");
  printf("score : %i, best_score : %i, path_img : %s\n", score, best_score, path_img);
  if (score < best_score){
    best_score = score;
    memcpy(best_path, path_img, MAX_IMAGE_NAME_LENGTH);
    printf("SON 2 FOUND NEW BETTER IMAGE NOW, best_score : %i and best_path : %s\n", best_score, best_path);

  }
  sem_post(&sem);
  printf("%s\n", "SON 2 OUT CRITIC SECTION");
  printf("\n");
  
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
