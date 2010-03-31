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

  This include file contains the message definitions for AvD and AvND
  communication.
  
******************************************************************************
*/

/*
 * Module Inclusion Control...
 */
#ifndef AVSV_D2NMSG_H
#define AVSV_D2NMSG_H

#include <saClm.h>
#include <mds_papi.h>
#include <avsv_defs.h>

/* In Service upgrade support */
#define AVND_MDS_SUB_PART_VERSION   2

/* Message format versions */
#define AVSV_AVD_AVND_MSG_FMT_VER_1    1
#define AVSV_AVD_AVND_MSG_FMT_VER_2    2

/* Internode/External Components Validation result */
typedef enum {
	AVSV_VALID_SUCC_COMP_NODE_UP = 1,	/* Component is configured and so, valid. And
						   the component node is UP and running. */
	AVSV_VALID_SUCC_COMP_NODE_DOWN,	/* Component is configured and so, valid. And
					   the component node is not UP. */
	AVSV_VALID_FAILURE,	/* Component not configured, validation failed. */
	AVSV_VALID_MAX
} AVSV_COMP_VALIDATION_RESULT_TYPE;

typedef enum {
	AVSV_N2D_CLM_NODE_UP_MSG = 1,
	AVSV_N2D_REG_SU_MSG,
	AVSV_N2D_REG_COMP_MSG,
	AVSV_N2D_OPERATION_STATE_MSG,
	AVSV_N2D_INFO_SU_SI_ASSIGN_MSG,
	AVSV_N2D_PG_TRACK_ACT_MSG,
	AVSV_N2D_OPERATION_REQUEST_MSG,
	AVSV_N2D_DATA_REQUEST_MSG,
	AVSV_N2D_SHUTDOWN_APP_SU_MSG,
	AVSV_N2D_VERIFY_ACK_NACK_MSG,
	AVSV_D2N_CLM_NODE_UP_MSG,
	AVSV_D2N_REG_SU_MSG,
	AVSV_D2N_REG_COMP_MSG,
	AVSV_D2N_INFO_SU_SI_ASSIGN_MSG,
	AVSV_D2N_PG_TRACK_ACT_RSP_MSG,
	AVSV_D2N_PG_UPD_MSG,
	AVSV_D2N_OPERATION_REQUEST_MSG,
	AVSV_D2N_PRESENCE_SU_MSG,
	AVSV_D2N_DATA_VERIFY_MSG,
	AVSV_D2N_DATA_ACK_MSG,
	AVSV_D2N_SHUTDOWN_APP_SU_MSG,
	AVSV_D2N_SET_LEDS_MSG,
	AVSV_D2N_COMP_VALIDATION_RESP_MSG,
	AVSV_D2N_ROLE_CHANGE_MSG,
	AVSV_D2N_ADMIN_OP_REQ_MSG,
	AVSV_N2D_COMP_VALIDATION_MSG,
	AVSV_DND_MSG_MAX
} AVSV_DND_MSG_TYPE;

typedef struct avsv_clm_info {
	SaClmNodeIdT node_id;	/* node id */
	SaClmNodeAddressT node_address;	/* node address */
	SaNameT node_name;	/* node name */
	SaBoolT member;		/* indicates if node is a 
				   member of the cluster */
	SaTimeT boot_timestamp;	/* node boot timestamp  */
	SaUint64T view_number;	/* view number when the node 
				   joined/left the cluster */
} AVSV_CLM_INFO;

typedef struct avsv_clm_info_msg {
	AVSV_CLM_INFO clm_info;
	struct avsv_clm_info_msg *next;
} AVSV_CLM_INFO_MSG;

typedef struct avsv_hlt_key_tag {
	SaNameT comp_name;
	uns32 key_len; 	                /* healthCheckKey length */
	SaAmfHealthcheckKeyT name;	/* name of the health check key.
					 * This field should be
					 * immediatly after key_len */
} AVSV_HLT_KEY;

