<html>
  <head>
    <meta charset="utf-8"/>
    <meta name="viewport" content="width=device-width, initial-scale=1.0"/>

    <link rel="stylesheet" href="https://maxcdn.bootstrapcdn.com/bootstrap/4.0.0-alpha.6/css/bootstrap.min.css" integrity="sha384-rwoIResjU2yc3z8GV/NPeZWAv56rSmLldC3R/AZzGRnGxQQKnKkoFVhFQhNUwEyJ" crossorigin="anonymous">

  </head>

<!-- On se connecte sur une base de données pour enregistrer les commandes
La BDD stoque 1 commande au maximum
Si on stoque une commande et qu'il y en a déjà une, on l'écrase.
Lors de la lecture de la commande, la ligne est effacée.

Résultat :
// Ok, on arrive a effacer la table entière lorsqu'une ligne est déjà présente
// et on stocke la nouvelle ligne, vérifié sur PHPMyAdmin

NE PAS OUBLIER DE FAIRE LA MODIF : les comds haut/bas doite/gauche doivent être exclusives
Eventuellement ajouter un bouton d'arrêt général

 -->

    <body>
      <div class="container">
        <div class="row mb-4">
          <div class="col align-self-start">
            <!-- <button type="button" class="btn btn-outline-primary">Warning</button> -->
          </div>
          <div class="col align-self-center">
            <button type="button" class="btn btn-danger" id="btnFwd">Fwd</button>
          </div>
          <div class="col align-self-end">
            <!-- <button type="button" class="btn btn-outline-primary">Warning</button> -->
          </div>
        </div>
        <div class="row mb-4">
          <div class="col align-self-start">
            <button type="button" class="btn btn-danger" id="btnLeft">Left</button>
          </div>
          <div class="col align-self-center">
            <!-- <button type="button" class="btn btn-outline-primary">Warning</button> -->
          </div>
          <div class="col align-self-end">
            <button type="button" class="btn btn-danger" id="btnRight">Right</button>
          </div>
        </div>
        <div class="row mb-4">
          <div class="col align-self-start">
            <!-- <button type="button" class="btn btn-outline-primary">Warning</button> -->
          </div>
          <div class="col align-self-center">
            <button type="button" class="btn btn-danger" id="btnBwd">Bwd</button>
          </div>
          <div class="col align-self-end">
            <!-- <button type="button" class="btn btn-outline-primary">Warning</button> -->
          </div>
        </div>
      </div>

      <script src="https://code.jquery.com/jquery-3.1.1.min.js"></script>
      <script src="https://cdnjs.cloudflare.com/ajax/libs/tether/1.4.0/js/tether.min.js" integrity="sha384-DztdAPBWPRXSA/3eYEEUWrWCy7G5KFbe8fFjk5JAIxUYHKkDx6Qin1DkWx51bBrb" crossorigin="anonymous"></script>
      <script src="https://maxcdn.bootstrapcdn.com/bootstrap/4.0.0-alpha.6/js/bootstrap.min.js" integrity="sha384-vBWWzlZJ8ea9aCX4pEW3rVHjgjt7zpkNpZk+02D9phzyeVkE+jo0ieGizqPLForn" crossorigin="anonymous"></script>

      <script>
      var bUpState = false;
      var bDownState = false;
      var bRightState = false;
      var bLeftState = false;
      var bConnected = true;

      $( document ).ready(function() {
          console.log( "ready!" );
          //update connected status

          $(window).on('beforeunload', function() {
            var x =logout();
            return x;
          });
          function logout(){
            //update connected status
            setDisconnectedStatus();
            Console.log("disconnected");
            return 1+3;
          }

          function setDisconnectedStatus(){
            $.get(
                'savecmd.php',
                {
                    up    : 2,
                    down  : 2,
                    right : 2,
                    left  : 2,
                },
                function(data){

                },
                'text'
             );
          };


          function printStg (){
            console.log("bUpState " + bUpState);
            console.log("bDownState " + bDownState);
            console.log("bLeftState " + bLeftState);
            console.log("bRightState " + bRightState);
          };

          $("#btnFwd").click(function(){
            bUpState = !bUpState;
            //printStg();
            sendCmd();
          });
          $("#btnBwd").click(function(){
            bDownState = !bDownState;
            //printStg();
            sendCmd();
          });
          $("#btnRight").click(function(){
            bRightState = !bRightState;
            //printStg();
            sendCmd();
          });
          $("#btnLeft").click(function(){
            bLeftState = !bLeftState;
            //printStg();
            sendCmd();
          });

          function sendCmd (){
            console.log("Send cmd");
            $.get(
                'savecmd.php', // Un script PHP que l'on va créer juste après
                {
                    up : bUpState?1:0,
                    down : bDownState?1:0,
                    right : bRightState?1:0,
                    left : bLeftState?1:0,
                },

                function(data){
                  //remove previous color
                  (!bUpState)     ?   $("#btnFwd").removeClass("btn-success")    :   $("#btnFwd").removeClass("btn-danger"),
                  (!bDownState)   ?   $("#btnBwd").removeClass("btn-success")    :   $("#btnBwd").removeClass("btn-danger"),
                  (!bRightState)  ?   $("#btnRight").removeClass("btn-success")  :   $("#btnRight").removeClass("btn-danger"),
                  (!bLeftState)   ?   $("#btnLeft").removeClass("btn-success")   :   $("#btnLeft").removeClass("btn-danger"),

                  //add the right color
                  bUpState      ?   $("#btnFwd").addClass("btn-success")    :   $("#btnFwd").addClass("btn-danger"),
                  bDownState    ?   $("#btnBwd").addClass("btn-success")    :   $("#btnBwd").addClass("btn-danger"),
                  bRightState   ?   $("#btnRight").addClass("btn-success")  :   $("#btnRight").addClass("btn-danger"),
                  bLeftState    ?   $("#btnLeft").addClass("btn-success")   :   $("#btnLeft").addClass("btn-danger")

                    if(data == false){

                    }
                    else{

                    }

                },

                'text'
             );
          };
      });
      </script>

    </body>
</html>
