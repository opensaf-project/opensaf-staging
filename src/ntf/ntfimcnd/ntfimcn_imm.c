/*      -*- OpenSAF  -*-
 *
 * (C) Copyright 2013 The OpenSAF Foundation
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
 * Author(s): Ericsson AB
 *
 */

#include "ntfimcn_imm.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <libgen.h>
#include <limits.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>

#include "osaf/saf/saAis.h"
#include "ntf/saf/saNtf.h"
#include "amf/saf/saAmf.h"
#include "base/logtrace.h"
#include "base/saf_error.h"
#include "base/ncsgl_defs.h"
#include "base/osaf_extended_name.h"

#include "imm/saf/saImmOm.h"
#include "imm/saf/saImmOi.h"
#include "osaf/immutil/immutil.h"

#include "ntfimcn_main.h"
#include "ntfimcn_notifier.h"

/*
 * Global, scope file
 */
/* Release code, major version, minor version */
static SaVersionT imm_version = { 'A', 2, 12 };
static const unsigned int sleep_delay_ms = 500;
static const unsigned int max_waiting_time_3s = (3 * 1000);	/* 60 sec */
static const unsigned int max_waiting_time_7s = (7 * 1000);	/* 7 sec */
static const unsigned int max_waiting_time_60s = (60 * 1000);	/* 60 sec */
static const SaImmOiImplementerNameT applier_nameA =
	(SaImmOiImplementerNameT)"@OpenSafImmReplicatorA";
static const SaImmOiImplementerNameT applier_nameB =
	(SaImmOiImplementerNameT)"@OpenSafImmReplicatorB";

/* Used with function get_rdn_attr_name() */
struct {
	char* attrName;
	char* saved_className;
} s_get_rdn_attr_name = {NULL, NULL};;

/* Used with function get_created_dn() */
struct s_get_created_dn {
	SaNameT objectName;
} s_get_created_dn;

/* Used with function get_operation_invoke_name_create() */
struct {
	SaNameT iname;
	SaNameT *iname_ptr;

} s_get_operation_invoke_name_create;

/* Used with function get_operation_invoke_name_modify() --*/
struct {
	SaNameT iname;
	SaNameT *iname_ptr;
} s_get_operation_invoke_name_modify;

#define NOTIFYING_OBJECT "safApp=safImmService"

extern ntfimcn_cb_t ntfimcn_cb;

/**
 * Get name of rdn attribute from IMM
 *
 * Note:
 * A valid ntf_cb.immOmHandle must exist
 * Uses in file global struct s_get_rdn_attr_name
 *
 * @param className[in]
 * 
 * @return String with name
 */
