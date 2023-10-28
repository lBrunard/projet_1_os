1.  on lance le launcher avec les parametres requis

2.  on lance le main avec comme argument la photo qui va etre comparée au autres

3.  le main créer 2 process fils et créer des pipe entre lui et les fils

	3.2 les fils ont donc chacuns acces a la fonction img search. 3.3 une mémoire partagée est crée entre les fils et le père

4.  le pere(main) lit le sur le stdin(cin) chaque path de photo qui arrive du script img-list.sh

	4.2 chaque entrée sur le stdin est distribuée a un fils (soit 1 sur 2 soit le premier disponible)

5.  chaque fils calcule la distance(programme donné) entre les images recues

	5.2 le fils compare le résultat à celui sur la mémoire partagée et 5.3 si le résultat est meilleur les 2 seules valeures dans la mémoire partagée sont changée en un nouveau score et un nouveau chemin

6.  une fois qu'il n'y a plus de photo a comparer, le pere donne la photo qui se rapproche le plus
<<<<<<< HEAD

=======
>>>>>>> 2b609583ef82c609c03ac533823b82381353c65c
