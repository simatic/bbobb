#include <assert.h>
#include "bbStateMachine.h"
#include "bbMsg.h"

BbState bbAutomatonState = BB_STATE_WAIT_VIEW_CHANGE;

BbState bbError(BbState, BbMsg*);
BbState bbProcessRecover(BbState, BbMsg*);
BbState bbProcessSet(BbState, BbMsg*);
BbState bbProcessViewChange(BbState, BbMsg*);
BbState bbSaveSet(BbState, BbMsg*);

BbStateMachineFunc bbTransitions[BB_LAST_STATE+1][BB_LAST_MSG+1] = {
  /*      State  /  Received msg :    BB_MSG_RECOVER             BB_MSG_SET                 BB_MSG_VIEW_CHANGE            */
  /* BB_STATE_ALONE              */ { bbError,                   bbError,                   bbProcessViewChange },
  /* BB_STATE_SEVERAL            */ { bbError,                   bbProcessSet,              bbProcessViewChange },
  /* BB_STATE_MANAGE_VIEW_CHANGE */ { bbProcessRecover,          bbSaveSet,                 bbProcessViewChange },
  /* BB_STATE_WAIT_VIEW_CHANGE   */ { bbError,                   bbError,                   bbProcessViewChange }
};

void bbStateMachineTransition(BbMsg* pMsg){
  assert( (pMsg->type >= 0) && (pMsg->type <= BB_LAST_MSG) );
  assert( (bbAutomatonState >= 0) && (bbAutomatonState <= BB_LAST_STATE) );
  bbAutomatonState = (*bbTransitions[bbAutomatonState][pMsg->type])(bbAutomatonState, pMsg);
}

BbState bbError(BbState state, BbMsg* pMsg){
  fprintf(stderr, "Received unexpected message %d in state %d\n", state, pMsg->type);
  abort();
}

BbState bbProcessRecover(BbState state, BbMsg* pMsg){
  // TODO : To be completed
  return BB_STATE_ALONE; // TODO : Put the correct return value
}

BbState bbProcessSet(BbState state, BbMsg* pMsg){
  // TODO : To be completed
  return BB_STATE_ALONE; // TODO : Put the correct return value
}

BbState bbProcessViewChange(BbState state, BbMsg* pMsg){
  // TODO : To be completed
  return BB_STATE_ALONE; // TODO : Put the correct return value
}

BbState bbSaveSet(BbState state, BbMsg* pMsg){
  // TODO : To be completed
  return BB_STATE_ALONE; // TODO : Put the correct return value
}