static char *get_rdn_attr_name(const SaImmClassNameT className)
{
	SaAisErrorT rc;
	int msecs_waited;
	int i = 0;

	SaImmClassCategoryT classCategory;
	SaImmAttrDefinitionT_2 **attrDescr;

	TRACE_ENTER();

	/* Just return the name if already looked up */
	if (s_get_rdn_attr_name.saved_className != NULL &&
		strcmp(className, s_get_rdn_attr_name.saved_className) == 0) {
		goto done;
	}
	s_get_rdn_attr_name.saved_className =
		realloc(s_get_rdn_attr_name.saved_className, strlen(className) + 1);
	if (s_get_rdn_attr_name.saved_className == NULL) {
		LOG_ER("Failed to realloc memory");
		goto error;
	}
	memcpy(s_get_rdn_attr_name.saved_className, className, strlen(className) + 1);

	/* Get class description */
	msecs_waited = 0;
	rc = saImmOmClassDescriptionGet_2(ntfimcn_cb.immOmHandle,
			className,
			&classCategory,
			&attrDescr);
	while (((rc == SA_AIS_ERR_TRY_AGAIN) || (rc == SA_AIS_ERR_TIMEOUT)) &&
			(msecs_waited < max_waiting_time_7s)) {
		usleep(sleep_delay_ms * 1000);
		msecs_waited += sleep_delay_ms;
		rc = saImmOmClassDescriptionGet_2(ntfimcn_cb.immOmHandle,
				className,
				&classCategory,
				&attrDescr);
	}
	if (rc != SA_AIS_OK) {
		LOG_ER("saImmOmClassDescriptionGet_2 failed %s", saf_error(rc));
		goto error;
	}

	/* Find the name of the attribute with the RDN flag set */
	for (i=0; attrDescr[i] != NULL; i++) {
		if (attrDescr[i]->attrFlags & SA_IMM_ATTR_RDN) {
			s_get_rdn_attr_name.attrName =
				realloc(s_get_rdn_attr_name.attrName,
						strlen(attrDescr[i]->attrName) + 1);
			if (s_get_rdn_attr_name.attrName == NULL) {
				LOG_ER("Failed to realloc memory");
				goto error;
			}
			memcpy(s_get_rdn_attr_name.attrName, attrDescr[i]->attrName
							, strlen(attrDescr[i]->attrName) + 1);
			break;
		}
	}

	/* Free memory allocated for attribute descriptions */
	rc = saImmOmClassDescriptionMemoryFree_2(ntfimcn_cb.immOmHandle,attrDescr);
	if (rc != SA_AIS_OK) {
		LOG_ER("saImmOmClassDescriptionMemoryFree_2 failed %s", saf_error(rc));
		goto error;
	}

done:
	TRACE_LEAVE();
	return s_get_rdn_attr_name.attrName;
error:
	osafassert(0);
	return 0; /* Dummy */
}


/**
 * Use information from the in parameters and get the class description
 * to create the DN for a created object (create callback).
 * Note:
 * Uses in file global struct s_get_created_dn
 *
 * @param className[in]
 * @param parentName[in]
 * @param attr[in]
 *
 * @return SaNameT *objectName
 *			Note must be used before next call
 */
static SaNameT *get_created_dn(const SaImmClassNameT className,
						const SaNameT *parentName,
						const SaImmAttrValuesT_2 **attr)
{
	int i = 0;

	const char* object_rdn = "";
	char *attrName;

	TRACE_ENTER();

	/* Get name of rdn attribute */
	attrName = get_rdn_attr_name(className);

	/* Get the value (RDN string) from the "RDN" attribute */
	for (i=0; attr[i] != NULL; i++) {
		if( strcmp(attr[i]->attrName, attrName) == 0) {
			if (attr[i]->attrValueType == SA_IMM_ATTR_SASTRINGT) {
				object_rdn = *((char**) attr[i]->attrValues[0]);
			} else if (attr[i]->attrValueType == SA_IMM_ATTR_SANAMET) {
				object_rdn = osaf_extended_name_borrow(
									(SaNameT*)(attr[i]->attrValues[0]));
			}
			break;
		}
	}

	/* Create the DN */
	osaf_extended_name_free(&s_get_created_dn.objectName);
	char* objectName;
	if (!osaf_is_extended_name_empty(parentName)) {
		size_t rdn_len = strlen(object_rdn);
		size_t parent_len = osaf_extended_name_length(parentName);
		objectName = malloc(rdn_len + parent_len + 2);
		memcpy(objectName, object_rdn, rdn_len);
		objectName[rdn_len] = ',';
		memcpy(objectName + rdn_len + 1, osaf_extended_name_borrow(parentName)
										, parent_len + 1);
	} else {
		objectName = strdup(object_rdn);
	}
	osaf_extended_name_steal(objectName, &s_get_created_dn.objectName);

	TRACE_LEAVE();
	return &s_get_created_dn.objectName;
}

/**
 * Get the operation invoker name.
 * If ccb id is 0 or >0 return the value in SaImmAttrImplementerName or
 * SaImmAttrAdminOwnerName respective
 * Note:
 * Uses in file global struct s_get_operation_invoke_name_create
 *
 * @param ccbId[in]
 * @param attr[in]
 * 
 * @return SaNameT *operation_invoke_name
 *			Note must be used before next call
 */
