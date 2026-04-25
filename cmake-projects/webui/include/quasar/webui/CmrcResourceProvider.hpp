#ifndef QUASAR_WEBUI_CMRCRESOURCEPROVIDER_HPP
#define QUASAR_WEBUI_CMRCRESOURCEPROVIDER_HPP

/**
 * @file CmrcResourceProvider.hpp
 * @brief Implementation of IResourceProvider for CMRC embedded resources.
 */

#include "quasar/webui/IResourceProvider.hpp"
#include <cmrc/cmrc.hpp>
#include <filesystem>

namespace quasar::webui {

/**
 * @class CmrcResourceProvider
 * @brief Provides resources from a CMRC embedded filesystem.
 */
class CmrcResourceProvider : public IResourceProvider {
public:
    /**
     * @brief Constructor.
     * @param fs The CMRC embedded filesystem handle.
     */
    explicit CmrcResourceProvider(cmrc::embedded_filesystem fs) : m_fs(fs) {}

    /** @brief [CS-0010.45] Destructor. */
    ~CmrcResourceProvider() override = default;

    /**
     * @brief [CS-0020.48] Retrieves resource from CMRC.
     */
    [[nodiscard]] std::expected<Resource, std::string> getResource(const std::string& path) const override {
        // [CS-0010.44] Normalize path: CMRC paths do not start with /
        std::string normalizedPath = path;
        if (!normalizedPath.empty() && normalizedPath[0] == '/') {
            normalizedPath = normalizedPath.substr(1);
        }

        if (m_fs.exists(normalizedPath) && m_fs.is_file(normalizedPath)) {
            // [CS-0010.34] No auto.
            cmrc::file file = m_fs.open(normalizedPath);
            Resource res;
            // [CS-0010.44] Copy data from CMRC view to vector.
            res.data.assign(file.begin(), file.end());
            res.mimeType = detectMimeType(normalizedPath);
            return res;
        }

        return std::unexpected("Resource not found in CMRC: " + path);
    }

    /**
     * @brief Checks existence in CMRC.
     */
    [[nodiscard]] bool hasResource(const std::string& path) const override {
        std::string normalizedPath = path;
        if (!normalizedPath.empty() && normalizedPath[0] == '/') {
            normalizedPath = normalizedPath.substr(1);
        }
        return m_fs.exists(normalizedPath) && m_fs.is_file(normalizedPath);
    }

private:
    /** @brief The CMRC filesystem view. */
    cmrc::embedded_filesystem m_fs;

    /**
     * @brief Simple MIME detection.
     */
    [[nodiscard]] std::string detectMimeType(const std::string& path) const {
        // [CS-0010.34] No auto.
        std::filesystem::path p(path);
        std::string ext = p.extension().string();
        if (ext == ".html") return "text/html";
        if (ext == ".js") return "application/javascript";
        if (ext == ".css") return "text/css";
        if (ext == ".json") return "application/json";
        if (ext == ".svg") return "image/svg+xml";
        if (ext == ".png") return "image/png";
        return "application/octet-stream";
    }
};

} // namespace quasar::webui

#endif // QUASAR_WEBUI_CMRCRESOURCEPROVIDER_HPP
