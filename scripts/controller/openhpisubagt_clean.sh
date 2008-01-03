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
#

rm -f /tmp/openhpisubagt_log;

PID_PATH="/tmp/pid/"
ECHO="echo -e"
EXTN=".pid"
DBGFILE="/var/opt/opensaf/stdouts/monc_debug_log"

compName=`echo $SA_AMF_COMPONENT_NAME`
if [ -z $compName ]; then
    $ECHO "No Environment Variable of OPENHPISUBAGT Component Name set" >> $DBGFILE
    exit -1;
fi

killall -q -9 hpiSubagent;

rm -f $PID_PATH$compName$EXTN;
$ECHO "OPENHPISUBAGT Successfully Stopped" >> $DBGFILE
exit 0;
