#ifndef TINY_SCOPED_REFPTR_H
#define	TINY_SCOPED_REFPTR_H


#include <memory>
#include <utility>
#include <atomic>
#include <type_traits>
#include <utility>

#include "tiny_constructor_magic.h"

namespace tiny
{
    /*
    *   class RefCountInterface
    */
    enum class RefCountReleaseStatus { kDroppedLastRef, kOtherRefsRemained };
    class RefCountInterface {
    public:
        virtual void AddRef() const = 0;
        virtual RefCountReleaseStatus Release() const = 0;
    protected:
        virtual ~RefCountInterface() {}
    };

    /*
    *       class RefCounter
    */
    class RefCounter {
    public:
        explicit RefCounter(int ref_count) : ref_count_(ref_count) {}
        RefCounter() = delete;

        void IncRef() {
            ref_count_.fetch_add(1, std::memory_order_relaxed);
        }

        RefCountReleaseStatus DecRef() {
            int ref_count_after_subtract =
                ref_count_.fetch_sub(1, std::memory_order_acq_rel) - 1;
            return ref_count_after_subtract == 0
                ? RefCountReleaseStatus::kDroppedLastRef
                : RefCountReleaseStatus::kOtherRefsRemained;
        }
        bool HasOneRef() const {
            return ref_count_.load(std::memory_order_acquire) == 1;
        }

    private:
        std::atomic<int> ref_count_;
    };


    /*
    *       class RefCountedObject
    */
    template <class T>
    class RefCountedObject : public T {
    public:
        RefCountedObject() {}

        template <class P0>
        explicit RefCountedObject(P0&& p0) : T(std::forward<P0>(p0)) {}

        template <class P0, class P1, class... Args>
        RefCountedObject(P0&& p0, P1&& p1, Args&&... args)
            : T(std::forward<P0>(p0),
                std::forward<P1>(p1),
                std::forward<Args>(args)...) {}

        virtual void AddRef() const { ref_count_.IncRef(); }

        virtual RefCountReleaseStatus Release() const {
            const auto status = ref_count_.DecRef();
            if (status == RefCountReleaseStatus::kDroppedLastRef) {
                delete this;
            }
            return status;
        }

        virtual bool HasOneRef() const { return ref_count_.HasOneRef(); }

    protected:
        virtual ~RefCountedObject() {}

        mutable webrtc::webrtc_impl::RefCounter ref_count_{ 0 };

        TINY_DISALLOW_COPY_AND_ASSIGN(RefCountedObject);
    };



    /*
    *       class FinalRefCountedObject
    */
    template <class T>
    class FinalRefCountedObject final : public T {
    public:
        using T::T;
        // Until c++17 compilers are allowed not to inherit the default constructor,
        // and msvc doesn't. Thus the default constructor is forwarded explicitly.
        FinalRefCountedObject() = default;
        FinalRefCountedObject(const FinalRefCountedObject&) = delete;
        FinalRefCountedObject& operator=(const FinalRefCountedObject&) = delete;

        void AddRef() const { ref_count_.IncRef(); }
        void Release() const {
            if (ref_count_.DecRef() == RefCountReleaseStatus::kDroppedLastRef) {
                delete this;
            }
        }
        bool HasOneRef() const { return ref_count_.HasOneRef(); }

    private:
        ~FinalRefCountedObject() = default;
        class ZeroBasedRefCounter : public webrtc::webrtc_impl::RefCounter {
        public:
            ZeroBasedRefCounter() : RefCounter(0) {}
        } mutable ref_count_;
    };



    /*
    *       class scoped_refptr
    */
    template <class T>
    class scoped_refptr {
    public:
        typedef T element_type;

        scoped_refptr() : ptr_(nullptr) {}

        scoped_refptr(T* p) : ptr_(p) {  // NOLINT(runtime/explicit)
            if (ptr_)
                ptr_->AddRef();
        }

        scoped_refptr(const scoped_refptr<T>& r) : ptr_(r.ptr_) {
            if (ptr_)
                ptr_->AddRef();
        }

        template <typename U>
        scoped_refptr(const scoped_refptr<U>& r) : ptr_(r.get()) {
            if (ptr_)
                ptr_->AddRef();
        }

        // Move constructors.
        scoped_refptr(scoped_refptr<T>&& r) noexcept : ptr_(r.release()) {}

        template <typename U>
        scoped_refptr(scoped_refptr<U>&& r) noexcept : ptr_(r.release()) {}

        ~scoped_refptr() {
            if (ptr_)
                ptr_->Release();
        }

        T* get() const { return ptr_; }
        operator T* () const { return ptr_; }
        T& operator*() const { return *ptr_; }
        T* operator->() const { return ptr_; }

        // Returns the (possibly null) raw pointer, and makes the scoped_refptr hold a
        // null pointer, all without touching the reference count of the underlying
        // pointed-to object. The object is still reference counted, and the caller of
        // release() is now the proud owner of one reference, so it is responsible for
        // calling Release() once on the object when no longer using it.
        T* release() {
            T* retVal = ptr_;
            ptr_ = nullptr;
            return retVal;
        }

        scoped_refptr<T>& operator=(T* p) {
            // AddRef first so that self assignment should work
            if (p)
                p->AddRef();
            if (ptr_)
                ptr_->Release();
            ptr_ = p;
            return *this;
        }

        scoped_refptr<T>& operator=(const scoped_refptr<T>& r) {
            return *this = r.ptr_;
        }

        template <typename U>
        scoped_refptr<T>& operator=(const scoped_refptr<U>& r) {
            return *this = r.get();
        }

        scoped_refptr<T>& operator=(scoped_refptr<T>&& r) noexcept {
            scoped_refptr<T>(std::move(r)).swap(*this);
            return *this;
        }

        template <typename U>
        scoped_refptr<T>& operator=(scoped_refptr<U>&& r) noexcept {
            scoped_refptr<T>(std::move(r)).swap(*this);
            return *this;
        }

        void swap(T** pp) noexcept {
            T* p = ptr_;
            ptr_ = *pp;
            *pp = p;
        }

        void swap(scoped_refptr<T>& r) noexcept { swap(&r.ptr_); }

    protected:
        T* ptr_;
    };
}
#endif // !TINY_SCOPED_REFPTR_H
