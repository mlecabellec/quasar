

eabi /dev/asdc1 0xFF 0x1234
labi /dev/asdc1 0xFF 

eabi /dev/asdc1 0x40000 1         

sleep 1
          
eabi /dev/asdc1 0x40 0xFFFF
eabi /dev/asdc1 0x40000 0        

sleep 1

labi /dev/asdc1 0x40
labi /dev/asdc1 0x3C
eabi /dev/asdc1 0x40 0
eabi /dev/asdc1 0x3A 0x0D
labi /dev/asdc1 0x40
labi /dev/asdc1 0x40
labi /dev/asdc1 0xEA
          
labi /dev/asdc1 0x43          

