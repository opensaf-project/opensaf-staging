/*      -- OpenSAF  --
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
 * Author(s): Ericsson AB
 *
 */
#include <stdlib.h>
#include <sys/stat.h>
#include <new>
#include <vector>
#include <string>

#include "SmfCampaign.hh"
#include "smfd.h"
#include "smfd_smfnd.h"
#include "SmfCampaignThread.hh"
#include "SmfCampaignXmlParser.hh"
#include "SmfUpgradeCampaign.hh"
#include "SmfUpgradeProcedure.hh"

#include <saSmf.h>
#include <logtrace.h>
#include <immutil.h>

/*====================================================================*/
/*  Class SmfCampaign                                                 */
/*====================================================================*/

/** 
 * Constructor
 */
 SmfCampaign::SmfCampaign(const SaNameT * parent, const SaImmAttrValuesT_2 ** attrValues):
	m_cmpgConfigBase(0),
	m_cmpgExpectedTime(0),
	m_cmpgElapsedTime(0),
	m_cmpgState(SA_SMF_CMPG_INITIAL),
	m_upgradeCampaign(NULL),
	m_campaignXmlDir("")
{
	init(attrValues);
	m_dn = m_cmpg;
	m_dn += ",";
	m_dn.append((char *)parent->value, parent->length);
}

/** 
 * Constructor
 */
SmfCampaign::SmfCampaign(const SaNameT * dn):
	m_cmpgConfigBase(0),
	m_cmpgExpectedTime(0),
	m_cmpgElapsedTime(0),
	m_cmpgState(SA_SMF_CMPG_INITIAL),
	m_upgradeCampaign(NULL)
{
	m_dn.append((char *)dn->value, dn->length);
}

/** 
 * Destructor
 */
SmfCampaign::~SmfCampaign()
{
	TRACE_ENTER();

	TRACE_LEAVE();
}

/** 
 * dn
 */
const std::string & 
SmfCampaign::getDn()
{
	return m_dn;
}

/** 
 * executing
 */
bool 
SmfCampaign::executing(void)
{
	bool rc = true;
	switch (m_cmpgState) {
	/* 
	   The following states are final states where the campaign is
	   considered as NOT executing and object can be removed/modified
	*/
	case SA_SMF_CMPG_INITIAL:
	case SA_SMF_CMPG_CAMPAIGN_COMMITTED:
	case SA_SMF_CMPG_EXECUTION_FAILED:
	case SA_SMF_CMPG_ROLLBACK_COMMITTED:
	case SA_SMF_CMPG_ROLLBACK_FAILED:
		rc = false;
		break;
	default:
		/*
		  At all other states the campaign is considered executing
		*/
		rc = true;
		break;
	}

	return rc;
}

/** 
 * verify
 * verifies that the attribute settings is OK
 */
SaAisErrorT 
SmfCampaign::verify(const SaImmAttrModificationT_2 ** attrMods)
{
	SaAisErrorT rc = SA_AIS_OK;
	int i = 0;
	const SaImmAttrModificationT_2 *attrMod;

	/* We don't allow attribute modifications if wrong state */
	switch (m_cmpgState) {
	case SA_SMF_CMPG_INITIAL:
	case SA_SMF_CMPG_CAMPAIGN_COMMITTED:
	case SA_SMF_CMPG_ROLLBACK_COMMITTED:
		break;

	default:
		LOG_ER("Attribute modification not allowed in state %u", m_cmpgState);
		return SA_AIS_ERR_BAD_OPERATION;
	}

	attrMod = attrMods[i++];
	while (attrMod != NULL) {
		void *value;
		const SaImmAttrValuesT_2 *attribute = &attrMod->modAttr;

		TRACE("attribute %s", attribute->attrName);

		if (attribute->attrValuesNumber != 1) {
			LOG_ER("Number of values for attribute %s is != 1 (%u)", attribute->attrName,
			       attribute->attrValuesNumber);
			rc = SA_AIS_ERR_BAD_OPERATION;
			goto done;
		}

		value = attribute->attrValues[0];

		if (!strcmp(attribute->attrName, "saSmfCmpgFileUri")) {
			struct stat pathstat;
			char *fileName = *((char **)value);

			TRACE("verifying saSmfCmpgFileUri = %s", fileName);

			if (stat(fileName, &pathstat) != 0) {
				LOG_ER("File %s doesn't exist", fileName);
				rc = SA_AIS_ERR_BAD_OPERATION;
				goto done;
			}
		} else {
			LOG_ER("invalid attribute %s", attribute->attrName);
			rc = SA_AIS_ERR_BAD_OPERATION;
			goto done;
		}

		attrMod = attrMods[i++];
	}
 done:
	return rc;
}