typedef struct avsv_hlt_info_msg {
	struct avsv_hlt_key_tag name;
	SaTimeT period;
	SaTimeT max_duration;
	NCS_BOOL is_ext;	/* Whether this HC is for external comp */
	struct avsv_hlt_info_msg *next;
} AVSV_HLT_INFO_MSG;

typedef struct avsv_su_info_msg {
	SaNameT name;
	uns32 num_of_comp;
	SaTimeT comp_restart_prob;
	uns32 comp_restart_max;
	SaTimeT su_restart_prob;
	uns32 su_restart_max;
	SaBoolT is_ncs;
	NCS_BOOL su_is_external;	/*indicates if this SU is external */
	struct avsv_su_info_msg *next;
} AVSV_SU_INFO_MSG;

typedef struct avsv_comp_info_tag {
	SaNameT name;	/* component name */

	saAmfCompCapabilityModelT cap;	/* component capability. See sec 4.6 of
					 * Saf AIS 
					 * Checkpointing - Sent as a one time update.
					 */

	NCS_COMP_TYPE_VAL category;	/* component category . 
					 * saAware, proxiedLocalPreinstantiable,
					 * proxiedLocalNonPreinstantiable,externalPreinstantiable or 
					 * externalNonPreinstantiable,unproxiedSaUnaware.
					 * Checkpointing - Sent as a one time update.
					 */

	uns32 init_len;		/* length of the component initiation
				 *information 
				 * Checkpointing - Sent as a one time update.
				 */

	char init_info[AVSV_MISC_STR_MAX_SIZE];	/* ASCII string of information for
						 * initialization of component 
						 * Checkpointing - Sent as a one time update.
						 */
	char init_cmd_arg_info[AVSV_MISC_STR_MAX_SIZE];

	SaTimeT init_time;	/* Time interval within which
				 * instantiate command should
				 * complete.
				 * Checkpointing - Sent as a one time update.
				 */

	uns32 term_len;		/* length of the component termination
				 * information 
				 * Checkpointing - Sent as a one time update.
				 */

	char term_info[AVSV_MISC_STR_MAX_SIZE];	/* ASCII string of information for
						 * termination of component 
						 * Checkpointing - Sent as a one time update.
						 */
	char term_cmd_arg_info[AVSV_MISC_STR_MAX_SIZE];

	SaTimeT term_time;	/* Time interval within which
				 * terminate command should
				 * complete.
				 * Checkpointing - Sent as a one time update.
				 */

	uns32 clean_len;	/* length of the component cleanup
				 * information 
				 * Checkpointing - Sent as a one time update.
				 */

	char clean_info[AVSV_MISC_STR_MAX_SIZE];	/* ASCII string of information for cleanup
							 * of component 
							 * Checkpointing - Sent as a one time update.
							 */
	char clean_cmd_arg_info[AVSV_MISC_STR_MAX_SIZE];

	SaTimeT clean_time;	/* Time interval within which
				 * cleanup command should
				 * complete. 
				 * Checkpointing - Sent as a one time update.
				 */

	uns32 amstart_len;	/* length of the AM start
				 * command String information
				 * Checkpointing - Sent as a one time update.
				 */

	char amstart_info[AVSV_MISC_STR_MAX_SIZE];	/* ASCII 
							 * string of information for
							 * AM start of a component
							 * Checkpointing - Sent as a one time update.
							 */
	char amstart_cmd_arg_info[AVSV_MISC_STR_MAX_SIZE];

	SaTimeT amstart_time;	/* Time interval within which
				 * AM start command should
				 * complete.
				 * Checkpointing - Sent as a one time update.
				 */

	uns32 amstop_len;	/* length of the AM stop
				 * command String information 
				 * Checkpointing - Sent as a one time update.
				 */

	char amstop_info[AVSV_MISC_STR_MAX_SIZE];	/* ASCII 
							 * string of information for
							 * AM start of a component.
							 * Checkpointing - Sent as a one time update.
							 */
	char amstop_cmd_arg_info[AVSV_MISC_STR_MAX_SIZE];

	SaTimeT amstop_time;	/* Time interval within which
				 * AM stop command should
				 * complete.
				 * Checkpointing - Sent as a one time update.
				 */

	SaTimeT terminate_callback_timeout;	/* Time in which AMF expects an SaAmfResponse call
						 * from the component after a preceding callback initiation.
						 * A value of 0 means AMF should use default value.
						 */
	SaTimeT csi_set_callback_timeout;	/* Time in which AMF expects an SaAmfResponse call
						 * from the component after a preceding CSI set callback initiation.
						 * A value of 0 means AMF should use default value.
						 */
	SaTimeT quiescing_complete_timeout;	/* Time in which AMF expects an saAmfCSIQuiescingComplete call
						 * from the component after a preceding CSI set callback initiation.
						 * A value of 0 means AMF should use default value.
						 * Note: this is not a timeout for saAMFResponse
						 */
	SaTimeT csi_rmv_callback_timeout;	/* Time in which AMF expects an SaAmfResponse call
						 * from the component after a preceding CSI Remove callback initiation.
						 * A value of 0 means AMF should use default value.
						 */
	SaTimeT proxied_inst_callback_timeout;	/* Time in which AMF expects an SaAmfResponse call
						 * from the component after a preceding  proxied component
						 * instantiate callback initiation.
						 * A value of 0 means AMF should use default value.
						 */
	SaTimeT proxied_clean_callback_timeout;	/* Time in which AMF expects an SaAmfResponse call
						 * from the component after a preceding  proxied component
						 * cleanup callback initiation.
						 * A value of 0 means AMF should use default value.
						 */
	NCS_BOOL am_enable;	/* Enable flag for AM.it also
				 * acts as a trigger for start
				 *  /stop EAM.
				 * Checkpointing - Sent as a one time update.*/
	uns32 max_num_amstart;	/* The maximum number of times
				   AMF tries to start EAM for 
				   for the comp. */
	uns32 max_num_amstop;	/* the maximum number of times
				 * AMF tries to AM start command
				 * for the component. 
				 * Checkpointing - Sent as a one time update.*/
	uns32 inst_level;	/* The components instantiation
				 * level in the SU.*/

	SaAmfRecommendedRecoveryT def_recvr;	/* the default component recovery
						 * Checkpointing - Sent as a one time update.
						 */

	uns32 max_num_inst;	/* the maximum number of times
				 * AMF tries to instantiate
				 * the component.
				 * Checkpointing - Sent as a one time update.
				 */

	NCS_BOOL comp_restart;	/* disables restarting the
				 * component as part of
				 * recovery.
				 * Checkpointing - Sent as a one time update.
				 */
	char comp_cmd_env_info[AVSV_MISC_STR_MAX_SIZE];

} AVSV_COMP_INFO;

