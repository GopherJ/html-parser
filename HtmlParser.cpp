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

const char* findFirstUnquotedChar(const char* pStr, char endchar)
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
			if(c == endchar) return p;
		}
		p++;
	}
	return NULL;
}

// pStr1 not NULL, pStr2 not NULL
static bool matchStr2in1(const char* pStr1, const char* pStr2, int lenStr2, bool bCaseSensitive)
{
	for(int i = 0; i < lenStr2; i++)
	{
		if(pStr1[i] == '\0') return false;
		bool match = bCaseSensitive ? (pStr1[i] == pStr2[i]) : (tolower(pStr1[i]) == tolower(pStr2[i]));
		if(!match) return false;
	}
	return true;
}

const char* findFirstUnquotedStr(const char* pSourceStr, const char* pDestStr, bool bCaseSensitive)
{
	if(pDestStr == NULL || *pDestStr == '\0') return NULL;

	char c;
	const char* p = pSourceStr;
	bool inQuote1 = false, inQuote2 = false; //'inQuote1', "inQuote2"
	int lenDestStr = strlen(pDestStr);
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
			if(matchStr2in1(p, pDestStr, lenDestStr, bCaseSensitive))
				return p;
		}
		p++;
	}
	return NULL;
}

const char* findFirstStr(const char* pSourceStr, const char* pDestStr, bool bCaseSensitive)
{
	if(pDestStr == NULL || *pDestStr == '\0') return NULL;

	char c;
	const char* p = pSourceStr;
	int lenDestStr = strlen(pDestStr);
	while(c = *p)
	{
		if(matchStr2in1(p, pDestStr, lenDestStr, bCaseSensitive))
			return p;
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
	const char* pSearched = (ignoreEndCharInQuoted ? findFirstUnquotedChar(pSrc,endchar) : strchr(pSrc, endchar));
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
	const char* pSearched = (ignoreEndCharInQuoted ? findFirstUnquotedChar(pSrc,endchar) : strchr(pSrc, endchar));;
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
	const char* r = findFirstUnquotedChar(p, ' ');
	if(!r)
		r = findFirstUnquotedChar(p, '\t');
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
HtmlTagType HtmlParser::onIdentifyHtmlTag(const char* szTagName, HtmlNodeType nodeType)
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
		{ "FRAME", TAG_FRAME },
		{ "IFRAME", TAG_IFRAME },
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
	bool bInQuote1  = false; //between ' and '
	bool bInQuote2  = false; //between " and "
	bool bInScript  = false; //between <scripte> and </script>
	bool bInStyle   = false; //between <style> and </style>
	bool bInsideTag = false; //between < and >

	while( c = *p )
	{
		if(bInsideTag)
		{
			//在 < 和 > 之间，跳过单引号或双引号内的任何文本（包括<和>）
			if(c == '\'')
			{
				bInQuote1 = !bInQuote1;
				p++; continue;
			}
			else if(c == '\"')
			{
				bInQuote2 = !bInQuote2;
				p++; continue;
			}
			if(bInQuote1 || bInQuote2)
			{
				p++; continue;
			}
		}

		if(bInScript)
		{
			//跳过<script>和</script>之间的任何文本
			const char* pEndScript = findFirstStr(p, "</script>", false);
			if(pEndScript)
			{
				bInScript = false;
				p = (char*)pEndScript;
				c = *p;
			}
			else
				goto onerror; //error: no </script>
		}
		else if(bInStyle)
		{
			//跳过<style>和</style>之间的任何文本
			const char* pEndStyle = findFirstStr(p, "</style>", false);
			if(pEndStyle)
			{
				bInStyle = false;
				p = (char*)pEndStyle;
				c = *p;
			}
			else
				goto onerror; //error: no </style>
		}

		if(c == '<') // <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
		{
			if(p > s)
			{
				//Add Text Node
				pNode = newHtmlNode();
				pNode->type = NODE_CONTENT;
				pNode->text = duplicateStr(s, p - s);
				if(onNodeReady(pNode) == false) goto onuserend;
			}

			//处理HTML注释 <!-- -->
			if(p[1] == '!' && p[2] == '-' && p[3] == '-')
			{
				const char* pEndRemarks = findFirstStr(p + 4, "-->", true);
				if(pEndRemarks)
				{
					//Add Remarks Node
					pNode = newHtmlNode();
					pNode->type = NODE_REMARKS;
					pNode->text = duplicateStr(p + 4, pEndRemarks - (p + 4));
					if(onNodeReady(pNode) == false) goto onuserend;
					s = p = (char*)pEndRemarks + 3;
					bInsideTag = bInQuote1 = bInQuote2 = false;
					continue;
				}
				//else: no -->, not very bad, try continue parsing
			}

			s = p + 1;
			bInsideTag = true; bInQuote1 = bInQuote2 = false;
		}
		else if(c == '>') // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
		{
			if(p > s)
			{
				//创建新节点(HtmlNode)，得到节点类型(NodeType)和名称(TagName)
				pNode = newHtmlNode();
				while(isspace(*s)) s++;
				pNode->type = (*s == '/' ? NODE_CLOSE_TAG : NODE_START_TAG);
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
						if(pNode->text)
						{
							int nodeTextLen = strlen(pNode->text);
							if(pNode->text[nodeTextLen-1] == '/') //去掉最后可能会有的'/'字符, 如这种情况: <img src="..." />
								pNode->text[nodeTextLen-1] = '\0';
						}
						pNode->tagName[i] = '\0';
						break;
					}
				}

				//识别节点类型(HtmlTagType) - 内部
				pNode->tagType = identifyHtmlTag_Internal(pNode->tagName); //内部识别SCRIPT,STYLE
				if(pNode->tagType == TAG_STYLE)
					bInStyle = (pNode->type == NODE_START_TAG);
				if(pNode->tagType == TAG_SCRIPT)
					bInScript = (pNode->type == NODE_START_TAG);
				//识别节点类型(HtmlTagType) - 用户
				if(pNode->tagType == TAG_UNKNOWN)
					pNode->tagType = onIdentifyHtmlTag(pNode->tagName, pNode->type);

				//解析节点属性
				if(pNode->type == NODE_START_TAG && parseProps && pNode->text)
					onParseNodeProps(pNode);

				if(onNodeReady(pNode) == false) goto onuserend;
			}

			s = p + 1;
			bInsideTag = bInQuote1 = bInQuote2 = false;
		}

		p++;
	}

	if(p > s)
	{
		//Add Text Node
		pNode = newHtmlNode();
		pNode->type = NODE_CONTENT;
		pNode->text = duplicateStr(s, -1);
		if(onNodeReady(pNode) == false) goto onuserend;
	}

	goto dumpnodes;
	return;

