# README

First of all, please have a look at the following links
 - Eagle Mode original [project page](https://sourceforge.net/projects/eaglemode/) (ℹ hosted from sourceforge.net),
 - [Olivier Hamann profile page (the project author, maintainer)](https://libregamewiki.org/Oliver_Hamann)
 - [This soft-fork tracking upsteam release](https://github.com/Osndok/eaglemode)
   maintained by [Robert Hailey](https://github.com/Osndok)

Now that all attribution and credits is clear,
we can move on to the next part which is only
about making it a little bit easier for /me
to install Eagle Mode for a linux distribution,
namely Ubuntu, and only the latest LTS release at this moment.

## Installation

The following commands appeared to be working
for Ubuntu when running the following command 
`lsb_release -a` outputs

```shell
lsb_release -a
No LSB modules are available.
Distributor ID: Ubuntu
Description:    Ubuntu 22.04.1 LTS
Release:        22.04
Codename:       jammy
```

```shell
git clone https://github.com/Osndok/eaglemode
cd eaglemode || exit
. install.sh && install-eaglemode
```
