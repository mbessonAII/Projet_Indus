/**********************************************
 * 
 * Description :
 *  - Serveur embarqué (système de commande embarqué)
 *  - Client http (connexion à système de commande distant)
 *  - 
 * 
 */

//pour les threads
#include <StaticThreadController.h>
#include <Thread.h>
#include <ThreadController.h>

//embedded web server
#include <ESP8266WebServer.h>

//pour les commandes via le serveur distant
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>

//système de fichiers SPIFFS
#include "FS.h"


/**********************************************
 * Compilation conditionnelle
 */
#define USE_SERIAL Serial
//#define PRINT_HTTP_DEBUG Serial
//#define PRINT_THREAD_DEBUG Serial
#define PRINT_PRIORITY_DEBUG
//#define PRINT_REC_COMMANDS_WITHOUT_PRIORITY
#define PRINT_COMMANDS_WITH_PRIORITY
/**********************************************/



/**********************************************
 * Variables
 */
boolean commandesWAN[4] = {false, false, false, false};//[0]fwd [1]left [2]right [3]bwd
boolean embeddedCommand = false;// -false: commande depuis serveur distant (default)     - true: commande depuis serveur embarqué
/**********************************************/



/**********************************************
 * Pins ESP8266
 */
//int gpio0_pin = 0;//?
int gpio2_pin = 2;//LED
/**********************************************/



/**********************************************
 * Variables WAN
 */
String ssid = ""; // Replace with your network credentials
String password = "";

//cibles interface de lecture des commandes
char cible1[] = "http://masterclassejazz.esy.es/wan/irbtwan.php";
char cible2[] = "http://masterclassejazz.esy.es/wan/irbtwan2.php";
char* cibles[] = {cible1, cible2};
#define MAX_CIBLES 2
int cptCible = 0;
/**********************************************/



/**********************************************
 * Variables serveur embarqué
 *    ordre pour les boutons : debuts[i] + btnLayout[i] + btnEnDis[etatBtns[i]] + fins[i]
 */
 
//page de configuration (possibilité de la mettre dans un fichier)
String redirection = "<head><meta http-equiv='refresh' content=\"0; URL='http://192.168.4.1/\"></head>";

String styleForm = "<style>input{font-size: 120%;height: 15%;width: 80%;}p{font-size: 300%;}</style>";
String firstLine = "<form action='http://192.168.4.1/submit' method='POST'>";
String inputs = "<p>SSID:</p><p><input type='text' name='ssid'></p><p>Pass:</p><p><input type='text' name='pass'></p><p><input type='submit' value='OK'></p>";
String lastLine = "</form>";
/**********************************************/
//fileName (à entrer avec chemin du fichier) ex : "/fichier.txt"
//*data : pointeur sur la chaine qui va contenir la lecture du fichier (vider la chaine avant de la passer en arg)
bool loadFromFile(String fileName, String *data);

bool saveToFile(String fileName, String data);

/**********************************************
 * Prototypes
 */
/**********************************************/


void setup(void){
  
  USE_SERIAL.begin(115200);
  if (!SPIFFS.begin()) {
    USE_SERIAL.println("Failed to mount file system");
    return;
  }
  String webPage = "";
  //construction de la page web
  webPage = styleForm;
  webPage += firstLine;
  webPage += inputs;
  webPage += lastLine;

  //sauvegarde dans le système de fichiers
  String fileName = "/form.html";
  saveToFile(fileName, webPage);

  //vérification
  String verif = "";
  loadFromFile(fileName, &verif);

  Serial.println("lecture fichier: ");
  Serial.println(verif);
}



void loop(void){
  String s ="";
  if(Serial.available() > 0){
    s = Serial.readStringUntil('\n');
    if(s.equals("hey")){
      USE_SERIAL.println("recu");
    }
  }
}





/**********************************************
 * Mise à jour de fichiers
 */

bool loadFromFile(String fileName, String *data) {
  File file = SPIFFS.open(fileName.c_str(), "r");
  
  //vérifications
  if (!file) {
    Serial.println("Failed to open config file");
    return false;
  }

  size_t size = file.size();
  if (size > 1024) {
    Serial.println("Config file size is too large");
    return false;
  }

  USE_SERIAL.println("Loading config");
  while(file.available()) {
    //Lets read line by line from the file
    (*data) += file.readStringUntil('\n');
  }
  file.close();
  return true;
}

bool saveToFile(String fileName, String data) {
  File file = SPIFFS.open(fileName.c_str(), "w");
  
  //vérification
  if (!file) {
    USE_SERIAL.println("File doesn't exist");
    //return false;
    file = SPIFFS.open("/style.css", "w+");//on crée le fichier s'il n'existe pas
    if(!file){
      USE_SERIAL.println("Failed to create file");
      return false;
    }
  }

  //ecriture du fichier
  file.println(data);

  file.close();
  return true;
}
/**********************************************/
