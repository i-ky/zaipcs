# zaipcs

[Zabbix](http://www.zabbix.com)
[loadable module](https://www.zabbix.com/documentation/3.2/manual/config/items/loadablemodules)
for monitoring
[System V](https://en.wikipedia.org/wiki/UNIX_System_V)
[IPC](https://en.wikipedia.org/wiki/Inter-process_communication)
facilities

## summary

This module tries to mimic
[`ipcs`](http://pubs.opengroup.org/onlinepubs/9699919799/utilities/ipcs.html) utility.
It can read various information about
[shared memory segments](https://en.wikipedia.org/wiki/Shared_memory#Support_on_Unix-like_systems),
[message queues](https://en.wikipedia.org/wiki/Message_queue#Implementation_in_UNIX)
and
[semaphore](https://en.wikipedia.org/wiki/Semaphore_(programming)) sets
given IPC object identifier.
On Linux it additionally supports
[low level discovery](https://www.zabbix.com/documentation/3.2/manual/discovery/low_level_discovery).

## compile

Place this folder in Zabbix source tree `src/modules/` alongside `dummy`.

Run `make` to build, it should produce `zaipcs.so`.

## install

Copy `zaipcs.so` it to a desired location, set up necessary permissions.

## configure

Set `LoadModulePath` and `LoadModule` parameters in Zabbix
[agent](https://www.zabbix.com/documentation/3.2/manual/appendix/config/zabbix_agentd)/
[proxy](https://www.zabbix.com/documentation/3.2/manual/appendix/config/zabbix_proxy)/
[server](https://www.zabbix.com/documentation/3.2/manual/appendix/config/zabbix_server) configuration file.

Restart Zabbix agent/proxy/server.

## use

Configure checks: use item type
[_Simple check_](https://www.zabbix.com/documentation/3.2/manual/config/items/itemtypes/simple_checks)
if the module is loaded by server or proxy;
use item type
[_Zabbix agent_ or _Zabbix agent (active)_](https://www.zabbix.com/documentation/3.2/manual/config/items/itemtypes/zabbix_agent)
if the module is loaded by agent.

### supported keys

`ipcs-shmem-discovery`

`ipcs-shmem-details[id,mode,option]`

### error messages

* _"invalid resource identifier"_ - failed to read IPC resource id, it should be a nonnegative integer number

* _"incorrect number of parameters"_ - misconfiguration

* _"invalid 'mode' parameter"_ - misconfiguration

* _"invalid 'option' parameter"_ - misconfiguration

* _"not supported on this platform"_ - author does not know yet how to implement feature on your platform, you can assist by providing documentation and/or testing

* various errors from system...

## templates

Coming later...

## contribute

Author is too lazy and too arrogant to test the module properly. Therefore the best thing you can do to help is to compile and test the module in your environment. Feedback, both positive and negative, will be highly appreciated.

### compilation and installation

Module comes without `configure` script. `Makefile` is very primitive. Installation and configuration process is 100% manual. Luckily there is only one file and no dependencies, therefore, it should compile *easily* on on any Unix-like. Mind that author earns his money being C developer and finds compiling really easy. If you encounter any problems or simply disagree with his opinion, feel free to provide information about compilation errors, warnings and other issues on your platform.

### feature support

Development was done on Linux, but is based on standard System V IPC functionality. Only low level discovery and a couple of `mode`/`option` combinations depend on non-standard Linux features. If you see _"not supported on this platform"_ for the feature you desperately need, if the module is working incorrectly or if it is crashing (unlikely but not impossible) - please provide the output of `man shmctl`, `man msgctl`, `man semctl` and other relevant information for investigation.

### usability

One more thing you need to know about the author - he does not use Zabbix more than he is obliged to, he has very little monitoring experience and he is an absolute zero in system administration. If you find module configuration too difficult, supported item keys useless or default parameter value choice illogical, feel free to throw your opinion at him.
