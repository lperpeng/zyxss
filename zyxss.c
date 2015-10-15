/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2013 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: lper<lper@foxmail.com>                                                             |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_zyxss.h"

/* If you declare any globals in php_zyxss.h uncomment this:
ZEND_DECLARE_MODULE_GLOBALS(zyxss)
*/

/* True global resources - no need for thread safety here */
static int le_zyxss;
zend_class_entry *zyxss_ce;
static zend_object_handlers zyxss_obj_handlers;

/* zyxss object dtor */
void zyxss_obj_dtor(void *object TSRMLS_DC) {/*{{{*/
	zyxss_object *obj = (zyxss_object *)object;
	zend_hash_destroy(obj->std.properties);
	FREE_HASHTABLE(obj->std.properties);
	efree(obj);
}/*}}}*/

/* zyxss object ctor */
zend_object_value zyxss_obj_ctor(zend_class_entry *ce TSRMLS_DC) {/*{{{*/
	zval *tmp;
	zend_object_value retval;

	zyxss_object *obj = emalloc(sizeof(zyxss_object));
	memset(obj, 0, sizeof(zyxss_object));
	obj->std.ce = ce;

	ALLOC_HASHTABLE(obj->std.properties);
	zend_hash_init(obj->std.properties, 0, NULL, ZVAL_PTR_DTOR, 0);
	zend_hash_copy(obj->std.properties, &ce->properties_info,
		(copy_ctor_func_t)zval_add_ref, (void *)&tmp, sizeof(zval *));

	retval.handle = zend_objects_store_put(obj, NULL, zyxss_obj_dtor, NULL TSRMLS_CC);
	retval.handlers = &zyxss_obj_handlers;
	/* other object initialization */
	return retval;
}/*}}}*/

static char *zyxss_preg_replace(char *strRegex, char* source, int source_len TSRMLS_DC) {/*{{{*/
	pcre *reCompiled;
	pcre_extra *pcreExtra;
	int pcreExecRet;  /* count of matches */
	const char *pcreErrorStr;
	int pcreErrorOffset;
	int num_subpats;  /* number of capturing subpatterns */
	int size_offsets;
	int *offsets;     /* store match offset index */
	int start_offset; /* starting point of pcre_exec */
	int copy_offset;  /* starting point of memcpy */
	int index;
	char *result;

	int coptions = 0;
	coptions |= PCRE_CASELESS; /* case insensitive search */
	reCompiled = pcre_compile(strRegex, coptions, &pcreErrorStr, &pcreErrorOffset, NULL);
	if(reCompiled == NULL) {
		return NULL;
	}

	pcreExtra = pcre_study(reCompiled, 0, &pcreErrorStr);
	if(pcreErrorStr != NULL) {
		return NULL;
	}

	int rc = pcre_fullinfo(reCompiled, pcreExtra, PCRE_INFO_CAPTURECOUNT, &num_subpats);
	if (rc < 0) {
		return NULL;
	}
	num_subpats++;
	size_offsets = num_subpats * 3;
	offsets = (int *)safe_emalloc(size_offsets, sizeof(int), 0);
	result = safe_emalloc(source_len + 1, sizeof(char), 0);

	copy_offset = start_offset = 0;
	while (1) {
		pcreExecRet = pcre_exec(reCompiled, pcreExtra, source, source_len, start_offset, 0, offsets, size_offsets);
		if (pcreExecRet < 0) {
			memcpy(result + copy_offset, source + start_offset, source_len - start_offset);
			*(result + copy_offset + source_len - start_offset) = '\0';
			break;
		} else {
			if (pcreExecRet == 0) {
				pcreExecRet = size_offsets / 3;
			}
			index = pcreExecRet - 1;
			memcpy(result + copy_offset, source + start_offset, offsets[index * 2] - start_offset);
			copy_offset += offsets[index * 2] - start_offset;
			start_offset = offsets[index * 2 + 1];
		}
	}

	pcre_free(reCompiled);
	if (pcreExtra != NULL) {
		pcre_free(pcreExtra);
	}
	efree(offsets);
	return result;
}
/*}}}*/

