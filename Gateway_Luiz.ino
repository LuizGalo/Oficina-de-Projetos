

#include "heltec.h"


#define pwr       14  // Potência utilizada no PA em dBm
#define led       23  // Led ligado ao pino 23



//-----Identificação dos nodes------------------------------------------------------
const byte master = 0x11; const byte node_1  = 0x1A; const byte gateway = 0x3A;


//-----Identificação das rotas------------------------------------------------------
const byte R0 = 0xFF; // M-G
const byte R1 = 0xB3; // M-1-G

//----Confirmação--------------------------------------------------------------------
 //  0xF4 Confirmação intermediária com erro
//------Armazenando Rota e Destino em Vetores----------------------------------------
int next[]={gateway,node_1};
int rot[]={R0,R1};
int i=0;
//-----Estrutura do pacote (transmissão)--------------------------------------------
byte sender; byte destination; byte repet; byte route; //String incoming;
//-----Estrutura do pacote (recepção)-----------------------------------------------
byte SenderReception; byte NextReception; byte FinalReception; byte RouteReception; String incoming = "";
      
byte incoming_length;

//-----Variáveis de controle-------------------------------------------------------- 
int rssi;
int packetsize;
const unsigned int threshold = 2000; //Define o tempo de espera da msg de confirmação
const int rssiThreshold = -80; //Valor mínimo de RSSI aceitável
bool confirmation;

//-----Temporização e contadores----------------------------------------------------
long lastSendTime =0;
int interval =2000;
 
String message = "Payload"; //Payload a ser enviado para o gateway

//-----Funções Utilizadas-------------------------------------------------------------
void sendMessage(byte Sender, byte NextDestiantion,byte FinalDestination,byte Route,String Outgoing);

void setup() 
{
   Heltec.begin(true,true,true,true,920E6);
 
 //-------Display----------------------------------------------------------------------
   Heltec.display->flipScreenVertically();
   Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
   Heltec.display->setFont(ArialMT_Plain_10);
   Heltec.display->setContrast(255);
   Heltec.display->drawString(63,32,"GATEWAY"); // (pos x0, pos y0,String)
   Heltec.display->display();
 
}

void loop() 
{
  confirmation=true;
  packetsize = LoRa.parsePacket();
  if (packetsize)
  {
     onReceive(packetsize);
  }
  if (not confirmation)
  {
    sendMessage(NextReception,SenderReception,0xF4,RouteReception,"Erro");
  }
  
}
void sendMessage(byte Sender, byte NextDestination, byte FinalDestination, byte Route, String Outgoing)
{
  Serial.println("|-------------------------SendMessage---------------------------|");
  Serial.println(" Prox Dest  = 0x" + String(NextDestination,HEX));
  Serial.println(" Dest final = 0x" + String(FinalDestination,HEX));
  Serial.println(" Route      = 0x" + String(Route,HEX));
   
  LoRa.beginPacket();
  LoRa.setTxPower(pwr,RF_PACONFIG_PASELECT_PABOOST); // configura a potência do PA para a transmissão
  LoRa.write(Sender);
  LoRa.write(NextDestination);
  LoRa.write(FinalDestination);
  LoRa.write(Route);
  LoRa.write(Outgoing.length());
  LoRa.print(Outgoing);
  LoRa.endPacket();
  LoRa.disableCrc();
  LoRa.receive();
}
void onReceive(int packetSize)
{
    //Lê os bytes de controle    
    SenderReception  = LoRa.read();   
    NextReception   = LoRa.read(); 
    FinalReception  = LoRa.read();
    RouteReception  = LoRa.read();
    incoming_length = LoRa.read();
    incoming = "";
    int projeto = LoRa.packetRssi() + 20;
    //Verifica se a msg é para esse node
    if (NextReception != gateway) 
    { 
      return;
      delay(500);
    } 

    //Lê o payload em cod ascii, converte em char e concatena em uma string
    while(LoRa.available())
    {
      incoming += (char)LoRa.read();
      
    }
         if(incoming_length != incoming.length()||projeto<rssiThreshold )
    {
      Serial.println("Perda de Pacote ou RSSI muito baixo!!!!!");
      confirmation=false;
      return;
    }
    Serial.print(incoming);  
    // print RSSI of packet
    Serial.println("' with RSSI "+ String(projeto));
}
 
