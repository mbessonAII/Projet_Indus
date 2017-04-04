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
#define PRINT_FILE_DEBUG
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
String webPage = "";
boolean etatsBtns[4] = {false, false, false, false};
String style = "<style>button{font-size: 300%;height: 15%;}.ud{width: 100%;}.rl{width: 50%;}.ip{display: inline;}.en{background-color: #5cb85c;}.dis{background-color: #d9534f;}.rst{background-color: #f0ad4e;}h1{text-align: center;}</style>";
String title = "<h1>ESP8266 Web Server</h1>";
String btnLayout[] = {"<button class=\"ud", "<button class=\"rl", "<button class=\"rl", "<button class=\"ud", "<button class=\"ud rst\""};//un pour chaque btn
String btnEnDis[] = {" dis\">", " en\">"};
String debuts[] = {"<p><a href=\"fwd\">", "<p class=\"ip\"><a href=\"left\">", "<p class=\"ip\"><a href=\"right\">", "<p><a href=\"bwd\">", "<p><a href=\"rst\">"};//un pour chaque btn
String fins[] = {"Fwd</button></a></p>", "Left</button></a></p>", "Right</button></a></p>", "Bwd</button></a></p>", ">RESET</button></a></p>"};//un pour chaque btn
String whoHaveControl[2] = {"<h1>you have control</h1>", "<h1>someone else have control</h1>"};//local, wan

//redirection vers l'interface de commande embarquée
String redirection = "<head><meta http-equiv='refresh' content=\"0; URL='http://192.168.4.1/\"></head>";

//page de configuration (maintenant dans un fichier)
/*String styleForm = "<style>input{font-size: 120%;height: 15%;width: 80%;}p{font-size: 300%;}</style>";
String firstLine = "<form action='http://192.168.4.1/submit' method='POST'>";
String inputs = "<p>SSID:</p><p><input type='text' name='ssid'></p><p>Pass:</p><p><input type='text' name='pass'></p><p><input type='submit' value='OK'></p>";
String lastLine = "</form>";*/
String formulaire = "";
/**********************************************/



/**********************************************
 * Prototypes
 */

//Traitements des threads
void wanCallback();
void wan();

void localCallback();
void local();

void executionCallback();
void executeCommands(boolean tabCommands[], int len);

//conversion
boolean isNumberChar(char c);
int charToInt(char c);

//autres
void blink(int);
void updateHTML();
boolean isClientConnectedOnTheInternet(boolean tabCommands[], int len);

//sauvegarde des changements de config via le serveur embarqué
void handleSubmit();
void updateWiFiConfig(String newSSID, String newPASS);

/*
 * Description: read a file specified by arg0 and append the result on arg1
 * arg0: file path with extension
 * arg1: String pointer to read result (must be empty)
 * return: true when operation succeed
 */
bool loadFromFile(String, String *);
bool saveToFile(String fileName, String data);
/**********************************************/



/**********************************************
 * Constantes priorités
 */
const int WAN_PRIORITY_LOW = 15000;   //15s
const int WAN_PRIORITY_HIGH = 10000;  //10s

const int LAN_PRIORITY_LOW = 4000;    //5s
const int LAN_PRIORITY_HIGH = 800;    //1s
/**********************************************/



/**********************************************
 * Objets utilisés
 */
ESP8266WiFiMulti WiFiMulti;
ESP8266WebServer server(80);

Thread wanThread = Thread();
Thread localThread = Thread();
Thread executionThread = Thread();
/**********************************************/




