/*! \file       deepseek_error_codes.h
 *  \brief      DeepSeek Error Codes (header file).
 *  \details    Contains the DeepSeek Error Codes.
 *  \author     Arthur Markaryan
 *  \date       09.11.2025
 *  \copyright  Arthur Markaryan
 */

#ifndef _DEEPSEEK_ERROR_CODES_H_
#define _DEEPSEEK_ERROR_CODES_H_

/* Error Codes */

/*
 *  When calling DeepSeek API, you may encounter errors. Here list the causes and solutions.
 */
/*
 *      NAME                                        CODE        DESCRIPTION
 */
#define DEEPSEEK_ERROR_CODES_INVALID_FORMAT         400 //!<    Invalid Format
                                                        //      Cause:      Invalid request body format.
                                                        //      Solution:   Please modify your request body according to the hints in the error message.
                                                        //                  For more API format details, please refer to DeepSeek API Docs.
#define DEEPSEEK_ERROR_CODES_AUTHENTICATION_FAILS   401 //!<    Authentication Fails
                                                        //      Cause:      Authentication fails due to the wrong API key.
                                                        //      Solution:   Please check your API key. If you don't have one, please create an API key first.
#define DEEPSEEK_ERROR_CODES_INSUFFICIENT_BALANCE   402 //!<    Insufficient Balance
                                                        //      Cause:      You have run out of balance.
                                                        //      Solution:   Please check your account's balance, and go to the Top up page to add funds.
#define DEEPSEEK_ERROR_CODES_INVALID_PARAMETERS     422 //!<    Invalid Parameters
                                                        //      Cause:      Your request contains invalid parameters.
                                                        //      Solution:   Please modify your request parameters according to the hints in the error message.
                                                        //                  For more API format details, please refer to DeepSeek API Docs.
#define DEEPSEEK_ERROR_CODES_RATE_LIMIT_REACHED     429 //!<    Rate Limit Reached
                                                        //      Cause:      You are sending requests too quickly.
                                                        //      Solution:   Please pace your requests reasonably.
                                                        //                  We also advise users to temporarily switch to the APIs of alternative LLM service providers, like OpenAI.
#define DEEPSEEK_ERROR_CODES_SERVER_ERROR           500 //!<    Server Error
                                                        //      Cause:      Our server encounters an issue.
                                                        //      Solution:   Please retry your request after a brief wait and contact us if the issue persists.
#define DEEPSEEK_ERROR_CODES_SERVER_OVERLOADED      503 //!<    Server Overloaded
                                                        //      Cause:      The server is overloaded due to high traffic.
                                                        //      Solution:   Please retry your request after a brief wait.

#endif /* _DEEPSEEK_ERROR_CODES_H_ */