/** 
 * modify
 * executes the attribute modifications
 */
SaAisErrorT 
SmfCampaign::modify(const SaImmAttrModificationT_2 ** attrMods)
{
	SaAisErrorT rc = SA_AIS_OK;
	int i = 0;
	const SaImmAttrModificationT_2 *attrMod;

	attrMod = attrMods[i++];
	while (attrMod != NULL) {
		void *value;
		const SaImmAttrValuesT_2 *attribute = &attrMod->modAttr;

		TRACE("attribute %s", attribute->attrName);

		if (attribute->attrValuesNumber != 1) {
			rc = SA_AIS_ERR_BAD_OPERATION;
			goto done;
		}

		value = attribute->attrValues[0];

		if (!strcmp(attribute->attrName, "saSmfCmpgFileUri")) {
			char *fileName = *((char **)value);
			m_cmpgFileUri = fileName;
			TRACE("modyfied saSmfCmpgFileUri = %s", fileName);

			/* Change state to initial */
			m_cmpgState = SA_SMF_CMPG_INITIAL;
			updateImmAttr(this->getDn().c_str(), (char*)"saSmfCmpgState", SA_IMM_ATTR_SAUINT32T, &m_cmpgState);
		} else {
			LOG_ER("modifying invalid attribute %s", attribute->attrName);
			rc = SA_AIS_ERR_BAD_OPERATION;
			goto done;
		}

		attrMod = attrMods[i++];
	}
 done:
	return rc;
}

/** 
 * init
 * executes the attribute initialization
 */
SaAisErrorT 
SmfCampaign::init(const SaImmAttrValuesT_2 ** attrValues)
{
	TRACE_ENTER();
	const SaImmAttrValuesT_2 **attribute;

	for (attribute = attrValues; *attribute != NULL; attribute++) {
		void *value;

		TRACE("init attribute = %s", (*attribute)->attrName);

		if ((*attribute)->attrValuesNumber != 1) {
			TRACE("invalid number of values %u for %s", (*attribute)->attrValuesNumber,
			      (*attribute)->attrName);
			continue;
		}

		value = (*attribute)->attrValues[0];

		if (strcmp((*attribute)->attrName, "safSmfCampaign") == 0) {
			char *rdn = *((char **)value);
			m_cmpg = rdn;
			TRACE("init safSmfCampaign = %s", rdn);
		} else if (strcmp((*attribute)->attrName, "saSmfCmpgFileUri") == 0) {
			char *fileName = *((char **)value);
			m_cmpgFileUri = fileName;
			TRACE("init saSmfCmpgFileUri = %s", fileName);
		} else if (strcmp((*attribute)->attrName, "saSmfCmpgState") == 0) {
			unsigned int state = *((unsigned int *)value);

			if ((state >= SA_SMF_CMPG_INITIAL) && (state <= SA_SMF_CMPG_ROLLBACK_FAILED)) {
				m_cmpgState = (SaSmfCmpgStateT) state;
			} else {
				LOG_ER("invalid state %u", state);
				m_cmpgState = SA_SMF_CMPG_INITIAL;
			}
			TRACE("init saSmfCmpgState = %d", (int)m_cmpgState);
		} else if (strcmp((*attribute)->attrName, "saSmfCmpgConfigBase")
			   == 0) {
			SaTimeT time = *((SaTimeT *) value);
			m_cmpgConfigBase = time;

			TRACE("init saSmfCmpgConfigBase = %llu", m_cmpgConfigBase);
		} else if (strcmp((*attribute)->attrName, "saSmfCmpgExpectedTime")
			   == 0) {
			SaTimeT time = *((SaTimeT *) value);
			m_cmpgExpectedTime = time;

			TRACE("init saSmfCmpgExpectedTime = %llu", m_cmpgExpectedTime);
		} else if (strcmp((*attribute)->attrName, "saSmfCmpgElapsedTime")
			   == 0) {
			SaTimeT time = *((SaTimeT *) value);
			m_cmpgElapsedTime = time;

			TRACE("init saSmfCmpgElapsedTime = %llu", m_cmpgElapsedTime);
		} else if (strcmp((*attribute)->attrName, "saSmfCmpgError") == 0) {
			char *error = *((char **)value);
			m_cmpgError = error;
			TRACE("init saSmfCmpgError = %s", error);
		} else {
			TRACE("init unknown attribute = %s", (*attribute)->attrName);
		}
	}

	TRACE_LEAVE();
	return SA_AIS_OK;
}

/** 
 * adminOperation
 * Executes the administatrative operation
 */