void setup(void){

  //construit la page web à servir dans la chaine webPage
  updateHTML();
  
  
  /**********************************************
  * preparing GPIOs
  */
  pinMode(gpio2_pin, OUTPUT);
  digitalWrite(gpio2_pin, HIGH);
  USE_SERIAL.begin(115200);
  /**********************************************/
  
  delay(1000);

  WiFi.softAP("Robot");
  /**********************************************
  * Réglages du serveur embarqué
  */
  
  //Affectation des actions a effectuer lors des requêtes GET reçues
  
  //appelé lors d'une requête http://192.168.4.1/ (ip de l'ESP en serveur embarqué)
  server.on("/", [](){
    updateHTML();
    server.send(200, "text/html", webPage);
  });
  //appelé lors d'une requête http://192.168.4.1/fwd
  server.on("/fwd", [](){
    blink(gpio2_pin);
    etatsBtns[0] = !etatsBtns[0];
    #ifdef PRINT_REC_COMMANDS_WITHOUT_PRIORITY
      USE_SERIAL.print("local command\tFORW: ");
      USE_SERIAL.println(etatsBtns[0]);
    #endif
    updateHTML();
    server.send(200, "text/html", webPage);
    delay(1000);
  });
  
  //appelé lors d'une requête http://192.168.4.1/bwd
  server.on("/bwd", [](){
    blink(gpio2_pin);
    etatsBtns[3] = !etatsBtns[3];
    #ifdef PRINT_REC_COMMANDS_WITHOUT_PRIORITY
      USE_SERIAL.print("local command\tBACK: ");
      USE_SERIAL.println(etatsBtns[3]);
    #endif
    updateHTML();
    server.send(200, "text/html", webPage);
    delay(1000); 
  });
  //appelé lors d'une requête http://192.168.4.1/right
  server.on("/right", [](){
    blink(gpio2_pin);
    etatsBtns[2] = !etatsBtns[2];
    #ifdef PRINT_REC_COMMANDS_WITHOUT_PRIORITY
      USE_SERIAL.print("local command\tRIGHT: ");
      USE_SERIAL.println(etatsBtns[2]);
    #endif
    updateHTML();
    server.send(200, "text/html", webPage);
    delay(1000);
  });
  //appelé lors d'une requête http://192.168.4.1/left
  server.on("/left", [](){
    blink(gpio2_pin);
    etatsBtns[1] = !etatsBtns[1];
    #ifdef PRINT_REC_COMMANDS_WITHOUT_PRIORITY
      USE_SERIAL.print("local command\tLEFT: ");
      USE_SERIAL.println(etatsBtns[1]);
    #endif
    updateHTML();
    server.send(200, "text/html", webPage);
    delay(1000); 
  });
  
  //appelé lors d'une requête http://192.168.4.1/rst
  server.on("/rst", [](){
    blink(gpio2_pin);
    
    //reset des états des boutons
    etatsBtns[0] = false;
    etatsBtns[1] = false;
    etatsBtns[2] = false;
    etatsBtns[3] = false;
    
    //affichage série
    #ifdef PRINT_REC_COMMANDS_WITHOUT_PRIORITY
    USE_SERIAL.print("local command\trst: ");
    USE_SERIAL.print("F: ");
    USE_SERIAL.print(etatsBtns[0]);
    USE_SERIAL.print(" B: ");
    USE_SERIAL.print(etatsBtns[1]);
    USE_SERIAL.print(" R: ");
    USE_SERIAL.print(etatsBtns[2]);
    USE_SERIAL.print(" L: ");
    USE_SERIAL.println(etatsBtns[3]);
    #endif
    //création et envoi de la page web
    updateHTML();
    server.send(200, "text/html", webPage);
    delay(1000); 
  });

  //lors de requête http://192.168.4.1/config
  server.on("/config", [](){
    blink(gpio2_pin);
    server.send(200, "text/html", formulaire);
    delay(1000); 
  });
  
  //lors de requête http://192.168.4.1/submit
  server.on("/submit", [](){
    blink(gpio2_pin);
    handleSubmit();
    //webPage = redirection;
    server.send(200, "text/html", redirection);
    delay(1000); 
  });

  //Démarrage du serveur embarqué
  server.begin();
  USE_SERIAL.println("HTTP server started");
  
  //Tempo de 5s pour le démarrage
  for(uint8_t t = 4; t > 0; t--) {
    USE_SERIAL.printf("[SETUP] WAIT %d...\n", t);
    USE_SERIAL.flush();
    delay(1000);
  }
  
  /**********************************************/

  
 
  /**********************************************
  * Réglagles du point de connexion à internet
  */
  boolean res;
  res = WiFiMulti.addAP(ssid.c_str(), password.c_str());
  USE_SERIAL.print("wifi setup = ");
  USE_SERIAL.println(res);

  /**********************************************/


  /**********************************************
  * Paramétrage des threads
  */
  wanThread.onRun(wanCallback);
  wanThread.setInterval(5000);
  
  localThread.onRun(localCallback);
  localThread.setInterval(1000);
  
  executionThread.onRun(executionCallback);
  executionThread.setInterval(1000);
  /**********************************************/

  if (!SPIFFS.begin()) {
    USE_SERIAL.println("Failed to mount file system");
    return;
  }
  //test fichiers SPIFFS
  
  loadFromFile("/form.html", &formulaire);
}





void loop(void){
  
  /**********************************************
  * Acquisition des commandes dans les threads correspondants
  */
  if(localThread.shouldRun())
    localThread.run();
    
  if(wanThread.shouldRun())
    wanThread.run();
  /**********************************************/
  

  /**********************************************
  * Execution de la commande retenue
  */
  if(executionThread.shouldRun())
    executionThread.run();
    
  /**********************************************/
}




