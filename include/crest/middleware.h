/**
 * @file middleware.h
 * @brief Built-in middleware for Crest
 */

#ifndef CREST_MIDDLEWARE_H
#define CREST_MIDDLEWARE_H

#include "crest.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief CORS middleware
 * @return Middleware function
 */
CREST_API crest_middleware_fn_t crest_middleware_cors(void);

/**
 * @brief Logger middleware
 * @return Middleware function
 */
CREST_API crest_middleware_fn_t crest_middleware_logger(void);

/**
 * @brief Body parser middleware
 * @return Middleware function
 */
CREST_API crest_middleware_fn_t crest_middleware_body_parser(void);

/**
 * @brief Static file server middleware
 * @param dir Directory to serve files from
 * @return Middleware function
 */
CREST_API crest_middleware_fn_t crest_middleware_static(const char *dir);

/**
 * @brief Rate limiting middleware
 * @param max_requests Maximum requests per window
 * @param window_seconds Time window in seconds
 * @return Middleware function
 */
CREST_API crest_middleware_fn_t crest_middleware_rate_limit(int max_requests, int window_seconds);

/**
 * @brief Authentication middleware (basic)
 * @param validate_fn Function to validate credentials
 * @return Middleware function
 */
CREST_API crest_middleware_fn_t crest_middleware_auth(bool (*validate_fn)(const char *token));

#ifdef __cplusplus
}
#endif

#endif /* CREST_MIDDLEWARE_H */
