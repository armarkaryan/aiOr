/**
 * @file		utils.h
 * @brief		Module of definitions and additional special functions (header file).
 * @details		Contains type definitions, structures and additional special functionality.
 * @author		Arthur Markaryan
 * @date		11.05.2026
 * @version		1.0.1
 * @license		LGPL v3.0
 * @copyright	Copyright (c) 2026
 *
 * @par Dependencies:
 * - Qt5/6 Core (QString, QDebug, QFileInfo, QTime)
 * - C++ Standard Library (cstdint, chrono, mutex)
 *
 * @par ChangeLog:
 * 11.05.2026	v1.0.1	Arthur Markaryan - Add pretty debug message function
 * 01.05.2026	v1.0	Arthur Markaryan - Base creation
 *
 * @see			UTILS_debug()
 * @see			UTILS_message()
 * @see			UTILS_getSystemTimeMicroseconds()
 * @see			UTILS_getSystemTimeMicrosecondsSafe()
 */

#ifndef _UTILS_H_
#define _UTILS_H_

#include <QString>
#include <QDebug>
#include <QFileInfo>
#include <QTime>
#include <cstdint>
#include <chrono>
#include <mutex>

/**
 * @defgroup utils_macros Debug Message Type Macros
 * @brief       Predefined message type identifiers for UTILS_message() macro.
 * @{
 */

/**
 * @brief       Empty message type (no prefix).
 * @details     Use this macro when no type prefix is desired in the debug output.
 */
#define UTILS_DEBUG_MESSAGE_TYPE_NONE		""		//!< No type prefix

/**
 * @brief       Success message type.
 * @details     Use this macro to mark successful operations in debug output.
 */
#define UTILS_DEBUG_MESSAGE_TYPE_SUCCESS	"[SUCCESS]"	//!< Success indicator

/**
 * @brief       Informational message type.
 * @details     Use this macro for general informational debug messages.
 */
#define UTILS_DEBUG_MESSAGE_TYPE_INFO		"[ INFO  ]"	//!< Information indicator

/**
 * @brief       Warning message type.
 * @details     Use this macro to highlight potential issues that don't break execution.
 */
#define UTILS_DEBUG_MESSAGE_TYPE_WARNING	"[WARNING]"	//!< Warning indicator

/**
 * @brief       Error message type.
 * @details     Use this macro to mark error conditions in debug output.
 */
#define UTILS_DEBUG_MESSAGE_TYPE_ERROR		"[ ERROR ]"	//!< Error indicator

/** @} */ // end of group utils_macros

/**
 * @defgroup utils_debug_macros Debug Output Macros
 * @brief       Macros for formatted debug output with timestamps and context.
 * @{
 */

/**
 * @brief       Pretty debug macro with file, line, and function context.
 * @details     Outputs a formatted debug message containing:
 *              - Current timestamp (hh:mm:ss.zzz)
 *              - Source file name
 *              - Line number
 *              - Function name (__PRETTY_FUNCTION__)
 *
 * @par Usage Example:
 * @code
 * UTILS_debug() << "User logged in successfully";
 * // Output: [14:32:15.123] main.cpp:42 - void MainWindow::login() - User logged in successfully
 * @endcode
 *
 * @note        Uses qDebug().noquote() to prevent automatic quotation of output.
 */
#define UTILS_debug() qDebug().noquote() \
    << QStringLiteral("[%1] %2:%3 - %4") \
        .arg(QTime::currentTime().toString("hh:mm:ss.zzz"), \
             QFileInfo(__FILE__).fileName(), \
             QString::number(__LINE__), \
             QString::fromUtf8(__PRETTY_FUNCTION__))

/**
 * @brief       Formatted message macro with type prefix.
 * @param       type    Message type (use UTILS_DEBUG_MESSAGE_TYPE_* macros)
 * @param       message Message text to output
 * @details     Outputs a formatted debug message with a type prefix and timestamp.
 *
 * @par Usage Example:
 * @code
 * UTILS_message(UTILS_DEBUG_MESSAGE_TYPE_ERROR, "Failed to connect to API");
 * // Output: [ ERROR ][14:32:15.123] Failed to connect to API
 * @endcode
 *
 * @note        Use this macro for categorizing debug output by message severity.
 */
#define UTILS_message(type, message) qDebug().noquote() \
    << QStringLiteral("%1[%2] %3") \
        .arg(QString::fromUtf8(type), \
             QTime::currentTime().toString("hh:mm:ss.zzz"), \
             QString::fromUtf8(message))

       /** @} */ // end of group utils_debug_macros

       /**
 * @brief		Gets monotonic system time in microseconds (thread-local version).
 * @return		Time in microseconds (uint64_t)
 * @details		Uses std::chrono::steady_clock which is monotonic and not affected
 *              by system clock adjustments. Thread-local storage ensures maximum
 *              performance without mutex locking.
 * @note		Fully thread-safe on all architectures (each thread has its own lastTime)
 * @note		Returns time from an arbitrary point (steady_clock), not wall-clock time
 * @note		Protects against time going backwards within the same thread
 */
       static inline uint64_t UTILS_getSystemTimeMicroseconds()
{
    // Use thread-local storage for maximum performance and to avoid locks
    static thread_local uint64_t lastTime = 0;

    auto now = std::chrono::steady_clock::now();
    auto duration = now.time_since_epoch();
    uint64_t current = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();

    // Check for time jumps within a single thread
    if (current < lastTime) {
        // Time went backwards in this thread, return the last valid value
        return lastTime;
    }

    lastTime = current;
    return current;
}

/**
 * @brief		Gets monotonic system time in microseconds with cross-thread protection.
 * @return		Time in microseconds (uint64_t)
 * @details		Similar to UTILS_getSystemTimeMicroseconds() but uses a global mutex
 *              to protect against time jumps that might occur when switching between threads.
 *              This version is slower but safer for multi-threaded time comparisons.
 * @note		Slower than the thread-local version, but protects against cross-thread time jumps
 * @note		Uses static mutex for synchronization across all threads
 * @note		Recommended when time values need to be compared across different threads
 */
static inline uint64_t UTILS_getSystemTimeMicrosecondsSafe()
{
    static std::mutex timeMutex;
    static uint64_t lastTime = 0;

    auto now = std::chrono::steady_clock::now();
    auto duration = now.time_since_epoch();
    uint64_t current = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();

    std::lock_guard<std::mutex> lock(timeMutex);
    if (current < lastTime) {
        return lastTime;
    }
    lastTime = current;
    return current;
}

#endif // _UTILS_H_