void updateHTML(){
  webPage = style;
  webPage += title;

  //choix des boutons suivant l'état de leurs commandes
  for(int i=0; i<5; i++){
    webPage += debuts[i];
    webPage += btnLayout[i];
    if(i!=4)// le dernier btn (reset) n'a pas d'état
      webPage += btnEnDis[etatsBtns[i]];
    webPage += fins[i];
  }
  
  //choix d'un texte pour dire qui a la main sur le système
  webPage += whoHaveControl[!embeddedCommand];
}




/**********************************************
 * Implémentations traitements Threads
 */
void wan() {
    // wait for WiFi connection
    if((WiFiMulti.run() == WL_CONNECTED)) {

        HTTPClient http;
        
        //variables pour les directions
        String str = "";
        int u = 0, d = 0, l = 0, r = 0;
        
        #ifdef PRINT_HTTP_DEBUG
          USE_SERIAL.print("[HTTP] begin...\n");
        #endif
        
        // configure target server and url
        http.setTimeout(15000);
        http.begin(cibles[cptCible]); //HTTP
        #ifdef PRINT_HTTP_DEBUG
          PRINT_HTTP_DEBUG.print("[HTTP] GET...\n");
        #endif
        
        // start connection and send HTTP header
        http.setTimeout(15000);
        int httpCode = http.GET();

        // httpCode will be negative on error
        if(httpCode > 0) {
            // HTTP header has been send and Server response header has been handled
            #ifdef PRINT_HTTP_DEBUG
              USE_SERIAL.printf("[HTTP] GET... code: %d\n", httpCode);
            #endif
            
            // file found at server
            if(httpCode == HTTP_CODE_OK || true) {
                String payload = http.getString();
                #ifdef PRINT_HTTP_DEBUG
                  USE_SERIAL.println(payload);
                #endif
                
                //si la réponse contient une erreur de type "trop de connexions"
                if(strstr(payload.c_str(), "Erreur : SQLSTATE[HY000] [1040] Too many connections") != NULL)
                {
                  USE_SERIAL.println("trop de co");
                  USE_SERIAL.println("Change http target");
                  cptCible++;//on passe à la cible http suivante
                  cptCible%=MAX_CIBLES;//saturation de l'index
                }
                  
                str = payload;
                if(str.c_str() != "")
                {
                  //on vérifie que le caractère est numérique et si c'est le cas on le convertit en int pour le mémoriser
                  u = isNumberChar(str.charAt(2))   ? charToInt(str.charAt(2))  :0;
                  d = isNumberChar(str.charAt(6))   ? charToInt(str.charAt(6))  :0;
                  r = isNumberChar(str.charAt(10))  ? charToInt(str.charAt(10)) :0;
                  l = isNumberChar(str.charAt(14))  ? charToInt(str.charAt(14)) :0;

                  //màj des valeurs dans le tableau
                  commandesWAN[0] = u;
                  commandesWAN[1] = l;
                  commandesWAN[2] = r;
                  commandesWAN[3] = d;

                  //affichage des variables contenant les commandes récupérées sur le web
                  #ifdef PRINT_REC_COMMANDS_WITHOUT_PRIORITY
                    USE_SERIAL.println("WAN Commands");
                    USE_SERIAL.print("u: "); USE_SERIAL.println(u);
                    USE_SERIAL.print("d: "); USE_SERIAL.println(d);
                    USE_SERIAL.print("r: "); USE_SERIAL.println(r);
                    USE_SERIAL.print("l: "); USE_SERIAL.println(l);
                  #endif

                  /*
                   * est ce qu'il y a un client de connecté au serveur de commandes distant ?
                   * - oui : commande distante
                   * - non : commande locale
                   */
                  embeddedCommand = !(isClientConnectedOnTheInternet(commandesWAN, 4));
                  
                  
                  blink(gpio2_pin);
                }
            }
        } else {
          
            #ifdef PRINT_HTTP_DEBUG
              USE_SERIAL.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
            #endif
            
            //si il y a une erreur on utilisera la commande locale
            embeddedCommand = true;
        }

        http.end();
    }
    else
    {
      //si on est pas connecté au WiFi alors on met la commande locale en priorité
      embeddedCommand = true;
    }
}

//callback pour l'execution des commandes
void executionCallback(){
  if(embeddedCommand)
  {
    executeCommands(etatsBtns, 4);
  }
  else
  {
    executeCommands(commandesWAN, 4);
  }
}

// callback for wanThread
void wanCallback(){
  wan();
  #ifdef PRINT_THREAD_DEBUG
  USE_SERIAL.print("wan runned: ");
  #endif
}

