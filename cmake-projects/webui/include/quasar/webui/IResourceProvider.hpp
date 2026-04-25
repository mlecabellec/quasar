#ifndef QUASAR_WEBUI_IRESOURCEPROVIDER_HPP
#define QUASAR_WEBUI_IRESOURCEPROVIDER_HPP

/**
 * @file IResourceProvider.hpp
 * @brief Interface for abstraction of web resource sources (filesystem, binary embedded).
 */

#include <string>
#include <vector>
#include <optional>
#include <expected>
#include <cstdint>
#include <span>

namespace quasar::webui {

/**
 * @struct Resource
 * @brief Represents a single web resource (file) with its content and metadata.
 */
struct Resource {
    /** @brief Binary content of the resource. */
    std::vector<std::uint8_t> data;
    /** @brief MIME type (e.g., "text/html"). */
    std::string mimeType;
};

/**
 * @class IResourceProvider
 * @brief Abstract interface for providing static web resources.
 * 
 * [CS-0010.45] Doxygen documentation mandatory for all classes.
 */
class IResourceProvider {
public:
    /** @brief Virtual destructor. [CS-0010.45] */
    virtual ~IResourceProvider() = default;

    /**
     * @brief Retrieves a resource by its path.
     * @param path The relative URL path (e.g., "/index.html").
     * @return std::expected containing the Resource if found, or an error message.
     * [CS-0020.48] Prefer std::expected for error propagation.
     */
    [[nodiscard]] virtual std::expected<Resource, std::string> getResource(const std::string& path) const = 0;

    /**
     * @brief Checks if the provider contains a resource at the given path.
     * @param path The relative URL path.
     * @return True if the resource exists.
     */
    [[nodiscard]] virtual bool hasResource(const std::string& path) const = 0;
};

} // namespace quasar::webui

#endif // QUASAR_WEBUI_IRESOURCEPROVIDER_HPP
