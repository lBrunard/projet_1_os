#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <regex.h>
#include "vectors/vector.h"

#define MAX_IMAGE_NAME_LENGTH 999

void Process_args(char* img_path, int argc, char* argv[]);

int img_dist(char* path_comp[] , char* path_img[]);

/**
*Doit recevoir comme argument la Photo a compareet comme second 
*le dossier conentant les photos avec lesquelles il doit comparer
*
*@return L'image la plus similaire (ex. Most similar image found: 'img/22.bmp' with a distance of 12.)
*
**/
int main(int argc, char* argv[]){
    char *image_to_compare;
    image_to_compare= (char *)malloc(sizeof(char)*MAX_IMAGE_NAME_LENGTH);
    if (image_to_compare == NULL) {
    fprintf(stderr, "Erreur d'allocation mémoire\n");
    exit(EXIT_FAILURE);
}

    Process_args(image_to_compare, argc, argv);
    
    /*test pour la fonction img_dist*/
    int dist=img_dist(image_to_compare , "img/1.bmp");
    
    printf("%d", dist );


    free(image_to_compare);
    return 0;
}

void Process_args(char* image_to_compare, int argc, char* argv[]){

    if (argc!=2)
    {

        printf("nombre d'argument incorecte");
        exit(1);        
        
    }
    else
    {
        strcat(image_to_compare,argv[1]);
    }

    

    
}

int img_dist(char* path_comp[] , char* path_img[]){
    char command[2048];
    /* verification des fichier donner a la fonction , en cas d'erreur , renvois -1 et affiche la raison*/
    if (access(path_comp,F_OK)==-1){

        printf("%s n'est pas un fichier valide \n" , path_comp);
        return -1;
    }
    else if (access(path_comp,F_OK)==-1)
    {
        printf("%s n'est pas un fichier valide \n", path_comp);
        return -1;
    }
    snprintf(command, sizeof(command),"img-dist/img-dist %s %s",path_comp , path_img);
    /* utilisation de system pour utiliser le programe img-dist,  en cas d'erreur le retour est superieur a 64 */
    int retour = system(command);

    return (retour/256);

}
