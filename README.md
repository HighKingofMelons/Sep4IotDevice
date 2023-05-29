# Sep4IotDevice

## Requirments
This project is developed only with support for Arduino Mega2560 programmed using an Atmel-ICE
Additionally a custom header is required, see [ihavn/IoT_Semester_project](https://github.com/ihavn/IoT_Semester_project)

## Cloning Project
To clone the project, as well as submodules, run the following command:

`git clone https://github.com/HighKingofMelons/Sep4IotDevice --recurse-submodules`

## Running using PlatformIO
* Install `PlatformIO` VSCode extension

* Open project folder

* Select appropriate env according to the computer of which is used to program the device.
E.g. if the programmer (Atmel-ICE) is plugged into a windows machine, use windows. By default Windows is selected.

* Run the script `scripts/init_lora.sh` from inside its directory, and configure the tokens in the generated ´include/LoRaWAN.h´.

* Press Upload/Remote Upload

## Docs
An api reference is generated using doxygen and deployed to:

https://highkingofmelons.github.io/Sep4IotDevice/
