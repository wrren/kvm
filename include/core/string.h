#ifndef KVM_CORE_STRING_H
#define KVM_CORE_STRING_H

#include <string>

namespace kvm {
    /**
     * Convert a wide-character string to a char string.
     */
    std::string WideStringToString(const wchar_t* input);
    std::string WideStringToString(const std::wstring& input);
}

#endif // KVM_CORE_STRING_H