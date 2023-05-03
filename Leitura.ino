//Codigo pronto pendente apenas testes
//by: Wagner Medeiros
//Leitura de digitais
//Melhoria pendente acrescenta hora no display (1,0);

#include <SoftwareSerial.h>
#include "RTClib.h"
#include "DHT.h"
#include <SD.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_Fingerprint.h>

const uint32_t password = 0x0;//Senha padrão do sensor de digitais

#define RX_PIN A2                                         
#define TX_PIN A3 
int Pino_CS = 10;//Pino CS do cartao SD

SoftwareSerial mySerial(RX_PIN, TX_PIN);
Adafruit_Fingerprint fingerprintSensor = Adafruit_Fingerprint(&mySerial, password);


LiquidCrystal_I2C lcd(0x27, 16, 2);


//Definicoes do sensor de temperatura DHT11
#define DHTPIN 5 //Pino de conexao
#define DHTTYPE DHT22 //Tipo de sensor
DHT dht(DHTPIN, DHTTYPE); //Cria objeto DHT

RTC_DS1307 rtc;
File file;

void setup()
{
  dht.begin();//Inicializa o DHT11
  lcd.begin();
  lcd.backlight();
  Serial.begin(9600);
  mySerial.begin(9600);

  fingerprintSensor.begin(57600); //Inicializa o sensor
  if(!fingerprintSensor.verifyPassword())//Verifica se a senha está correta
  {
    Serial.println(F("Não foi possível conectar ao sensor. Verifique a senha ou a conexão"));//Se chegou aqui significa que a senha está errada ou o sensor está problemas de conexão
    while(true);
  }

  Serial.println("Digital Maker");
  Serial.println();
  lcd.setCursor(0,2);
  lcd.print(F("Digital Maker"));
  delay(500);

  
  //Inicia o cartao SD
  Serial.println("Iniciando cartao SD...");

  
  if (!SD.begin(Pino_CS))
  {
  Serial.println("Falha na inicializacao do SD!");
  Serial.println();
  lcd.setCursor(0,2);
  lcd.print(F("Falha Cartao"));
  lcd.delay(500);
    return;
  }
  

else{
Serial.println("SD Card iniciado");
return;
}

  //Verifica as condicoes do RTC
  if (! rtc.begin())
  {
    Serial.println("RTC nao encontrado!");
    lcd.clear();
    lcd.setCursor(0,3);
    lcd.print(F("RTC Falha"));
    delay(500);
    
    while (1);
  }
  if (! rtc.isrunning())
  {
    Serial.println("RTC operante!");
    //A linha abaixo ajusta o RTC com a data e hora do momento da compilacao
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    //A linha abaixo permite ajustar manualmente a data e hora do RTC
    //Formato: DateTime(ano, mes, dia, hora, minuto, segundo)
    //rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }


  
}

void loop(void)
{
Serial.println(F("Encoste o dedo no sensor"));
  lcd.clear();
  lcd.setCursor(0,1);
  lcd.print(F("Encoste O Dedo"));
  delay(500);
 
  while (fingerprintSensor.getImage() != FINGERPRINT_OK);//Espera até pegar uma imagem válida da digital

 
  if (fingerprintSensor.image2Tz() != FINGERPRINT_OK) //Converte a imagem para o padrão que será utilizado para verificar com o banco de digitais
  {
    
    Serial.println(F("Erro image2Tz"));//Se chegou aqui deu erro, então abortamos os próximos passos
    lcd.clear();
    lcd.setCursor(0,4);
    lcd.print(F("Erro IMg"));
    lcd.delay(500);
    return;
  }

 
  if (fingerprintSensor.fingerFastSearch() != FINGERPRINT_OK) //Procura por este padrão no banco de digitais
  {
   
    Serial.println(F("Digital não encontrada")); //Se chegou aqui significa que a digital não foi encontrada
    lcd.clear();
    lcd.setCursor(0,1);
    lcd.print(F("Nao cadastrada"));
    lcd.delay(500);
    return;
  }

  //Se chegou aqui a digital foi encontrada
  
  Serial.print(F("Digital encontrada com confiança de ")); //Quanto mais alta a confiança melhor
  lcd.clear();
  lcd.setCursor(0,1);
  lcd.print(F("D. Encontrada"));
  lcd.delay(500);
  Serial.print(fingerprintSensor.confidence);//Mostramos a posição onde a digital estava salva e a confiança
  Serial.print(F(" na posição "));
  Serial.println(fingerprintSensor.fingerID);
  lcd.clear();
  lcd.setCursor(0,1);
  lcd.print(fingerprintSensor.fingerID);
  lcd.delay(500);

    dataehora();
}
void dataehora()
{
  int id = 0;
  int confidence = 0 ;
  
  float t = dht.readTemperature();
  float h = dht.readHumidity();

  id =  fingerprintSensor.confidence;
  confidence =fingerprintSensor.fingerID;
  
  //Le as informacoes do RTC
  DateTime now = rtc.now();

  Serial.print(now.hour() < 10 ? "0" : "");
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute() < 10 ? "0" : "");
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second() < 10 ? "0" : "");
  Serial.print(now.second(), DEC);
  Serial.print(" ");
  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(" (");
  Serial.print("Temperatura: ");
  Serial.println(t);
  
  //Gravacao do cartao
  //Abre arquivo no SD para gravacao
  file = SD.open("dados.csv", FILE_WRITE);
  //Grava os dados no cartao SD
  file.print(now.hour() < 10 ? "0" : "");
  file.print(now.hour(), DEC);
  file.print(':');
  file.print(now.minute() < 10 ? "0" : "");
  file.print(now.minute(), DEC);
  file.print(':');
  file.print(now.second() < 10 ? "0" : "");
  file.print(now.second(), DEC);
  file.print(",");
  file.print(now.year(), DEC);
  file.print('/');
  file.print(now.month(), DEC);
  file.print('/');
  file.print(now.day(), DEC);
  file.print(" (");
  file.print(t);
  file.print('/');
  file.println(fingerprintSensor.fingerID);
  file.print('/');
  file.println(fingerprintSensor.confidence);
  file.print('/');
  file.print(id);
  file.print('/');
  file.print(confidence);
  file.close();  //Fecha arquivo
  //Delay ate a proxima leitura
  delay(1000);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("-Salvo com Exito-");
  
  }