onerror:
	pNode = newHtmlNode();
	pNode->type = NODE_CONTENT;
	pNode->text = duplicateStr(s, -1);
	goto dumpnodes;
	return;

onuserend:
	goto dumpnodes;
	return;

dumpnodes:
#ifdef _DEBUG
	outputHtmlNodes(); //just for test
#endif
}

size_t HtmlParser::getHtmlNodeCount()
{
	return (size_t)(m_HtmlNodes.getDataSize() / sizeof(HtmlNode));
}

HtmlNode* HtmlParser::getHtmlNodes(size_t index)
{
	return (HtmlNode*)m_HtmlNodes.getData() + index;
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
void HtmlParser::onParseNodeProps(HtmlNode* pNode)
{
	if(pNode->tagType != NODE_UNKNOWN)
		parseNodeProps(pNode);
}

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

const HtmlNodeProp* HtmlParser::getNodeProp(const HtmlNode* pNode, size_t index)
{
	return pNode->props + index;
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

void HtmlParser::outputHtmlNodes(FILE* f)
{
	char buffer[256] = {0};
	fprintf(f, "\r\n-------- begin HtmlParser.outputHtmlNodes() --------\r\n");
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
		fprintf(f, "%s", buffer);
		if(pNode->text)
			fprintf(f, ", text: %s", pNode->text);
		fprintf(f, "\r\n");

		if(pNode->propCount > 0)
		{
			fprintf(f, "    props: ");
			for(int i = 0; i < pNode->propCount; i++)
			{
				HtmlNodeProp* prop = pNode->props + i;
				if(prop->szValue)
					fprintf(f, "%s = \"%s\"", prop->szName, prop->szValue);
				else
					fprintf(f, "%s", prop->szName);
				if(i < pNode->propCount - 1)
				{
					fprintf(f, ", ");
				}
			}
			fprintf(f, "\r\n");
		}
	}
	fprintf(f, "-------- end of HtmlParser.outputHtmlNodes() --------\r\n");
}

void HtmlParser::outputHtml(MemBuffer& mem)
{
	int propIndex = 0;
	for(int nodeIndex = 0, count = getHtmlNodeCount(); nodeIndex < count; nodeIndex++)
	{
		HtmlNode* pNode = getHtmlNodes(nodeIndex);
		switch(pNode->type)
		{
		case NODE_CONTENT:
			mem.appendText(pNode->text);
			break;
		case NODE_START_TAG:
			mem.appendChar('<');
			mem.appendText(pNode->tagName);
			if(pNode->propCount > 0)
				mem.appendChar(' ');
			for(propIndex = 0; propIndex < pNode->propCount; propIndex++)
			{
				const HtmlNodeProp* pProp = getNodeProp(pNode, propIndex);
				mem.appendText(pProp->szName);
				if(pProp->szValue)
				{
					mem.appendText("=\"");
					mem.appendText(pProp->szValue);
					mem.appendChar('\"');
				}
				if(propIndex < pNode->propCount - 1)
					mem.appendChar(' ');
			}
			mem.appendChar('>');
			break;
		case NODE_CLOSE_TAG:
			mem.appendText("</");
			mem.appendText(pNode->tagName);
			mem.appendChar('>');
			break;
		case NODE_REMARKS:
			mem.appendText("<!--");
			mem.appendText(pNode->text);
			mem.appendText("-->");
			break;
		case NODE_UNKNOWN:
		default:
			fprintf(stderr, "HtmlParser.outputHtml(): NODE_UNKNOWN\n");
			break;
		} //end switch
	}//end for
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

void MemBuffer::shrink()
{
	if(m_pBuffer == NULL || m_nBufferSize == m_nDataSize)
		return;
	//assert(m_nBufferSize > m_nDataSize);
	m_nBufferSize = (m_nDataSize > 0 ? m_nDataSize : MEM_DEFAULT_BUFFER_SIZE);
	m_pBuffer = (unsigned char*) realloc(m_pBuffer, m_nBufferSize);
}

size_t MemBuffer::appendData(const void* pData, size_t nSize)
{
	if(pData == NULL) return m_nDataSize;
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
		memset(m_pBuffer + m_nDataSize, 0, oldDataSize - m_nDataSize);
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

size_t MemBuffer::appendText(const char* szText, size_t len, bool appendZeroChar)
{
	if(szText == NULL)
		return m_nDataSize;
	if(len == (size_t)-1)
		len = strlen(szText);
	size_t offset = appendData(szText, len);
	if(appendZeroChar)
		appendChar('\0');
	return offset;
}

bool MemBuffer::loadFromFile(const char* szFileName, bool keepExistData, bool appendZeroChar, size_t* pReadBytes)
{
	if(!keepExistData) empty();
	if(pReadBytes) *pReadBytes = 0;
	if(szFileName == NULL) return false;
	FILE* pfile = fopen(szFileName, "rb");
	if(pfile)
	{
		fseek(pfile, 0, SEEK_END);
		long filelen = ftell(pfile);
		fseek(pfile, 0, SEEK_SET);
		size_t n = 0;
		if(filelen > 0)
		{
			size_t oldDataSize = getDataSize();
			resetDataSize(oldDataSize + filelen);
			n = fread(getOffsetData(oldDataSize), 1, filelen, pfile);
			resetDataSize(oldDataSize + n);
		}
		if(appendZeroChar) appendChar('\0');
		if(pReadBytes) *pReadBytes = n;

		fclose(pfile);
		return (n == (size_t)filelen); // n != filelen 的情况下，返回false，同时返回了不完整的数据。不确认这种处理方法好不好。
	}
	
	return false;
}

bool MemBuffer::saveToFile(const char* szFileName, const void* pBOM, size_t bomLen)
{
	FILE* pfile = fopen(szFileName, "wb+");
	if(pfile)
	{
		if(pBOM)
			fwrite(pBOM, 1, bomLen, pfile);
		if(getDataSize() > 0)
			fwrite(getData(), 1, getDataSize(), pfile);
		fclose(pfile);
	}
	return false;
}


//-----------------------------------------------------------------------------

