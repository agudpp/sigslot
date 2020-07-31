#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <map>
#include <list>
#include <set>
#include <algorithm>
#include <functional>
// test includes
#include <gtest/gtest.h>
// own includes

#include <sigslot/signal.h>
#include <sigslot/slot.h>



// test some apis
struct Receiver {
    int val;

    Receiver() : val(0) {};

    void method(int m) {val = m;}
};
struct ReceiverAcum {
    int val;

    ReceiverAcum() : val(0) {};

    void method(int m) {val += m;}
};

typedef sigslot::slot<void, int> RecvSlot;
typedef RecvSlot::Signal RecvSignal;


////////////////////////////////////////////////////////////////////////////////
TEST(SignalSlot, NormalFunctionalities)
{
    Receiver r1, r2;
    RecvSlot sl1,sl2;
    RecvSignal sig;

    sl1.setCallback(std::bind(&Receiver::method, &r1, std::placeholders::_1));
    sl2.setCallback(std::bind(&Receiver::method, &r2, std::placeholders::_1));

    EXPECT_EQ(sl1.isLinked(), false);
    EXPECT_EQ(sl2.isLinked(), false);
    sig.addSlot(&sl1);
    sig.addSlot(&sl2);
    EXPECT_EQ(0, r1.val);
    EXPECT_EQ(0, r2.val);
    EXPECT_EQ(sl1.isLinked(), true);
    EXPECT_EQ(sl2.isLinked(), true);

    sig.emit(3);
    EXPECT_EQ(3, r1.val);
    EXPECT_EQ(3, r2.val);

    sig.removeSlot(&sl1);
    EXPECT_EQ(sl1.isLinked(), false);
    EXPECT_EQ(sl2.isLinked(), true);

    sig.emit(11);
    EXPECT_EQ(3, r1.val);
    EXPECT_EQ(11, r2.val);

    sl2.unlinkAll();
    EXPECT_EQ(sl2.isLinked(), false);

    sig.emit(111);
    EXPECT_EQ(3, r1.val);
    EXPECT_EQ(11, r2.val);

}

////////////////////////////////////////////////////////////////////////////////
TEST(SignalSlot, ScopeFunctions)
{
    Receiver r1, r2;
    RecvSlot sl1, sl2;
    RecvSignal sig;

    sl1.setCallback(std::bind(&Receiver::method, &r1, std::placeholders::_1));

    sig.addSlot(&sl1);
    EXPECT_EQ(0, r1.val);
    EXPECT_EQ(0, r2.val);
    EXPECT_EQ(sl1.isLinked(), true);
    EXPECT_EQ(sl2.isLinked(), false);

    sig.emit(3);

    EXPECT_EQ(3, r1.val);
    EXPECT_EQ(0, r2.val);

    EXPECT_EQ(sl1.isLinked(), true);

    {
        RecvSignal sig2;
        sig2.addSlot(&sl1);

        sig.emit(6);
        EXPECT_EQ(6, r1.val);
        EXPECT_EQ(0, r2.val);

        sig2.emit(11);
        EXPECT_EQ(11, r1.val);
        EXPECT_EQ(0, r2.val);
    }

    EXPECT_EQ(sl1.isLinked(), true);
    sl1.unlink(&sig);
    EXPECT_EQ(sl1.isLinked(), false);

}

////////////////////////////////////////////////////////////////////////////////
TEST(SignalSlot, CopySlot)
{
    ReceiverAcum r1, r2;
    RecvSlot sl1, sl2;
    RecvSignal sig;

    sl1.setCallback(std::bind(&ReceiverAcum::method, &r1, std::placeholders::_1));

    sig.addSlot(&sl1);
    EXPECT_EQ(0, r1.val);
    EXPECT_EQ(sl1.isLinked(), true);
    EXPECT_EQ(sl2.isLinked(), false);

    sl2 = sl1;
    EXPECT_EQ(sl1.isLinked(), false);
    EXPECT_EQ(sl2.isLinked(), true);

    sig.emit(3);
    EXPECT_EQ(3, r1.val);

    sl1 = sl2;
    sig.addSlot(&sl1);
    sig.emit(3);
    EXPECT_EQ(6, r1.val);

    sig.addSlot(&sl2);
    sig.emit(3);
    EXPECT_EQ(12, r1.val);
    sl2.unlinkAll();

    RecvSlot sl3 = sl1;
    sl3.setCallback(std::bind(&ReceiverAcum::method, &r2, std::placeholders::_1));
    sig.emit(3);
    EXPECT_EQ(12, r1.val);
    EXPECT_EQ(3, r2.val);

}

