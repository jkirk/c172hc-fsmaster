14_Checksum_SerIn_SOT_EOT_LCD_TOut_V2.ino

Simulated received bytes as in Darshaka's sample
Real Serial Input
HEX data on LCD with TimeOut 
Preparation for PC Demo as Serial Monitor cannot be used
Added Result sum of bytes upto CheckSum
Calculated own Checksun One's complement of Sum
Received Checksum extracted for display.
Displayed in 2nd line LCD after moving data string to first line
Final check should be SUM plus Received Checksum should be 00
Shown as OK in LCD. If error XX and flash backlight
Serial Output still active for DEBUG

Tested via RealTerm and Darshaka's test program
Also with RealTerm 
Included following:
1. verification of multiple STX
2. display Number of bytes received until ETX

See screenshot RealTerm
Simulation multiple STX manually and correct STX via binary file shown 

 
27.07.2022