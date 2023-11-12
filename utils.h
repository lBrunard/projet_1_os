/**
 * @file utils.h
 * @brief Contient les fonctions utilitaires du programme
*/
#ifndef __UTILS_H
#define __UTILS_H
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MAX_IMAGE_NAME_LENGTH 999

/**
 * @brief Fonction qui permet de gérer les erreurs
 * @param ret la valeur de retour de la fonction à tester
 * @param calling_function le nom de la fonction appelante
 * @return la valeur de retour de la fonction à tester
 * @note Si la valeur de retour est négative, la fonction affiche un message d'erreur et quitte le programme avec EXIT_FAILURE
 * @note Cette de ce fichier est inspirée de la fonctions donnée en séance de TP
*/
int _checked(int ret, char*calling_function){
  if(ret == -1){
    perror(calling_function);
    exit(EXIT_FAILURE);
  }
  return ret;
}
#define checked(call) _checked(call, #call)




struct shared_memory {
  int best_score;
  char best_path[MAX_IMAGE_NAME_LENGTH];

};

/**
 * Fonction qui cree une memoire partagee et renvoie un pointeur vers celle-ci
 * @return un pointeur vers la memoire partagee
 * **/
static struct shared_memory* create_mem_share(){
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

#endif