// callback for localThread
void localCallback(){
  local();
  #ifdef PRINT_THREAD_DEBUG
  USE_SERIAL.print("local runned: ");
  #endif
}
void local(){
  server.handleClient();
}

void executeCommands(boolean tabCommands[], int len){
  if(embeddedCommand){
    #ifdef PRINT_PRIORITY_DEBUG
      USE_SERIAL.println("CMD EMB");
    #endif

    printCmds(etatsBtns);

    //on change les intervalles d'appel des thread selon leur priorité
    localThread.setInterval(LAN_PRIORITY_HIGH);
    wanThread.setInterval(WAN_PRIORITY_LOW);
  }
  else{
    #ifdef PRINT_PRIORITY_DEBUG
      USE_SERIAL.println("CMD DIST");
    #endif

    printCmds(commandesWAN);
    
    //on change les intervalles d'appel des thread selon leur priorité
    localThread.setInterval(LAN_PRIORITY_LOW);
    wanThread.setInterval(WAN_PRIORITY_HIGH);//voir si on peut aller plus vite
  }
}
/**********************************************/



/**********************************************
 * Utilitaires
 */
void blink(int pin){
  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);
  delay(50);
  digitalWrite(pin, HIGH);
}

//return : true if c is alphabetic
boolean isNumberChar(char c){
  if(c < '0' || c > '9')
    return false;
  return true;
}

//parse char to int
int charToInt(char c){
  return (c - '0');
}

//return : true si un client
boolean isClientConnectedOnTheInternet(boolean tabCommands[], int len){
  for(int i = 0; i<len; i++){
    if(tabCommands[i] == 2)
      return false;
  }
  return true;
}

//utilitaire visualisation des commandes reçues
void printCmds(boolean tabCommands[]){
  #ifdef PRINT_COMMANDS_WITH_PRIORITY
    USE_SERIAL.print("F: "); USE_SERIAL.println(tabCommands[0]);
    USE_SERIAL.print("L: "); USE_SERIAL.println(tabCommands[1]);
    USE_SERIAL.print("R: "); USE_SERIAL.println(tabCommands[2]);
    USE_SERIAL.print("B: "); USE_SERIAL.println(tabCommands[3]);
  #endif
}
/**********************************************/



/**********************************************
 * Traitement requêtes http sur le serveur embarqué
 */

void handleSubmit(){
  if (server.args() > 0 ) {
    if(server.hasArg("ssid"))// check if argument exists
    {
      //overwrite previous SSID
      ssid = String(server.arg("ssid"));
      USE_SERIAL.println(ssid);
      
      //overwrite previous password
      if(server.hasArg("pass"))// check if argument exists
      {
        password = String(server.arg("pass"));
        USE_SERIAL.println(password);
      }

      //Connexion au nouveau point d'accès WiFi
      updateWiFiConfig(ssid, password);
    }
  }
}
/**********************************************/



/**********************************************
 * Update de la config WiFi
 */

void updateWiFiConfig(String newSSID, String newPASS){
  boolean res = WiFiMulti.addAP(newSSID.c_str(), newPASS.c_str());
  USE_SERIAL.print("wifi setup = ");
  USE_SERIAL.println(res);
}
/**********************************************/



/**********************************************
 * Mise à jour de fichiers via le serveur embarqué
 */

bool loadFromFile(String filePath, String *data) {
  File configFile = SPIFFS.open(filePath, "r");
  
  //vérifications
  if (!configFile) {
    #ifdef PRINT_FILE_DEBUG
      USE_SERIAL.println("Failed to open config file");
    #endif
    
    return false;
  }

  size_t size = configFile.size();
  if (size > 1024) {
    #ifdef PRINT_FILE_DEBUG
      USE_SERIAL.println("Config file size is too large");
    #endif
    
    return false;
  }

  USE_SERIAL.println("Loading config");
  while(configFile.available()) {
    //Lets read line by line from the file
    (*data) += configFile.readStringUntil('\n');
    
    #ifdef PRINT_FILE_DEBUG
      USE_SERIAL.println(*data);
    #endif
  }
  configFile.close();
  return true;
}

bool saveToFile(String fileName, String data) {
  File file = SPIFFS.open(fileName.c_str(), "w");
  
  //vérification
  if (!file) {
    #ifdef PRINT_FILE_DEBUG
      USE_SERIAL.println("File doesn't exist");
    #endif
    file = SPIFFS.open("/style.css", "w+");//on crée le fichier s'il n'existe pas
    if(!file){
      #ifdef PRINT_FILE_DEBUG
        USE_SERIAL.println("Failed to create file");
      #endif
      return false;
    }
  }

  //ecriture du fichier
  file.println(data);

  file.close();
  return true;
}
/**********************************************/
