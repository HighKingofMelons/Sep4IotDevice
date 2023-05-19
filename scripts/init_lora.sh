#!/bin/bash
chmod +rw ../include/LoRaWAN.h
truncate -s 0 ../include/LoRaWAN.h
printf "#pragma once\n#define LORA_appEUI \"%s\"\n#define LORA_appKEY \"%s\"\n" $1 $2 > ../include/LoRaWAN.h
exit