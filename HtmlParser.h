#ifndef __HtmlParser_H__
#define __HtmlParser_H__

#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>

namespace liigo
{

//HtmlParser�࣬���ڽ���HTML�ı�
//by liigo, @2010-2012

//MemBuffer: �ڴ滺�����࣬
#define MEM_DEFAULT_BUFFER_SIZE  256
class MemBuffer
{
public:
	//nBufferSizeָ����������ʼ��С(�ֽ���), Ϊ-1��ʾʹ��Ĭ�ϳ�ʼ��С(MEM_DEFAULT_BUFFER_SIZE)
	//nBufferSizeΪ0ʱ�ݲ����仺�����ڴ棬�ӳٵ���һ��д������ʱ�ٷ���
	MemBuffer(size_t nBufferSize = -1);
	MemBuffer(const MemBuffer& other); //��other���������ݣ������������з����
	~MemBuffer(); //����ʱ���������ͷ��ڴ棬�����Ѿ�detach()
	const MemBuffer& operator= (const MemBuffer& other); //������ݺ��ٰ�other�ڵ����ݸ��ƽ���

public:
	//�򻺴����������ݿ飬д���������ݵ�ĩβ����Ҫʱ���Զ����仺����
	//������д������ݿ��׵�ַ�ڻ������е�ƫ����
	size_t appendData(const void* pData, size_t nSize);
	//ȡ�����׵�ַ(�����ݳ���Ϊ0ʱ����NULL)���˵�ַ��NULLʱҲ���ǻ������׵�ַ
	//��appendXXX()��resetSize()��exchange()��operator=����֮����ܻᵼ�������׵�ַ�����ı�
	void* getData() const { return (m_nDataSize == 0 ? NULL : m_pBuffer); }
	//ȡָ��ƫ�ƴ����ݵ�ַ��ƫ��offsetӦС��getDataSize()�����򲻱�֤���صĵ�ַ��Ч
	void* getOffsetData(int offset) const { return (m_nDataSize == 0 ? NULL : ((unsigned char*)m_pBuffer + offset)); }
	//ȡ���ݳ���
	size_t getDataSize() const { return m_nDataSize; }
	//�������ݳ��ȣ��³��ȿ���Ϊ����ֵ����Ҫʱ���Զ����仺����
	void resetDataSize(size_t size = 0);
	//������ݣ���Ч��resetDataSize(0)
	void empty() { resetDataSize(0); }
	//�����������ͷ��ڴ�
	void clean();
	//�����������������е����ݣ��û�Ӧ���и�����free()�ͷ�����:
	//�����׵�ַΪdetach()ǰgetData()���صĵ�ַ�����ݳ���Ϊdetach()ǰgetDataSize()���صĳ���
	void detach();
	//������������(this & other)���Թ�����������ݣ��������ݺͻ�������
	void exchange(MemBuffer& other);

	//��ӻ�����������
	size_t appendInt(int i) { return appendData(&i, sizeof(i)); }
	size_t appendChar(char c) { return appendData(&c, sizeof(c)); }
	//��ָ��p�����ֵ������pָ������ݣ���ӵ�������
	size_t appendPointer(const void* p) { return appendData(&p, sizeof(p)); }
	//���ı�������ӵ�������, lenΪд����ֽ�����-1��ʾstrlen(szText)����appendZeroChar��ʾ�Ƿ�������'\0'
	size_t appendText(const char* szText, size_t len = -1, bool appendZeroChar = false);

