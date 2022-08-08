module GoKu
{
    sequence<string> RequestBatch;
    struct MoveProb
    {
        int pos;
        float prob;
    }

    sequence<MoveProb> PolicyReply;

    struct Reply
    {
        PolicyReply p;
        float v;
    }

    sequence<Reply> ReplyBatch;

    interface InterfaceEval
    {
        idempotent void evaluate(RequestBatch req, out ReplyBatch rep);
    }
}
