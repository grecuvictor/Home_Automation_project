/**
 * Ardino Uno | Entrance Module | RFID + RGB_Led + Buzzer + PIR_Sensor + 3 Led Band + Relay 
 * ----------------------------------------------------------------------------------------- 
 * Pin layout used:
 * -----------------------------------------------------------------------------------------
 *             MFRC522 Reader/PCD     Arduino                    
 * Signal      Pin                    Pin           
 * -----------------------------------------------------------------------------------------
 * RST/Reset   RST                    5             
 * SPI SS      SDA(SS)                10            
 * SPI MOSI    MOSI                   11 
 * SPI MISO    MISO                   12 
 * SPI SCK     SCK                    13  
 * -----------------------------------------------------------------------------------------
 * DIGITAL     R_LED                  6
 * DIGITAL     G_LED                  7
 * DIGITAL     B_LED                  4
 * DIGITAL     Buzz_PIN               8
 * DIGITAL     Door_PIN               9
 */

#include <SPI.h>
#include <MFRC522.h>
#include <FastIO.h>
#include <I2CIO.h>
#include <Wire.h>

#define RST_PIN         5              //  RST PIN from RFID 
#define R_select_PIN    6              //  Set/Reset Red pin from RGB led
#define G_select_PIN    7              //  Set/Reset Green pin from RGB led
#define B_select_PIN    4              //  Set/Reset Blue pin from RGB led
#define SS_PIN          10             //  SS (SDA) Pin from RFID
#define Buzz_PIN        8              //  Buzzer pin
#define Door_PIN        9              //  Door Pin (Open/Close)

MFRC522 mfrc522(SS_PIN, RST_PIN);        // Create MFRC522 instance.
MFRC522::MIFARE_Key key;                 //Create a MIFARE_Key struct for holding the card information

byte sector         = 1;                 //Desired Sector for information reading
byte blockMaster    = 5;                 //Master block info
byte blockAddr      = 6;                 //Block Address for acces level
byte trailerBlock   = 7;
byte Write_enable   = 0;                 //Delete / Add enter permision on regular user (Default = 0)
byte readbackblock[18];                  //Store a readed block. Requires 18 bits even if only 16 bits are stored

void setup() { 
    Serial.begin(9600);                                                                 // Initialize serial communications with the PC
    while (!Serial);                                                                    // Wait untill serial communication starts
    SPI.begin();                                                                        // Initialize SPI bus
    mfrc522.PCD_Init();                                                                 // Initialize MFRC522 card
    pinMode(R_select_PIN, OUTPUT);
    pinMode(G_select_PIN, OUTPUT);
    pinMode(B_select_PIN, OUTPUT);
    pinMode(Door_PIN,     OUTPUT);                                                      //Set Output Pins
    for (byte i = 0; i < 6; i++) {
        key.keyByte[i] = 0xFF;
    }                                                                                 // using FFFFFFFFFFFFh which is the default at chip delivery from the factory
    Serial.println(F("Setup Ready: Scanning ..."));
    digitalWrite(R_select_PIN, HIGH);                                                 //Start with RED Led (Permission denied)     
    digitalWrite(G_select_PIN, LOW); 
    digitalWrite(B_select_PIN, LOW); 
    digitalWrite(Door_PIN,     LOW);                                                  //Rest of PIN's are set LOW as default untill Arduino receives an event
}

void loop() {
    byte status;                                                                    //Scan satus
    byte readbackblock[18];                                                         //Variable use for reading a block
    byte CardAccess_Level;                                                          //Determine if it's master or not
    byte Access_Status;                                                             //Determine if User has access (block 6 info)
    
    if ( ! mfrc522.PICC_IsNewCardPresent())
      return;                                                                     // Look for new cards
    if ( ! mfrc522.PICC_ReadCardSerial())
      return;                                                                     // Select one of the cards
      
    Show_info_RFID(mfrc522);                                                     // Dump key scanned info
    Serial.println(F("Key scanned"));
    status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
    Status_check(status);                                                       //Check if card is validated
  
    Serial.println(F("Current data in sector:"));
    mfrc522.PICC_DumpMifareClassicSectorToSerial(&(mfrc522.uid), &key, sector);
    Serial.println();                                                           // Show the whole sector

    Serial.print("Secor 6 :");
    CardAccess_Level = readBlock(blockMaster, readbackblock);    
    if (CardAccess_Level == 2)                                                //read the block back
    {
      Serial.println("MASTER FOUND");
      Open_Door(CardAccess_Level);                                            //Grants access inside
      Write_enable = 1;                                                       //Next card will have the access info inverted (OK -> NOK & NOK -> OK)
    }
    else if(CardAccess_Level == 0)
          {
            Access_Status = Has_Access(blockAddr, readbackblock);
            if(Write_enable)
            {
              Card_write_info(Access_Status);                                 //Check info written in block 6(Info Block)
              Write_enable = 0;                                               //Dezactivate write option 
            }
            else if((CardAccess_Level == 0) && (Access_Status == 2))
                    Open_Door(CardAccess_Level);                              // Door opened for 2 seconds  
          }             
    mfrc522.PICC_HaltA();                                                     // Stop encryption on PCD
    mfrc522.PCD_StopCrypto1();
    Serial.println();
}