static SaNameT *get_operation_invoke_name_create(
			SaImmOiCcbIdT ccbId,
			const SaImmAttrValuesT_2 **attr)
{
	int i = 0;
	char *attrName;

	TRACE_ENTER();

	s_get_operation_invoke_name_create.iname_ptr = &s_get_operation_invoke_name_create.iname;

	if (ccbId == 0) {
		attrName = NTFIMCN_IMPLEMENTER_NAME;
	} else {
		attrName = NTFIMCN_ADMIN_OWNER_NAME;
	}
	
	/* Get the value from Admin owner name or Implementer name */
	osaf_extended_name_free(&s_get_operation_invoke_name_create.iname);
	osaf_extended_name_clear(&s_get_operation_invoke_name_create.iname);
	for (i=0; attr[i] != NULL; i++) {
		if( strcmp(attr[i]->attrName, attrName) == 0) {
			osaf_extended_name_alloc(*((char**) attr[i]->attrValues[0]),
				&s_get_operation_invoke_name_create.iname);
			goto done;
		}
	}
	/* If we get here no name is found! */
	LOG_ER("%s no name was found",__FUNCTION__);
	osafassert(0);

done:
	TRACE_LEAVE();
	return s_get_operation_invoke_name_create.iname_ptr;
}

/**
 * Get the operation invoker name.
 * If ccb id is 0 or >0 return the value in SaImmAttrImplementerName or
 * SaImmAttrAdminOwnerName respective.
 *
 * Note:
 *
 * @param ccbId[in]
 * @param attrMods[in]
 * 
 * @return SaNameT *operation_invoke_name
 *			Note must be used before next call
 */
static SaNameT *get_operation_invoke_name_modify(
			SaImmOiCcbIdT ccbId,
			const SaImmAttrModificationT_2 **attrMods)
{
	int i = 0;
	char *attrName;

	TRACE_ENTER();

	s_get_operation_invoke_name_modify.iname_ptr = &s_get_operation_invoke_name_modify.iname;

	if (ccbId == 0) {
		attrName = NTFIMCN_IMPLEMENTER_NAME;
	} else {
		attrName = NTFIMCN_ADMIN_OWNER_NAME;
	}

	/* Get the value from Admin owner name or Implementer name */
	osaf_extended_name_free(&s_get_operation_invoke_name_modify.iname);
	osaf_extended_name_clear(&s_get_operation_invoke_name_modify.iname);
	for (i=0; attrMods[i] != NULL; i++) {
		if( strcmp(attrMods[i]->modAttr.attrName, attrName) == 0) {
			osaf_extended_name_alloc(*((char**) attrMods[i]->modAttr.attrValues[0]),
				&s_get_operation_invoke_name_modify.iname);
			goto done;
		}
	}
	/* If we get here no name is found! */
	LOG_ER("%s no name was found",__FUNCTION__);
	osafassert(0);

done:
	TRACE_LEAVE();
	return s_get_operation_invoke_name_modify.iname_ptr;
}

/**
 *	Save the CCB. Will be handled and freed in apply callback
 * @param immOiHandle[in]
 * @param ccbId[in]
 * @param objectName[in]
 * 
 * @return SaAisErrorT
 */
