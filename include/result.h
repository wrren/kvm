#ifndef KVM_RESULT_H
#define KVM_RESULT_H

#include <variant>

namespace kvm {
    /**
     * Represents the result of an operation that may fail.
     */
    template<typename Value, typename Error>
    class Result {
    public:

        Result(const Value& value) :
        m_variant(value)
        {}

        Result(const Error& error) :
        m_variant(error)
        {}

        /**
         * Check whether the operation that produced this result succeeded.
         */
        bool DidSucceed() const {
            return std::holds_alternative<Value>(m_variant);
        }

        /**
         * Check whether the operation that produced this result failed.
         */
        bool DidFail() const {
            return std::holds_alternative<Error>(m_variant);
        }

        /**
         * Get the underlying value from this result. Throws if this is an Error-containing result. Check
         * whether the operation that produced the result succeeded first by calling DidSucceed().
         */
        const Value& GetValue() const {
            return std::get<Value>(m_variant);
        }

        /**
         * Get the underlying error from this result. Throws if this is a Value-containing result. Check
         * whether the operation that produced the result failed first by calling DidFail().
         */
        const Error& GetError() const {
            return std::get<Error>(m_variant);
        }

    private:

        std::variant<Value, Error> m_variant;
    };
}

#endif // KVM_RESULT_H