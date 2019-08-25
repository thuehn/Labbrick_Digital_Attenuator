## Con-test - automated testbed for attenuation tests
### Installation
#### Requirements
##### Controller

` sudo ` - optional, can be run as root

` getopt `

` sshpass ` - optional, provide key files in the conf file

installed or run con-test as root

If you don't start the tool with sudo or as root con-test will ask you for your
password during the runs to get privileges.

###### Client/Server

` iperf3 `

` nohup `

` timeout `

` tcpdump `

### Usage
#### Command line options
`-c/--config - set path for config file `

`-h/--help - show usage options`

`-o/--output - set path for output log files `

`-v/--verbose - print additional information `

`-V/--version - show con-test version `

#### Configuration file options
##### Server options
` SERVER_IP `- ip address of machine running iperf as server

`SERVER_USER `- user name for remote login

`SERVER_WIFI_IP`- ip of machines wifi interface

`SERVER_PASSWORD `- optional, password for remote login

`SERVER_CERTIFICATE `- optional, path to identity file for remote login

You need to provide either identity file or password

##### Client options
`CLIENT_IP `- ip address of machine running iperf as client

`CLIENT_USER`- user name for remote login

`CLIENT_PASSWORD`- optinal, password for remote login

`CLIENT_CERTIFICATE`- optional, path to identity file for remote login

You need to provide either identity file or password

##### General configuration options

`NO_RUNS`- Number of test to run

`ATTENUATOR_PARAMS`- options for attenuation program provide in form
=('options test 1' 'options test 2' ... )

`IPERF_PARAM_SERVER`- configuration options for iperf server. =('options test 1'
 'options test 2' ... )

`IPERF_PARAM_CLIENT`- configuration options for iperf client. =('options test 1'
 'options test 2' ... )

 `UPDATE_PACKAGE`- path to package for updates. =('path test 1' 'path test 2' ... )


### Evaluation Scripts
#### Command line options
`-c/--config - set path for config file `

`-h/--help - show usage options`

`-i/--input - set path to input file to evaluate`

`-o/--output - set path for output files `

`-v/--verbose - print additional information `

`-V/--version - show con-test version `
