#ifndef SIGSLOT_SLOT_H_
#define SIGSLOT_SLOT_H_

#include <functional>
#include <vector>


// fw
namespace sigslot {
template<typename R, typename... A>
class signal;
}

namespace sigslot {


template <typename Ret = void, typename... Args>
class slot {
public:

    typedef signal<Ret, Args...> Signal;
    typedef std::function<Ret (Args...)> Callback;

public:
    inline slot();
    inline slot(const Callback& cb);
    inline ~slot();

    ///
    /// \note IMPORTANT: everytime we copy the element the other one (old one)
    ///                  becomes useless, this class will work as unique_ptr
    /// \brief slot
    /// \param s
    ///
    inline slot(const slot& s);
    inline slot& operator=(const slot& s);
    inline slot(slot&&);
    inline slot(const slot&&) = delete;
    inline slot& operator=(slot&&);

    ///
    /// \brief isLinked will check if we are linked to some slot
    /// \return true if we are | false otherwise.
    ///
    inline bool
    isLinked(void) const;
    inline bool
    isLinked(const Signal* s) const;

    ///
    /// \brief setCallback
    /// \param cb
    ///
    inline void
    setCallback(const Callback& cb);

    ///
    /// \brief callback
    /// \return
    ///
    inline const Callback&
    callback(void) const;
    inline Callback&
    callback(void);

    ///
    /// \brief unlink
    ///
    inline void
    unlink(const Signal* s);
    inline void
    unlinkAll(void);

    ///
    /// \brief link
    /// \param s
    ///
    inline void
    link(Signal* s);

private:

    friend class signal<Ret, Args...>;



    ///
    /// \brief resetSlot
    ///
    inline void
    resetSignal(void);

    ///
    /// \brief changeSignalsRef
    /// \param old
    ///
    inline void
    changeSignalsRef(const slot* old);

    ///
    /// \brief getSignalIndex
    /// \param s
    /// \return
    ///
    inline int
    getSignalIndex(const Signal* s);

    ///
    /// \brief unlink
    /// \param s
    ///
    void
    unlink(const Signal* s, bool notifySignal);


private:
    Callback callback_;
    mutable std::vector<Signal*> signals_;
};











////////////////////////////////////////////////////////////////////////////////
/// Inline stuff
///
template<typename Ret, typename... Args>
inline slot<Ret, Args...>::slot()
{
}

template<typename Ret, typename... Args>
inline slot<Ret, Args...>::slot(const Callback& cb) :
    callback_(cb)
{
}

////////////////////////////////////////////////////////////////////////////////

template<typename Ret, typename... Args>
inline slot<Ret, Args...>::~slot()
{
    unlinkAll();
}

template<typename Ret, typename... Args>
inline slot<Ret, Args...>::slot(const slot& s) :
    callback_(s.callback_)
,   signals_(std::move(s.signals_))
{
    // invalidate the other one (the old one)
    s.signals_.clear();
    // change references
    changeSignalsRef(&s);
}

template<typename Ret, typename... Args>
inline slot<Ret, Args...>&
slot<Ret, Args...>::operator=(const slot& s)
{
    if (&s == this) {
        return *this;
    }
    callback_ = s.callback();
    signals_ = std::move(s.signals_);
    s.signals_.clear();

    // invalidate the other one (the old one)
    changeSignalsRef(&s);

    return *this;
}

template<typename Ret, typename... Args>
inline slot<Ret, Args...>::slot(slot&& s) :
    callback_(std::move(s.callback_))
,   signals_(std::move(s.signals_))
{
    // invalidate the other one (the old one)
    s.signals_.clear();
    changeSignalsRef(&s);
}

template<typename Ret, typename... Args>
inline slot<Ret, Args...>&
slot<Ret, Args...>::operator=(slot&& s)
{
    if (&s == this) {
        return *this;
    }
    callback_ = std::move(s.callback_);
    signals_ = std::move(s.signals_);

    // invalidate the other one (the old one)
    s.signals_.clear();
    changeSignalsRef(&s);

    return *this;
}




template<typename Ret, typename... Args>
inline bool
slot<Ret, Args...>::isLinked(void) const
{
    return !signals_.empty();
}
template<typename Ret, typename... Args>
inline bool
slot<Ret, Args...>::isLinked(const Signal* s) const
{
    return getSignalIndex(s) >= 0;
}


template<typename Ret, typename... Args>
inline void
slot<Ret, Args...>::unlink(const Signal* s, bool notifySignal)
{
    const int index = getSignalIndex(s);
    if (index < 0) {
        return;
    }

    // else remove this
    if (notifySignal) {
        signals_[index]->slotRemoveRef(this);
    }
    signals_[index] = signals_.back();
    signals_.pop_back();
}

template<typename Ret, typename... Args>
inline void
slot<Ret, Args...>::unlink(const Signal* s)
{
    unlink(s, true);
}
template<typename Ret, typename... Args>
inline void
slot<Ret, Args...>::unlinkAll(void)
{
    for (Signal* s : signals_) {
        s->slotRemoveRef(this);
    }
    signals_.clear();
}

template<typename Ret, typename... Args>
inline void
slot<Ret, Args...>::link(Signal* s)
{
    const int index = getSignalIndex(s);
    if (index >= 0) {
        // already linked
        return;
    }

    signals_.push_back(s);
    s->slotAddRef(this);
}

template<typename Ret, typename... Args>
inline void
slot<Ret, Args...>::setCallback(const Callback& cb)
{
    callback_ = cb;
}

template<typename Ret, typename... Args>
inline const typename slot<Ret, Args...>::Callback&
slot<Ret, Args...>::callback(void) const
{
    return callback_;
}

template<typename Ret, typename... Args>
inline typename slot<Ret, Args...>::Callback&
slot<Ret, Args...>::callback(void)
{
    return callback_;
}

////////////////////////////////////////////////////////////////////////////////

template<typename Ret, typename... Args>
inline void
slot<Ret, Args...>::changeSignalsRef(const slot* old)
{
    for (Signal* s : signals_) {
        s->replaceReference(old, this);
    }
}

template<typename Ret, typename... Args>
inline int
slot<Ret, Args...>::getSignalIndex(const Signal* s)
{
    for (unsigned int i = 0; i < signals_.size(); ++i) {
        if (signals_[i] == s) return static_cast<int>(i);
    }
    return -1;
}

} // namespace sigslot



#endif // SIGSLOT_SLOT_H_
