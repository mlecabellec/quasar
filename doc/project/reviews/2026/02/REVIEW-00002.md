# REVIEW-00002: Review of quasar::named namespace. (WIP)

## Description

This review is a preliminary review of the quasar::named namespace.

## Review scope

### Reviewed code

- [REVIEW-00002.1] quasar::named namespace at commit [a723425c3bbdd16325457402ae73075d1ecc2c4f].

### Reviewed features and constraints

- [REVIEW-00002.2] Review of the whole [FE-0020] feature, including:
  - [FE-0020.1] Provide a "NamedObject" class.
    - [FE-0020.1.1] The class shall have a "name" property that shall be initialized at construction time.
    - [FE-0020.1.2] The class shall have an optional "parent" property.
    - [FE-0020.1.3] The lifecycle of the object shall be managed by the parent object via strong references.
  - [FE-0020.2] The NamedObject class shall provide methods for comparison.
  - [FE-0020.3] The NamedObject class shall provide getter methods.
  - [FE-0020.4] For each class of namespace quasar::coretypes, a derivated class shall be provided.
  - [FE-0020.5] Operations on named objects shall be thread safe.
  - [FE-0020.6] The NamedObject class shall provide a static method "create".
  - [FE-0020.7] The NamedObject class shall support a "related" property.
  - [FE-0020.8] Relations between NamedObject instances shall be implemented using weak pointers.
  - [FE-0020.9] Capabilities shall be provided to convert NamedObject to/from JSON, BSON, YAML, and XML.
  - [FE-0020.10] NamedObject shall provide methods for comparison.
  - [FE-0020.11] NamedObject shall provide methods for hashing.
  - [FE-0020.12] Utilities shall be provided for traversing the tree.
  - [FE-0020.13] Utilities shall be provided for searching the tree.
  - [FE-0020.14] Utilities shall be provided for copying, moving, and removing parts of the tree.

## Review results

- [REVIEW-00002.3] [FE-0020.1] A NamedObject class is provided. Compliant.
- [REVIEW-00002.4] [FE-0020.1.1] The class shall have a "name" property that shall be initialized at construction time. Found no way to build a NamedObject without a name. Compliant.
- [REVIEW-00002.5] [FE-0020.1.2] The class shall have an optional "parent" property. Compliant.
- [REVIEW-00002.6] [FE-0020.1.3] The lifecycle of the object shall be managed by the parent object via strong references. Not demonstrated compliance in code. Not demonstrated in tests.
- [REVIEW-00002.7] [FE-0020.2] The NamedObject class shall provide methods for comparison. Overloaded operators == and < are provided but not tested. Not fully compliant.
- [REVIEW-00002.8] [FE-0020.3] The NamedObject class shall provide getter methods. Implemented but not tested. Not fully compliant.
- [REVIEW-00002.9] [FE-0020.4] For each class of namespace quasar::coretypes, a derivated class shall be provided. Classes found. Support for templated classes found for subtypes of number. Compliant.
- [REVIEW-00002.10] [FE-0020.5] Operations on named objects shall be thread safe. Implementation seems to be thread safe. Need to verify with tests. Not compliant.
- [REVIEW-00002.11] [FE-0020.6] The NamedObject class shall provide a static method "create". Implemented and tested. Compliant.
- [REVIEW-00002.12] [FE-0020.7] The NamedObject class shall support a "related" property. Implemented and tests with a single object instead expected collection of object. Probably badly specified but still not compliant. Implementation shall support multiple related named objects.
- [REVIEW-00002.13] [FE-0020.8] Relations between NamedObject instances shall be implemented using weak pointers. Compliant.
- [REVIEW-00002.14] [FE-0020.9] Capabilities shall be provided to convert NamedObject to/from JSON, BSON, YAML, and XML. JSON serialization seems to be implemented but not tested. BSON serialization not implemented. Not compliant.
- [REVIEW-00002.15] [FE-0020.10] NamedObject shall provide methods for comparison. Implemented with overloaded operators and tested. Compliant.
- [REVIEW-00002.16] [FE-0020.11] NamedObject shall provide methods for hashing. Not implemented. Not compliant.
- [REVIEW-00002.17] [FE-0020.12] Utilities shall be provided for traversing the tree. Implemented and tested. Compliant.
- [REVIEW-00002.18] [FE-0020.13] Utilities shall be provided for searching the tree. Implemented and tested. Compliant.
- [REVIEW-00002.19] [FE-0020.14] Utilities shall be provided for copying, moving, and removing parts of the tree. Implemented cloning of one object. No shallw copy or deep copy found. Not compliant.

### Issues found

- [ISSUE-00002.1] [FE-0020.1.3] The lifecycle of the object shall be managed by the parent object via strong references. Not demonstrated compliance in code. Not demonstrated in tests.
- [ISSUE-00002.2] [FE-0020.2] The NamedObject class shall provide methods for comparison. Overloaded operators == and < are provided but not tested. Not fully compliant.
- [ISSUE-00002.3] [FE-0020.3] The NamedObject class shall provide getter methods. Implemented but not tested. Not fully compliant.
- [ISSUE-00002.4] [FE-0020.5] Operations on named objects shall be thread safe. Implementation seems to be thread safe. Need to verify with tests. Not compliant.
- [ISSUE-00002.5] [FE-0020.7] The NamedObject class shall support a "related" property. Implemented and tests with a single object instead expected collection of object. Probably badly specified but still not compliant. Implementation shall support multiple related named objects.
- [ISSUE-00002.6] [FE-0020.9] Capabilities shall be provided to convert NamedObject to/from JSON, BSON, YAML, and XML. JSON serialization seems to be implemented but not tested. BSON serialization not implemented. Not compliant.
- [ISSUE-00002.7] [FE-0020.11] NamedObject shall provide methods for hashing. Not implemented. Not compliant.
- [ISSUE-00002.8] [FE-0020.14] Utilities shall be provided for copying, moving, and removing parts of the tree. Implemented cloning of one object. No shallw copy or deep copy found. Not compliant.

### Issues resolved

### Issues not resolved

### Issues to be resolved in the future

- [ISSUE-00002.1] [FE-0020.1.3] The lifecycle of the object shall be managed by the parent object via strong references. Not demonstrated compliance in code. Not demonstrated in tests.
- [ISSUE-00002.2] [FE-0020.2] The NamedObject class shall provide methods for comparison. Overloaded operators == and < are provided but not tested. Not fully compliant.
- [ISSUE-00002.3] [FE-0020.3] The NamedObject class shall provide getter methods. Implemented but not tested. Not fully compliant.
- [ISSUE-00002.4] [FE-0020.5] Operations on named objects shall be thread safe. Implementation seems to be thread safe. Need to verify with tests. Not compliant.
- [ISSUE-00002.5] [FE-0020.7] The NamedObject class shall support a "related" property. Implemented and tests with a single object instead expected collection of object. Probably badly specified but still not compliant. Implementation shall support multiple related named objects.
- [ISSUE-00002.6] [FE-0020.9] Capabilities shall be provided to convert NamedObject to/from JSON, BSON, YAML, and XML. JSON serialization seems to be implemented but not tested. BSON serialization not implemented. Not compliant.
- [ISSUE-00002.7] [FE-0020.11] NamedObject shall provide methods for hashing. Not implemented. Not compliant.
- [ISSUE-00002.8] [FE-0020.14] Utilities shall be provided for copying, moving, and removing parts of the tree. Implemented cloning of one object. No shallw copy or deep copy found. Not compliant.

## Conclusion

Review done. 8 non compliances found. Need rework.

