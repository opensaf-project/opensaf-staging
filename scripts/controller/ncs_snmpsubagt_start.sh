#!/bin/bash
#
#           -*- OpenSAF  -*-
# 
# (C) Copyright 2008 The OpenSAF Foundation 
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
# or FITNESS FOR A PARTICULAR PURPOSE. This file and program are licensed
# under the GNU Lesser General Public License Version 2.1, February 1999.
# The complete license can be accessed from the following location:
# http://opensource.org/licenses/lgpl-license.php 
# See the Copying file included with the OpenSAF distribution for full
# licensing terms.
# 
# Author(s): Emerson Network Power
#


XTERM=""
PIDPATH=/var/run
PIDFILE=ncs_snmp_subagt.pid

echo "Executing SUBAGENT init-script..."
###  The following environment variables are useful when running in
###  in SIMULATION mode, where multiple NODEs are simulated on the
###  on the same linux Desktop
if [ ":$NCS_STDOUTS_PATH" == ":" ]
then
    export NCS_STDOUTS_PATH=/var/opt/opensaf/stdouts
fi
echo "NCS_STDOUTS_PATH=$NCS_STDOUTS_PATH"

echo $SA_AMF_COMPONENT_NAME

#remove the existing /etc/opt/opensaf/ncs_subagent_comp_name.txt
rm -f /etc/opt/opensaf/ncs_subagent_comp_name.txt

#echo the component name into a temporary text file
echo $SA_AMF_COMPONENT_NAME > /etc/opt/opensaf/ncs_subagent_comp_name.txt

# check for the PID availability
if test -f $PIDPATH/$PIDFILE
then
   l_pid=`cat $PIDPATH/$PIDFILE`
   echo $l_pid
fi
if [ "$l_pid" ]
   then
   echo "Sending the SIGUSR1 to ncs_snmp_subagt ..."
   `kill -USR1 $l_pid`
else
    echo "Restarting SUBAGENT..."
    killall ncs_snmp_subagt
    $XTERM /opt/opensaf/controller/bin/ncs_snmp_subagt >$NCS_STDOUTS_PATH/ncs_snmp_subagt.log 2>&1 &
    sleep 10s
    if test -f $PIDPATH/$PIDFILE
    then
      l_pid=`cat $PIDPATH/$PIDFILE`
    fi
    echo $l_pid
    if [ "$l_pid" ]
    then `kill -USR1 $l_pid`
    fi
fi
exit 0

