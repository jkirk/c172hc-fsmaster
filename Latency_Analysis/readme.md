2023.05.16
Latency_Analysis_V1.ino
Internal File.ID: 14C
Test reliability of data transfer @9600 Baud
Check blocks received within time frame specified in waitRestart set 10 sec.
Calculates Checksum according to 2's complement of sum of bytes (excl. SOT and EOT)
Implemented here as Checksum = ((sum of data bytes) XOR 0xFF ) + 1)
Count number of blocks received.
At the end of time frame, display number of blocks received with correct and wrong Checksum caused by latency