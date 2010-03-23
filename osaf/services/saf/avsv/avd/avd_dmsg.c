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

  This module does the encode/decode of messages(heartbeat) between the two
  Availability Directors.

..............................................................................

  FUNCTIONS INCLUDED in this module:

  
******************************************************************************
*/

/*
 * Module Inclusion Control...
 */

#include "avd.h"
#include "logtrace.h"

/*****************************************************************************
 * Function: avd_mds_d_enc
 *
 * Purpose:  This is a callback routine that is invoked to encode
 *           AvD-AvD messages(heartbeat).
 *
 * Input: 
 *
 * Returns: None. 
 *
 * NOTES: None.
 *
 * 
 **************************************************************************/

void avd_mds_d_enc(MDS_CALLBACK_ENC_INFO *enc_info)
{
	NCS_UBAID *uba = NULL;
	uns8 *data;
	AVD_D2D_MSG *msg = 0;

	m_AVD_LOG_FUNC_ENTRY("avd_mds_d_enc");

	msg = (AVD_D2D_MSG *)enc_info->i_msg;
	uba = enc_info->io_uba;

	data = ncs_enc_reserve_space(uba, 3 * sizeof(uns32));
	ncs_encode_32bit(&data, msg->msg_type);
	if (AVD_D2D_HEARTBEAT_MSG == msg->msg_type ) {
		ncs_encode_32bit(&data, msg->msg_info.d2d_hrt_bt.node_id);
		ncs_encode_32bit(&data, msg->msg_info.d2d_hrt_bt.avail_state);
	} else if (AVD_D2D_CHANGE_ROLE_REQ == msg->msg_type) {
		ncs_encode_32bit(&data, msg->msg_info.d2d_chg_role_req.cause);
		ncs_encode_32bit(&data, msg->msg_info.d2d_chg_role_req.role);
	} else {
		ncs_encode_32bit(&data, msg->msg_info.d2d_chg_role_rsp.role);
		ncs_encode_32bit(&data, msg->msg_info.d2d_chg_role_rsp.status);
	}
	ncs_enc_claim_space(uba, 3 * sizeof(uns32));

	m_AVD_LOG_MDS_SUCC(AVSV_LOG_MDS_ENC_CBK);
	return;

}

/*****************************************************************************
 * Function: avd_mds_d_dec
 *
 * Purpose:  This is a callback routine that is invoked to decode
 *           AvD-AvD messages(heartbeat).
 *
 * Input: 
 *
 * Returns: 
 *
 * NOTES:
 *
 * 
 **************************************************************************/

void avd_mds_d_dec(MDS_CALLBACK_DEC_INFO *dec_info)
{
	uns8 *data;
	uns8 data_buff[24];
	AVD_D2D_MSG *d2d_msg = 0;
	NCS_UBAID *uba = dec_info->io_uba;

	m_AVD_LOG_FUNC_ENTRY("avd_mds_d_dec");

	d2d_msg = calloc(1, sizeof(AVD_D2D_MSG));
	if (d2d_msg == AVD_D2D_MSG_NULL) {
		/* log error that the director is in degraded situation */
		m_AVD_LOG_MEM_FAIL_LOC(AVD_D2D_MSG_ALLOC_FAILED);
		return;
	}

	data = ncs_dec_flatten_space(uba, data_buff, 3 * sizeof(uns32));
	d2d_msg->msg_type = ncs_decode_32bit(&data);

	if (AVD_D2D_HEARTBEAT_MSG == d2d_msg->msg_type) {
		d2d_msg->msg_info.d2d_hrt_bt.node_id = ncs_decode_32bit(&data);
		d2d_msg->msg_info.d2d_hrt_bt.avail_state = ncs_decode_32bit(&data);
	} else if (AVD_D2D_CHANGE_ROLE_REQ == d2d_msg->msg_type) {
		d2d_msg->msg_info.d2d_chg_role_req.cause = ncs_decode_32bit(&data);
		d2d_msg->msg_info.d2d_chg_role_req.role = ncs_decode_32bit(&data);
	} else {
		d2d_msg->msg_info.d2d_chg_role_rsp.role = ncs_decode_32bit(&data);
		d2d_msg->msg_info.d2d_chg_role_rsp.status = ncs_decode_32bit(&data);
	}       

	ncs_dec_skip_space(uba, 3 * sizeof(uns32));
	dec_info->o_msg = (NCSCONTEXT)d2d_msg;

}

/****************************************************************************
  Name          : avd_d2d_msg_snd

  Description   : This is a routine invoked by AVD for transmitting
  messages to the other director.
 
  Arguments     : cb     :  The control block of AvD.
                  snd_msg: the send message(heartbeat) that needs to be sent.
                  
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/

uns32 avd_d2d_msg_snd(AVD_CL_CB *cb, AVD_D2D_MSG *snd_msg)
{
	NCSMDS_INFO snd_mds;
	uns32 rc;

	m_AVD_LOG_FUNC_ENTRY("avd_d2d_msg_snd");
	m_AVD_LOG_MSG_DND_DUMP(NCSFL_SEV_DEBUG, snd_msg, sizeof(AVD_D2D_MSG), snd_msg);

	memset(&snd_mds, '\0', sizeof(NCSMDS_INFO));

	snd_mds.i_mds_hdl = cb->adest_hdl;
	snd_mds.i_svc_id = NCSMDS_SVC_ID_AVD;
	snd_mds.i_op = MDS_SEND;
	snd_mds.info.svc_send.i_msg = (NCSCONTEXT)snd_msg;
	snd_mds.info.svc_send.i_to_svc = NCSMDS_SVC_ID_AVD;
	snd_mds.info.svc_send.i_priority = MDS_SEND_PRIORITY_HIGH;
	snd_mds.info.svc_send.i_sendtype = MDS_SENDTYPE_SND;
	snd_mds.info.svc_send.info.snd.i_to_dest = cb->other_avd_adest;

	/*
	 * Now do MDS send.
	 */
	if ((rc = ncsmds_api(&snd_mds)) != NCSCC_RC_SUCCESS) {
		m_AVD_LOG_MDS_ERROR(AVSV_LOG_MDS_SEND);
		return rc;
	}

	return rc;

}

