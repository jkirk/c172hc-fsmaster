Steps when tested with RealTerm:
1. Check Port of Arduino (Example COM6)
2. Start RealTerm
3. Select "Port"
4. set Port number to Arduino Port (Example 6)
5. Click Open
6. Verify message sent by Ardunio on serial port "Checksum Test Serial Simulation"
7. Select "Send" in RealTerm
8. Click "Send File" to send pre-programmed Hex String with correcr format:
SOT, 8 Data Bytes, CS, EOT (see ino how CS is calculated)
Screenshot: 03_RealTerm_Send_Bin-File.jpg

Arduino response on Serial and LCD.
LCD Screenshots: 
LCD01_DataReceived.jpg
LCD02_Data_S_C_R_Evaluation.jpg

Arduino response when correct Checksum 0x42 was sent manually:
8  HEX values received...                                               
0x02 0x20 0x00 0x00 0x1D 0x80 0x40 0x03                                                                                                                                                                                                 
Received data bytes: 0x02  0x20  0x00  0x00  0x1D  0x80  0x42  0x03        
Sum of received data bytes                       = 0xBE                       
Calculated checksum                              = 0x42                      
Received checksum byte                           = 0x42                      
Sum of received data bytes and received checksum = 0x00
Screenshot: 04_RealTerm_Send_manual_as Hex_OK.jpg

9. To send Hex Strin Manually enter in "Send" click "as Hex"
Artduino response when correct Checksum 0x42 was sent manually same as sending bin-file

************************************************************************

Arduino response when wrong Checksum 0x40 was sent manually:
8  HEX values received...                                               
0x02 0x20 0x00 0x00 0x1D 0x80 0x40 0x03                                                                                                                                                                                                 
Received data bytes: 0x02  0x20  0x00  0x00  0x1D  0x80  0x40  0x03        
Sum of received data bytes                       = 0xBE                       
Calculated checksum                              = 0x42                      
Received checksum byte                           = 0x40                      
Sum of received data bytes and received checksum = 0xFE
Screenshot: 05_RealTerm_Send_manual_as Hex_CS Error.jpg
   