static SaAisErrorT saImmOiCcbObjectDeleteCallback(SaImmOiHandleT immOiHandle,
					     SaImmOiCcbIdT ccbId, const SaNameT *objectName)
{
	SaAisErrorT rc = SA_AIS_OK;
	struct CcbUtilCcbData *ccbUtilCcbData;
	struct CcbUtilOperationData *ccbUtilOperationData;
	SaNameT invoke_name;
	osaf_extended_name_clear(&invoke_name);
	int internal_rc = 0;

	TRACE_ENTER();

	if ((ccbUtilCcbData = ccbutil_findCcbData(ccbId)) == NULL) {
		if ((ccbUtilCcbData = ccbutil_getCcbData(ccbId)) == NULL) {
			LOG_ER("%s Failed to get CCB object for %llu",__FUNCTION__,ccbId);
			rc = SA_AIS_ERR_NO_MEMORY;
			goto done;
		}
	}

	/* "memorize the delete request" */
	ccbutil_ccbAddDeleteOperation(ccbUtilCcbData, objectName);

	if (ccbId == 0) {
		/* Runtime object delete */
		ccbUtilOperationData = ccbUtilCcbData->operationListHead;

		internal_rc = ntfimcn_send_object_delete_notification(ccbUtilOperationData,
				&invoke_name,
				SA_FALSE);
		if (internal_rc != 0) {
			LOG_ER("%s send_object_delete_notification fail",
					__FUNCTION__);
			goto done;
		}


		ccbutil_deleteCcbData(ccbUtilCcbData);

		if (internal_rc != 0) {
			LOG_ER("%s send_object_create_notification fail",
					__FUNCTION__);
			goto done;
		}
	}


done:
	if (internal_rc != 0) {
		/* If we fail to send a notification we exit. This will signal that
		 * a notification is missing.
		 */
		LOG_ER("saImmOiCcbObjectCreateCallback Fail, internal_rc=%d",
				internal_rc);
		imcn_exit(EXIT_FAILURE);
	}

	TRACE_LEAVE();
	return rc;
}

/**
 * Save the CCB. Will be handled and freed in apply callback
 * If ccbId = 0:
 *		This means that a runtime object is created and the
 *		corresponding notification shall be sent from here
 *
 * @param immOiHandle[in]
 * @param ccbId[in]
 * @param className[in]
 * @param parentName[in]
 * @param attr[in]
 *
 * @return SaAisErrorT
 */
static SaAisErrorT saImmOiCcbObjectCreateCallback(SaImmOiHandleT immOiHandle,
					       SaImmOiCcbIdT ccbId,
					       const SaImmClassNameT className,
					       const SaNameT *parentName, const SaImmAttrValuesT_2 **attr)
{
	SaAisErrorT rc = SA_AIS_OK;
	CcbUtilCcbData_t *ccbUtilCcbData;
	SaNameT *dn_ptr;
	SaNameT *invoke_name_ptr = NULL;
	struct CcbUtilOperationData *ccbUtilOperationData;
	SaStringT rdn_attr_name;
	int internal_rc = 0;

	TRACE_ENTER();

	dn_ptr = get_created_dn(className, parentName, attr);

	if ((ccbUtilCcbData = ccbutil_findCcbData(ccbId)) == NULL) {
		if ((ccbUtilCcbData = ccbutil_getCcbData(ccbId)) == NULL) {
			LOG_ER("%s: Failed to get CCB object for ccb Id %llu",__FUNCTION__, ccbId);
			rc = SA_AIS_ERR_NO_MEMORY;
			goto done;
		}
		invoke_name_ptr = get_operation_invoke_name_create(ccbId, attr);
		ccbUtilCcbData->userData = invoke_name_ptr;
	}

	/* "memorize the create request" */
	ccbutil_ccbAddCreateOperation_2(ccbUtilCcbData, dn_ptr,
			className, parentName, attr);
	
	if (ccbId == 0) {
		/* Runtime object create */
		ccbUtilOperationData = ccbUtilCcbData->operationListHead;

		rdn_attr_name = get_rdn_attr_name(
				ccbUtilOperationData->param.create.className);
		internal_rc = ntfimcn_send_object_create_notification(
				ccbUtilOperationData, rdn_attr_name,
				SA_FALSE);

		ccbutil_deleteCcbData(ccbUtilCcbData);

		if (internal_rc != 0) {
			LOG_ER("%s send_object_create_notification fail",
					__FUNCTION__);
			goto done;
		}
	}

done:
	if (internal_rc != 0) {
		/* If we fail to send a notification we exit. This will signal that
		 * a notification is missing.
		 */
		LOG_ER("saImmOiCcbObjectCreateCallback Fail, internal_rc=%d",
				internal_rc);
		imcn_exit(EXIT_FAILURE);
	}

	TRACE_LEAVE();
	return rc;
}

/**
 * Save the CCB. Will be handled and freed in apply callback
 *
 */
