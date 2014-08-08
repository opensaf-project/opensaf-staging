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
 *            Ericsson AB
 *
 */

#include <logtrace.h>
#include <si.h>
#include <imm.h>


AmfDb<std::string, AVD_SVC_TYPE_CS_TYPE> *svctypecstypes_db = NULL;
static void svctypecstype_db_add(AVD_SVC_TYPE_CS_TYPE *svctypecstype)
{
	uint32_t rc = svctypecstypes_db->insert(Amf::to_string(&svctypecstype->name),svctypecstype);
	osafassert(rc == NCSCC_RC_SUCCESS);
}


static AVD_SVC_TYPE_CS_TYPE *svctypecstypes_create(SaNameT *dn, const SaImmAttrValuesT_2 **attributes)
{
	AVD_SVC_TYPE_CS_TYPE *svctypecstype;

	TRACE_ENTER2("'%s'", dn->value);

	svctypecstype = new AVD_SVC_TYPE_CS_TYPE();

	memcpy(svctypecstype->name.value, dn->value, dn->length);
	svctypecstype->name.length = dn->length;

	if (immutil_getAttr(const_cast<SaImmAttrNameT>("saAmfSvctMaxNumCSIs"), attributes, 0, &svctypecstype->saAmfSvctMaxNumCSIs) != SA_AIS_OK)
		svctypecstype->saAmfSvctMaxNumCSIs = -1; /* no limit */

	return svctypecstype;
}

/**
 * Get configuration for all SaAmfSvcTypeCSTypes objects from 
 * IMM and create AVD internal objects. 
 * 
 * @param sutype_name 
 * @param sut 
 * 
 * @return SaAisErrorT 
 */
SaAisErrorT avd_svctypecstypes_config_get(SaNameT *svctype_name)
{
	AVD_SVC_TYPE_CS_TYPE *svctypecstype;
	SaAisErrorT error;
	SaImmSearchHandleT searchHandle;
	SaImmSearchParametersT_2 searchParam;
	SaNameT dn;
	const SaImmAttrValuesT_2 **attributes;
	const char *className = "SaAmfSvcTypeCSTypes";

	searchParam.searchOneAttr.attrName = const_cast<SaImmAttrNameT>("SaImmAttrClassName");
	searchParam.searchOneAttr.attrValueType = SA_IMM_ATTR_SASTRINGT;
	searchParam.searchOneAttr.attrValue = &className;

	error = immutil_saImmOmSearchInitialize_2(avd_cb->immOmHandle, svctype_name, SA_IMM_SUBTREE,
		SA_IMM_SEARCH_ONE_ATTR | SA_IMM_SEARCH_GET_ALL_ATTR, &searchParam,
		NULL, &searchHandle);
	
	if (SA_AIS_OK != error) {
		LOG_ER("saImmOmSearchInitialize_2 failed: %u", error);
		goto done1;
	}

	while (immutil_saImmOmSearchNext_2(searchHandle, &dn, (SaImmAttrValuesT_2 ***)&attributes) == SA_AIS_OK) {

		if ((svctypecstype = svctypecstypes_db->find(Amf::to_string(&dn)))== NULL) {
			if ((svctypecstype = svctypecstypes_create(&dn, attributes)) == NULL) {
				error = SA_AIS_ERR_FAILED_OPERATION;
				goto done2;
			}
			svctypecstype_db_add(svctypecstype);
		}
	}

	error = SA_AIS_OK;

 done2:
	(void)immutil_saImmOmSearchFinalize(searchHandle);
 done1:
	return error;
}

/**
 * Handles the CCB Completed operation for the 
 * SaAmfSvcTypeCSTypes class.
 * 
 * @param opdata 
 */
static SaAisErrorT svctypecstypes_ccb_completed_cb(CcbUtilOperationData_t *opdata)
{
	SaAisErrorT rc = SA_AIS_ERR_BAD_OPERATION;
	AVD_SVC_TYPE_CS_TYPE *svctypecstype;

	TRACE_ENTER2("CCB ID %llu, '%s'", opdata->ccbId, opdata->objectName.value);

	switch (opdata->operationType) {
	case CCBUTIL_CREATE:
		rc = SA_AIS_OK;
		break;
	case CCBUTIL_MODIFY:
		report_ccb_validation_error(opdata, "Modification of SaAmfSvcTypeCSTypes not supported");
		break;
	case CCBUTIL_DELETE:
		svctypecstype = svctypecstypes_db->find(Amf::to_string(&opdata->objectName));
		if (svctypecstype->curr_num_csis == 0) {
			rc = SA_AIS_OK;
			opdata->userData = svctypecstype;
		}
		break;
	default:
		osafassert(0);
		break;
	}

	return rc;
}

/**
 * Handles the CCB Apply operation for the SaAmfSvcTypeCSTypes 
 * class. 
 * 
 * @param opdata 
 */
static void svctypecstypes_ccb_apply_cb(CcbUtilOperationData_t *opdata)
{
	AVD_SVC_TYPE_CS_TYPE *svctypecstype;

	TRACE_ENTER2("CCB ID %llu, '%s'", opdata->ccbId, opdata->objectName.value);

	switch (opdata->operationType) {
	case CCBUTIL_CREATE:
		svctypecstype = svctypecstypes_create(&opdata->objectName, opdata->param.create.attrValues);
		osafassert(svctypecstype);
		svctypecstype_db_add(svctypecstype);
		break;
	case CCBUTIL_DELETE:
		svctypecstype = svctypecstypes_db->find(Amf::to_string(&opdata->objectName));
		if (svctypecstype != NULL) {
			svctypecstypes_db->erase(Amf::to_string(&opdata->objectName));
			delete svctypecstype;
		}

		break;
	default:
		osafassert(0);
		break;
	}
}

void avd_svctypecstypes_constructor(void)
{
	svctypecstypes_db = new AmfDb<std::string, AVD_SVC_TYPE_CS_TYPE>;
	avd_class_impl_set("SaAmfSvcTypeCSTypes", NULL, NULL,
		svctypecstypes_ccb_completed_cb, svctypecstypes_ccb_apply_cb);
}
