/*
  +----------------------------------------------------------------------+
  | tast-process                                                         |
  +----------------------------------------------------------------------+
  | Copyright (c) 2017 xujiajun                                          |
  +----------------------------------------------------------------------+
  | This source file is subject to MIT license,                          |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | https://opensource.org/licenses/MIT                                  |
  | If you did not receive a copy of the MIT license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | me@xujiajun.cn so we can mail you a copy immediately.                |
  +----------------------------------------------------------------------+
  | Author:  xujiajun <me@xujiajun.cn>                                   |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifndef PHP_TAST_PROCESS_H
#define PHP_TAST_PROCESS_H

extern zend_module_entry tast_process_module_entry;
#define phpext_tast_process_ptr &tast_process_module_entry

#define PHP_TAST_PROCESS_VERSION "0.1.0"

#ifdef PHP_WIN32
# define PHP_TAST_PROCESS_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
# define PHP_TAST_PROCESS_API __attribute__ ((visibility("default")))
#else
# define PHP_TAST_PROCESS_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

// define for tast_process
#define tast_php_error(level, fmt_str, ...)   php_error_docref(NULL, level, fmt_str, ##__VA_ARGS__)
/*
    Declare any global variables you may need between the BEGIN
  and END macros here:

ZEND_BEGIN_MODULE_GLOBALS(tast_process)
  zend_long  global_value;
  char *global_string;
ZEND_END_MODULE_GLOBALS(tast_process)
*/

/* Always refer to the globals in your function as TAST_PROCESS_G(variable).
   You are encouraged to rename these macros something shorter, see
   examples in any other php module directory.
*/
#define TAST_PROCESS_G(v) ZEND_MODULE_GLOBALS_ACCESSOR(tast_process, v)

#if defined(ZTS) && defined(COMPILE_DL_TAST_PROCESS)
ZEND_TSRMLS_CACHE_EXTERN()
#endif

#endif  /* PHP_TAST_PROCESS_H */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