typedef struct avsv_comp_info_msg {
	AVSV_COMP_INFO comp_info;
	struct avsv_comp_info_msg *next;
} AVSV_COMP_INFO_MSG;

typedef struct avsv_susi_asgn {
	SaNameT comp_name;
	SaNameT csi_name;
	SaNameT active_comp_name;
	uns32 csi_rank;		/* The rank of the CSI in the SI */
	uns32 stdby_rank;
	SaAmfCSITransitionDescriptorT active_comp_dsc;
	NCS_AVSV_CSI_ATTRS attrs;	/* Inside the struct there is a 
					 * array of param vals.*/
	struct avsv_susi_asgn *next;
} AVSV_SUSI_ASGN;

typedef enum {
	AVSV_OBJ_OPR_MOD = 1,
	AVSV_OBJ_OPR_DEL
} AVSV_OBJ_OPR_ACT;

typedef struct avsv_param_info {
	uns32 class_id; /* value from enum AVSV_AMF_CLASS_ID */
	uns32 attr_id;
	SaNameT name;	/* The length field is in network order */
	SaNameT name_sec;
	AVSV_OBJ_OPR_ACT act;
	uns32 value_len;
	char value[AVSV_MISC_STR_MAX_SIZE];
} AVSV_PARAM_INFO;

/*
 * Tese values are mapped to SU_SI fsm states */
