<?php

include 'connexionbdd.php';

// test sur wan
if(TRUE)
{
  $haut = $_GET['up'];
  $bas = $_GET['down'];
  $droite = $_GET['right'];
  $gauche = $_GET['left'];

  echo 'haut'.$haut.'</ br>';
  echo 'bas'.$bas.'</ br>';
  echo 'droite'.$droite.'</ br>';
  echo 'gauche'.$gauche.'</ br>';

  /* On s'assure que le titre n'est pas enregistré */
  $cmdAlreadyPresent = FALSE;
  $reponse = $bdd -> query('SELECT COUNT(*) AS nb_lines FROM robot');

  while ($donnees = $reponse->fetch(PDO::FETCH_OBJ))
  {
    if($donnees->nb_lines>0)
    {
        $cmdAlreadyPresent = TRUE;
        $reponse = $bdd -> query('DELETE FROM `robot` WHERE 1');
    }
  }
  $reponse->closeCursor();

  $reponse = $bdd -> query('INSERT INTO `robot`(`haut`, `bas`, `droite`, `gauche`) VALUES ('.$haut.', '.$bas.', '.$droite.', '.$gauche.')');
  $reponse->closeCursor();
}

?>