	//��ȡ�ļ�ȫ�����ݣ����keepExistData=true������������ԭ�����ݣ��������ԭ������
	//����appendZeroChar��ʾ�Ƿ��������ַ�'\0'������pReadBytesr�����NULL��д����ļ��ж�ȡ���ֽ���
	//�ڴ��̶�д����δ������ȡ�ļ����ݵ�����£�������false�����Ѿ���ȡ�Ĳ���������Ȼ������pReadBytesr�л�д���ȡ���ֽ���
	bool loadFromFile(const char* szFileName, bool keepExistData = false, bool appendZeroChar = false, size_t* pReadBytes = NULL);
	//�����ݱ��浽�ļ�������������(pBOM,bomLen)Ϊ��д���ļ�ͷ����BOM(Byte Order Mark)
	//����ļ��Ѿ����ڣ���ֱ�Ӹ��ǵ�ԭ���ļ�����
	bool saveToFile(const char* szFileName, const void* pBOM = NULL, size_t bomLen = 0);

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
	NODE_START_TAG, //��ʼ�ڵ㣬�� <a href="liigo.com">
	NODE_CLOSE_TAG, //�����ڵ㣬�� </a>
	NODE_CONTENT,   //�ı�
	NODE_REMARKS,   //ע�� <!-- -->
};

enum HtmlTagType
{
	TAG_UNKNOWN = 0,
	TAG_SCRIPT, TAG_STYLE, //���ڽ�����Ҫ����ʶ��,�ڲ��ر���
	TAG_A, TAG_B, TAG_BODY, TAG_BR, TAG_DIV, TAG_FONT, TAG_FORM, TAG_FRAME, TAG_FRAMESET, TAG_HR, 
	TAG_I, TAG_IFRAME, TAG_INPUT, TAG_IMG, TAG_LABEL, TAG_LI, TAG_META, TAG_P, TAG_SPAN, TAG_TITLE, TAG_UL, 
	//TAG_COLOR, TAG_BGCOLOR, //�Ǳ�׼HTML��ǩ, ��������ʹ��: <color=red>, ��Ч�� <color color=red>
};

struct HtmlNodeProp
{
	char* szName;
	char* szValue;
};

#define MAX_HTML_TAG_LENGTH  15 //�ڵ����Ƶ�����ַ�����,���������ض�

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
	void parseHtml(const char* szHtml, bool parseProps = false);

	//nodes
	int getHtmlNodeCount();
	HtmlNode* getHtmlNodes(int i);
	//props
	const HtmlNodeProp* getNodeProp(const HtmlNode* pNode, const char* szPropName);
	const char* getNodePropStringValue(const HtmlNode* pNode, const char* szPropName, const char* szDefaultValue = NULL);
	int getNodePropIntValue(const HtmlNode* pNode, const char* szPropName, int defaultValue = 0);
	void parseNodeProps(HtmlNode* pNode); //�����ڵ�����
	//debug
	void dumpHtmlNodes(FILE* f = stdout);
protected:
	//�������า��, �Ա�ʶ�������(��߽�������), ����ʶ����ٽ��(��߽����ٶ�)
	//Ĭ�Ͻ�ʶ���漰HTML�����ṹ����Ϣ�����޼���TAG: A,IMG,META,BODY,TITLE,FRAME,IFRAME
	virtual HtmlTagType onIdentifyHtmlTag(const char* szTagName, HtmlNodeType nodeType);
	//�������า��, �Ա���õĽ����ڵ�����, ���߲��ֽ��������ɴ಻�����ڵ�����(��߽����ٶ�)
	//���Ը��ݽڵ�����(pNode->tagName)��ڵ�����(pNode->tagType)�ж��Ƿ���Ҫ��������
	//Ĭ�Ͻ�������ʶ����ڵ����͵Ŀ�ʼ�ڵ�����ԣ���pNode->type == NODE_START_TAG && pNode->tagType != NODE_UNKNOWN��
	virtual void onParseNodeProps(HtmlNode* pNode);
	//�������า��, ��ĳ�ڵ������ɺ󱻵��ã��������false��ֹͣ����HTML
	virtual bool onNodeReady(HtmlNode* pNode) { return true; }

private:
	HtmlNode* newHtmlNode();
	void freeHtmlNodes();

private:
	MemBuffer m_HtmlNodes;
};

} //namespace liigo

#endif //__HtmlParser_H__
