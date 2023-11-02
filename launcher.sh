# usage : launcher [-i|--interactive|-a|--automatic] image [database_path]
#!/bin/sh

usage="usage : launcher [-i|--interactive|-a|--automatic] image [database_path]
\nLes paramètres doivent être interprétés en fonction du mode :
  \t\n— mode interactif (option -i ou --interactive) :
  \t    \n\t— La banque d’image sera transmise manuellement (c’est-à-dire image par image) sur stdin
  \t    \n\tpar l’utilisateur ; ### SUR PARAMETRE SUR LE CODE img-search.c
  \t    \n\t— image : correspond à l’image à comparer avec celles de la banque d’images ;
  \t    \n\t— database_path : préfixe du chemin des images transmises sur stdin (vide par défaut si non spécifié). 
  \t                \n\t\t\tL’ajout du préfixe doit se faire dans le script Bash ;
  \t\n\n— mode automatique (option -a ou --automatic) :
  \t    \n\t— La banque d’image sera transmise automatiquement par le script ;
  \t    \n\t— image : correspond à l’image à comparer avec celles de la banque d’images ;
  \t    \n\t— database_path : dossier dont tous les fichiers (mais pas ceux des sous-dossiers) consti-
  \t        \n\t\t\ttuent la banque d’images que le script transmet lui-même à img-search. 
  \t      \n\t\t\tSi ce paramètre n’est pas spécifié, sa valeur est fixée à « ./img/ »."

mode=""
image=""
database_path=""

key=$1
if [ -z "$key" ]; then
    echo $usage
    exit 0
fi

case $key in 
    -i|--interactive )
        mode="interactiv"
        shift
        ;;
    -a|--automatic )
        mode="automatic"
        shift
        ;;
    *)
        echo $usage
        exit 1
esac

if [ -z "$image" ] && [ -f "$1" ]; then
    image="$1"
else 
    echo $usage
    exit 2
fi

if [ "$mode" = "automatic" ] && [ -d "$2" ]; then
    database_path="$2"
    echo "lancement du mode auto, sur le fichier $image et avec la db $database_path"
    ./list-file.sh $database_path | ./img-search "$image"
elif [ "$mode" = "automatic" ] && [ -z "$2" ]; then
    database_path="./img/"
    echo "lancement du mode auto, sur le fichier $image et avec la db $database_path"
    ./list-file.sh $database_path | ./img-search "$image"

elif [ "$mode" = "interactiv" ]; then
    database_path="$2"
    echo "lancement du mode interactiv, sur le fichier $image et avec passage par défaut des fichiers depuis la le path $database_path"
    
    while true; do
        read input
        if [ "$input" = "exit" ]; then
            break
        fi
        echo "$database_path$input"
    done | ./img-search "$image"
fi

    

    
