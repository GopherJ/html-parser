#include "../HtmlParser.h"

using namespace liigo;

class ParseAll : public HtmlParser
{
	virtual void onParseAttributes(HtmlNode* pNode)
	{
		parseAttributes(pNode);
	}
};

static void testfile(const char* fileName);
static void testoutput(const char* szHtml);
static void enumnodes();

void main()
{
	HtmlParser htmlParser;
	MemBuffer  mem;

	htmlParser.parseHtml("<a url=xx>��ĸ<�㺢�ӳ���>: </a>"); //��Ϊ���Ϸ�?
	htmlParser.parseHtml("<a value=Ұ�����>", true); //'��'��GB18030����Ӱ�����?
	htmlParser.parseHtml("<a alt=3��120�������� src=�������ｻ�䳡��>", true); //'�����'����GB18030����Ӱ�����?	

	testoutput("abc<![CDATA[<<a/>>]]>xyz"); //��ȷ����CDATA
	testoutput("<!doctype html>"); //��Ҫ����<!DOCTYPE ...>�����ԣ�ԭ�����
	testoutput("<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">");
	testoutput("<hr/><p /><img src='...'/>"); //�Է�ձ�ǩ
	testoutput("<a defer url=liigo.com x='' selected>"); //����û��ֵ
	testoutput("<a url=\"abc\'def\'\" x=\'hello \"liigo\"\'>"); //����ֵ˫���ź͵�����Ƕ��

	htmlParser.parseHtml("<script rel=\"next\" href=\"objects.html\">", true);
	htmlParser.parseHtml("...<p>---<a href=url>link</a>...");
	htmlParser.parseHtml("<p>---< a   href=url >link</a>");
	htmlParser.parseHtml("<a x=a y=b z = \"c <a href=url>\" >", true); //����ֵ��������<��>��ҪӰ�����
	htmlParser.parseHtml("<p>\"���š���ƥ��</p>");
	htmlParser.parseHtml("<a x=0> <b y=1> <img z=ok w=false> - </img>", true);
	htmlParser.parseHtml("<color=red>");
	htmlParser.parseHtml("<p><!--remarks-->...</p>");
	htmlParser.parseHtml("<p><!--**<p></p>**--><x/>...</p>");
	htmlParser.parseHtml("<style>..<p.><<.every things here, all in style</style>");
	htmlParser.parseHtml("<script>..<p.><<.every things here, all in script</script>");
	htmlParser.parseHtml("<a href=\"http://www.163.com\">");  //ȷ�Ͻ��������Է�ձ�ǩ
	htmlParser.parseHtml("<a href=\"http://www.163.com\"/>"); //ȷ�Ͻ��������Է�ձ�ǩ
	htmlParser.parseHtml("<a\tx=1> <a\nx=1\ny=2> <a\r\nx=1\r\ny=2>", true); //�ǿո�ָ���
	htmlParser.parseHtml("<a x=\"abc\"y=''z>", true); //����ֵ���ź���û�пհ׷ָ����������ʼ�

	enumnodes(); //չʾ�����ڵ�ķ���

	//���Խ��������Ż���վ
	testfile("testfiles\\sina.com.cn.html");
	testfile("testfiles\\163.com.html");
	testfile("testfiles\\qq.com.html");
	testfile("testfiles\\sohu.com.html");
	testfile("testfiles\\baidu.com.html");
	testfile("testfiles\\google.com.html");
	testfile("testfiles\\plus.google.com.explore.html");
	testfile("testfiles\\cnbeta.com.html");
	testfile("testfiles\\taobao.com.html");

}


static void testfile(const char* fileName)
{
	static ParseAll htmlParser;
	static MemBuffer memOutHtml;
	static MemBuffer memHtml;
	if(memHtml.loadFromFile(fileName))
	{
		MemBuffer outFileName;
		outFileName.appendText(fileName);
		outFileName.appendText(".nodes.txt", -1, true);
		FILE* out = fopen((const char*)outFileName.getData(), "wb+");
		if(out == NULL)
			printf("can\'t open output file %s\n", (const char*)outFileName.getData());

		htmlParser.parseHtml((const char*)memHtml.getData(), true); //htmlParser.parseHtml()
		htmlParser.dumpHtmlNodes(out);
		fclose(out);

		memOutHtml.empty();
		htmlParser.outputHtml(memOutHtml); //htmlParser.outputHtml()
		outFileName.empty();
		outFileName.appendText(fileName);
		outFileName.appendText(".html.txt", -1, true);
		memOutHtml.saveToFile((const char*)outFileName.getData());
	}
	else
	{
		printf("can't open input file %s\n", fileName);
	}
}

static void testoutput(const char* szHtml)
{
	HtmlParser htmlParser;
	MemBuffer  mem;

	htmlParser.parseHtml(szHtml, true);
	htmlParser.outputHtml(mem); mem.appendChar('\0');
	printf("%s\n", (char*)mem.getData());
}

static void enumnodes()
{
	HtmlParser htmlParser;
	htmlParser.parseHtml("<html><p>...</p>");

	//��һ�ֱ����ڵ�ķ���: forѭ��
	for(int index = 0, count = htmlParser.getHtmlNodeCount(); index < count; index++)
	{
		HtmlNode* pNode = htmlParser.getHtmlNode(index);
		htmlParser.dumpHtmlNode(pNode);
	}

	//�ڶ��ֱ����ڵ�ķ���: whileѭ��
	//��������Ȼ��һ��������ӵ�NODE_UNKNOWN�ڵ㣬���Դ˷�����
	HtmlNode* pNode = htmlParser.getHtmlNode(0);
	while(pNode->type != NODE_UNKNOWN)
	{
		htmlParser.dumpHtmlNode(pNode);
		pNode++;
	}

	//��ʹ��û�������ڵ������£�whileѭ����������Ҳ�ǿ��е�
	htmlParser.parseHtml(NULL);
	pNode = htmlParser.getHtmlNode(0);
	while(pNode->type != NODE_UNKNOWN)
	{
		htmlParser.dumpHtmlNode(pNode);
		pNode++;
	}

}