/****************************************************************************
  Name          : avd_d2d_msg_rcv
 
  Description   : This routine is invoked by AvD when a message arrives
  from AvD. It converts the message to the corresponding event and posts
  the message to the mailbox for processing by the HB loop.
 
  Arguments     : rcv_msg    -  ptr to the received message
 
  Return Values : NCSCC_RC_SUCCESS/NCSCC_RC_FAILURE
 
  Notes         : None.
******************************************************************************/

uns32 avd_d2d_msg_rcv(AVD_D2D_MSG *rcv_msg)
{
	AVD_EVT *evt = AVD_EVT_NULL;
	AVD_CL_CB *cb = avd_cb;

	m_AVD_LOG_FUNC_ENTRY("avd_d2d_msg_rcv");

	/* check that the message ptr is not NULL */
	if (rcv_msg == NULL) {
		m_AVD_LOG_INVALID_VAL_ERROR(0);
		return NCSCC_RC_FAILURE;
	}

	if (AVD_D2D_HEARTBEAT_MSG == rcv_msg->msg_type) {
		/* create the message event */
		evt = calloc(1, sizeof(AVD_EVT));
		if (evt == AVD_EVT_NULL) {
			/* log error */
			m_AVD_LOG_MEM_FAIL_LOC(AVD_EVT_ALLOC_FAILED);
			/* free the message and return */
			avsv_d2d_msg_free(rcv_msg);
			return NCSCC_RC_FAILURE;
		}

		m_AVD_LOG_RCVD_VAL(((long)evt));

		evt->info.avd_msg = rcv_msg;
		evt->rcv_evt = AVD_EVT_D_HB;
		if (m_NCS_IPC_SEND(&cb->avd_hb_mbx, evt, NCS_IPC_PRIORITY_VERY_HIGH)
	    			!= NCSCC_RC_SUCCESS) {
			m_AVD_LOG_MBX_ERROR(AVSV_LOG_MBX_SEND);
			/* log error */
			/* free the message */
			avsv_d2d_msg_free(rcv_msg);
			evt->info.avd_msg = NULL;
			/* free the event and return */
			free(evt);
			return NCSCC_RC_FAILURE;
		}		
	} else {
		if (AVD_D2D_CHANGE_ROLE_REQ == rcv_msg->msg_type) {
			if (SA_AMF_HA_ACTIVE == rcv_msg->msg_info.d2d_chg_role_req.role) {
				cb->swap_switch = SA_TRUE;
				LOG_NO("amfd role change req stdby -> actv, posting to mail box");
				avd_post_amfd_switch_role_change_evt(cb, SA_AMF_HA_ACTIVE);
			} else {
				/* We will never come here, request only comes for active change role */
				assert(0);
			}
		} else {
			/* Response for the request */
			if (SA_AMF_HA_ACTIVE == rcv_msg->msg_info.d2d_chg_role_rsp.role) {
				if (NCSCC_RC_SUCCESS == rcv_msg->msg_info.d2d_chg_role_rsp.status) {
					/* Peer AMFD went to Active state successfully, make local AVD standby */
					LOG_NO("amfd role change rsp stdby -> actv succ, posting to mail box for qsd -> stdby");
					avd_post_amfd_switch_role_change_evt(cb, SA_AMF_HA_STANDBY);
				} else {
					/* Other AMFD rejected the active request, move the local amfd back to Active from qsd */ 
					LOG_ER("amfd role change rsp stdby -> actv fail, posting to mail box for qsd -> actv");
					avd_post_amfd_switch_role_change_evt(cb, SA_AMF_HA_ACTIVE);
				}
			} else {
				/* We should never fell into this else case */
				assert(0);
			}
		}
		avsv_d2d_msg_free(rcv_msg);
	}

	m_AVD_LOG_MBX_SUCC(AVSV_LOG_MBX_SEND);

	m_AVD_LOG_MDS_SUCC(AVSV_LOG_MDS_RCV_CBK);
	return NCSCC_RC_SUCCESS;
}

/****************************************************************************
  Name          : avsv_d2d_msg_free 
 
  Description   : This routine is invoked by AvD when a d2d_msg is to be
                  freed.

  Arguments     : d2d_msg    -  ptr to the message which has to be freed
 
  Return Values : None. 
 
  Notes         : None.
******************************************************************************/

void avsv_d2d_msg_free(AVD_D2D_MSG *d2d_msg)
{
	free(d2d_msg);
	d2d_msg = NULL;
}