typedef enum {
	AVSV_SUSI_ACT_ASGN = 2,	/*AVD_SU_SI_STATE_ASGN */
	AVSV_SUSI_ACT_DEL = 4,	/* AVD_SU_SI_STATE_UNASGN */
	AVSV_SUSI_ACT_MOD = 5,	/* AVD_SU_SI_STATE_MODIFY */
} AVSV_SUSI_ACT;

typedef enum {
	AVSV_PG_TRACK_ACT_START,
	AVSV_PG_TRACK_ACT_STOP
} AVSV_PG_TRACK_ACT;

typedef enum {
	AVSV_AVND_CARD_PAYLOAD,
	AVSV_AVND_CARD_SYS_CON
} AVSV_AVND_CARD;

typedef struct avsv_n2d_clm_node_up_msg_info_tag {
	uns32 msg_id;
	SaClmNodeIdT node_id;
	MDS_DEST adest_address;
} AVSV_N2D_CLM_NODE_UP_MSG_INFO;

typedef struct avsv_n2d_comp_validation_msg_info_tag {
	uns32 msg_id;
	SaClmNodeIdT node_id;
	SaNameT comp_name;	/* comp name */

   /****************************************************************
          The following attributes : hdl, proxy_comp_name, mds_dest and mds_ctxt 
          wouldn't be sent to AvD, these are just to maintain the information in the 
          AvD message list when the validation response comes back.
          ***************************************************************/
	SaAmfHandleT hdl;
	SaNameT proxy_comp_name;
	MDS_DEST mds_dest;	/* we need to have mds_dest and mds_ctxt to send 
				   response back to ava, when AvD responds. */
	MDS_SYNC_SND_CTXT mds_ctxt;
   /******************************************************************/

} AVSV_N2D_COMP_VALIDATION_INFO;

typedef struct avsv_n2d_reg_su_msg_info_tag {
	uns32 msg_id;
	SaClmNodeIdT node_id;
	SaNameT su_name;
	uns32 error;
} AVSV_N2D_REG_SU_MSG_INFO;

typedef struct avsv_n2d_reg_comp_msg_info_tag {
	uns32 msg_id;
	SaClmNodeIdT node_id;
	SaNameT comp_name;
	uns32 error;
} AVSV_N2D_REG_COMP_MSG_INFO;

typedef struct avsv_n2d_operation_state_msg_info_tag {
	uns32 msg_id;
	SaClmNodeIdT node_id;
	AVSV_ERR_RCVR rec_rcvr;
	SaAmfOperationalStateT node_oper_state;
	SaNameT su_name;
	SaAmfOperationalStateT su_oper_state;
} AVSV_N2D_OPERATION_STATE_MSG_INFO;

typedef struct avsv_n2d_info_su_si_assign_msg_info_tag {
	uns32 msg_id;
	SaClmNodeIdT node_id;
	AVSV_SUSI_ACT msg_act;
	SaNameT su_name;
	SaNameT si_name;
	SaAmfHAStateT ha_state;
	uns32 error;
} AVSV_N2D_INFO_SU_SI_ASSIGN_MSG_INFO;

typedef struct avsv_n2d_pg_track_act_msg_info_tag {
	uns32 msg_id;
	SaClmNodeIdT node_id;
	NCS_BOOL msg_on_fover;	/* If TRUE indicates that message is sent on 
				 * fail-over. So AVD should process it in
				 * a special manner */
	SaNameT csi_name;
	AVSV_PG_TRACK_ACT actn;
} AVSV_N2D_PG_TRACK_ACT_MSG_INFO;

typedef struct avsv_n2d_operation_request_msg_info_tag {
	uns32 msg_id;
	SaClmNodeIdT node_id;
	AVSV_PARAM_INFO param_info;
	uns32 error;
} AVSV_N2D_OPERATION_REQUEST_MSG_INFO;

typedef struct avsv_n2d_data_request_msg_info_tag {
	uns32 msg_id;
	SaClmNodeIdT node_id;
	AVSV_PARAM_INFO param_info;
} AVSV_N2D_DATA_REQUEST_MSG_INFO;

