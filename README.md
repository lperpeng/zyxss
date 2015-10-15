PHP Xss À©Õ¹: a simple XSS 

Function signature:


string filter_attributes(string)  /* Filter out bad attributes like onevent and onabort. */


string filter_tags(string [, int = 0]) /* Filter out bad tags like javascript and vbscript. Second parameter equals 1 means also filtering out unclosed tags.*/


string filter_characters(string) /* Filter out bad characters like \x00. */


string filter_xss(string [, int = 0]) /* Does all the three functions above. Second parameter equals 1 means also filtering out unclosed tags.*/


How to use:

create instance like $o = new zyXSS(), then invoke the above functions.
