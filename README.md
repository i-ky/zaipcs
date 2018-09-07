# zaipcs [![Build Status](https://www.travis-ci.com/i-ky/zaipcs.svg?branch=master)](https://www.travis-ci.com/i-ky/zaipcs)

[Zabbix](http://www.zabbix.com)
[loadable module](https://www.zabbix.com/documentation/3.4/manual/config/items/loadablemodules)
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
[low level discovery](https://www.zabbix.com/documentation/3.4/manual/discovery/low_level_discovery).

## compile

[Download](http://www.zabbix.com/download)
Zabbix source or check it out from
[SVN repository](https://www.zabbix.org/websvn/wsvn/zabbix.com?):
`svn checkout svn://svn.zabbix.com/branches/3.4`

> Any version higher than 2.2 (when loadable module support was added) will do. But you need to compile module using sources of the version you will be using it with!

Place module source folder in Zabbix source tree `src/modules/` alongside `dummy`.

Run `make` to build, it should produce `zaipcs.so`.

## install

Copy `zaipcs.so` it to a desired location, set up necessary permissions.

## configure

Set `LoadModulePath` and `LoadModule` parameters in Zabbix
[agent](https://www.zabbix.com/documentation/3.4/manual/appendix/config/zabbix_agentd) /
[proxy](https://www.zabbix.com/documentation/3.4/manual/appendix/config/zabbix_proxy) /
[server](https://www.zabbix.com/documentation/3.4/manual/appendix/config/zabbix_server) configuration file.

Restart Zabbix agent / proxy / server.

## use

Configure checks:
* use item type
[_Simple check_](https://www.zabbix.com/documentation/3.4/manual/config/items/itemtypes/simple_checks)
if the module is loaded by server or proxy;
* use item type
[_Zabbix agent_ or _Zabbix agent (active)_](https://www.zabbix.com/documentation/3.4/manual/config/items/itemtypes/zabbix_agent)
if the module is loaded by agent.

### supported item keys

#### __`ipcs-shmem-details[id,mode,option]`__

This key mimics `ipcs --shmems --id ...`. __`id`__ is a shared memory segment identifier. Possible combinations of __`mode`__ and __`option`__ with their resulting value are given in the table below.

`mode`        | `option`                             | result
--------------|--------------------------------------|--------------------------------------
`owner`       | `uid` <br> `gid`                     | owner's user id <br> owner's group id
`creator`     | `uid` <br> `gid`                     | creator's user id <br> creator's group id
`status`      | `dest` <br> `locked`                 | 1 if marked for destruction, 0 otherwise <br> 1 if locked, 0 otherwise
`permissions` |                                      | access permissions (three octal digits)
`size`        |                                      | allocated bytes
`time`        | `attach` <br> `detach` <br> `change` | timestamp of the last attachment, 0 if not set <br> timestamp of the last detachment, 0 if not set <br> timestamp of the last change, 0 if not set
`pid`         | `creator` <br> `last`                | creator process id <br> last attached or detached process id
`nattch`      |                                      | number of currect attaches

#### __`ipcs-queue-details[id,mode,option]`__

This key mimics `ipcs --queues --id ...`. __`id`__ is a message queue identifier. Possible combinations of __`mode`__ and __`option`__ with their resulting value are given in the table below.

`mode`        | `option`                            | result
--------------|-------------------------------------|--------------------------------------
`owner`       | `uid` <br> `gid`                    | owner's user id <br> owner's group id
`creator`     | `uid` <br> `gid`                    | creator's user id <br> creator's group id
`permissions` |                                     | access permissions (three octal digits)
`time`        | `send` <br> `receive` <br> `change` | timestamp of the last send operation, 0 if not set <br> timestamp of the last receive operation, 0 if not set <br> timestamp of the last change, 0 if not set
`messages`    |                                     | current number of messages in queue
`size`        |                                     | maximum number of bytes allowed in queue
`pid`         | `send` <br> `receive`               | id of the process that performed the last send operation <br> id of the process that performed the last receive operation

#### __`ipcs-semaphore-details[id,mode,option]`__

This key mimics `ipcs --semaphores --id ...`. __`id`__ is a semaphore array identifier. Possible combinations of __`mode`__ and __`option`__ with their resulting value are given in the table below.

`mode`        | `option`                    | result
--------------|-----------------------------|--------------------------------------
`owner`       | `uid` <br> `gid`            | owner's user id <br> owner's group id
`creator`     | `uid` <br> `gid`            | creator's user id <br> creator's group id
`permissions` |                             | access permissions (three octal digits)
`time`        | `semop` <br> `change`       | timestamp of the last semaphore operation, 0 if not set <br> timestamp of the last change, 0 if not set
`nsems`       |                             | number of semaphores in set
`ncount`      | `sum` <br> `max` <br> `idx` | total number of processes waiting for an increase of semaphore values <br> maximum number of processes waiting for an increase of the semaphore value <br> index of the semaphore with the most processes waiting for semaphore value to increase
`zcount`      | `sum` <br> `max` <br> `idx` | total number of processes waiting for semaphore values to become 0 <br> maximum number of processes waiting for the semaphore value to become 0 <br> index of the semaphore with the most processes waiting for semaphore value to become 0

### supported discovery rules

> These keys use non-standard Linux-specific calls and may not be universally supported

#### __`ipcs-shmem-discovery`__ or __`ipcs-shmem-discovery[...]`__

#### __`ipcs-queue-discovery`__ or __`ipcs-queue-discovery[...]`__

#### __`ipcs-semaphore-discovery`__ or __`ipcs-semaphore-discovery[...]`__

These keys can accept parameters, but ignore them. This way you can set up several discoveries on the same template or host without many troubles. Low level discovery provides the following macros:

`{#MACRO}` | value
-----------|----------------------------------------------------
`{#KEY}`   | key supplied to `shmget(2)`/`msgget(2)`/`semget(2)`
`{#ID}`    | resource identifier (can be supplied as __`id`__ to __`ipcs-...-details[...]`__)
`{#OWNER}` | owner's user id
`{#PERMS}` | access permissions (three octal digits)

### error messages

* _"invalid resource identifier"_ - failed to read IPC resource id, it should be a nonnegative integer number

* _"incorrect number of parameters"_ - misconfiguration, either too few or too many parameters (depends on `mode`)

* _"invalid 'mode' parameter"_ - misconfiguration, `mode` parameter is not recognized

* _"invalid 'option' parameter"_ - misconfiguration, please refer to the tables of supported `option` and `mode` combinations

* _"not supported on this platform"_ - author does not know yet how to implement feature on your platform, you can assist by providing documentation and/or testing

* various errors from system...

> Zabbix agent / proxy / server needs at least read permissions to discover and obtain details of IPC resources!

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
