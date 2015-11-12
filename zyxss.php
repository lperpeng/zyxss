<?php 
function test(){
//$t=microtime(true);
$o = new zyxss();
//$str ="<a herf>fwfe</a> <script>alert('ss')</script>";
$str="PCRE(Perl Compatible Regular Expressions即<html<scirpt>alert(\"ss\")</script>><div onload=\"alert(1)\">fwefw</div> pictures of <a href=\"javascript:void(window.setInterval(function () {window.open(\'http://evil.com\');}, 1000));\">kittens</a> here：perl语言兼容正则表达式)是一个用C语言编写的正则表达式函数库，由菲利普.海泽(Philip Hazel)编写。PCRE是一个轻量级的函数库，比Boost之>中的正则表达式库小得多。PCRE十分易用，同时功能也很强大<html<scirpt>alert(\"ss\")</script>><div onload=\"alert(1)\">fwefw</div> pictures of <a href=\"javascript:void(window.setInterval(function () {window.open(\'http://evil.com\');}, 1000));\">kittens</a> here，性能超过了POSIX正则表达式库和一些经典的正则表达式库[1]。";

$s = $o->filter_xss($str);
echo $s."\n";

}

// start profiling
xhprof_enable();

// run program
test();

// stop profiler
$xhprof_data = xhprof_disable();

// display raw xhprof data for the profiler run
print_r($xhprof_data);


$XHPROF_ROOT = '/root/lper/soft/xhprof-0.9.4';
include_once $XHPROF_ROOT . "/xhprof_lib/utils/xhprof_lib.php";
include_once $XHPROF_ROOT . "/xhprof_lib/utils/xhprof_runs.php";

// save raw data for this profiler run using default
// implementation of iXHProfRuns.
$xhprof_runs = new XHProfRuns_Default();

// save the run under a namespace "xhprof_foo"
$run_id = $xhprof_runs->save_run($xhprof_data, "xhprof_foo");

echo "---------------\n".
     "Assuming you have set up the http based UI for \n".
     "XHProf at some address, you can view run at \n".
     "http://<xhprof-ui-address>/index.php?run=$run_id&source=xhprof_foo\n".
     "---------------\n";
