#ifndef __HtmlParser_H__
#define __HtmlParser_H__

#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>

//HtmlParser�࣬���ڽ���HTML�ı�
//by liigo, @2010-2012

//���ڲ�ʹ�õ��ڴ滺��������
#define MEM_DEFAULT_BUFFER_SIZE  256
class MemBuffer
{
public:
	//ָ����������ʼ��С(�ֽ���), Ϊ-1��ʾʹ��Ĭ�ϳ�ʼ��С(MEM_DEFAULT_BUFFER_SIZE)
	MemBuffer(size_t nBufferSize = -1); //nBufferSize
	~MemBuffer();
public:
	//�򻺴����������ݿ飬д���������ݵ�ĩβ����Ҫʱ���Զ����仺����
	//������д������ݿ��׵�ַ�ڻ������е�ƫ����
	size_t appendData(void* pData, size_t nSize);
	//��ָ��p�����ֵ������pָ������ݣ���ӵ�������
	size_t appendPointer(void* p) { return appendData(&p, sizeof(p)); }
	//ȡ�����׵�ַ(�����ݳ���Ϊ0ʱ����NULL)���˵�ַ��NULLʱҲ���ǻ������׵�ַ
	//��append()��resetSize()����֮����ܻᵼ�������׵�ַ�����ı�
	void* getData() { return (m_nDataSize == 0 ? NULL : m_pBuffer); }
	//ȡָ��ƫ�ƴ����ݵ�ַ��ƫ��offsetӦС��getDataSize()�����򲻱�֤���صĵ�ַ��Ч
	void* getOffsetData(int offset) { return (m_nDataSize == 0 ? NULL : ((unsigned char*)m_pBuffer + offset)); }
	//ȡ���ݳ���
	size_t getDataSize() { return m_nDataSize; } 
	//�������ݳ��ȣ��³��ȿ���Ϊ����ֵ����Ҫʱ���Զ����仺����
	void resetDataSize(size_t size = 0);
	//�����������ͷ��ڴ�
	void clean();
	//�����������������е����ݣ��û�Ӧ������free()�ͷ�
	//�����׵�ַΪdetach()ǰgetData()���صĵ�ַ�����ݳ���Ϊdetach()ǰgetDataSize()���صĳ���
	void detach();
private:
	//Ҫ�󻺴�����������������Ϊsize��δʹ�ÿռ�
	//����δʹ�ÿռ���׵�ַ�����������ݵ�ĩβ
	void* require(size_t size);
private:
	unsigned char* m_pBuffer; //�������׵�ַ
	size_t m_nDataSize, m_nBufferSize; //���ݳ��ȣ�����������
};

enum HtmlNodeType
{
	NODE_UNKNOWN = 0,
	NODE_START_TAG,
	NODE_CLOSE_TAG,
	NODE_CONTENT,
};

enum HtmlTagType
{
	TAG_UNKNOWN = 0,
	TAG_A, TAG_DIV, TAG_FONT, TAG_IMG, TAG_P, TAG_SPAN, TAG_BR, TAG_B, TAG_I, TAG_HR, 
	TAG_COLOR, TAG_BGCOLOR, //�Ǳ�׼HTML��ǩ, ��������ʹ��: <color=red>, ��Ч�� <color color=red>
};

struct HtmlNodeProp
{
	char* szName;
	char* szValue;
};

#define MAX_HTML_TAG_LENGTH  15 //�ڵ����Ƶ�����ַ�����

struct HtmlNode
{
	HtmlNodeType type;
	HtmlTagType  tagType;
	char tagName[MAX_HTML_TAG_LENGTH+1];
	char* text;
	int propCount;
	HtmlNodeProp* props;
	void* pUser; //user customized, default to NULL
};


class HtmlParser
{
public:
	HtmlParser() {}
	~HtmlParser() { freeHtmlNodes(); }

public:
	//html
	void parseHtml(const char* szHtml);

	//const char* getHtml() const { return m_html.GetText(); } //���������Ƿ��ṩ�˹���

	//nodes
	unsigned int getHtmlNodeCount();
	HtmlNode* getHtmlNodes();
	//props
	const HtmlNodeProp* getNodeProp(const HtmlNode* pNode, const char* szPropName);
	const char* getNodePropStringValue(const HtmlNode* pNode, const char* szPropName, const char* szDefaultValue = NULL);
	int getNodePropIntValue(const HtmlNode* pNode, const char* szPropName, int defaultValue = 0);
	//debug
	void dumpHtmlNodes(FILE* f = stdout);
protected:
	//�������า��, �Ա�ʶ�������(��߽�������), ����ʶ����ٽ��(��߽����ٶ�)
	virtual HtmlTagType getHtmlTagTypeFromName(const char* szTagName);
	//�������า��, �Ա���õĽ����ڵ�����, ���߸ɴ಻�����ڵ�����(��߽����ٶ�)
	virtual void parseNodeProps(HtmlNode* pNode);

private:
	HtmlNode* newHtmlNode();
	void freeHtmlNodes();

private:
	MemBuffer m_HtmlNodes;
};

//һЩ�ı�������
char* duplicateStr(const char* pSrc, unsigned int nChar);
void freeDuplicatedStr(char* p);
unsigned int copyStr(char* pDest, unsigned int nDest, const char* pSrc, unsigned int nChar);


#endif //__HtmlParser_H__