/**
 * Helper routine to dump a byte array as hex values to Serial.
 */
 
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

/*
 * Dump Info to Serial Console
 */
 
void dump_byte_array(byte *buffer, byte bufferSize) {
    for (byte i = 0; i < bufferSize; i++) {
        Serial.print(buffer[i] < 0x10 ? " 0" : " ");
        Serial.print(buffer[i], HEX);
    }
}

/*
 *Card Authenticate status
*/

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

/*/
 * Read a specific block(blockNumber) from RFID and save the data in the arrayAddress variable
 */
 
int readBlock(int blockNumber, byte arrayAddress[]) 
{     
  byte buffersize = 18; 
  byte status = mfrc522.MIFARE_Read(blockNumber, arrayAddress, &buffersize);
  if (status != MFRC522::STATUS_OK) {
          Serial.print("Failed reading");
          Serial.println(mfrc522.GetStatusCodeName(status));
          return 1;                   //Error
  }
  Serial.println("Success reading");
  return 0;                          //status?0:Succes;1:Error
}

/*/
 * Write in a specific block(blockNumber)
 */
 
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

