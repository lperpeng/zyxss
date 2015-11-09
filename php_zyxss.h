/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2015 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:                                                              |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifndef PHP_ZYXSS_H
#define PHP_ZYXSS_H

/* yum install pcre-devel */
/* yum install pcre-7.8-6.el6.x86_64 */
#include "pcre.h"

extern zend_module_entry zyxss_module_entry;
#define phpext_zyxss_ptr &zyxss_module_entry

#ifdef PHP_WIN32
#	define PHP_ZYXSS_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_ZYXSS_API __attribute__ ((visibility("default")))
#else
#	define PHP_ZYXSS_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

/* obj */
typedef struct _zyxss_object {
	zend_object std;
} zyxss_object;

PHP_MINIT_FUNCTION(zyxss);
PHP_MSHUTDOWN_FUNCTION(zyxss);
PHP_RINIT_FUNCTION(zyxss);
PHP_RSHUTDOWN_FUNCTION(zyxss);
PHP_MINFO_FUNCTION(zyxss);

PHP_FUNCTION(filter_attributes);
PHP_FUNCTION(filter_tags);
PHP_FUNCTION(filter_characters);
PHP_FUNCTION(filter_xss);

/* 
  	Declare any global variables you may need between the BEGIN
	and END macros here:     
*/
ZEND_BEGIN_MODULE_GLOBALS(zyxss)
	
	char *unsafe_js;
	char *unsafe_html;
	char *unsafe_char;
ZEND_END_MODULE_GLOBALS(zyxss)


/* In every utility function you add that needs to use variables 
   in php_zyxss_globals, call TSRMLS_FETCH(); after declaring other 
   variables used by that function, or better yet, pass in TSRMLS_CC
   after the last function argument and declare your utility function
   with TSRMLS_DC after the last declared argument.  Always refer to
   the globals in your function as ZYXSS_G(variable).  You are 
   encouraged to rename these macros something shorter, see
   examples in any other php module directory.
*/

#ifdef ZTS
#define ZYXSS_G(v) TSRMG(zyxss_globals_id, zend_zyxss_globals *, v)
#else
#define ZYXSS_G(v) (zyxss_globals.v)
#endif

#endif	/* PHP_ZYXSS_H */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