typedef struct avsv_n2d_verify_ack_nack_msg_info {
	uns32 msg_id;
	SaClmNodeIdT node_id;
	NCS_BOOL ack;
} AVSV_N2D_VERIFY_ACK_NACK_MSG_INFO;

typedef struct avsv_d2n_clm_node_up_msg_info_tag {
	SaClmNodeIdT node_id;
	AVSV_AVND_CARD node_type;
	SaTimeT su_failover_prob;
	uns32 su_failover_max;
} AVSV_D2N_CLM_NODE_UP_MSG_INFO;

typedef struct avsv_d2n_reg_su_msg_info_tag {
	uns32 msg_id;
	NCS_BOOL msg_on_fover;	/* If 1 indicates that message is sent on 
				 * fail-over. So AVND should process it in
				 * a special manner */

	SaClmNodeIdT nodeid;
	uns32 num_su;
	AVSV_SU_INFO_MSG *su_list;
} AVSV_D2N_REG_SU_MSG_INFO;

typedef struct avsv_d2n_reg_comp_msg_info_tag {
	uns32 msg_id;
	NCS_BOOL msg_on_fover;	/* If 1 indicates that message is sent on 
				 * fail-over. So AVND should process it in
				 * a special manner */

	SaClmNodeIdT node_id;
	uns32 num_comp;
	AVSV_COMP_INFO_MSG *list;
} AVSV_D2N_REG_COMP_MSG_INFO;

typedef struct avsv_d2n_info_su_si_assign_msg_info_tag {
	uns32 msg_id;
	SaClmNodeIdT node_id;
	AVSV_SUSI_ACT msg_act;
	SaNameT su_name;
	SaNameT si_name;	/* This field is filled if the action is for a
				 * particulat SUSI. if action is for 
				 * all SIs of this SU only the SU name field
				 * is filled. */
	SaAmfHAStateT ha_state;
	uns32 num_assigns;
	AVSV_SUSI_ASGN *list;
} AVSV_D2N_INFO_SU_SI_ASSIGN_MSG_INFO;

typedef struct avsv_d2n_pg_track_act_rsp_msg_info_tag {
	uns32 msg_id_ack;
	SaClmNodeIdT node_id;
	NCS_BOOL msg_on_fover;	/* If 1 indicates that message is sent on 
				 * fail-over. So AVND should process it in
				 * a special manner */
	AVSV_PG_TRACK_ACT actn;	/* determines if rsp is sent for start/stop action */
	SaNameT csi_name;
	NCS_BOOL is_csi_exist;	/* indicates if the csi exists */
	SaAmfProtectionGroupNotificationBufferT mem_list;	/* current member list */
} AVSV_D2N_PG_TRACK_ACT_RSP_MSG_INFO;

typedef struct avsv_d2n_pg_upd_msg_info_tag {
	SaClmNodeIdT node_id;
	SaNameT csi_name;
	NCS_BOOL is_csi_del;	/* indicates if the csi is deleted */
	SaAmfProtectionGroupNotificationT mem;	/* updated member */
} AVSV_D2N_PG_UPD_MSG_INFO;

typedef struct avsv_d2n_operation_request_msg_info_tag {
	uns32 msg_id;
	SaClmNodeIdT node_id;
	AVSV_PARAM_INFO param_info;
} AVSV_D2N_OPERATION_REQUEST_MSG_INFO;

typedef struct avsv_d2n_presence_su_msg_info_tag {
	uns32 msg_id;
	SaClmNodeIdT node_id;
	SaNameT su_name;
	NCS_BOOL term_state;
} AVSV_D2N_PRESENCE_SU_MSG_INFO;

typedef struct avsv_d2n_data_verify_msg_info {
	uns32 snd_id_cnt;
	uns32 rcv_id_cnt;
	SaClmNodeIdT node_id;
	SaTimeT su_failover_prob;
	uns32 su_failover_max;
} AVSV_D2N_DATA_VERIFY_MSG_INFO;

/*
 * Message for sending Ack to all the messages received from the ND.
 */
