#include "HtmlParser.h"
#include <memory.h>
#include <string.h>
#include <ctype.h>

//HtmlParser类，用于解析HTML文本
//by liigo, @2010-2012

using namespace liigo;

const char* strnchr(const char* pStr, int len, char c)
{
	if(pStr == NULL || len <= 0)
		return NULL;
	const char *p = pStr;
	while(1)
	{
		if(*p == c) return p;
		p++;
		if((p - pStr) == len) break;
	}
	return NULL;
}

const char* getFirstUnquotedChar(const char* pStr, char endcahr)
{
	char c;
	const char* p = pStr;
	bool inQuote1 = false, inQuote2 = false; //'inQuote1', "inQuote2"
	while(c = *p)
	{
		if(c == '\'')
		{
			inQuote1 = !inQuote1;
		}
		else if(c == '\"')
		{
			inQuote2 = !inQuote2;
		}

		if(!inQuote1 && !inQuote2)
		{
			if(c == endcahr) return p;
		}
		p++;
	}
	return NULL;
}

//nDest and nChar can by -1
size_t copyStr(char* pDest, size_t nDest, const char* pSrc, size_t nChar)
{
	if(pDest == NULL || nDest == 0)
		return 0;
	if(pSrc == NULL)
	{
		pDest[0] = '\0';
		return 0;
	}
	if(nChar == (size_t)-1)
		nChar = strlen(pSrc);
	if(nChar > nDest)
		nChar = nDest;
	memcpy(pDest, pSrc, nChar * sizeof(char));
	pDest[nChar] = '\0';
	return nChar;
}

int copyStrUtill(char* pDest, size_t nDest, const char* pSrc, char endchar, bool ignoreEndCharInQuoted)
{
	if(nDest == 0) return 0;
	pDest[0] = '\0';
	const char* pSearched = (ignoreEndCharInQuoted ? getFirstUnquotedChar(pSrc,endchar) : strchr(pSrc, endchar));
	if(pSearched <= pSrc) return 0;
	return copyStr(pDest, nDest, pSrc, pSearched - pSrc);
}

//nChar can be -1
char* duplicateStr(const char* pSrc, size_t nChar)
{
	if(nChar == (size_t)-1)
		nChar = strlen(pSrc);
	char* pNew = (char*) malloc( (nChar+1) * sizeof(char) );
	copyStr(pNew, -1, pSrc, nChar);
	return pNew;
}

char* duplicateStrUtill(const char* pSrc, char endchar, bool ignoreEndCharInQuoted)
{
	const char* pSearched = (ignoreEndCharInQuoted ? getFirstUnquotedChar(pSrc,endchar) : strchr(pSrc, endchar));;
	if(pSearched <= pSrc) return NULL;
	int n = pSearched - pSrc;
	return duplicateStr(pSrc, n);
}

void freeDuplicatedStr(char* p)
{
	if(p) free(p);
}

void skipSpaceChars(char*& p)
{
	if(p)
	{
		while(isspace(*p)) p++;
	}
}

const char* nextUnqotedSpaceChar(const char* p)
{
	const char* r = getFirstUnquotedChar(p, ' ');
	if(!r)
		r = getFirstUnquotedChar(p, '\t');
	return r;
}

char* duplicateStrAndUnquote(const char* str, size_t nChar)
{
	if( nChar > 1 && (str[0] == '\"' && str[nChar-1] == '\"') || (str[0] == '\'' && str[nChar-1] == '\'') )
	{
		str++; nChar-=2;
	}
	return duplicateStr(str, nChar);
}

//-----------------------------------------------------------------------------

// TagName & TagType
struct N2T { const char* name; HtmlTagType type; };

static HtmlTagType identifyHtmlTagInTable(const char* szTagName, N2T* table, int count)
{
	for(int i = 0; i < count; i++)
	{
		N2T* p = table + i;
		if(stricmp(p->name, szTagName) == 0)
			return p->type;
	}
	return TAG_UNKNOWN;
}

//出于解析需要必须识别的HtmlTagType
static HtmlTagType identifyHtmlTag_Internal(const char* szTagName)
{
	static N2T n2tTable[] = 
	{
		{ "SCRIPT", TAG_SCRIPT },
		{ "STYLE", TAG_STYLE },
	};

	return identifyHtmlTagInTable(szTagName, n2tTable, sizeof(n2tTable)/sizeof(n2tTable[0]));
}

//[virtual]
HtmlTagType HtmlParser::identifyHtmlTag(const char* szTagName, HtmlNodeType nodeType)
{
	//默认仅识别涉及HTML基本结构和信息的有限几个TAG
	//交给用户自行扩展以便识别更多或更少

	if(nodeType != NODE_START_TAG)
		return TAG_UNKNOWN;

	static N2T n2tTable[] = 
	{
		{ "A", TAG_A },
		{ "IMG", TAG_IMG },
		{ "META", TAG_META },
		{ "BODY", TAG_BODY },
		{ "TITLE", TAG_TITLE },
	};

	return identifyHtmlTagInTable(szTagName, n2tTable, sizeof(n2tTable)/sizeof(n2tTable[0]));
}