SaAisErrorT 
SmfCampaign::adminOperation(const SaImmAdminOperationIdT opId, const SaImmAdminOperationParamsT_2 ** params)
{
	if (SmfCampaignThread::instance() != NULL) {
		if (SmfCampaignThread::instance()->campaign() != this) {
			LOG_ER("Another campaign is executing %s",
			       SmfCampaignThread::instance()->campaign()->getDn().c_str());
			return SA_AIS_ERR_BAD_OPERATION;
		}
	}

	TRACE("Received admin operation %llu", opId);

	switch (opId) {
	case SA_SMF_ADMIN_EXECUTE:
		{
			switch (m_cmpgState) {
			case SA_SMF_CMPG_INITIAL:
				if (SmfCampaignThread::instance() != NULL) {
					LOG_NO("Campaign execute operation rejected, prerequisites check/backup ongoing (%s)",
					       SmfCampaignThread::instance()->campaign()->getDn().c_str());
					return SA_AIS_ERR_BUSY;
				}
				break;
			case SA_SMF_CMPG_EXECUTION_SUSPENDED:
				break;
			default:
				{
					LOG_NO("Campaign execute operation rejected, wrong state (%u)", m_cmpgState);
					return SA_AIS_ERR_BAD_OPERATION;
				}
			}

			if (SmfCampaignThread::instance() == NULL) {
				TRACE("Starting campaign thread %s", this->getDn().c_str());
				if (SmfCampaignThread::start(this) != 0) {
					LOG_ER("Failed to start campaign");
					return SA_AIS_ERR_CAMPAIGN_ERROR_DETECTED;
				}
			}
			TRACE("Sending execute event to thread");
			CAMPAIGN_EVT *evt = new CAMPAIGN_EVT();
			evt->type = CAMPAIGN_EVT_EXECUTE;
			SmfCampaignThread::instance()->send(evt);
			break;
		}

	case SA_SMF_ADMIN_ROLLBACK:
		{
			/* TODO remove lines below when rollback is implemented */
			LOG_ER("Rollback is not yet implemented");
			return SA_AIS_ERR_BAD_OPERATION;

			switch (m_cmpgState) {
			case SA_SMF_CMPG_EXECUTION_COMPLETED:
			case SA_SMF_CMPG_ERROR_DETECTED:
			case SA_SMF_CMPG_EXECUTION_SUSPENDED:
			case SA_SMF_CMPG_ROLLBACK_SUSPENDED:
				break;
			default:
				{
					LOG_ER("Failed to rollback campaign, wrong state %u", m_cmpgState);
					return SA_AIS_ERR_CAMPAIGN_ERROR_DETECTED;
				}
			}

			if (SmfCampaignThread::instance() == NULL) {
				LOG_ER("Failed to rollback campaign, campaign not executing");
				return SA_AIS_ERR_CAMPAIGN_ERROR_DETECTED;
			}

			TRACE("Sending rollback event to thread");
			CAMPAIGN_EVT *evt = new CAMPAIGN_EVT();
			evt->type = CAMPAIGN_EVT_ROLLBACK;
			SmfCampaignThread::instance()->send(evt);
			break;
		}

	case SA_SMF_ADMIN_SUSPEND:
		{
			switch (m_cmpgState) {
			case SA_SMF_CMPG_EXECUTING:
			case SA_SMF_CMPG_ROLLING_BACK:
				break;
			default:
				{
					LOG_ER("Failed to suspend campaign, wrong state %u", m_cmpgState);
					return SA_AIS_ERR_CAMPAIGN_ERROR_DETECTED;
				}
			}

			if (SmfCampaignThread::instance() == NULL) {
				LOG_ER("Failed to suspend campaign, campaign not executing");
				return SA_AIS_ERR_CAMPAIGN_ERROR_DETECTED;
			}

			TRACE("Sending suspend event to thread");
			CAMPAIGN_EVT *evt = new CAMPAIGN_EVT();
			evt->type = CAMPAIGN_EVT_SUSPEND;
			SmfCampaignThread::instance()->send(evt);
			break;
		}

	case SA_SMF_ADMIN_COMMIT:
		{
			switch (m_cmpgState) {
			case SA_SMF_CMPG_EXECUTION_COMPLETED:
			case SA_SMF_CMPG_ROLLBACK_COMPLETED:
				break;
			default:
				{
					LOG_ER("Failed to commit campaign, wrong state %u", m_cmpgState);
					return SA_AIS_ERR_CAMPAIGN_ERROR_DETECTED;
				}
			}

			if (SmfCampaignThread::instance() == NULL) {
				LOG_ER("Failed to commit campaign, campaign not executing");
				return SA_AIS_ERR_CAMPAIGN_ERROR_DETECTED;
			}

			TRACE("Sending commit event to thread");
			CAMPAIGN_EVT *evt = new CAMPAIGN_EVT();
			evt->type = CAMPAIGN_EVT_COMMIT;
			SmfCampaignThread::instance()->send(evt);
			break;
		}

	default:
		{
			LOG_ER("adminOperation, unknown operation %llu", opId);
			return SA_AIS_ERR_BAD_OPERATION;
		}
	}
	return SA_AIS_OK;
}