typedef struct avsv_d2n_ack_msg {
	uns32 msg_id_ack;	/* Message ID for which Ack is being sent */
	SaClmNodeIdT node_id;
} AVSV_D2N_ACK_MSG;

typedef struct avsv_dnd_shutdown_app_su_msg_info_tag {
	uns32 msg_id;
	SaClmNodeIdT node_id;	/* strictly we dont need this too msg_type
				   is enough */
} AVSV_D2N_SHUTDOWN_APP_SU_MSG_INFO, AVSV_N2D_SHUTDOWN_APP_SU_MSG_INFO;

typedef struct avsv_d2n_set_leds_msg_info_tag {
	uns32 msg_id;
	SaClmNodeIdT node_id;	/* strictly we dont need this too msg_type
				   is enough */
} AVSV_D2N_SET_LEDS_MSG_INFO;

typedef struct avsv_d2n_admin_op_req_msg_info_tag {
	uns32         msg_id;
	SaClmNodeIdT  node_id;
	SaNameT       comp_name;
	SaAmfAdminOperationIdT  oper_id;
} AVSV_D2N_ADMIN_OP_REQ_MSG_INFO;

typedef struct avsv_d2n_comp_validation_resp_info_tag {
	uns32 msg_id;
	SaClmNodeIdT node_id;
	SaNameT comp_name;
	AVSV_COMP_VALIDATION_RESULT_TYPE result;
} AVSV_D2N_COMP_VALIDATION_RESP_INFO;

typedef struct avsv_d2n_role_change_info_tag {
	uns32 msg_id;
	SaClmNodeIdT node_id;
	uns32 role;
} AVSV_D2N_ROLE_CHANGE_INFO;

typedef struct avsv_dnd_msg {
	AVSV_DND_MSG_TYPE msg_type;
	union {
		AVSV_N2D_CLM_NODE_UP_MSG_INFO n2d_clm_node_up;
		AVSV_N2D_REG_SU_MSG_INFO n2d_reg_su;
		AVSV_N2D_REG_COMP_MSG_INFO n2d_reg_comp;
		AVSV_N2D_OPERATION_STATE_MSG_INFO n2d_opr_state;
		AVSV_N2D_INFO_SU_SI_ASSIGN_MSG_INFO n2d_su_si_assign;
		AVSV_N2D_PG_TRACK_ACT_MSG_INFO n2d_pg_trk_act;
		AVSV_N2D_OPERATION_REQUEST_MSG_INFO n2d_op_req;
		AVSV_N2D_DATA_REQUEST_MSG_INFO n2d_data_req;
		AVSV_N2D_VERIFY_ACK_NACK_MSG_INFO n2d_ack_nack_info;
		AVSV_N2D_SHUTDOWN_APP_SU_MSG_INFO n2d_shutdown_app_su;
		AVSV_D2N_CLM_NODE_UP_MSG_INFO d2n_clm_node_up;
		AVSV_D2N_REG_SU_MSG_INFO d2n_reg_su;
		AVSV_D2N_REG_COMP_MSG_INFO d2n_reg_comp;
		AVSV_D2N_INFO_SU_SI_ASSIGN_MSG_INFO d2n_su_si_assign;
		AVSV_D2N_PG_TRACK_ACT_RSP_MSG_INFO d2n_pg_track_act_rsp;
		AVSV_D2N_PG_UPD_MSG_INFO d2n_pg_upd;
		AVSV_D2N_OPERATION_REQUEST_MSG_INFO d2n_op_req;
		AVSV_D2N_PRESENCE_SU_MSG_INFO d2n_prsc_su;
		AVSV_D2N_DATA_VERIFY_MSG_INFO d2n_data_verify;
		AVSV_D2N_ACK_MSG d2n_ack_info;
		AVSV_D2N_SHUTDOWN_APP_SU_MSG_INFO d2n_shutdown_app_su;
		AVSV_D2N_SET_LEDS_MSG_INFO d2n_set_leds;
		AVSV_D2N_COMP_VALIDATION_RESP_INFO d2n_comp_valid_resp_info;
		AVSV_D2N_ROLE_CHANGE_INFO d2n_role_change_info;
		AVSV_D2N_ADMIN_OP_REQ_MSG_INFO       d2n_admin_op_req_info;
		AVSV_N2D_COMP_VALIDATION_INFO n2d_comp_valid_info;
	} msg_info;
} AVSV_DND_MSG;