static SaAisErrorT saImmOiCcbObjectModifyCallback(SaImmOiHandleT immOiHandle,
						SaImmOiCcbIdT ccbId,
						const SaNameT *objectName,
						const SaImmAttrModificationT_2 **attrMods)
{
	SaAisErrorT rc = SA_AIS_OK;
	struct CcbUtilCcbData *ccbUtilCcbData;
	SaNameT *invoke_name_ptr = NULL;
	struct CcbUtilOperationData *ccbUtilOperationData;
	int internal_rc = 0;

	TRACE_ENTER();

	if ((ccbUtilCcbData = ccbutil_findCcbData(ccbId)) == NULL) {
		if ((ccbUtilCcbData = ccbutil_getCcbData(ccbId)) == NULL) {
			LOG_ER("%s: Failed to get CCB object for ccb Id %llu",__FUNCTION__, ccbId);
			rc = SA_AIS_ERR_NO_MEMORY;
			goto done;
		}
		invoke_name_ptr = get_operation_invoke_name_modify(ccbId, attrMods);
		ccbUtilCcbData->userData = invoke_name_ptr;
	}

	/* "memorize the modification request" */
	ccbutil_ccbAddModifyOperation(ccbUtilCcbData, objectName,
						attrMods);
	SaNameT *invoker_name_ptr;
	invoker_name_ptr = ccbUtilCcbData->userData;

	if (ccbId == 0) {
		/* Attribute change object create */
		ccbUtilOperationData = ccbUtilCcbData->operationListHead;

		internal_rc = ntfimcn_send_object_modify_notification(
				ccbUtilOperationData,
				invoker_name_ptr,
				SA_FALSE);

		ccbutil_deleteCcbData(ccbUtilCcbData);

		if (internal_rc != 0) {
			LOG_ER("%s send_object_modify_notification fail",
					__FUNCTION__);
			goto done;
		}
	}

done:
	if (internal_rc != 0) {
		/* If we fail to send a notification we exit. This will signal that
		 * a notification is missing.
		 */
		LOG_ER("saImmOiCcbObjectCreateCallback Fail, internal_rc=%d",
				internal_rc);
		imcn_exit(EXIT_FAILURE);
	}

	TRACE_LEAVE();
	return rc;
}

/**
 * Free saved (allocated) CCB data
 *
 */
static void saImmOiCcbAbortCallback(SaImmOiHandleT immOiHandle, SaImmOiCcbIdT ccbId)
{
	struct CcbUtilCcbData *ccbUtilCcbData;

	TRACE_ENTER();

	if ((ccbUtilCcbData = ccbutil_findCcbData(ccbId)) != NULL) {
		ccbutil_deleteCcbData(ccbUtilCcbData);
	} else {
		LOG_ER("%s: Failed to find CCB object for ccb Id %llu",__FUNCTION__, ccbId);
	}

	TRACE_LEAVE();
}

/**
 * The CCB is now complete.
 * Since we are an applier we do not have to check if the CCB is valid
 *
 */
static SaAisErrorT saImmOiCcbCompletedCallback(SaImmOiHandleT immOiHandle, SaImmOiCcbIdT ccbId)
{
	/* Nothing to do here */
	return SA_AIS_OK;
}


/**
 * Configuration changes are done fill in and send corresponding notification.
 *
 */