/** 
 * adminOpExecute
 * Executes the administrative operation Execute
 */
SaAisErrorT 
SmfCampaign::adminOpExecute(void)
{
	if (getUpgradeCampaign() == NULL) {
		//Check if campaign file exist
		struct stat filestat;
		if (stat(m_cmpgFileUri.c_str(), &filestat) == -1) {
			std::string error = "Campaign file does not exist " + m_cmpgFileUri;
			LOG_ER("%s", error.c_str());
			setError(error);
			setState(SA_SMF_CMPG_INITIAL);	/* Set initial state to allow reexecution */
			return SA_AIS_OK;
		}

		//The directory where the campaign.xml file is stored
		std::string path = m_cmpgFileUri.substr( 0, m_cmpgFileUri.find_last_of( '/' ) + 1);
		setCampaignXmlDir(path);

		//Parse campaign file
		SmfCampaignXmlParser parser;
		SmfUpgradeCampaign *p_uc = parser.parseCampaignXml(m_cmpgFileUri);
		if (p_uc == NULL) {
			std::string error = "Error when parsing the campaign file " + m_cmpgFileUri;
			LOG_ER("%s", error.c_str());
			setError(error);
			setState(SA_SMF_CMPG_INITIAL);	/* Set initial state to allow reexecution */
			return SA_AIS_OK;
		}

		//Initiate the campaign with the current campaign object state
		//The SmfUpgradeCampaign needs to be set to the right SmfCampState
		//to execute properly in case of a switchower
		p_uc->setCampState(getState());

		setUpgradeCampaign(p_uc);
	}

	//Execute campaign
	TRACE("adminOpExecute, Executing campaign %s", getUpgradeCampaign()->getCampaignName().c_str());
	getUpgradeCampaign()->execute();
	return SA_AIS_OK;
}

/** 
 * adminOpSuspend
 * Executes the administrative operation Suspend
 */
SaAisErrorT 
SmfCampaign::adminOpSuspend(void)
{
	getUpgradeCampaign()->suspend();
	return SA_AIS_OK;
}

/** 
 * adminOpCommit
 * Executes the administrative operation Commit
 */
SaAisErrorT 
SmfCampaign::adminOpCommit(void)
{
	getUpgradeCampaign()->commit();
	return SA_AIS_OK;
}

/** 
 * adminOpRollback
 * Executes the administrative operation Rollback
 */
SaAisErrorT 
SmfCampaign::adminOpRollback(void)
{
	getUpgradeCampaign()->rollback();
	return SA_AIS_OK;
}

/** 
 * procResult
 * Takes care of procedure result
 */
void 
SmfCampaign::procResult(SmfUpgradeProcedure * procedure, int rc)
//SmfCampaign::procResult(SmfUpgradeProcedure* procedure, PROCEDURE_RESULT rc)
{
	TRACE("procResult, Received Procedure result %s : %u", procedure->getProcName().c_str(), rc);

	/* Execute next procedure or campaign wrapup */
	TRACE("procResult, Continue executing campaign procedures %s", m_upgradeCampaign->getCampaignName().c_str());
	getUpgradeCampaign()->executeProc();
}

/** 
 * state
 * Sets new state and updates IMM
 */
void 
SmfCampaign::setState(SaSmfCmpgStateT state)
{
	TRACE("Update campaign state from %u to %u", m_cmpgState, state);

	m_cmpgState = state;

	updateImmAttr(this->getDn().c_str(), (char*)"saSmfCmpgState", SA_IMM_ATTR_SAUINT32T, &m_cmpgState);

	SmfCampaignThread::instance()->sendStateNotification(m_dn, MINOR_ID_CAMPAIGN, SA_NTF_MANAGEMENT_OPERATION,
							     SA_SMF_CAMPAIGN_STATE, m_cmpgState);
}
/** 
 * state
 * Get state
 */
SaSmfCmpgStateT 
SmfCampaign::getState()
{
	return m_cmpgState;
}

/** 
 * setConfigBase
 * Sets new config base and updates IMM
 */
