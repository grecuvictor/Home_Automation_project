
#define Timer_Front_Door    500    //Time untill the front door remains Open (ms)
#define Timer_Buzzer        250    //Time for buzzer ON

byte dataBlock_Access_Denied[]    = {
        0x00, 0x00, 0x00, 0x00, //  0, 0, 0, 0,
        0x00, 0x00, 0x00, 0x00, //  0, 0, 0, 0,
        0x00, 0x00, 0x00, 0x00, //  0, 0, 0, 0,
        0x00, 0x00, 0x00, 0x00  //  0, 0, 0, 0
    };                          //Door access Denied ( Block 6 )
byte dataBlock_Access_Granted[]    = {
        0xff, 0xff, 0xff, 0xff, //  255, 255, 255, 255,
        0xff, 0xff, 0xff, 0xff, //  255, 255, 255, 255,
        0x00, 0x00, 0x00, 0x00, //  0,   0,   0,   0,
        0x00, 0x00, 0x00, 0x00  //  0,   0,   0,   0
    };                          //Door access Granted ( Block 6 )
byte dataBlock_Access_Master[]    = {
        0xff, 0xff, 0xff, 0xff, //  255, 255, 255, 255,
        0xff, 0xff, 0xff, 0xff, //  255, 255, 255, 255,
        0xff, 0xff, 0xff, 0xff, //  255, 255, 255, 255,
        0x00, 0x00, 0x00, 0x00  //  0,   0,   0,   0
    };                          //Master info ( Block 5 )
    
/*--------------------------------------------------------------------------------------------
 * Helper routine to dump a byte array as hex values to Serial.
 -------------------------------------------------------------------------------------------*/
 
void Show_info_RFID(MFRC522 Card_tmp)
{
  Serial.print(F("Card UID:"));
    dump_byte_array(Card_tmp.uid.uidByte, Card_tmp.uid.size);      
    Serial.println();
    Serial.print(F("PICC type: "));
    byte piccType = Card_tmp.PICC_GetType(mfrc522.uid.sak);
    Serial.println(Card_tmp.PICC_GetTypeName(piccType));
    if (    piccType != MFRC522::PICC_TYPE_MIFARE_MINI
        &&  piccType != MFRC522::PICC_TYPE_MIFARE_1K
        &&  piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
        Serial.println(F("Only MIFARE Classic cards are accepted."));
        return;
    }
    char *master[] = { "34","DE","43","CF"};
    if (Card_tmp.uid.uidByte[0] == *master[0])
    Serial.println("MASTERRRRRRR");
}

/*--------------------------------------------------------------------------------------------
 * Dump Info to Serial Console
 -------------------------------------------------------------------------------------------*/
 
void dump_byte_array(byte *buffer, byte bufferSize) {
    for (byte i = 0; i < bufferSize; i++) {
        Serial.print(buffer[i] < 0x10 ? " 0" : " ");
        Serial.print(buffer[i], HEX);
    }
}

/*--------------------------------------------------------------------------------------------
 *Card Authenticate status
  -------------------------------------------------------------------------------------------*/

