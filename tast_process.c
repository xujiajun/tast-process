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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_tast_process.h"

#include <errno.h>

/* If you declare any globals in php_tast_process.h uncomment this:
ZEND_DECLARE_MODULE_GLOBALS(tast_process)
*/

/* {{{ arginfo */
ZEND_BEGIN_ARG_INFO_EX(arginfo_process, 0, 0, 2)
    ZEND_ARG_ARRAY_INFO(0, funs_arr, 0)
    ZEND_ARG_INFO(0, open_process_num)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_fork_void, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_kill, 0, 0, 2)
    ZEND_ARG_INFO(0, pid)
    ZEND_ARG_INFO(0, sig)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_waitpid, 0, 0, 2)
    ZEND_ARG_INFO(0, pid)
    ZEND_ARG_INFO(1, z_status)
    ZEND_ARG_INFO(0, options)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_getpid_void, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_getppid_void, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_wifexitstatus, 0, 0, 1)
    ZEND_ARG_INFO(1, status)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_strerror, 0, 0, 1)
    ZEND_ARG_INFO(0, errno)
ZEND_END_ARG_INFO()



/* True global resources - no need for thread safety here */
static int le_tast_process;

/* {{{ PHP_INI
 */
/* Remove comments and fill if you need to have entries in php.ini
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("tast_process.global_value",      "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_tast_process_globals, tast_process_globals)
    STD_PHP_INI_ENTRY("tast_process.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_tast_process_globals, tast_process_globals)
PHP_INI_END()
*/
/* }}} */

/* Remove the following function when you have successfully modified config.m4
   so that your module can be compiled into PHP, it exists only for testing
   purposes. */

/* Every user-visible function in PHP should document itself in the source */
/* {{{ proto string confirm_tast_process_compiled(string arg)
   Return a string to confirm that the module is compiled in */

#define MAX_PROCESS_NUM 100

static pid_t createProcess(void(*ptr) (),zval *arg);
static int addInProcessList(pid_t pid);
static void waitAllProcess();
static int isOutsideProcess(pid_t pid);
static int isOutsideAllProcess();
static int processError();
static void killProcess();
static void killChildren();
static void childProcess(zval *arg);
static void parentForkInfo(pid_t pid);
static int tprintf(const char* fmt, ...);

int processCount = 0;

pid_t processList[MAX_PROCESS_NUM];

PHP_FUNCTION(tast_process)
{
    zval *funs_arr,*fun;

    long open_process_num;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "al", &funs_arr, &open_process_num) == FAILURE) {
        return;
    }

    if (open_process_num < 0 || open_process_num > MAX_PROCESS_NUM) {
        tast_php_error(E_ERROR, "Invalid argument `open_process_num` (%d)", open_process_num);
    }

    printf("Hello from Parent Process, PID is %d.\n", getpid());
    
    for (int i = 0; i < open_process_num; ++i) {
        fun = zend_hash_index_find(Z_ARRVAL_P(funs_arr), i);
        createProcess(childProcess,fun);
    }

    if (isOutsideAllProcess()) {
        for (int i = 0; i < open_process_num; ++i) {
            parentForkInfo(processList[i]);
        }
        waitAllProcess();
        tprintf("Children processes have exited.\n");
        tprintf("Parent had exited.\n");
    } else if (!processError()) {
        tprintf("Everything was gone without error.\n");
    }

    RETURN_NULL();
}

PHP_FUNCTION(tast_fork)
{
    pid_t pid;

    pid = fork();

    if (pid < 0) {
        tast_php_error(E_WARNING, "Fork error (%s) \n", strerror(errno));
    }

    RETURN_LONG((zend_long) pid);
}

PHP_FUNCTION(tast_getpid)
{
    pid_t pid = getpid();
    RETURN_LONG((zend_long) pid);
}

PHP_FUNCTION(tast_getppid)
{
    pid_t ppid = getppid();
    RETURN_LONG((zend_long) ppid);
}

PHP_FUNCTION(tast_kill)
{
    int pid;
    int sig = SIGTERM;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l|l", &pid, &sig) == FAILURE) {
        return;
    }

    int ret = kill((int) pid,(int) sig);

    if (ret < 0)
    {
        if (!(sig == 0 && errno == ESRCH))
        {
            tast_php_error(E_WARNING, "kill(%d, %d) failed. Error: %s[%d]", (int) pid, (int) sig, strerror(errno), errno);
        }

        RETURN_FALSE;
    }

    RETURN_TRUE;
}

PHP_FUNCTION(tast_waitpid)
{
    zend_long pid, options = 0;
    zval *z_status = NULL;
    int status;
    pid_t child_id;
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "lz/|l", &pid, &z_status, &options) == FAILURE) {
        return;
    }

    status = zval_get_long(z_status);

    child_id = waitpid((pid_t) pid, &status, options);

    if (child_id < 0 && errno != ECHILD) {
        tast_php_error(E_WARNING, "error in waitpid (%s) \n", strerror(errno));
    }

    zval_dtor(z_status);
    ZVAL_LONG(z_status, status);

    RETURN_LONG((zend_long) child_id);
}

PHP_FUNCTION(tast_wexitstatus)
{
    #ifdef WEXITSTATUS
        zend_long status_word;

        if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &status_word) == FAILURE) {
               return;
        }

        RETURN_LONG(WEXITSTATUS(status_word));
    #else
        RETURN_FALSE;
    #endif
}

