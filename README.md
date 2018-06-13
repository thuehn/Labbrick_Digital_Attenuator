# Labbrick_Digital_Attenuator
Control Tool to use the digital attenuator from Labbrick in your WiFi experiments under Linux.

This is a small tool to use the LabBrick Digital Atenuator from Vaunix under Linux. It is tested and validated with the USB LDA-602 attenuator (http://vaunix.com/products/digital-attenuators/overview/) under Ubuntu 14.04.

![alt tag](https://cloud.githubusercontent.com/assets/1880886/9039179/033e9f86-39fa-11e5-869c-4fd7ee60424e.jpg)

As the Code of Vaunix is not open source it will not be included in this
repository so that everyone who wishes to use this program needs to download
their source and include it into the folder in order to build the tool with the provided makefile.

A typical experimental setup, where the digital anttenuator is connecto to a Laptop via USB looks like:

![alt tag](https://cloud.githubusercontent.com/assets/1880886/9039288/a4e8f3c2-39fa-11e5-8fd9-68f4c43a9418.jpg)

## How to compile ?

1. clone or download this resository to your local linux machine
2. request the Linux Library from Vaunix Lab brick via Email
3. copy the files: "LDAhid.h LDAhid.c" from Lab Bricks SDK into the src folder
5. ensure that you have the libusb-dev package installed in your system
6. build the tool via "make all"
7. [optional] install the tool and its man page with "make install"
8. use the compiled "attenuator_lab_brick" tool to instruct the digital attenuator in your experiments

## How to use our tool ?

1. start our tool with  "sudo attenuator_lab_brick -h" to get a list of supported commands
2. either you set the attenuation level directly or for more complex attenuation patters via a csv file
3. You can get the attenuator frequency resolution with "sudo attenuator_lab_brck -i". We do support 0.25dB steps

Additional information about each command can be found in the man page

## Example usage with a csv file
For a better user experience and understanding, some .csv files are provided in the examples folder.
The provided files are in the format: first row = step time (to be specified as seconds, milliseconds or microseconds in command execution), second row = attenuation in dB.

To read the attenuation.csv file continiously with milliseconds step time, you can use:
```
"sudo attenuator_lab_brick ms -r -f attenuation.csv -l att_log.txt > /dev/null"
```

The min_max_att.csv in the examples folder will set attenuation to 0dB followed by 63dB two times.
```
"sudo attenuator_lab_brick s -q -f min_max_att.csv -l att_log.txt"
```

The 2_sided_ramp.csv generates an attenuation starting from 0dB increasing to 63dB in 1dB steps and vice versa.
```
"sudo attenuator_lab_brick s -f 2_sided_ramp.csv"
```

## Example usage with a generated sawtooth signal

To create a sawtooth signal starting at 0dB increasing in 2dB steps every 50 microseconds and repeat it eight times, you can use:
```
"sudo attenuator_lab_brick -ramp -start 0 -end 60 -step 2 -t 50 us -rr 8"
```
For a more enhanced usage example of our tool within your wireless experiments look at the "run-experiment.sh" shell script.

## More (specific) usage examples
Set 40dB attenuation to single device (automatically detected) for 20 seconds

"sudo attenuator_lab_brick s -a 40 -t 20"

Set 40dB attenuation to specific device (detected by serial number) for 20 milliseconds

"sudo attenuator_lab_brick ms -a 40 -t 20 -n 12655"

Create a sawtooth signal starting at 0dB increasing in 2dB steps every 50 microseconds and repeat it 8 times

"sudo attenuator_lab_brick -ramp -start 0 -end 60 -step 2 -t 50 us -rr 8 -n 12655"

Set attenuation to specific device (detected by serial number) using csv file

"sudo attenuator_lab_brick s -n 12655 -f test1.csv"

Multi-device handling (automatically detected)
"sudo attenuator_lab_brick -md test1.csv test2.csv test3.csv"

Multi-device handling (detected by serial numbers)
"sudo attenuator_lab_brick -mds 12655.csv 12656.csv 10314.csv"

## Notes
Calling application with -t 0 will not reset attenuation to 0

For multi-device support, -md (or -mds) option should be used without any other options

CSV file format:
"step time (mandatory)","attenuation in dB (mandatory)","time unit [s|ms|us](optional)"
e.g.:
2,5
2,5,ms

Using -mds option, filename should be formatted in the following way:
"serial number" + .csv
e.g:
12655.csv