void Status_check(byte status_received)
{
  if (status_received != MFRC522::STATUS_OK) {
        Serial.print(F("PCD_Authenticate() failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status_received));
        return;
    }
    else 
    Serial.println(F("PCD_Authenticate() finished. UID validation... "));
}

/*--------------------------------------------------------------------------------------------
 * Read a specific block(blockNumber) from RFID and save the data in the arrayAddress variable
 -------------------------------------------------------------------------------------------*/
 
int readBlock(int blockNumber, byte arrayAddress[]) 
{     
  byte buffersize = 18; 
  byte status = mfrc522.MIFARE_Read(blockNumber, arrayAddress, &buffersize);
  byte is_true = 1;                   //Boolean variable, -> 0 if one bit != 
  
  if (status != MFRC522::STATUS_OK) {
          Serial.print("Failed reading");
          Serial.println(mfrc522.GetStatusCodeName(status));
          return 1;                   //Error
  }
  for (byte i=0; i<15; i++)
  {
     if(arrayAddress[i] != dataBlock_Access_Master[i])
          is_true = 0;                 //Verify if Master card info 
  }
  Serial.println("Success reading");
  if (is_true)
      return 2;                       // Status 2 if Master Card scanned
  return 0;                          //status?0:Succes;1:Error
}

/*--------------------------------------------------------------------------------------------
 * Verify if the card has access at entrance
 -------------------------------------------------------------------------------------------*/
 
int Has_Access(int blockNumber, byte arrayAddress[]) 
{     
  byte buffersize = 18; 
  byte status = mfrc522.MIFARE_Read(blockNumber, arrayAddress, &buffersize);
  byte is_true = 1;                   //Boolean variable, -> 0 if one bit != 
  
  if (status != MFRC522::STATUS_OK) {
          Serial.print("Failed reading");
          Serial.println(mfrc522.GetStatusCodeName(status));
          return 1;                   //Error Reading 
  }
  for (byte i=0; i<15; i++)
  {
     if(arrayAddress[i] != dataBlock_Access_Granted[i])
          is_true = 0;                 //Verify if the card has access 
  }
  Serial.println("Success reading");
  if (is_true)
      return 2;                       // Status 2 if Card has access
  return 0;                          //  0 if card OK but has no access
}

/*--------------------------------------------------------------------------------------------
 * Open door / Switch to GREEN Led
 -------------------------------------------------------------------------------------------*/

void Open_Door(byte level)
{
  if(level == 0)
  {
  digitalWrite(Door_PIN,     HIGH); //Open Door
  digitalWrite(G_select_PIN, HIGH);
  digitalWrite(R_select_PIN, LOW);
  for (byte i = 0; i < 4; i++)
  {
    Buzzer_beep();
    delay(Timer_Front_Door);
  }
  digitalWrite(G_select_PIN, LOW);
  digitalWrite(Door_PIN,     LOW);  //Close Door
  digitalWrite(R_select_PIN, HIGH); //Green LED ON for Timer_Front_Door seconds (User access)
  }
  else
  {
  digitalWrite(B_select_PIN, HIGH);
  digitalWrite(Door_PIN,     HIGH); //Open Door
  digitalWrite(R_select_PIN, LOW);
  for (byte i = 0; i < 4; i++)
  {
    Buzzer_beep();
    delay(Timer_Front_Door);
  }
  digitalWrite(B_select_PIN, LOW);
  digitalWrite(Door_PIN,     LOW);  //Close Door
  digitalWrite(R_select_PIN, HIGH); //Blue LED ON for Timer_Front_Door seconds (Master access)
  }
}

/*--------------------------------------------------------------------------------------------
 * Write in a specific block(blockNumber)
 -------------------------------------------------------------------------------------------*/
 
int writeBlock(int blockNumber, byte arrayAddress[]) 
{
  byte status = mfrc522.MIFARE_Write(blockNumber, arrayAddress, 16);//valueBlockA is the block number, MIFARE_Write(block number (0-15), byte array containing 16 values, number of bytes in block (=16))
  //status = mfrc522.MIFARE_Write(9, value1Block, 16);
  if (status != MFRC522::STATUS_OK) {
           Serial.print("MIFARE_Write() failed: ");
           Serial.println(mfrc522.GetStatusCodeName(status));
           return 4;//return "4" as error message
  }
  Serial.println("block was written");
}

/*--------------------------------------------------------------------------------------------
 * Write info on RFID depending the current data in block 6
 -------------------------------------------------------------------------------------------*/
 
void Card_write_info(byte Access_Status)
{
  if(Access_Status == 2)
  {
    writeBlock(blockAddr, dataBlock_Access_Denied);               //Write Not Access info in block 6 (Access Information Block)
    mfrc522.PICC_DumpMifareClassicSectorToSerial(&(mfrc522.uid), &key, sector);
    Serial.println("Acc Denied write DNE");
  }
  else if(Access_Status == 0)
         {
            writeBlock(blockAddr, dataBlock_Access_Granted);      //Write Access info in block 6 (Access Information Block)
            mfrc522.PICC_DumpMifareClassicSectorToSerial(&(mfrc522.uid), &key, sector);
            Serial.println("Acc GRANTED write DNE + Open Door");
            Open_Door(Access_Status);
         }
         else
            Serial.println("Error writing. Please Scan Master Card again...");
  for(byte i = 0; i<2; i++)
    Buzzer_beep(); 
}

/*--------------------------------------------------------------------------------------------
 * Keeps Buzzer ON for 300 ms
 -------------------------------------------------------------------------------------------*/
 
void Buzzer_beep(void)
{
    digitalWrite(Buzz_PIN, HIGH);
    delay(Timer_Buzzer);
    digitalWrite(Buzz_PIN, LOW);
}