////////////////////////////////////////////////////////////////////////////////
TEST(SignalSlot, CopySlotVector)
{
    ReceiverAcum r1, r2;
    RecvSlot sl1, sl2;
    RecvSignal sig;
    std::vector<RecvSlot> sVec;

    sVec.push_back(RecvSlot(std::bind(&ReceiverAcum::method, &r1, std::placeholders::_1)));
    {
        sl2 = sVec[0];
    }
    sVec.clear();
    sig.addSlot(&sl2);
    EXPECT_EQ(sl1.isLinked(), false);
    EXPECT_EQ(sl2.isLinked(), true);

    sig.emit(3);
    EXPECT_EQ(3, r1.val);

    struct S1 {
        RecvSlot sl;

        S1(const RecvSlot& s) : sl(s){}

    };

    S1 a(sl2);
    S1 b = a;
    EXPECT_EQ(sl2.isLinked(), false);
    EXPECT_EQ(a.sl.isLinked(), false);
    EXPECT_EQ(b.sl.isLinked(), true);
    sig.emit(3);
    EXPECT_EQ(6, r1.val);
    sVec.push_back(b.sl);
    for (unsigned int i = 0; i < 104; ++i) {
        sVec.push_back(sVec.back());
    }

    sig.emit(3);
    EXPECT_EQ(9, r1.val);
}

////////////////////////////////////////////////////////////////////////////////
TEST(SignalSlot, MultipleSignalsPerSlot)
{
    ReceiverAcum r1, r2;
    RecvSlot sl1, sl2;
    RecvSignal sig, sig2, sig3;
    std::vector<RecvSlot> sVec;

    sVec.push_back(RecvSlot(std::bind(&ReceiverAcum::method, &r1, std::placeholders::_1)));

    // connect to 3 of them
    sl1 = sVec.back();
    sig.addSlot(&sl1);
    sig2.addSlot(&sl1);
    sig3.addSlot(&sl1);

    sig.emit(1);
    EXPECT_EQ(1, r1.val);
    sig2.emit(1);
    EXPECT_EQ(2, r1.val);
    sig3.emit(1);
    EXPECT_EQ(3, r1.val);

    sig.emit(1);
    sig2.emit(1);
    sig3.emit(1);
    EXPECT_EQ(6, r1.val);

    EXPECT_EQ(true, sl1.isLinked());
    sig.removeSlot(&sl1);
    EXPECT_EQ(true, sl1.isLinked());
    sig2.removeSlot(&sl1);
    EXPECT_EQ(true, sl1.isLinked());
    sig3.removeSlot(&sl1);
    EXPECT_EQ(false, sl1.isLinked());

    sig.emit(1);
    sig2.emit(1);
    sig3.emit(1);
    EXPECT_EQ(6, r1.val);

    // add the slots and now copy to the sl2
    sig.addSlot(&sl1);
    sig2.addSlot(&sl1);
    sig3.addSlot(&sl1);
    EXPECT_EQ(true, sl1.isLinked());

    sig.emit(1);
    sig2.emit(1);
    sig3.emit(1);
    EXPECT_EQ(9, r1.val);

    EXPECT_EQ(false, sl2.isLinked());
    sl2 = sl1;
    EXPECT_EQ(false, sl1.isLinked());
    EXPECT_EQ(true, sl2.isLinked());

    sig.emit(1);
    sig2.emit(1);
    sig3.emit(1);
    EXPECT_EQ(12, r1.val);
}



////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
int
main(int argc, char** argv) 
{    
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
