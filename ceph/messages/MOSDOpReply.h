#ifndef __MOSDOPREPLY_H
#define __MOSDOPREPLY_H

#include "msg/Message.h"
#include "osd/OSDCluster.h"

#include "MOSDOp.h"

/*
 * OSD op reply
 *
 * oid - object id
 * op  - OSD_OP_DELETE, etc.
 *
 */


typedef struct {
  // req
  long tid;
  long pcid;

  object_t oid;

  int op;
  
  // reply
  int    result;
  size_t length, offset;
  size_t object_size;

  __uint64_t _new_ocv;
  size_t _data_len, _oc_len;
} MOSDOpReply_st;


class MOSDOpReply : public Message {
  MOSDOpReply_st st;
  bufferlist data;
  bufferlist osdcluster;

 public:
  long     get_tid() { return st.tid; }
  object_t get_oid() { return st.oid; }
  int      get_op()  { return st.op; }
  
  int    get_result() { return st.result; }
  size_t get_length() { return st.length; }
  size_t get_offset() { return st.offset; }
  size_t get_object_size() { return st.object_size; }

  void set_result(int r) { st.result = r; }
  void set_length(size_t s) { st.length = s; }
  void set_offset(size_t o) { st.offset = o; }
  void set_object_size(size_t s) { st.object_size = s; }

  // data payload
  void set_data(bufferlist &d) {
	data.claim(d);
	st._data_len = data.length();
  }
  bufferlist& get_data() {
	return data;
  }

  // osdcluster
  __uint64_t get_ocv() { return st._new_ocv; }
  bufferlist& get_osdcluster() { 
	return osdcluster;
  }

  // keep a pcid (procedure call id) to match up request+reply
  void set_pcid(long pcid) { this->st.pcid = pcid; }
  long get_pcid()          { return st.pcid; }

  MOSDOpReply(MOSDOp *req, int result, OSDCluster *oc) :
	Message(MSG_OSD_OPREPLY) {
	memset(&st, 0, sizeof(st));
	this->st.pcid = req->st.pcid;
	this->st.tid = req->st.tid;

	this->st.oid = req->st.oid;
	this->st.op = req->st.op;
	this->st.result = result;

	this->st.length = req->st.length;   // speculative... OSD should ensure these are correct
	this->st.offset = req->st.offset;

	// attach updated cluster spec?
	if (req->get_ocv() < oc->get_version()) {
	  oc->encode(osdcluster);
	  st._new_ocv = oc->get_version();
	  st._oc_len = osdcluster.length();
	}
  }
  MOSDOpReply() {}


  // marshalling
  virtual void decode_payload() {
	payload.copy(0, sizeof(st), (char*)&st);
	payload.splice(0, sizeof(st));
	if (st._data_len) payload.splice(0, st._data_len, &data);
	if (st._oc_len) payload.splice(0, st._oc_len, &osdcluster);
  }
  virtual void encode_payload() {
	payload.push_back( new buffer((char*)&st, sizeof(st)) );
	payload.claim_append( data );
	payload.claim_append( osdcluster );
  }

  virtual char *get_type_name() { return "oopr"; }
};

#endif