HtmlNode* HtmlParser::newHtmlNode()
{
	static char staticHtmlNodeTemplate[sizeof(HtmlNode)] = {0};
	size_t offset = m_HtmlNodes.appendData(staticHtmlNodeTemplate, sizeof(HtmlNode));
	HtmlNode* pNode = (HtmlNode*) m_HtmlNodes.getOffsetData(offset);
	return pNode;
}

void HtmlParser::parseHtml(const char* szHtml, bool parseProps)
{
	freeHtmlNodes();
	if(szHtml == NULL || *szHtml == '\0') return;

	char* p = (char*) szHtml;
	char* s = (char*) szHtml;
	HtmlNode* pNode = NULL;
	char c;
	bool bInQuotes = false;

	while( c = *p )
	{
		if(c == '\"')
		{
			bInQuotes = !bInQuotes;
			p++; continue;
		}
		if(bInQuotes)
		{
			p++; continue;
		}

		if(c == '<')
		{
			if(p > s)
			{
				//Add Text Node
				pNode = newHtmlNode();
				pNode->type = NODE_CONTENT;
				pNode->text = duplicateStrUtill(s, '<', true);
			}
			s = p + 1;
		}
		else if(c == '>')
		{
			if(p > s)
			{
				//创建新节点(HtmlNode)，得到节点类型(NodeType)和名称(TagName)
				pNode = newHtmlNode();
				if(p - s >= 5 && *s == '!' && *(s+1) == '-' && *(s+2) == '-'
					&& *(p - 2) == '-' && *(p-1) == '-') //HTML注释: <!-- -->
				{
					pNode->type = NODE_REMARKS;
					pNode->text = duplicateStr(s+3, p-s-5);
					s = p + 1;
					p++;
					continue;
				}
				while(isspace(*s)) s++;
				pNode->type = (*s != '/' ? NODE_START_TAG : NODE_CLOSE_TAG);
				if(*s == '/') s++;
				//这里得到的tagName可能包含一部分属性文本，需在下面修正
				copyStrUtill(pNode->tagName, MAX_HTML_TAG_LENGTH, s, '>', true);
				int tagNamelen = strlen(pNode->tagName);
				if(pNode->tagName[tagNamelen-1] == '/')
				{
					//处理自封闭的结点, 如 <br/>, 删除tagName中可能会有的'/'字符
					//自封闭的结点的type设置为NODE_START_TAG应该可以接受(否则要引入新的节点类型NODE_STARTCLOSE_TAG)
					pNode->tagName[tagNamelen-1] = '\0';
					tagNamelen--;
				}
				//修正节点名称，提取属性文本(存入pNode->text)
				for(int i = 0; i < tagNamelen; i++)
				{
					if(pNode->tagName[i] == ' ' //第一个空格后面跟的是属性文本
						|| pNode->tagName[i] == '=') //扩展支持这种格式: <tagName=value>, 等效于<tagName tagName=value>
					{
						char* props = (pNode->tagName[i] == ' ' ? s + i + 1 : s);
						pNode->text = duplicateStrUtill(props, '>', true);
						int nodeTextLen = strlen(pNode->text);
						if(pNode->text[nodeTextLen-1] == '/') //去掉最后可能会有的'/'字符, 如这种情况: <img src="..." />
							pNode->text[nodeTextLen-1] = '\0';
						pNode->tagName[i] = '\0';
						break;
					}
				}

				//识别节点类型(HtmlTagType)
				pNode->tagType = identifyHtmlTag_Internal(pNode->tagName); //内部识别SCRIPT,STYLE
				if(pNode->tagType == TAG_UNKNOWN)
					pNode->tagType = identifyHtmlTag(pNode->tagName, pNode->type);

				//解析节点属性
				if(pNode->type == NODE_START_TAG && parseProps && pNode->text)
					parseNodeProps(pNode);
			}
			s = p + 1;
		}

		p++;
	}

	if(p > s)
	{
		//Add Text Node
		pNode = newHtmlNode();
		pNode->type = NODE_CONTENT;
		pNode->text = duplicateStr(s, -1);
	}

#ifdef _DEBUG
	dumpHtmlNodes(); //just for test
#endif
}

int HtmlParser::getHtmlNodeCount()
{
	return (int)(m_HtmlNodes.getDataSize() / sizeof(HtmlNode));
}

HtmlNode* HtmlParser::getHtmlNodes(int i)
{
	return (HtmlNode*)m_HtmlNodes.getData() + i;
}

