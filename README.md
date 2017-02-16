# zaipcs

Zabbix loadable module for monitoring IPC facilities

## summary

This module tries to mimic 'ipcs' utility. It can read various information about shared memory segments, message queues and semaphore sets given IPC object identifier. On Linux it additionally supports low level discovery.

## compile

Place this folder in Zabbix source tree 'src/modules/' alongside 'dummy'.

Run 'make' to build, it should produce 'zaipcs.so'.

## install

Copy 'zaipcs.so' it to a desired location, set up necessary permissions.

## configure

Set 'LoadModulePath' and 'LoadModule' parameters in Zabbix agent/proxy/server configuration file.

Restart Zabbix agent/proxy/server.

## use

Configure checks: use item type 'Simple check' if the module is loaded by server or proxy;
use item type 'Zabbix agent' or 'Zabbix agent (active)' if the module is loaded by agent.

### supported keys

ipcs-shmem-discovery

ipcs-shmem-details[id,mode,option]

### error messages

"invalid 'mode' parameter"

"invalid 'option' parameter"

"invalid resource identifier"

"incorrect number of parameters"

"not supported on this platform"

various errors from system...

## templates

Coming later...

## contribute

Author is too lazy and too arrogant to test the module properly. Therefore the best thing you can do to help is to compile and test the module in your environment. Feedback, both positive and negative, will be highly appreciated.

### compilation and installation

Module comes without ./configure script. Makefile is very primitive. Installation and configuration process is 100% manual. Luckily there is only one file and no dependencies. However, it should compile easily on on any Unix-like. Mind that author earns his money being C developer, so if you encounter any problems or simply disagree with his opinion, feel free to provide information about compilation errors, warnings and other issues on your platform.

### feature support

Development was done on Linux, but is based on standard System V IPC functionality. Only low level discovery and a couple of 'mode'/'option' combinations depend on non-standard Linux features. If you see "not supported on this platform" for the feature you desperately need, if the module is working incorrectly or if it is crashing (unlikely but not impossible) - please provide the output of 'man shmctl', 'man msgctl' and 'man semctl' for investigation.

### usability

One more thing you need to know about the author - he does not use Zabbix more than he is obliged to, he has very little monitoring experience and he is an absolute zero in system administration. If you find module configuration too difficult, supported item keys useless or default parameter value choice illogical, feel free to throw your opinion at him.
