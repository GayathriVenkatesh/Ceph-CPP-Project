
#ifndef __OSD_H
#define __OSD_H

#include "msg/Dispatcher.h"

#include "common/Mutex.h"

#include <map>
using namespace std;


class Messenger;
class Message;


// ways to be dirty
#define RG_DIRTY_LOCAL_LOG     1
#define RG_DIRTY_LOCAL_SYNC    2
#define RG_DIRTY_REPLICA_MEM   4
#define RG_DIRTY_REPLICA_SYNC  8


class ReplicaGroup {
 public:
  repgroup_t rg;
  int        role;    // 1 = primary, 2 = secondary, etc.  0=undef.
  int        state;   

  map<object_t, int>  dirty_map;  // dirty objects
  
  ReplicaGroup(repgroup_t rg);

  void enumerate_objects(list<object_t>& ls);
};


class OSD : public Dispatcher {
 protected:
  Messenger *messenger;
  int whoami;

  class OSDCluster  *osdcluster;
  class ObjectStore *store;
  class HostMonitor *monitor;

  list<class MOSDOp*> waiting_for_osdcluster;

  Mutex osd_lock;

 public:
  OSD(int id, Messenger *m);
  ~OSD();
  
  // startup/shutdown
  int init();
  int shutdown();

  // OSDCluster
  void update_osd_cluster(__uint64_t ocv, bufferlist& blist);

  // messages
  virtual void dispatch(Message *m);

  void handle_ping(class MPing *m);
  void handle_op(class MOSDOp *m);
  void op_read(class MOSDOp *m);
  void op_write(class MOSDOp *m);
};

#endif
