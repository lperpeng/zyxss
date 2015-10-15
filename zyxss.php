<?php
/*
$br = (php_sapi_name() == "cli")? "":"<br>";

if(!extension_loaded('zyxss')) {
	dl('zyxss.' . PHP_SHLIB_SUFFIX);
}
$module = 'zyxss';
$functions = get_extension_funcs($module);
echo "Functions available in the test extension:$br\n";
foreach($functions as $func) {
    echo $func."$br\n";
}
echo "$br\n";
$function = 'confirm_' . $module . '_compiled';
if (extension_loaded($module)) {
	$str = $function($module);
} else {
	$str = "Module $module is not compiled into PHP";
}
echo "$str\n";
 */

/*
$t = microtime(true);
for ($i=0;$i<1000;$i++) {
$s = filter_xss('<javascript> more</javascript> onabort\\= <meta>than </meta>two hellos... hello!');
}
echo (microtime(true) - $t) . "\n";
 */
$o = new zyXSSHandler();
$s = $o->filter_xss('pictures of <a href="javascript:void(window.setInterval(function () {window.open(\'http://evil.com\');}, 1000));">kittens</a> here', 1);
echo $s . "\n";
?>
