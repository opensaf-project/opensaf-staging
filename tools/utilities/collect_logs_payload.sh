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

#########################################################################################

# 
#  File name: collect_logs.sh
#
#  Logs collected are given below
#
#  /etc/opt/opensaf/
#  /var/crash/opensaf
#  /var/log/messages
#  /var/opt/opensaf/
#  
#  This script will create a Master .tar file with host name and time stamp in /var/log 
#  eg."/var/log/collected_logs_PC1_2006-08-01_04:14:32.tar"
#  
#  Master tar will in turn have Sub Tars (like the one given below) compressed with bzip2 utility
#  eg. "@var@opt@opensaf/.tar.gz"
#
#  Use following command to untar these subtars
#  # tar -zxvf "@var@opt@opensaf@.tar.gz"
#
#
#########################################################################################


MAX_LOG_DIR=4
 
declare LOG_DIR[$MAX_LOG_DIR]=(/etc/opt/opensaf/ coredumps /var/log/messages /var/opt/opensaf/) 

if [ $# -eq 0 ]
then
echo
echo
echo Usage :
echo
echo 1 /etc/opt/opensaf/
echo "2 coredumps /var/crash/core*ncs*"
echo 3 /var/log/messages
echo 4 /var/opt/opensaf/
echo 99 ALL of the above files/directories.
echo 
echo "Enter the number(s) corresponding to files/directories you want to collect seperated by SPACE" 
echo "eg. \"./collect_logs.sh 1 2 3\"" 
echo "-OR-"  
echo "Enter 99 for collecting all above mentioned log files/directories" 
echo "eg. \"./collect_logs.sh 99\""
echo
echo =====================================================================
echo NOTE : Ensure following before running this script 
echo
echo "      1. Stop OpenSAF services"
echo =====================================================================
echo
echo
exit
fi

zip_current_object()
{
    # Get Available and Required Disk space 
    DISK_SPACE_AVAILABLE=`df /var/log | grep -iv used | tr -s " " " " | cut -d" " -f4`
    if [ $CURRENT_OBJECT = "coredumps" ]
    then
        DISK_SPACE_REQUIRED=`du -sck /var/crash/core*ncs* | grep total | cut -f1`
    else
        DISK_SPACE_REQUIRED=`du -sck $CURRENT_OBJECT | grep total | cut -f1`
    fi

    # If enough disk space not present then return error and continue with next directory
    if [ $DISK_SPACE_REQUIRED -gt $DISK_SPACE_AVAILABLE ]
    then
      echo "ERROR:Compressing $CURRENT_OBJECT Failed : No disk space in /var/log"
      continue
    fi

    # Create a Sub Tar filename with "/" removed and replaced with "@"
    SUB_TAR_NAME=`echo $CURRENT_OBJECT | tr -s "/" "@"`.tar.gz

    # Create Sub Tar file
    if [ $CURRENT_OBJECT = "coredumps" ]
    then
        nice -n 17 tar -h -zcf /var/log/$SUB_TAR_NAME  /var/crash/core*ncs* 2>/dev/null
    else
        nice -n 17 tar -h -zcf /var/log/$SUB_TAR_NAME  $CURRENT_OBJECT 2>/dev/null
    fi

    # Add it to Master TAR
    nice -n 17 tar -rf $MASTER_TAR_NAME $SUB_TAR_NAME 2>/dev/null

    # Remove Sub Tar
    rm $SUB_TAR_NAME

    echo Compressed $CURRENT_OBJECT
}

MASTER_TAR_NAME=collected_logs_payload_${HOSTNAME}_`date +%F_%H-%M-%S`.tar
SEPERATE_FILES=0
CURRENT_OBJECT=""
# Ensure that /var/log directory is available
mkdir -p /var/log/

CURRENT_DIR=`pwd`
cd /var/log

for i in $* 
do
    case "$i" in
    "1"|"2"|"3"|"4" )
        SEPERATE_FILES=1
        CURRENT_OBJECT=${LOG_DIR[$i-1]}
        zip_current_object
        CURRENT_OBJECT=""
        ;;
    "99" )
        if [ $SEPERATE_FILES -eq 1 ]
        then
            echo "Can Not give option 99 with other options"
            continue
        fi
        for ((j=0; j<$MAX_LOG_DIR; j++))
        do
            CURRENT_OBJECT=${LOG_DIR[$j]}
            zip_current_object
        done
        break
        ;;

    *   )
        echo ERROR: Invalid argument number.
        ;;

    esac
done

cd $CURRENT_DIR 

[ -f /var/log/$MASTER_TAR_NAME ] && echo Find collected logs at /var/log/$MASTER_TAR_NAME

exit

#########################################################################################
