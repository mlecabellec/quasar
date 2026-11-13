/**
 * @file ICodec.cpp
 * @brief Translation unit anchor for the purely virtual ICodec interface.
 * @details Compiles ICodec.hpp standalone to verify header self-containment and reserve linkage space
 * within namespace datacodec.
 * @task TSK-20261113-003, TSK-20260303-002
 * @feature FE-0010, FE-0100
 * @compliance Provides Doxygen file and namespace documentation for pure virtual interface anchor (CS-0090.1, CS-0090.2).
 */
// Include primary abstract codec interface definition.
#include "datacodec/ICodec.hpp"

/**
 * @brief Namespace for Quasar binary and schema-driven data codecs and mappers.
 * @details Reserved translation-unit scope for future non-inline ICodec helpers or vtable anchors.
 * @task TSK-20261113-003, TSK-20260303-002
 * @feature FE-0010, FE-0100
 * @compliance Encloses translation unit anchor inside namespace datacodec (CS-0010.33).
 */
namespace datacodec {
// Empty for now as ICodec is purely virtual
}