void HtmlParser::freeHtmlNodes()
{
	for(int i = 0, count = getHtmlNodeCount(); i < count; i++)
	{
		HtmlNode* pNode = getHtmlNodes(i);
		if(pNode->text)
			freeDuplicatedStr(pNode->text);

		if(pNode->props)
		{
			for(int propIndex = 0; propIndex < pNode->propCount; propIndex++)
			{
				HtmlNodeProp* prop = pNode->props + propIndex;
				if(prop->szName)  freeDuplicatedStr(prop->szName);
				if(prop->szValue) freeDuplicatedStr(prop->szValue);
			}
			free(pNode->props); //see: HtmlParser.parseNodeProps(), MemBuffer.detach()
		}
	}
	m_HtmlNodes.clean();
}

//[virtual]
void HtmlParser::parseNodeProps(HtmlNode* pNode)
{
	if(pNode == NULL || pNode->propCount > 0 || pNode->text == NULL)
		return;
	char* p = pNode->text;
	char *ps = NULL;
	MemBuffer mem;

	bool inQuote1 = false, inQuote2 = false;
	char c;
	while(c = *p)
	{
		if(c == '\"')
		{
			inQuote1 = !inQuote1;
		}
		else if(c == '\'')
		{
			inQuote2 = !inQuote2;
		}

		if((!inQuote1 && !inQuote2) && (c == ' ' || c == '\t' || c == '='))
		{
			if(ps)
			{
				mem.appendPointer(duplicateStrAndUnquote(ps, p - ps));
				ps = NULL;
			}
			if(c == '=')
				mem.appendPointer(NULL);
		}
		else
		{
			if(ps == NULL)
				ps = p;
		}

		p++;
	}

	if(ps)
		mem.appendPointer(duplicateStrAndUnquote(ps, p - ps));

	mem.appendPointer(NULL);
	mem.appendPointer(NULL);

	char** pp = (char**) mem.getData();

	MemBuffer props;
	for(int i = 0, n = mem.getDataSize() / sizeof(char*) - 2; i < n; i++)
	{
		props.appendPointer(pp[i]); //prop name
		if(pp[i+1] == NULL)
		{
			props.appendPointer(pp[i+2]); //prop value
			i += 2;
		}
		else
			props.appendPointer(NULL); //prop vlalue
	}

	pNode->propCount = props.getDataSize() / sizeof(char*) / 2;
	pNode->props = (HtmlNodeProp*) props.getData();
	props.detach();
}

const HtmlNodeProp* HtmlParser::getNodeProp(const HtmlNode* pNode, const char* szPropName)
{
	if(pNode == NULL || pNode->propCount <= 0)
		return NULL;

	for(int i = 0; i < pNode->propCount; i++)
	{
		HtmlNodeProp* prop = pNode->props + i;
		if(stricmp(prop->szName, szPropName) == 0)
			return prop;
	}
	return NULL;
}

const char* HtmlParser::getNodePropStringValue(const HtmlNode* pNode, const char* szPropName, const char* szDefaultValue /*= NULL*/)
{
	const HtmlNodeProp* pProp = getNodeProp(pNode, szPropName);
	if(pProp)
		return pProp->szValue;
	else
		return szDefaultValue;
}

int HtmlParser::getNodePropIntValue(const HtmlNode* pNode, const char* szPropName, int defaultValue /*= 0*/)
{
	const HtmlNodeProp* pProp = getNodeProp(pNode, szPropName);
	if(pProp && pProp->szValue)
		return atoi(pProp->szValue);
	else
		return defaultValue;
}

void HtmlParser::dumpHtmlNodes(FILE* f)
{
	char buffer[256] = {0};
	fprintf(f, "\n-------- begin HtmlParser.dumpHtmlNodes() --------\n");
	for(int i = 0, count = getHtmlNodeCount(); i < count; i++)
	{
		HtmlNode* pNode = getHtmlNodes(i);
		switch(pNode->type)
		{
		case NODE_CONTENT:
			sprintf(buffer, "%2d) type: NODE_CONTENT", i);
			break;
		case NODE_START_TAG:
			sprintf(buffer, "%2d) type: NODE_START_TAG, tagName: %s (%d)", i, pNode->tagName, pNode->tagType);
			break;
		case NODE_CLOSE_TAG:
			sprintf(buffer, "%2d) type: NODE_CLOSE_TAG, tagName: %s (%d)", i, pNode->tagName, pNode->tagType);
			break;
		case NODE_REMARKS:
			sprintf(buffer, "%2d) type: NODE_REMARKS", i);
			break;
		case NODE_UNKNOWN:
		default:
			sprintf(buffer, "%2d) type: NODE_UNKNOWN", i);
			break;
		}
		fprintf(f, buffer);
		if(pNode->text)
		{
			fprintf(f, ", text: ");
			fprintf(f, pNode->text);
		}
		fprintf(f, "\n");

		if(pNode->propCount > 0)
		{
			fprintf(f, "    props: ");
			for(int i = 0; i < pNode->propCount; i++)
			{
				HtmlNodeProp* prop = pNode->props + i;
				if(prop->szValue)
				{
					fprintf(f, prop->szName);
					fprintf(f, " = ");
					fprintf(f, prop->szValue);
				}
				else
					fprintf(f, prop->szName);
				if(i < pNode->propCount - 1)
				{
					fprintf(f, ", ");
				}
			}
			fprintf(f, "\n");
		}
	}
	fprintf(f, "-------- end of HtmlParser.dumpHtmlNodes() --------\n");
}

