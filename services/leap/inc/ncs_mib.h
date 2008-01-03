/*      -*- OpenSAF  -*-
 *
 * (C) Copyright 2008 The OpenSAF Foundation 
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
 * or FITNESS FOR A PARTICULAR PURPOSE. This file and program are licensed
 * under the GNU Lesser General Public License Version 2.1, February 1999.
 * The complete license can be accessed from the following location:
 * http://opensource.org/licenses/lgpl-license.php 
 * See the Copying file included with the OpenSAF distribution for full
 * licensing terms.
 *
 * Author(s): Emerson Network Power
 *   
 */

/*****************************************************************************
..............................................................................


  


..............................................................................

  DESCRIPTION:
 

******************************************************************************
*/

/*
 * Module Inclusion Control...
 */
#ifndef NCS_MIB_H
#define NCS_MIB_H

#include "ncs_mib_pub.h"

/***************************************************************************
 *
 *     P r i v a t e  sync/timed CB   F u n c t i o n s
 *
 ***************************************************************************/

EXTERN_C uns32      mib_sync_response  ( NCSMIB_ARG*    arg);

EXTERN_C void       mib_timed_expiry   ( void*         mib_timed);

EXTERN_C uns32      mib_timed_response ( NCSMIB_ARG*    arg);

#endif

