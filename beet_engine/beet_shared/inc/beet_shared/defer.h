#ifndef BEETROOT_DEFER_H
#define BEETROOT_DEFER_H

template<typename CallbackType>
struct DeferCallback {
    CallbackType m_callback;
    DeferCallback(CallbackType inCallback) : m_callback(inCallback) {}
    ~DeferCallback() { m_callback(); }

    DeferCallback(DeferCallback &&) = delete;
    void operator=(DeferCallback &&) = delete;
};

#define STR_NAME_IN_PLACE(a, b) a##b
#define STR_NAME(a, b) STR_NAME_IN_PLACE(a,b)

//===API================================================================================================================
#define DEFER(callback) auto STR_NAME(a, __COUNTER__) = DeferCallback(callback)
//======================================================================================================================

#endif //BEETROOT_DEFER_H
