/*!
 * \file       deepseek_error_codes.h
 * \brief      DeepSeek Error Codes (header file).
 * \details    Contains the DeepSeek Error Codes and their descriptions.
 * \author     Arthur Markaryan
 * \date       09.11.2025
 * \copyright  Arthur Markaryan
 */

#ifndef _DEEPSEEK_ERROR_CODES_H_
#define _DEEPSEEK_ERROR_CODES_H_

/*!
 * \defgroup error_codes DeepSeek Error Codes
 * \{
 */

/*!
 * \brief Invalid request body format.
 * \code 400
 *
 * \details
 * **Cause:** Invalid request body format.\n
 * **Solution:** Please modify your request body according to the hints in the error message.\n
 * For more API format details, please refer to DeepSeek API Docs.
 */
#define DEEPSEEK_ERROR_CODES_INVALID_FORMAT         400

/*!
 * \brief Authentication fails due to wrong API key.
 * \code 401
 *
 * \details
 * **Cause:** Authentication fails due to the wrong API key.\n
 * **Solution:** Please check your API key. If you don't have one, please create an API key first.
 */
#define DEEPSEEK_ERROR_CODES_AUTHENTICATION_FAILS   401

/*!
 * \brief Insufficient account balance.
 * \code 402
 *
 * \details
 * **Cause:** You have run out of balance.\n
 * **Solution:** Please check your account's balance, and go to the Top up page to add funds.
 */
#define DEEPSEEK_ERROR_CODES_INSUFFICIENT_BALANCE   402

/*!
 * \brief Invalid request parameters.
 * \code 422
 *
 * \details
 * **Cause:** Your request contains invalid parameters.\n
 * **Solution:** Please modify your request parameters according to the hints in the error message.\n
 * For more API format details, please refer to DeepSeek API Docs.
 */
#define DEEPSEEK_ERROR_CODES_INVALID_PARAMETERS     422

/*!
 * \brief Request rate limit exceeded.
 * \code 429
 *
 * \details
 * **Cause:** You are sending requests too quickly.\n
 * **Solution:** Please pace your requests reasonably.\n
 * We also advise users to temporarily switch to the APIs of alternative LLM service providers, like OpenAI.
 */
#define DEEPSEEK_ERROR_CODES_RATE_LIMIT_REACHED     429

/*!
 * \brief Internal server error.
 * \code 500
 *
 * \details
 * **Cause:** Our server encounters an issue.\n
 * **Solution:** Please retry your request after a brief wait and contact us if the issue persists.
 */
#define DEEPSEEK_ERROR_CODES_SERVER_ERROR           500

/*!
 * \brief Server overloaded due to high traffic.
 * \code 503
 *
 * \details
 * **Cause:** The server is overloaded due to high traffic.\n
 * **Solution:** Please retry your request after a brief wait.
 */
#define DEEPSEEK_ERROR_CODES_SERVER_OVERLOADED      503

/*! \} */ // end of group error_codes

#endif /* _DEEPSEEK_ERROR_CODES_H_ */
