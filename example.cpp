#include <string>
#include <iostream>
#include <functional>

#include <sigslot/signal.h>
#include <sigslot/slot.h>

// Define the type of slot and signal we are emitting. 
// The slot defines the callback signature, and at the same time
// the signal type.
//
typedef sigslot::slot<void, const std::string&> MessageSlot;
typedef MessageSlot::Signal MessageSignal;


struct Receiver {
    Receiver(const std::string& theName) : 
        name(theName)
    ,   msgSlot(std::bind(&Receiver::callback, this, std::placeholders::_1))
    {}

    void callback(const std::string& msg)
    {
        std::cout << "Receiver " << name << " received: " << msg << std::endl;
    }

    MessageSlot msgSlot;
    std::string name;
};

struct Emitter {
    Emitter(const std::string& theName) : name(theName) {}

    void emit(const std::string& msg)
    {
        std::cout << "Emitter " << name << " emitting: " << msg << std::endl;
        msgSignal.emit(msg);
    }

    // connects a slot to receive messages from
    void connect(MessageSlot* slot)
    {
        msgSignal.addSlot(slot);
    }

    std::string name;
    MessageSignal msgSignal;
};


int main(void)
{
    // 2 emitters, 2 receivers
    Emitter emitter1("E1"), emitter2("E2");
    Receiver receiver1("R1"), receiver2("R2");

    // connect R1 to both, R2 to E2
    emitter1.connect(&receiver1.msgSlot);
    emitter2.connect(&receiver1.msgSlot);
    emitter2.connect(&receiver2.msgSlot);

    std::cout << "Emitting first message: \"test-message\": " << std::endl;
    emitter1.emit("test-message");
    emitter2.emit("test-message");
    std::cout << std::endl << std::endl;

    // test scope works
    {
        Receiver scopeReceiver("SCOPE-R");
        emitter1.connect(&scopeReceiver.msgSlot);

        std::cout << "Emitting in the scope for emitter 1 only" << std::endl;
        emitter1.emit("test-message");
        std::cout << std::endl << std::endl;
    }

    std::cout << "Emitting emitter 1 only outside the scope" << std::endl;
    emitter1.emit("test-message");

    return 0;
}