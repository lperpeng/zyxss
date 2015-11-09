/**
 * 判断总共有多少个分隔符，目的是在split函数中构造相应的arry指针数组
 * @param char str 内容
 * @param chr del 分隔符
 * @return int.
 */
int str_split_num( char *str, const char *del)
{
	char *first = NULL;
	char *second = NULL;
	int num = 0;

	first = strstr(str,del);
	while(first != NULL)
    {
        second = first+1;
        num++;
        first = strstr(second,del);
    }
	return num;
}
/**
 * 字符分割函数的简单定义和实现
 * @param char arry 分隔后的指针数组
 * @param char str 需要分隔的内容
 * @param char separator 分隔符
 * @return void.
 */
void str_split( char **arry, char *str, const char *separator)
{
	char *val;
	char *s;
	/*s=strtok(str,del);*/
	val=php_strtok_r(str,separator,&s);
	while(val != NULL)
    {
        *arry++ = val;
        val = php_strtok_r(NULL,separator,&s);
    }
}