static char *getBadTagsPattern(int unclosed TSRMLS_DC) {/*{{{*/
	char *badTags[] = {"img","javascript", "vbscript", "expression", "applet", "meta", "xml", "blink", "link", "style", "script", "embed", "object", "iframe", "frame", "frameset", "ilayer", "layer", "bgsound", "title", "base", NULL};
	char **tag, *concat_buffer, *buffer;
	int total_len, shared_len, tag_len, cpy_offset, num_tags;
	total_len = cpy_offset = 0;
	if (0 == unclosed) {
		shared_len = strlen("<[^>]*>[]*</[^>]*>|<[/]*[^>]*>|");
		num_tags = 3;
	} else {
		shared_len = strlen("<[^>]*>[]*</[^>]*>|<[/]*[^>]*>|<[\\s]*[^>]*|");
		num_tags = 4;
	}
	for (tag = badTags; *tag != NULL; tag++) {
		total_len += shared_len + num_tags * strlen(*tag);
	}
	concat_buffer = (char *)emalloc(total_len + 1);
	for (tag = badTags; *tag != NULL; tag++) {
		tag_len = shared_len + num_tags * strlen(*tag) + 1;
		buffer = (char *)emalloc(tag_len);
		if (0 == unclosed) {
			snprintf(buffer, tag_len, "<%s[^>]*>[]*</%s[^>]*>|<[/]*%s[^>]*>|", *tag, *tag, *tag);
		} else {
			snprintf(buffer, tag_len, "<%s[^>]*>[]*</%s[^>]*>|<[/]*%s[^>]*>|<[\\s]*%s[^>]*|", *tag, *tag, *tag, *tag);
		}
		memcpy(concat_buffer + cpy_offset, buffer, tag_len);
		efree(buffer);
		cpy_offset += tag_len - 1;
	}
	/* trim the last "|" */
	concat_buffer[cpy_offset - 1] = '\0';
	return concat_buffer;
}
/*}}}*/

static char *getBadAttrPattern() {/*{{{*/
	char *badAttrs[] = {"onabort\\=","onevent\\=", "onactivate\\=", "onafterprint\\(", "\"onafterupdate\"", "onbeforeactivate\\=", "onbeforecopy\\=", "onbeforecut\\=", "onbeforedeactivate\\=", "onbeforeeditfocus\\=", "onbeforepaste\\=", "onbeforeprint\\=", "onbeforeunload\\=", "=\"onbeforeupdate\"", "onblur\\=", "onbounce\\=", "=\"oncellchange\"", "onchange\\=", "onclick\\=", "oncontextmenu\\=", "oncontrolselect\\=", "oncopy\\=", "oncut\\=", "ondataavaible\\=", "ondatasetchanged\\=", "\"ondatasetcomplete\"=", "ondblclick\\=", "ondeactivate\\=", "ondrag\\=", "ondragdrop\\=", "ondragend\\=", "ondragenter\\=", "ondragleave\\=", "ondragover\\=", "ondragstart\\=", "ondrop\\=", "onerror\\=", "onerrorupdate\\=", "onfilterupdate\\=", "onfinish\\=", "onfocus\\=", "onfocusin\\=", "onfocusout\\=", "onhelp\\=", "onkeydown\\=", "onkeypress\\=", "onkeyup\\=", "onlayoutcomplete\\=", "onload\\=", "onlosecapture\\=", "onmousedown\\=", "onmouseenter\\=", "onmouseleave\\=", "onmousemove\\=", "onmoveout\\=", "onmouseover\\=", "onmouseup\\=", "onmousewheel\\=", "onmove\\=", "onmoveend\\=", "onmovestart\\=", "onpaste\\=", "onpropertychange\\=", "onreadystatechange\\=", "onreset\\=", "onresize\\=", "onresizeend\\=", "onresizestart\\=", "onrowexit\\=", "onrowsdelete\\=", "onrowsinserted\\=", "onscroll\\=", "onselect\\=", "onselectionchange\\=", "onselectstart\\=", "onstart\\=", "onstop\\=", "onsubmit\\=", "onunload\\=", "\"javascript\"", "void\\(", "document.forms", NULL};
	char **tag, *concat_buffer, *buffer;
	int total_len, shared_len, tag_len, cpy_offset, num_tags;
	total_len = cpy_offset = 0;
	num_tags = 2;
	shared_len = strlen("|[\\s.]*=[\\s]*\"[^\"]*\"|");
	for (tag = badAttrs; *tag != NULL; tag++) {
		total_len += shared_len + num_tags * strlen(*tag);
	}
	concat_buffer = (char *)emalloc(total_len + 1);
	for (tag = badAttrs; *tag != NULL; tag++) {
		tag_len = shared_len + num_tags * strlen(*tag) + 1;
		buffer = (char *)emalloc(tag_len);
		snprintf(buffer, tag_len, "%s|%s[\\s.]*=[\\s]*\"[^\"]*\"|", *tag, *tag);
		memcpy(concat_buffer + cpy_offset, buffer, tag_len);
		efree(buffer);
		cpy_offset += tag_len - 1;
	}
	/* trim the last "|" */
	concat_buffer[cpy_offset - 1] = '\0';
	return concat_buffer;
}
/*}}}*/