static void saImmOiCcbApplyCallback(SaImmOiHandleT immOiHandle, SaImmOiCcbIdT ccbId)
{
	struct CcbUtilCcbData *ccbUtilCcbData;
	struct CcbUtilOperationData *ccbUtilOperationData;
	int internal_rc = 0;
	SaStringT rdn_attr_name;
	SaNameT *invoke_name_ptr;
	SaBoolT ccbLast = SA_FALSE;
	SaUint64T ccb_entries_cnt = 0;

	TRACE_ENTER();

	if ((ccbUtilCcbData = ccbutil_findCcbData(ccbId)) == NULL) {
		LOG_ER("%s: Failed to find CCB object for ccb Id %llu",__FUNCTION__, ccbId);
		internal_rc = (-1);
		goto done;
	}

	/* Count the number of ccb entries in order to know which is the last one */
	ccbUtilOperationData = ccbUtilCcbData->operationListHead;
	ccb_entries_cnt = 0;
	while (ccbUtilOperationData != NULL) {
		ccb_entries_cnt++;
		ccbUtilOperationData = ccbUtilOperationData->next;
	}

	ccbUtilOperationData = ccbUtilCcbData->operationListHead;
	while (ccbUtilOperationData != NULL) {
		if (ccb_entries_cnt > 0) {
			ccb_entries_cnt--;
		}
		
		if (ccb_entries_cnt == 0) {
			ccbLast = SA_TRUE;
		}

		switch (ccbUtilOperationData->operationType) {
		case CCBUTIL_CREATE:
			rdn_attr_name = get_rdn_attr_name(
					ccbUtilOperationData->param.create.className);

			internal_rc = ntfimcn_send_object_create_notification(
					ccbUtilOperationData, rdn_attr_name,
					ccbLast);
			if (internal_rc != 0) {
				LOG_ER("%s send_object_create_notification fail",
						__FUNCTION__);
				goto done;
			}
			break;

		case CCBUTIL_DELETE:
			invoke_name_ptr = (SaNameT *)ccbUtilCcbData->userData;
			internal_rc = ntfimcn_send_object_delete_notification(ccbUtilOperationData,
					invoke_name_ptr,
					ccbLast);
			if (internal_rc != 0) {
				LOG_ER("%s send_object_delete_notification fail",
						__FUNCTION__);
				goto done;
			}
			break;
			
		case CCBUTIL_MODIFY:
			invoke_name_ptr = ccbUtilCcbData->userData;

			/* send_object_modify_notification */
			internal_rc = ntfimcn_send_object_modify_notification(
					ccbUtilOperationData,
					invoke_name_ptr,
					ccbLast);
			if (internal_rc != 0) {
				LOG_ER("%s send_object_modify_notification fail",
						__FUNCTION__);
				goto done;
			}
			break;
		}

		ccbUtilOperationData = ccbUtilOperationData->next;
	}

done:
	if (ccbUtilCcbData != NULL) {
		ccbutil_deleteCcbData(ccbUtilCcbData);
	}
	TRACE_LEAVE();
	if (internal_rc != 0) {
		/* If we fail to send a notification we exit. This will signal that
		 * a notification is missing.
		 */
		imcn_exit(EXIT_FAILURE);
	}
}

static const SaImmOiCallbacksT_2 callbacks = {
	.saImmOiAdminOperationCallback = NULL,
	.saImmOiCcbAbortCallback = saImmOiCcbAbortCallback,
	.saImmOiCcbApplyCallback = saImmOiCcbApplyCallback,
	.saImmOiCcbCompletedCallback = saImmOiCcbCompletedCallback,
	.saImmOiCcbObjectCreateCallback = saImmOiCcbObjectCreateCallback,
	.saImmOiCcbObjectDeleteCallback = saImmOiCcbObjectDeleteCallback,
	.saImmOiCcbObjectModifyCallback = saImmOiCcbObjectModifyCallback,
	.saImmOiRtAttrUpdateCallback = NULL
};

static const SaImmCallbacksT omCallbacks = {
	.saImmOmAdminOperationInvokeCallback = NULL
};

/**
 * Initialize the OI interface, get a selection object and become applier
 * 
 * @global_param max_waiting_time_ms: Wait max time for each operation.
 * @global_param applier_name: The name of the "configuration change" applier
 * @param *cb[out]
 *
 * @return (-1) if init fail
 */
