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
  MODULE NAME: NCS_IFSV_IFIP_MIB_AGT_STUB.H 
******************************************************************************/
#ifndef _NCS_IFSV_IFIP_MIB_H_
#define _NCS_IFSV_IFIP_MIB_H_


/*
 * This C header file has been generated by smidump 0.4.1.
 * It is intended to be used with the NET-SNMP package 
 * for integration with NCS MASv.
 *
 * This header is derived from the NCS-IFSV-IFIP-MIB module.
 *
 * $Id$
 */

#include <stdlib.h>

#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif

#include "subagt_mab.h"
#include "subagt_oid_db.h"

#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/agent/table.h>
#include <net-snmp/agent/instance.h>
#include <net-snmp/agent/table_data.h>
#include <net-snmp/agent/table_dataset.h>


/*
 * Initialization/Termination functions:
 */

/* Common Registration routine */
extern uns32 __register_ncs_ifsv_ifip_mib_module(void);

/* Common Unregistration routine */
extern uns32 __unregister_ncs_ifsv_ifip_mib_module(void);

/* This variable is imported from subAgent module */
extern NCSMIB_PARAM_VAL rsp_param_val;

#endif /* _NCS_IFSV_IFIP_MIB_H_ */