/* macro to determine if the n2d msg is a response to some d2n msg */
#define m_AVSV_N2D_MSG_IS_RSP(avd) \
           ( AVSV_N2D_REG_SU_MSG == (avd)->msg_type || \
             AVSV_N2D_REG_COMP_MSG == (avd)->msg_type || \
             AVSV_N2D_INFO_SU_SI_ASSIGN_MSG == (avd)->msg_type || \
             AVSV_N2D_OPERATION_REQUEST_MSG == (avd)->msg_type || \
             AVSV_N2D_OPERATION_REQUEST_MSG == (avd)->msg_type )

/* macro to determine if the n2d msg is a Verify Ack Nack message */
#define m_AVSV_N2D_MSG_IS_VER_ACK_NACK(avd) \
           ( AVSV_N2D_VERIFY_ACK_NACK_MSG == (avd)->msg_type)

/* macro to determine if the n2d msg is a PG track Action message */
#define m_AVSV_N2D_MSG_IS_PG_TRACK_ACT(avd) \
           ( AVSV_N2D_PG_TRACK_ACT_MSG == (avd)->msg_type)

/* macro to determine if the n2d msg is a data-req  msg */
#define m_AVSV_N2D_MSG_IS_DATA_REQ(avd) \
           ( AVSV_N2D_DATA_REQUEST_MSG == (avd)->msg_type)

typedef void (*AVSV_FREE_DND_MSG_INFO) (AVSV_DND_MSG *);
typedef uns32 (*AVSV_COPY_DND_MSG) (AVSV_DND_MSG *, AVSV_DND_MSG *);

/* Extern Fuction Prototypes */

EXTERN_C void avsv_free_d2n_node_up_msg_info(AVSV_DND_MSG *node_up_msg);
EXTERN_C uns32 avsv_cpy_d2n_node_up_msg(AVSV_DND_MSG *d_node_up_msg, AVSV_DND_MSG *s_node_up_msg);
EXTERN_C void avsv_free_d2n_clm_node_fover_info(AVSV_DND_MSG *node_up_msg);
EXTERN_C uns32 avsv_cpy_d2n_clm_node_fover_info(AVSV_DND_MSG *d_node_up_msg, AVSV_DND_MSG *s_node_up_msg);
EXTERN_C void avsv_free_d2n_su_msg_info(AVSV_DND_MSG *su_msg);
EXTERN_C uns32 avsv_cpy_d2n_su_msg(AVSV_DND_MSG *d_su_msg, AVSV_DND_MSG *s_su_msg);
EXTERN_C void avsv_free_d2n_comp_msg_info(AVSV_DND_MSG *comp_msg);
EXTERN_C uns32 avsv_cpy_d2n_comp_msg(AVSV_DND_MSG *d_comp_msg, AVSV_DND_MSG *s_comp_msg);
EXTERN_C void avsv_free_d2n_susi_msg_info(AVSV_DND_MSG *susi_msg);
EXTERN_C uns32 avsv_cpy_d2n_susi_msg(AVSV_DND_MSG *d_susi_msg, AVSV_DND_MSG *s_susi_msg);
EXTERN_C void avsv_free_d2n_pg_msg_info(AVSV_DND_MSG *susi_msg);
EXTERN_C uns32 avsv_cpy_d2n_pg_msg(AVSV_DND_MSG *d_susi_msg, AVSV_DND_MSG *s_susi_msg);

EXTERN_C void avsv_dnd_msg_free(AVSV_DND_MSG *);

EXTERN_C uns32 avsv_dnd_msg_copy(AVSV_DND_MSG *, AVSV_DND_MSG *);

#endif   /* !AVSV_D2NMSG_H */
