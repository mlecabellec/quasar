/**
 * @class FloatCodec
 * @brief Codec for encoding/decoding floating-point values (float, double).
 *
 * This codec handles the serialization and deserialization of floating-point
 * numbers. While less common than integers in basic EtherCAT control, floats
 * may be used in advanced process data (e.g., sensor readings, control loops)
 * or configuration parameters where precision is required. This supports
 * FE-0040 by providing the necessary tools for handling such data types.
 *
 * Contribution to FE-0020: Implements the ICodec interface by creating and returning
 * `quasar::named::NamedFloatingPoint<T>` objects, directly fulfilling FE-0020.4.
 * This demonstrates the ability to represent floating-point data as named objects,
 * extending the framework's capability to handle diverse data types.
 */
template <typename T> class FloatCodec : public ICodec {
public:
  static_assert(std::is_floating_point<T>::value,
                "FloatCodec only supports floating-point types");

  /**
   * @brief Constructs a FloatCodec.
   * @param isBigEndian True for Big Endian, False for Little Endian.
   */
  FloatCodec(bool isBigEndian = true)
      : m_bitSize(sizeof(T) * 8), m_isBigEndian(isBigEndian) {}

  std::shared_ptr<quasar::named::NamedObject>
  decode(const quasar::coretypes::BitBufferSlice &buffer) const override {

    if (buffer.size() < m_bitSize) {
      throw std::out_of_range("Buffer too small for FloatCodec");
    }

    // Placeholder for bit reading logic.
    // We would read sizeof(T) bytes/bits and reinterpret cast to T.
    // For now, assuming T initialized to 0.0
    T value = 0.0;

    // Return a NamedFloatingPoint unnamed (name to be assigned by parent
    // schema/container), fulfilling FE-0020.4.
    return quasar::named::NamedFloatingPoint<T>::create("", value);
  }

  void encode(const std::shared_ptr<quasar::named::NamedObject> &value,
              quasar::coretypes::BitBufferSlice &buffer) const override {

    std::shared_ptr<quasar::named::NamedFloatingPoint<T>> namedFloat =
        std::dynamic_pointer_cast<quasar::named::NamedFloatingPoint<T>>(value);
    if (!namedFloat) {
      throw std::invalid_argument("Invalid type for FloatCodec encoding");
    }

    if (buffer.size() < m_bitSize) {
      throw std::out_of_range("Buffer too small for FloatCodec encoding");
    }

    // T rawValue = namedFloat->value();
    // Placeholder for bit writing logic.
  }

  size_t getBitSize() const override { return m_bitSize; }

  size_t getEncodedBitSize(
      const std::shared_ptr<quasar::named::NamedObject> &value) const override {
    return m_bitSize;
  }

private:
  size_t m_bitSize;
  bool m_isBigEndian;
};

} // namespace datacodec

#endif // DATACODEC_FLOATCODEC_HPP
