#ifndef SIGSLOT_SIGNAL_H_
#define SIGSLOT_SIGNAL_H_


#include <sigslot/slot.h>


namespace sigslot {

template <typename Ret, typename... Args>
class signal {

    friend class slot<Ret, Args...>;
    typedef slot<Ret, Args...> Slot;

public:

    inline signal() {};
    inline ~signal() { clear(); }

    inline signal(const signal&) = delete;
    inline signal(signal&&) = delete;
    inline signal& operator=(signal&&) = delete;
    inline signal& operator=(signal&&) = delete;

    ///
    /// \brief addSlot add a new slot to this signal
    /// \param s
    ///
    inline void
    addSlot(Slot* s)
    {
        if (s == 0 || contains(s)) return;
        // automatically call to the function to add the slot in this class
        s->link(this);
    }

    ///
    /// \brief contains check if a slot is linked to here
    /// \param s
    /// \return
    ///
    inline bool
    contains(const Slot* s) const
    {
        for (const Slot* sl : slots_) if (s == sl) return true;
        return false;
    }

    ///
    /// \brief containsSlots
    /// \return
    ///
    inline bool
    containsSlots(void) const
    {
        return !slots_.empty();
    }

    ///
    /// \brief removeSlot remove a slot from here
    /// \param s
    ///
    inline void
    removeSlot(Slot* s)
    {
        if (s == 0) return;
        // automatically call to the function to remove the slot in this class
        s->unlink(this, true);
    }

    inline void
    clear(void)
    {
        for (Slot* s : slots_) {
            s->unlink(this, false);
        }
        slots_.clear();
    }

    ///
    /// \brief emitSignal emit the signal
    /// \param args
    ///
    inline void
    emitSignal(Args... args)
    {
        for (Slot* sl : slots_) {
            ASSERT(sl != 0);
            if (sl->callback()) {
                sl->callback()(args...);
            }
        }
    }

    inline unsigned int
    size(void) const
    {
        return slots_.size();
    }


private:

    ///
    /// \brief replaceReference will and should be called only by the slots
    ///        we need to
    /// \param oldRef
    /// \param newRef
    ///
    inline void
    replaceReference(const Slot* oldRef, Slot* newRef)
    {
        for (unsigned int i = 0; i < slots_.size(); ++i) {
            if (slots_[i] == oldRef) {
                slots_[i] = newRef;
                return;
            }
        }
    }

    /// Functions called by the slot
    ///
    inline void
    slotRemoveRef(const Slot* slot)
    {
        for (unsigned int i = 0; i < slots_.size(); ++i) {
            if (slots_[i] == slot) {
                slots_[i] = slots_.back();
                slots_.pop_back();
                return;
            }
        }
    }
    inline void
    slotAddRef(Slot* slot)
    {
        // if the slot is calling this we shouldn't have the slot already
        ASSERT(!contains(slot));
        slots_.push_back(slot);
    }

private:
    std::vector<Slot*> slots_;

};



} // namespace sigslot



#endif // SIGSLOT_SIGNAL_H_
