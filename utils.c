/**
 * @file utils.h
 * @brief Contient les fonctions utilitaires du programme
 * @authors Elias, Deni Shkembi, Luis Brunard
 * @date 2023-11-12
*/
#define _GNU_SOURCE
#define _POSIX_C_SOURCE 201710L

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <signal.h>

#include "utils.h"


int _checked(int ret, char*calling_function){
  if(ret == -1){
    perror(calling_function);
    exit(EXIT_FAILURE);
  }
  return ret;
}


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


int img_dist(char path_comp[] , char path_img[]){
    char command[2048];
    checked(access(path_img,F_OK));
    
    snprintf(command, sizeof(command),"./img-dist/img-dist %s %s",path_comp , path_img);
    /* utilisation de system pour utiliser le programe img-dist,  en cas d'erreur le retour est superieur a 64 */
    int retour = system(command);

    return (retour/256);

}


void SIGINT_HANDLE (){
   keep_running = 0;
}


void SIGPIPE_HANDLE(){
   // Arrête les processus fils 
   kill(child_pids[0], SIGTERM);
   kill(child_pids[1], SIGTERM);
   keep_running = 0;
}