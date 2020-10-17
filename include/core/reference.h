#ifndef KVM_REFERENCE_COUNTER_H
#define KVM_REFERENCE_COUNTER_H

#include <mutex>
#include <functional>

namespace kvm {
  /**
   * Utility class for causing the intialization or destruction of some state
   * based on a reference count.
   */
  template<typename T>
  class ReferenceCounter {
  public:

    typedef std::function<T()>    Initializer;
    typedef std::function<T(T&)>  Destructor;

    ReferenceCounter()
    {}

    ReferenceCounter(Initializer initializer, Destructor destructor) :
    m_initializer(initializer),
    m_destructor(destructor)
    {}

    ReferenceCounter<T>& operator++() {
      m_mutex.lock();

      if(m_references == 0) {
        m_state = m_initializer();
      }
      m_references++;

      m_mutex.unlock();
      return *this;
    }

    ReferenceCounter<T>& operator--() {
      m_mutex.lock();

      if(m_references > 0) {
        m_references--;

        if(m_references == 0) {
          m_state = m_destructor(m_state);
        }
      }

      m_mutex.unlock();
      return *this;
    }

  private:

    /// Reference mutex
    std::mutex m_mutex;
    /// Current Number of References
    uint32_t m_references;
    /// State
    T m_state;
    /// Initializer Function
    Initializer m_initializer;
    /// Destructor Function
    Destructor m_destructor;
  };
}

#endif // KVM_REFERENCE_COUNTER_H