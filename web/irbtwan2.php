<?php

//connexion à la BDD
include 'connexionbdd.php';

/* On  fait une page de récupération des commandes pour le robot :
- Données formatées de la manière suivante :
  U ou D ou R ou L=1 ou 0;
  U = up
  D = down
  R = right
  L = left

  1 = TRUE
  0 = FALSE
*/

$cmdAlreadyPresent = FALSE;

$cmdHaut = 0;
$cmdBas = 0;
$cmdDroite = 0;
$cmdGauche = 0;

$reponse = $bdd -> query('SELECT * FROM `robot` WHERE 1');
//Vérifie qu'une commande est bien présente
while ($donnees = $reponse->fetch(PDO::FETCH_OBJ))
{
  $cmdAlreadyPresent = TRUE;
  $cmdHaut = $donnees->haut;
  $cmdBas = $donnees->bas;
  $cmdDroite = $donnees->droite;
  $cmdGauche = $donnees->gauche;
}
$reponse->closeCursor();


//si une commande est présente, alors affichage des infos et suppression de la commande
if($cmdAlreadyPresent)
{
  //Affichage des données
  echo 'U='.$cmdHaut.';';
  echo 'D='.$cmdBas.';';
  echo 'R='.$cmdDroite.';';
  echo 'L='.$cmdGauche.';';

  //Suppression de la commande lue (toute la table car normalement il n'y a qu'une commande à la fois)
  //SAUF SI la bdd contient le status "déconnecté"
  /*
  if($cmdHaut != 2 && $cmdBas != 2 && $cmdDroite != 2 && $cmdGauche != 2){
    $reponse = $bdd -> query('DELETE FROM `robot` WHERE 1');
    $reponse->closeCursor();
  }
  */
}

?>
