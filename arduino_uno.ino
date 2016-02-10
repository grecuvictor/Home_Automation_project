/**
 * Ardino Uno | Entrance Module | RFID + RGB_Led + Buzzer + PIR_Sensor
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
 */

#include <SPI.h>
#include <MFRC522.h>
#include <FastIO.h>
#include <I2CIO.h>
#include <Wire.h>

#define RST_PIN         5           //  RST PIN from RFID 
#define R_G_select_PIN  6           //  Red / Green Relay pin from RGB led
#define SS_PIN          10          //  SS (SDA) Pin from RFID

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.
MFRC522::MIFARE_Key key;            //Create a MIFARE_Key struct for holding the card information

byte blockcontent[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};//all zeros. This can be used to delete a block.
byte readbackblock[18];//This array is used for reading out a block. The MIFARE_Read method requires a buffer that is at least 18 bytes to hold the 16 bytes of a block.
byte sector         = 1;          //Desired Sector for information reading
byte blockAddr      = 4;          //Block Address for acces level
byte trailerBlock   = 7;
byte dataBlock_Access[]    = {
        0xff, 0xff, 0xff, 0xff, //  255, 255, 255, 255,
        0xff, 0xff, 0xff, 0xff, //  255, 255, 255, 255,
        0xff, 0xff, 0xff, 0xff, //  255, 255, 255, 255,
        0x00, 0x00, 0x00, 0x00  //  0,   0,   0,   0
    };                    //Data Block form Acces Granted
    
void setup() { 
    Serial.begin(9600); // Initialize serial communications with the PC
    while (!Serial);    // Wait untill serial communication starts
    SPI.begin();        // Initialize SPI bus
    mfrc522.PCD_Init(); // Initialize MFRC522 card
    pinMode(R_G_select_PIN, OUTPUT);
    for (byte i = 0; i < 6; i++) {
        key.keyByte[i] = 0xFF;
    }  // using FFFFFFFFFFFFh which is the default at chip delivery from the factory
    Serial.println(F("Setup Ready: Scanning ..."));
    //digitalWrite(R_G_select_PIN, LOW);
}

void loop() {
    if ( ! mfrc522.PICC_IsNewCardPresent())
      return;                 // Look for new cards
    // Select one of the cards
    if ( ! mfrc522.PICC_ReadCardSerial())
      return;                 // Select one of the cards
    Show_info_RFID(mfrc522);  // Dump key scanned info
    
    byte status;              //Scan satus
   
    
    

         
    Serial.println(F("Key scanned"));
    status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
    Status_check(status);
  
    Serial.println(F("Current data in sector:"));
    mfrc522.PICC_DumpMifareClassicSectorToSerial(&(mfrc522.uid), &key, sector);
    Serial.println();  // Show the whole sector

    byte readbackblock[18];    //Variable use for reading a block
    readBlock(blockAddr, readbackblock);//read the block back
         Serial.print("read block: ");
         for (int j=0 ; j<16 ; j++)//print the block contents
         {
           Serial.print(readbackblock[j]);//Serial.write() transmits the ASCII numbers as human readable characters to serial monitor
           //Serial.print(dataBlock_Access[j]);
           Serial.print(" ");
         }
         Serial.println("");
    writeBlock(5, dataBlock_Access);
         
    // Halt PICC
    mfrc522.PICC_HaltA();// Stop encryption on PCD
    mfrc522.PCD_StopCrypto1();
    Serial.println();
}