//-----------------------------------------------------------------------------
// class MemBuffer

MemBuffer::MemBuffer(size_t nBufferSize) : m_pBuffer(NULL), m_nDataSize(0), m_nBufferSize(0)
{
	if(nBufferSize == (size_t)-1)
		nBufferSize = MEM_DEFAULT_BUFFER_SIZE;
	if(nBufferSize > 0)
		require(nBufferSize);
}

MemBuffer::MemBuffer(const MemBuffer& other) : m_pBuffer(NULL), m_nDataSize(0), m_nBufferSize(0)
{
	*this = other; // invoke operator=()
}

MemBuffer::~MemBuffer()
{
	clean();
}

const MemBuffer& MemBuffer::operator= (const MemBuffer& other)
{
	resetDataSize(0);
	appendData(other.getData(), other.getDataSize());
	return *this;
}

void MemBuffer::clean()
{
	if(m_pBuffer) free(m_pBuffer);
	m_pBuffer = NULL;
	m_nDataSize = m_nBufferSize = 0;
}

void MemBuffer::detach()
{
	//数据长度为0时getData()返回NULL,用户没有机会释放内存,所以必须在内部释放m_pBuffer
	if(m_nDataSize == 0 && m_pBuffer)
		free(m_pBuffer);
	m_pBuffer = NULL;
	m_nDataSize = m_nBufferSize = 0;
}

void* MemBuffer::require(size_t size)
{
	if(size == 0 || (m_nBufferSize - m_nDataSize) >= size)
		return (m_pBuffer + m_nDataSize); //现有缓存区足够使用，不需要扩充缓存区

	//计算新的缓存区大小
	size_t newBufferSize;
	if(m_nBufferSize == 0)
	{
		newBufferSize = size; //缓存区初始大小
	}
	else
	{
		//扩充缓存区
		newBufferSize = (m_nBufferSize == 0 ? MEM_DEFAULT_BUFFER_SIZE : m_nBufferSize);
		do {
			newBufferSize <<= 1; //每次扩充一倍
		}while(newBufferSize - m_nDataSize < size);
	}

	//分配缓存区内存
	if(m_pBuffer == NULL)
	{
		m_pBuffer = (unsigned char*) malloc(newBufferSize);
		memset(m_pBuffer, 0, newBufferSize);
	}
	else
	{
		m_pBuffer = (unsigned char*) realloc(m_pBuffer, newBufferSize);
		memset(m_pBuffer + m_nBufferSize, 0, newBufferSize - m_nBufferSize);
	}

	m_nBufferSize = newBufferSize; //设置新的缓存区大小

	return (m_pBuffer + m_nDataSize); //返回
}

size_t MemBuffer::appendData(void* pData, size_t nSize)
{
	void* p = require(nSize);
	memcpy(p, pData, nSize);
	m_nDataSize += nSize;
	return (m_nDataSize - nSize);
}

void MemBuffer::resetDataSize(size_t size)
{
	size_t oldDataSize = m_nDataSize;

	if(size <= m_nBufferSize)
	{
		m_nDataSize = size;
	}
	else
	{
		m_nDataSize = 0;
		require(size);
		m_nDataSize = size;
	}

	if(m_nDataSize < oldDataSize)
	{
		//如果数据长度被压缩，把被裁掉的部分数据清零
		memset(getOffsetData(m_nDataSize), 0, oldDataSize - m_nDataSize);
	}
}

void MemBuffer::exchange(MemBuffer& other)
{
	unsigned char* pBuffer = m_pBuffer;
	size_t nBufferSize = m_nBufferSize;
	size_t nDataSize = m_nDataSize;

	m_pBuffer = other.m_pBuffer;
	m_nBufferSize = other.m_nBufferSize;
	m_nDataSize = other.m_nDataSize;

	other.m_pBuffer = pBuffer;
	other.m_nBufferSize = nBufferSize;
	other.m_nDataSize = nDataSize;
}

//-----------------------------------------------------------------------------

