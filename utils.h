/** 
 * @file utils.h
 * @brief Prototypes de fonctions et variables externes
 * @authors Elias, Deni Shkembi, Luis Brunard
 * @date 2023-11-12
 */
#ifndef __UTILS_H
#define __UTILS_H

// Variables constantes
#define READ 0
#define WRITE 1
#define MAX_IMAGE_NAME_LENGTH 999

// Variables globales
extern int child_pids[2]; /// Stocke les ID des processus fils
extern volatile int keep_running;  // Contrôle la boucle du programme


// Validation des appels systèmes

/**
 * @brief Fonction qui permet de gérer les erreurs
 * @param ret la valeur de retour de la fonction à tester
 * @param calling_function le nom de la fonction appelante
 * @return la valeur de retour de la fonction à tester
 * @note Si la valeur de retour est négative, la fonction affiche un message d'erreur et quitte le programme avec EXIT_FAILURE
 * @note Cette idée ce fichier est inspirée des fonctions donnée en séance de TP
*/
int _checked(int ret, char*calling_function);
#define checked(call) _checked(call, #call)


struct shared_memory {
  int best_score;
  char best_path[MAX_IMAGE_NAME_LENGTH];

};


// Prototypes de fonctions

/**
 * @brief Fonction qui execute le programme img-dist avec les arguments path_comp et path_img
 * @param path_comp le chemin vers l'image a comparer
 * @param path_img le chemin vers l'image avec laquelle il faut comparer
 * @return le retour de la fonction system qui execute le programme img-dist, le score de similarite
*/
int img_dist(char path_comp[] , char path_img[]);

/**
 * @brief Fonction qui cree une memoire partagee et renvoie un pointeur vers celle-ci
 * @return un pointeur vers la memoire partagee
 * **/
struct shared_memory* create_mem_share();


// Gestionnaires de signaux

/**
 * @brief Gestion du signal SIGINT qui arrête le programme après avoir attendu que les enfants aient fini
*/
void SIGINT_HANDLE ();

/**
 * @brief Gestion du signal SIGPIPE qui entraîne l’arrêt de img-search et de ses fils sans générer un crash
*/
void SIGPIPE_HANDLE();

#endif