int ntfimcn_imm_init(ntfimcn_cb_t *cb)
{
	SaAisErrorT rc;
	int internal_rc = 0;
	int msecs_waited;

	TRACE_ENTER();

	/*
	 * Set IMM environment variable for synchronous timeout to 1 sec
	 */
	setenv("IMMA_SYNCR_TIMEOUT","100",1);

	/*
	 * Initialize the IMM OI API
	 * -------------------------
	 */
	msecs_waited = 0;
	for (;;) {
		if (msecs_waited >= max_waiting_time_60s) {
			LOG_ER("%s Timeout when initalizing OI", __FUNCTION__);
			internal_rc = NTFIMCN_INTERNAL_ERROR;
			goto done;
		}
		cb->immOiHandle = 0;
		rc = saImmOiInitialize_2(&cb->immOiHandle, &callbacks,
					 &imm_version);
		while ((rc == SA_AIS_ERR_TRY_AGAIN || rc == SA_AIS_ERR_TIMEOUT)
		       && msecs_waited < max_waiting_time_60s) {
			if (rc == SA_AIS_ERR_TIMEOUT) {
				LOG_WA("%s saImmOiInitialize_2() returned %s",
				       __FUNCTION__, saf_error(rc));
			}
			usleep(sleep_delay_ms * 1000);
			msecs_waited += sleep_delay_ms;
			if (rc == SA_AIS_ERR_TIMEOUT && cb->immOiHandle != 0) {
				while (saImmOiFinalize(cb->immOiHandle) ==
				       SA_AIS_ERR_TRY_AGAIN &&
				       msecs_waited < max_waiting_time_60s) {
					usleep(sleep_delay_ms * 1000);
					msecs_waited += sleep_delay_ms;
				}
			}
			cb->immOiHandle = 0;
			rc = saImmOiInitialize_2(&cb->immOiHandle, &callbacks,
						 &imm_version);
		}
		if (rc != SA_AIS_OK) {
			LOG_ER("%s saImmOiInitialize_2 failed %s", __FUNCTION__,
			       saf_error(rc));
			internal_rc = NTFIMCN_INTERNAL_ERROR;
			goto done;
		}

		/*
		 * Get a selection object for the IMM OI
		 * -------------------------------------
		 */
		rc = saImmOiSelectionObjectGet(cb->immOiHandle,
					       &cb->immSelectionObject);
		while ((rc == SA_AIS_ERR_TRY_AGAIN || rc == SA_AIS_ERR_TIMEOUT)
		       && msecs_waited < max_waiting_time_60s) {
			if (rc == SA_AIS_ERR_TIMEOUT) {
				LOG_WA("%s saImmOiSelectionObjectGet() "
				       "returned %s", __FUNCTION__,
				       saf_error(rc));
			}
			usleep(sleep_delay_ms * 1000);
			msecs_waited += sleep_delay_ms;
			rc = saImmOiSelectionObjectGet(cb->immOiHandle,
						       &cb->immSelectionObject);
		}
		if (rc == SA_AIS_ERR_TIMEOUT || rc == SA_AIS_ERR_BAD_HANDLE) {
			LOG_WA("%s saImmOiSelectionObjectGet() returned %s",
			       __FUNCTION__, saf_error(rc));
			usleep(sleep_delay_ms * 1000);
			msecs_waited += sleep_delay_ms;
			while (saImmOiFinalize(cb->immOiHandle) ==
			       SA_AIS_ERR_TRY_AGAIN &&
				msecs_waited < max_waiting_time_60s) {
				usleep(sleep_delay_ms * 1000);
				msecs_waited += sleep_delay_ms;
			}
			cb->immOiHandle = 0;
			cb->immSelectionObject = -1;
			continue;
		}
		if (rc != SA_AIS_OK) {
			LOG_ER("%s saImmOiSelectionObjectGet failed %s",
			       __FUNCTION__, saf_error(rc));
			internal_rc = NTFIMCN_INTERNAL_ERROR;
			goto done;
		}

		/*
		 * Become the "configuration change" applier
		 * -----------------------------------------
		 */
		SaImmOiImplementerNameT applier_name = applier_nameA;
		rc = saImmOiImplementerSet(cb->immOiHandle, applier_name);
		while ((rc == SA_AIS_ERR_TRY_AGAIN || rc == SA_AIS_ERR_EXIST) &&
		       msecs_waited < max_waiting_time_60s) {
			usleep(sleep_delay_ms * 1000);
			msecs_waited += sleep_delay_ms;

			if (rc == SA_AIS_ERR_EXIST) {
				if (strcmp( applier_name, applier_nameA) == 0) {
					applier_name = applier_nameB;
				} else {
					applier_name = applier_nameA;
				}
			}
			rc = saImmOiImplementerSet(cb->immOiHandle,
						   applier_name);
		}
		if (rc == SA_AIS_ERR_TIMEOUT || rc == SA_AIS_ERR_BAD_HANDLE) {
			LOG_WA("%s saImmOiImplementerSet() returned %s",
			       __FUNCTION__, saf_error(rc));
			usleep(sleep_delay_ms * 1000);
			msecs_waited += sleep_delay_ms;
			while (saImmOiFinalize(cb->immOiHandle) ==
			       SA_AIS_ERR_TRY_AGAIN &&
				msecs_waited < max_waiting_time_60s) {
				usleep(sleep_delay_ms * 1000);
				msecs_waited += sleep_delay_ms;
			}
			cb->immOiHandle = 0;
			cb->immSelectionObject = -1;
			continue;
		}

		if (rc != SA_AIS_OK) {
			LOG_ER("%s Becoming an applier failed %s", __FUNCTION__,
			       saf_error(rc));
			internal_rc = NTFIMCN_INTERNAL_ERROR;
			goto done;
		}
		break;
	}

	/*
	 * Initialize the IMM OM API
	 * -------------------------
	 */
	msecs_waited = 0;
	cb->immOmHandle = 0;
	rc = saImmOmInitialize(&cb->immOmHandle, &omCallbacks, &imm_version);
	while ((rc == SA_AIS_ERR_TRY_AGAIN || rc == SA_AIS_ERR_TIMEOUT)
	       && msecs_waited < max_waiting_time_60s) {
		if (rc == SA_AIS_ERR_TIMEOUT) {
			LOG_WA("%s saImmOmInitialize() returned %s",
			       __FUNCTION__, saf_error(rc));
		}
		usleep(sleep_delay_ms * 1000);
		msecs_waited += sleep_delay_ms;
		if (rc == SA_AIS_ERR_TIMEOUT && cb->immOmHandle != 0) {
			while (saImmOmFinalize(cb->immOmHandle) ==
			       SA_AIS_ERR_TRY_AGAIN &&
			       msecs_waited < max_waiting_time_60s) {
				usleep(sleep_delay_ms * 1000);
				msecs_waited += sleep_delay_ms;
			}
		}
		cb->immOmHandle = 0;
		rc = saImmOmInitialize(&cb->immOmHandle, &omCallbacks,
				       &imm_version);
	}
	if (rc != SA_AIS_OK) {
		LOG_ER("%s saImmOmInitialize failed %s", __FUNCTION__,
		       saf_error(rc));
		internal_rc = NTFIMCN_INTERNAL_ERROR;
		goto done;
	}

done:
	TRACE_LEAVE();
	return internal_rc;
}

/**
 * Clears the special applier name
 * 
 */
void ntfimcn_special_applier_clear(void)
{
	SaAisErrorT rc;
	int msecs_waited;

	msecs_waited = 0;
	rc = saImmOiImplementerClear(ntfimcn_cb.immOiHandle);
	while (((rc == SA_AIS_ERR_TRY_AGAIN) ||
			(rc == SA_AIS_ERR_TIMEOUT))	&& 
			(msecs_waited < max_waiting_time_3s)) {
		usleep(sleep_delay_ms * 1000);
		msecs_waited += sleep_delay_ms;
		rc = saImmOiImplementerClear(ntfimcn_cb.immOiHandle);
	}
	if (rc != SA_AIS_OK) {
		if (rc == SA_AIS_ERR_BAD_HANDLE) {
			TRACE("%s saImmOiImplementerClear failed %s",__FUNCTION__,saf_error(rc));
		}
	}
}