PHP_FUNCTION(tast_strerror)
{
    zend_long error;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &error) == FAILURE) {
        RETURN_FALSE;
    }

    RETURN_STRING(strerror(error));
}

static int addInProcessList(pid_t pid)
{
    if (processCount < MAX_PROCESS_NUM) {
        processList[processCount] = pid;
        processCount = processCount + 1;
        return SUCCESS;
    }

    return FAILURE;
}

static void waitAllProcess()
{
    int i;

    for(i = 0;i < processCount; ++i){
        waitpid(processList[i], NULL, 0);
    }
}

static int processError()
{
    int i;

    for(i = 0;i < processCount; ++i){
        if(processList[i] == -1){
            return SUCCESS;
        }
    }
    return FAILURE;
}

static pid_t createProcess(void(*ptr) (),zval *arg)
{
    pid_t pid;

    if (isOutsideAllProcess()) {
        pid = fork();
        if (FAILURE != addInProcessList(pid)) {
            if (pid == 0) {
                (*ptr)(arg);
            }
        } else {
            printf("Full processes!");
            killChildren();
            exit(1);
        }
    }
    return pid;
}

static void killProcess(pid_t pid)
{
    if (pid == 0) {
        printf("Process %d exit!", getpid());
        exit(1);
    }
}

static void killChildren()
{
    int i;

    for (i = 0;i < processCount; ++i) {
        killProcess(processList[i]);
    }
}

static int isOutsideProcess(pid_t pid)
{
    return (int) (pid != -1 && pid != 0);
}

static int isOutsideAllProcess()
{
    int i;

    for (i = 0;i < processCount;++i) {
        if (isOutsideProcess(processList[i]) == 0) {
            return 0;
        }
    }

    return 1;
}

static void childProcess(zval *arg)
{

    int i;
    zval ret;

    if (FAILURE == call_user_function_ex(EG(function_table), NULL, arg, &ret, 0, NULL, 0, NULL TSRMLS_CC)) {
         tast_php_error(E_WARNING, "failed to call function ");
    }

    tprintf("Hello from Child Process %d. %d times\n", getpid(), i+1);
}

static void parentForkInfo(pid_t pid)
{
    tprintf("Parent forked one child process -- %d.\n", pid);
    tprintf("Parent is waiting for children to exit.\n");
}

static int tprintf(const char* fmt, ...)
{
    va_list args;

    struct tm * tstruct;
    time_t tsec = time(NULL);
    tstruct = localtime (&tsec);
    printf("%02d:%02d:%02d: %5d|", tstruct->tm_hour, tstruct->tm_min, tstruct->tm_sec, getpid());
    va_start(args, fmt);

    return vprintf(fmt, args);
}

/* }}} */
/* The previous line is meant for vim and emacs, so it can correctly fold and
   unfold functions in source code. See the corresponding marks just before
   function definition, where the functions purpose is also documented. Please
   follow this convention for the convenience of others editing your code.
*/
const zend_function_entry tast_process_functions[] = {
    PHP_FE(tast_process, arginfo_process)
    PHP_FE(tast_fork, arginfo_fork_void)
    PHP_FE(tast_kill, arginfo_kill)
    PHP_FE(tast_getpid, arginfo_getpid_void)
    PHP_FE(tast_getppid, arginfo_getppid_void)
    PHP_FE(tast_waitpid, arginfo_waitpid)
    PHP_FE(tast_wexitstatus, arginfo_wifexitstatus)
    PHP_FE(tast_strerror, arginfo_strerror)
    PHP_FE_END
};

/* {{{ php_tast_process_init_globals
 */
/* Uncomment this function if you have INI entries
static void php_tast_process_init_globals(zend_tast_process_globals *tast_process_globals)
{
    tast_process_globals->global_value = 0;
    tast_process_globals->global_string = NULL;
}
*/
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(tast_process)
{
    /* If you have INI entries, uncomment these lines
    REGISTER_INI_ENTRIES();
    */
    return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(tast_process)
{
    /* uncomment this line if you have INI entries
    UNREGISTER_INI_ENTRIES();
    */
    return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(tast_process)
{
#if defined(COMPILE_DL_TAST_PROCESS) && defined(ZTS)
    ZEND_TSRMLS_CACHE_UPDATE();
#endif
    return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(tast_process)
{
    return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(tast_process)
{
    php_info_print_table_start();
    php_info_print_table_header(2, "tast_process support", "enabled");
    php_info_print_table_row(2, "version", PHP_TAST_PROCESS_VERSION);
    php_info_print_table_end();

    /* Remove comments if you have entries in php.ini
    DISPLAY_INI_ENTRIES();
    */
}
/* }}} */

/* {{{ tast_process_module_entry
 */
zend_module_entry tast_process_module_entry = {
    STANDARD_MODULE_HEADER,
    "tast_process",
    tast_process_functions,
    PHP_MINIT(tast_process),
    PHP_MSHUTDOWN(tast_process),
    PHP_RINIT(tast_process),        /* Replace with NULL if there's nothing to do at request start */
    PHP_RSHUTDOWN(tast_process),    /* Replace with NULL if there's nothing to do at request end */
    PHP_MINFO(tast_process),
    PHP_TAST_PROCESS_VERSION,
    STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_TAST_PROCESS
#ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
#endif
ZEND_GET_MODULE(tast_process)
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