void 
SmfCampaign::setConfigBase(SaTimeT configBase)
{
	m_cmpgConfigBase = configBase;
	updateImmAttr(this->getDn().c_str(), (char*)"saSmfCmpgConfigBase", SA_IMM_ATTR_SATIMET, &m_cmpgConfigBase);
}

/** 
 * setExpectedTime
 * Sets new expected time and updates IMM
 */
void 
SmfCampaign::setExpectedTime(SaTimeT expectedTime)
{
	m_cmpgExpectedTime = expectedTime;
	updateImmAttr(this->getDn().c_str(), (char*)"saSmfCmpgExpectedTime", SA_IMM_ATTR_SATIMET, &m_cmpgExpectedTime);
}

/** 
 * setElapsedTime
 * Sets new elapsed time and updates IMM
 */
void 
SmfCampaign::setElapsedTime(SaTimeT elapsedTime)
{
	m_cmpgElapsedTime = elapsedTime;
	updateImmAttr(this->getDn().c_str(), (char*)"saSmfCmpgElapsedTime", SA_IMM_ATTR_SATIMET, &m_cmpgElapsedTime);
}

/** 
 * setError
 * Sets new error string and updates IMM
 */
void 
SmfCampaign::setError(const std::string & error)
{
	m_cmpgError = error;
	const char *errorStr = m_cmpgError.c_str();

	updateImmAttr(this->getDn().c_str(), (char*)"saSmfCmpgError", SA_IMM_ATTR_SASTRINGT, &errorStr);
}

/** 
 * setUpgradeCampaign
 * Sets the pointer to the campaign object created from the campaign xml
 */
void 
SmfCampaign::setUpgradeCampaign(SmfUpgradeCampaign * i_campaign)
{
	m_upgradeCampaign = i_campaign;
}

/** 
 * getUpgradeCampaign
 * Gets the pointer to the campaign object created from the campaign xml
 */
SmfUpgradeCampaign *
SmfCampaign::getUpgradeCampaign()
{
	return m_upgradeCampaign;
}

/** 
 * setCampaignXmlDir
 * Set the directory of the campaign.xml file used
 */
void 
SmfCampaign::setCampaignXmlDir(std::string i_path)
{
	m_campaignXmlDir = i_path;
}

/** 
 * getCampaignXmlDir
 * Get the directory of the campaign.xml file used
 */
const std::string
SmfCampaign::getCampaignXmlDir()
{
	return m_campaignXmlDir;
}

/*====================================================================*/
/*  Class SmfCampaignList                                             */
/*====================================================================*/

SmfCampaignList *SmfCampaignList::s_instance = NULL;

/** 
 * CampaignList::instance
 * creates (if necessary) and returns the only instance of
 * CampaignList.
 */
SmfCampaignList *
SmfCampaignList::instance(void)
{
	if (s_instance == NULL) {
		s_instance = new SmfCampaignList();
	}
	return s_instance;
}

/** 
 * Constructor
 */
SmfCampaignList::SmfCampaignList()
{
}

/** 
 * Destructor
 */
SmfCampaignList::~SmfCampaignList()
{
	this->cleanup();
}

/** 
 * get
 */
SmfCampaign *
SmfCampaignList::get(const SaNameT * dn)
{
	std::list < SmfCampaign * >::iterator it = m_campaignList.begin();

	while (it != m_campaignList.end()) {
		SmfCampaign *campaign = *it;
		if (strcmp(campaign->getDn().c_str(), (char *)dn->value) == 0) {
			return campaign;
		}
		it++;
	}

	return NULL;
}

/** 
 * add
 */
SaAisErrorT 
SmfCampaignList::add(SmfCampaign * newCampaign)
{
	m_campaignList.push_back(newCampaign);

	return SA_AIS_OK;
}

/** 
 * del
 */
SaAisErrorT 
SmfCampaignList::del(const SaNameT * dn)
{
	std::list < SmfCampaign * >::iterator it = m_campaignList.begin();

	while (it != m_campaignList.end()) {
		SmfCampaign *campaign = *it;
		if (strcmp(campaign->getDn().c_str(), (char *)dn->value) == 0) {
			delete campaign;
			m_campaignList.erase(it);
			return SA_AIS_OK;
		}
		it++;
	}

	return SA_AIS_ERR_NOT_EXIST;
}

/** 
 * cleanup
 */
void 
SmfCampaignList::cleanup(void)
{
	TRACE_ENTER();
	std::list < SmfCampaign * >::iterator it = m_campaignList.begin();

	while (it != m_campaignList.end()) {
		SmfCampaign *campaign = *it;
		delete campaign;
		it++;
	}

       	m_campaignList.clear();
	TRACE_LEAVE();
}
