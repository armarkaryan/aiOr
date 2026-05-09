/**
 * @file        error_codes_groq.h
 * @brief       Groq API Error Codes for aiOr application.
 * @details     Contains HTTP status code definitions for Groq API error responses.
 *              Each error code includes the status code, cause description, and solution guidance.
 *              This file includes additional Groq-specific error codes such as MODEL_NOT_FOUND
 *              and MODEL_NOT_AVAILABLE that extend beyond standard HTTP status codes.
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
 * @par Groq Error Codes Reference:
 * - 400: Invalid request body format
 * - 401: Authentication failure
 * - 402: Insufficient balance
 * - 422: Invalid parameters
 * - 429: Rate limit exceeded
 * - 500: Internal server error
 * - 503: Server overloaded
 * - 504: Model not found (Groq-specific)
 * - 505: Model not available (Groq-specific)
 *
 * @par Usage Example:
 * @code
 * if (httpStatusCode == GROQ_ERROR_CODES_MODEL_NOT_FOUND) {
 *     qWarning() << "Requested model does not exist";
 * }
 * @endcode
 *
 * @par ChangeLog:
 * 09.05.2026   v1.0.1  Arthur Markaryan - Rename, modify header of the file
 * 04.12.2025   v1.0    Arthur Markaryan - Initial implementation
 *
 * @see         https://console.groq.com/docs/errors (Groq API Error Documentation)
 */

#ifndef _ERROR_CODES_GROQ_H_
#define _ERROR_CODES_GROQ_H_

/**
 * @defgroup    error_codes_groq Groq API Error Codes
 * @brief       Collection of HTTP status codes and Groq-specific error codes.
 * @details     These error codes include standard HTTP status codes used by Groq API,
 *              as well as custom codes (504, 505) for Groq-specific error conditions.
 *              Error causes and resolution steps are documented for each code.
 * @{
 */

/**
 * @brief       Invalid Request Body Format.
 * @def         ERROR_CODES_GROQ_INVALID_FORMAT
 * @value       400
 * @details     **Cause:** Invalid request body format.\n
 *              **Solution:** Please modify your request body according to the hints in the error message.\n
 *              For more API format details, please refer to Groq API Docs.
 */
#define ERROR_CODES_GROQ_INVALID_FORMAT		400

/**
 * @brief       Authentication Failure.
 * @def         ERROR_CODES_GROQ_AUTHENTICATION_FAILS
 * @value       401
 * @details     **Cause:** Authentication fails due to the wrong API key.\n
 *              **Solution:** Please check your API key. If you don't have one, please create an API key first.
 */
#define ERROR_CODES_GROQ_AUTHENTICATION_FAILS	401

/**
 * @brief       Insufficient Account Balance.
 * @def         ERROR_CODES_GROQ_INSUFFICIENT_BALANCE
 * @value       402
 * @details     **Cause:** You have run out of balance.\n
 *              **Solution:** Please check your account's balance, and go to the Top up page to add funds.
 */
#define ERROR_CODES_GROQ_INSUFFICIENT_BALANCE	402

/**
 * @brief       Invalid Request Parameters.
 * @def         ERROR_CODES_GROQ_INVALID_PARAMETERS
 * @value       422
 * @details     **Cause:** Your request contains invalid parameters.\n
 *              **Solution:** Please modify your request parameters according to the hints in the error message.\n
 *              For more API format details, please refer to Groq API Docs.
 */
#define ERROR_CODES_GROQ_INVALID_PARAMETERS	422

/**
 * @brief       Rate Limit Exceeded.
 * @def         ERROR_CODES_GROQ_RATE_LIMIT_REACHED
 * @value       429
 * @details     **Cause:** You are sending requests too quickly.\n
 *              **Solution:** Please pace your requests reasonably.\n
 *              We also advise users to temporarily switch to the APIs of alternative LLM service providers, like OpenAI.
 */
#define ERROR_CODES_GROQ_RATE_LIMIT_REACHED	429

/**
 * @brief       Internal Server Error.
 * @def         GROQ_ERROR_CODES_SERVER_ERROR
 * @value       500
 * @details     **Cause:** Our server encounters an issue.\n
 *              **Solution:** Please retry your request after a brief wait and contact us if the issue persists.
 */
#define ERROR_CODES_GROQ_SERVER_ERROR		500

/**
 * @brief       Server Overloaded (High Traffic).
 * @def         GROQ_ERROR_CODES_SERVER_OVERLOADED
 * @value       503
 * @details     **Cause:** The server is overloaded due to high traffic.\n
 *              **Solution:** Please retry your request after a brief wait.
 */
#define ERROR_CODES_GROQ_SERVER_OVERLOADED	503

/**
 * @brief       Model Not Found.
 * @def         GROQ_ERROR_CODES_MODEL_NOT_FOUND
 * @value       504
 * @details     **Cause:** The requested model is not found.\n
 *              **Solution:** Please check the model name and try again.
 */
#define ERROR_CODES_GROQ_MODEL_NOT_FOUND	504

/**
 * @brief       Model Not Available.
 * @def         ERROR_CODES_GROQ_MODEL_NOT_AVAILABLE
 * @value       505
 * @details     **Cause:** The requested model is not available.\n
 *              **Solution:** Please try another model or contact us for more information.
 */
#define ERROR_CODES_GROQ_MODEL_NOT_AVAILABLE	505

/** @} */ // end of group groq_error_codes

#endif /* _ERROR_CODES_GROQ_H_ */
