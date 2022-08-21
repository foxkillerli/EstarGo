#ifndef __ProtoI_h__
#define __ProtoI_h__

#include <Proto.h>

namespace GoKu
{

class InterfaceEvalI : public virtual InterfaceEval
{
public:

    virtual void evaluate(const RequestBatch&,
                          ReplyBatch&,
                          const Ice::Current&);
};

}

#endif
