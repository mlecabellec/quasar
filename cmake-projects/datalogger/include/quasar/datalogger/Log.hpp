#pragma once

#include "quasar/datalogger/DataLoggerService.hpp"

/**
 * @file Log.hpp
 * @brief High-level logging macros for the Quasar framework.
 * 
 * Provides a simplified interface to the DataLoggerService singleton.
 * All logs are timestamped and queued asynchronously.
 * 
 * @reference [CS-0010] C++ level and standards
 */

/**
 * @brief Base logging macro.
 * @param lvl The LogLevel (e.g., LogLevel::INFO).
 * @param msg The message string to log.
 */
#define QUASAR_LOG(lvl, msg) \
    do { \
        std::shared_ptr<quasar::datalogger::DataLoggerService> logger_inst = \
            quasar::datalogger::DataLoggerService::getInstance(); \
        if (logger_inst) { \
            logger_inst->logEvent(lvl, msg); \
        } \
    } while (0)

/** @brief Log a DEBUG level message. */
#define LOG_DEBUG(msg)    QUASAR_LOG(quasar::datalogger::LogLevel::DEBUG, msg)

/** @brief Log an INFO level message. */
#define LOG_INFO(msg)     QUASAR_LOG(quasar::datalogger::LogLevel::INFO, msg)

/** @brief Log a WARNING level message. */
#define LOG_WARN(msg)     QUASAR_LOG(quasar::datalogger::LogLevel::WARNING, msg)

/** @brief Log an ERROR level message. */
#define LOG_ERROR(msg)    QUASAR_LOG(quasar::datalogger::LogLevel::ERROR, msg)

/** @brief Log a CRITICAL level message. */
#define LOG_CRITICAL(msg) QUASAR_LOG(quasar::datalogger::LogLevel::CRITICAL, msg)