PHP_METHOD(zyxss, __construct) {/*{{{*/
	zval *object = getThis();

	zyxss_object *obj = (zyxss_object *)zend_object_store_get_object(object TSRMLS_CC);
}/*}}}*/

PHP_METHOD(zyxss, __destruct) {/*{{{*/
}/*}}}*/

/* {{{ zyxss_functions[]
 *
 * Every user visible function must have an entry in zyxss_functions[].
 */
const zend_function_entry zyxss_functions[] = {
	PHP_ME(zyxss, __construct, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
	PHP_ME(zyxss, __destruct, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_DTOR)
	PHP_FE(filter_attributes,	NULL)
	PHP_FE(filter_tags,	NULL)
	PHP_FE(filter_characters,	NULL)
	PHP_FE(filter_xss,	NULL)
	PHP_FE_END	/* Must be the last line in zyxss_functions[] */
};
/* }}} */

/* {{{ zyxss_module_entry
 */
zend_module_entry zyxss_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	"zyxss",
	zyxss_functions,
	PHP_MINIT(zyxss),
	PHP_MSHUTDOWN(zyxss),
	PHP_RINIT(zyxss),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(zyxss),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(zyxss),
#if ZEND_MODULE_API_NO >= 20010901
	"1.0", /* Replace with version number for your extension */
#endif
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_ZYXSS
ZEND_GET_MODULE(zyxss)
#endif

/* {{{ PHP_INI
 */
/* Remove comments and fill if you need to have entries in php.ini
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("zyxss.global_value",      "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_zyxss_globals, zyxss_globals)
    STD_PHP_INI_ENTRY("zyxss.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_zyxss_globals, zyxss_globals)
PHP_INI_END()
*/
/* }}} */

/* {{{ php_zyxss_init_globals
 */
/* Uncomment this function if you have INI entries
static void php_zyxss_init_globals(zend_zyxss_globals *zyxss_globals)
{
	zyxss_globals->global_value = 0;
	zyxss_globals->global_string = NULL;
}
*/
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(zyxss)
{
	/* If you have INI entries, uncomment these lines 
	REGISTER_INI_ENTRIES();
	*/
	zend_class_entry ce;
	/* register dll list object */
	INIT_CLASS_ENTRY(ce, "ZYXSS", zyxss_functions);
	zyxss_ce = zend_register_internal_class(&ce TSRMLS_CC);
	zyxss_ce->create_object = zyxss_obj_ctor;
	/* create custom handler table */
	memcpy(&zyxss_obj_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(zyxss)
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
PHP_RINIT_FUNCTION(zyxss)
{
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(zyxss)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(zyxss)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "zyxss support", "enabled");
	php_info_print_table_end();

	/* Remove comments if you have entries in php.ini
	DISPLAY_INI_ENTRIES();
	*/
}
/* }}} */


/* The previous line is meant for vim and emacs, so it can correctly fold and 
   unfold functions in source code. See the corresponding marks just before 
   function definition, where the functions purpose is also documented. Please 
   follow this convention for the convenience of others editing your code.
*/

/* {{{ proto string filter_attributes(string source)
   Array exception) */
PHP_FUNCTION(filter_attributes)
{
	char *source = NULL;
	int argc = ZEND_NUM_ARGS();
	int source_len;
	char *result;

	if (zend_parse_parameters(argc TSRMLS_CC, "s", &source, &source_len) == FAILURE) 
		return;

	char *badAttrsPattern = getBadAttrPattern();
	result = zyxss_preg_replace(badAttrsPattern, source, source_len TSRMLS_CC);
	efree(badAttrsPattern);
	if (NULL == result) {
		RETVAL_FALSE;
	} else {
		RETVAL_STRINGL(result, strlen(result), 0);
	}
}
/* }}} */

/* {{{ proto string filter_tags(string source [, int unclosed = 0])
   Array exception) */
PHP_FUNCTION(filter_tags)
{
	char *source = NULL;
	int argc = ZEND_NUM_ARGS();
	int source_len;
	long unclosed = 0;
	char *result;

	if (zend_parse_parameters(argc TSRMLS_CC, "s|l", &source, &source_len, &unclosed) == FAILURE) 
		return;

	char *badTagsPattern = getBadTagsPattern(unclosed TSRMLS_CC);
	result = zyxss_preg_replace(badTagsPattern, source, source_len TSRMLS_CC);
	efree(badTagsPattern);
	if (NULL == result) {
		RETVAL_FALSE;
	} else {
		RETVAL_STRINGL(result, strlen(result), 0);
	}
}
/* }}} */

/* {{{ proto string filter_characters(string source)
   Array exception) */
PHP_FUNCTION(filter_characters)
{
	char *source = NULL;
	int argc = ZEND_NUM_ARGS();
	int source_len;
	char *result;

	if (zend_parse_parameters(argc TSRMLS_CC, "s", &source, &source_len) == FAILURE) 
		return;

	char *badCharsPattern = "\\x00|\\x01|\\x02|\\x03|\\x04|\\x05|\\x06|\\x07|\\x08|\\x0b|\\x0c|\\x0e|\\x0f|\\x11|\\x12|\\x13|\\x14|\\x15|\\x16|\\x17|\\x18|\\x19";
	result = zyxss_preg_replace(badCharsPattern, source, source_len TSRMLS_CC);
	if (NULL == result) {
		RETVAL_FALSE;
	} else {
		RETVAL_STRINGL(result, strlen(result), 0);
	}
}
/* }}} */

/* {{{ proto string filter_xss(string source [, int unclosed = 0])
    */
PHP_FUNCTION(filter_xss)
{
	char *source = NULL;
	int argc = ZEND_NUM_ARGS();
	int source_len;
	long unclosed = 0;
	char *result;

	if (zend_parse_parameters(argc TSRMLS_CC, "s|l", &source, &source_len, &unclosed) == FAILURE) 
		return;

	char *badAttrsPattern = getBadAttrPattern();
	char *badTagsPattern = getBadTagsPattern(unclosed TSRMLS_CC);
	char *badCharsPattern = "\\x00|\\x01|\\x02|\\x03|\\x04|\\x05|\\x06|\\x07|\\x08|\\x0b|\\x0c|\\x0e|\\x0f|\\x11|\\x12|\\x13|\\x14|\\x15|\\x16|\\x17|\\x18|\\x19";
	int pattern_len = strlen(badAttrsPattern) + strlen(badTagsPattern) + strlen(badCharsPattern) + 2;
	char *pattern = (char *)emalloc(pattern_len + 1);
	snprintf(pattern, pattern_len, "%s|%s|%s", badAttrsPattern, badTagsPattern, badCharsPattern);
	efree(badAttrsPattern);
	efree(badTagsPattern);

	result = zyxss_preg_replace(pattern, source, source_len TSRMLS_CC);
	efree(pattern);
	if (NULL == result) {
		RETVAL_FALSE;
	} else {
		RETVAL_STRINGL(result, strlen(result), 0);
	}
}
/* }}} */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
