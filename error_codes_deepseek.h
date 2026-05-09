/**
 * @file        error_codes_deepseek.h
 * @brief       DeepSeek API Error Codes for aiOr application.
 * @details     Contains HTTP status code definitions for DeepSeek API error responses.
 *              Each error code includes the status code, cause description, and solution guidance.
 *              This file is organized as a Doxygen module (defgroup) for easy reference.
 *
 * @author      Arthur Markaryan
 * @date        09.05.2026
 * @version     1.0.1
 * @license     LGPL v3.0
 * @copyright   Copyright (c) 2026
 *
 * @par Dependencies:
 * - None (pure C/C++ preprocessor macros)
 *
 * @par Usage Example:
 * @code
 * if (httpStatusCode == ERROR_CODES_DEEPSEEK_AUTHENTICATION_FAILS) {
 *     qWarning() << "Invalid API key provided";
 * }
 * @endcode
 *
 * @par ChangeLog:
 * 09.05.2026   v1.0.1  Arthur Markaryan - Rename, modify header of the file
 * 09.11.2025   v1.0    Arthur Markaryan - Initial implementation
 *
 * @see         https://platform.deepseek.com/api-docs/ (DeepSeek API Documentation)
 */

#ifndef _ERROR_CODES_DEEPSEEK_H_
#define _ERROR_CODES_DEEPSEEK_H_

/**
 * @defgroup    error_codes_deepseek DeepSeek API Error Codes
 * @brief       Collection of HTTP status codes returned by DeepSeek API.
 * @details     These error codes follow standard HTTP conventions with DeepSeek-specific
 *              error messages and resolution steps. Use these constants instead of raw
 *              integer values for better code readability and maintainability.
 * @{
 */

/**
 * @brief       Invalid Request Body Format.
 * @def         ERROR_CODES_DEEPSEEK_INVALID_FORMAT
 * @value       400
 * @details     **Cause:** Invalid request body format.\n
 *              **Solution:** Please modify your request body according to the hints in the error message.\n
 *              For more API format details, please refer to DeepSeek API Docs.
 */
#define ERROR_CODES_DEEPSEEK_INVALID_FORMAT		400

/**
 * @brief       Authentication Failure.
 * @def         ERROR_CODES_DEEPSEEK_AUTHENTICATION_FAILS
 * @value       401
 * @details     **Cause:** Authentication fails due to the wrong API key.\n
 *              **Solution:** Please check your API key. If you don't have one, please create an API key first.
 */
#define ERROR_CODES_DEEPSEEK_AUTHENTICATION_FAILS	401

/**
 * @brief       Insufficient Account Balance.
 * @def         ERROR_CODES_DEEPSEEK_INSUFFICIENT_BALANCE
 * @value       402
 * @details     **Cause:** You have run out of balance.\n
 *              **Solution:** Please check your account's balance, and go to the Top up page to add funds.
 */
#define ERROR_CODES_DEEPSEEK_INSUFFICIENT_BALANCE	402

/**
 * @brief       Invalid Request Parameters.
 * @def         ERROR_CODES_DEEPSEEK_INVALID_PARAMETERS
 * @value       422
 * @details     **Cause:** Your request contains invalid parameters.\n
 *              **Solution:** Please modify your request parameters according to the hints in the error message.\n
 *              For more API format details, please refer to DeepSeek API Docs.
 */
#define ERROR_CODES_DEEPSEEK_INVALID_PARAMETERS	422

/**
 * @brief       Rate Limit Exceeded.
 * @def         ERROR_CODES_DEEPSEEK_RATE_LIMIT_REACHED
 * @value       429
 * @details     **Cause:** You are sending requests too quickly.\n
 *              **Solution:** Please pace your requests reasonably.\n
 *              We also advise users to temporarily switch to the APIs of alternative LLM service providers, like OpenAI.
 */
#define ERROR_CODES_DEEPSEEK_RATE_LIMIT_REACHED	429

/**
 * @brief       Internal Server Error.
 * @def         ERROR_CODES_DEEPSEEK_SERVER_ERROR
 * @value       500
 * @details     **Cause:** Our server encounters an issue.\n
 *              **Solution:** Please retry your request after a brief wait and contact us if the issue persists.
 */
#define ERROR_CODES_DEEPSEEK_SERVER_ERROR		500

/**
 * @brief       Server Overloaded (High Traffic).
 * @def         ERROR_CODES_DEEPSEEK_SERVER_OVERLOADED
 * @value       503
 * @details     **Cause:** The server is overloaded due to high traffic.\n
 *              **Solution:** Please retry your request after a brief wait.
 */
#define ERROR_CODES_DEEPSEEK_SERVER_OVERLOADED	503

/** @} */ // end of group error_codes

#endif /* ERROR_CODES_DEEPSEEK_H_ */
