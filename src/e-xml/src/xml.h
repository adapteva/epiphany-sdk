//from http://www.turboirc.com/xml/
//and doc from http://www.codeproject.com/cpp/FlexibleParser.asp

// xml.h
// v.0x132
// revision 18 - 10 - 2007
/*
This library is free for personal, experimental and commercial use, as long 
as you give me credit in your 'about box' and/or your help files. Absolutely 
no warranties for any misuse of all or parts of the supplied code; The code 
is provided 'as is'.
Free, for any kind or freeware, shareware, commercial, or whateverware 
project, as long as you give me credit for the library in your 'About box' or 
your application's documentation.

Authored, 2007 - Michael Chourdakis
*/

#ifndef _XML_H
#define _XML_H

#ifdef __BORLANDC__
#pragma warn -pck
#endif

#ifdef _MSC_VER
#define _USERENTRY __cdecl
#pragma warning(disable : 4996)
#pragma warning(disable : 4068)
#pragma warning(disable : 4311)
#pragma warning(disable : 4312)
#endif

#if defined(__linux__) || defined(VXWORKS)
#define _USERENTRY
/*#define __cdecl*/
#endif

#ifndef __cdecl
#define __cdecl
#endif


#define XML_VERSION 0x133
#define XML_VERSION_REVISION_DATE "18-10-2007"

// ANSI includes
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#ifdef _WIN32
#ifndef __SYMBIAN32__
#include <windows.h>
#include <commctrl.h>
#include <wininet.h>
#include <tchar.h>
#endif
#endif

#ifdef _WIN32
#define LIBEXP __declspec(dllexport)
#else
#define LIBEXP 
#endif

#ifdef __SYMBIAN32__
#define _USERENTRY
#define strcmpi strcmp
#include <unistd.h>
#endif


#ifndef __SYMBIAN32__
#ifdef XML_USE_STL
#include <vector>
using namespace std;
#endif
#endif

#ifndef _Z_H
#define _Z_H
// Z template class
template <class T>class Z
	{
   private:

   	T* d;
      unsigned int ss;

   public:

   	Z(int s = 0)
      {
			if (s == 0)
				s = 1;
		   d = new T[s];
		   memset(d,0,s*sizeof(T));
         ss = s;
      }
      ~Z()
      {
	      delete[] d;
      }

      operator T*()
      {
         return d;
      }

   void _clear()
		{
      ZeroMemory(d,ss*sizeof(T));
      }


   };
#endif // Z_H



class XMLHeader;
class XMLElement;
class XMLVariable;
class XMLComment;
class XMLContent;
class XMLCData;
class XML;

typedef struct
	{
   int VersionHigh;
   int VersionLow;
   char RDate[20];
   } XML_VERSION_INFO;


#ifdef _WIN32
struct IMPORTDBTABLEDATA
   {
   char name[256];
   char itemname[100];
   int nVariables;
   char** Variables;
   char** ReplaceVariables;
   };

struct IMPORTDBPARAMS
   {
   char* dbname;
   char* provstr;
   int nTables;
   IMPORTDBTABLEDATA* Tables;
   };
#endif

struct XMLEXPORTFORMAT
	{
	bool UseSpace;
	int nId;
	bool ElementsNoBreak;
	};

#ifdef _WIN32
struct IMPORTRKEYDATA
   {
   HKEY pK;
   int StorageType; // 0 - Native
					// 1 - Registry key from native XML
					// 2 - Registry key from registry XML
   };
#endif



// UNLOAD elements
struct XMLUNLOADELEMENT
	{
	int i;
	char* fn[300];
	};


// Global functions

class LIBEXP XML
   {
   public:

      // constructors/destructor

      XML();
      XML(const char* file,int LoadMode = 0,class XMLTransform* eclass = 0,class XMLTransformData* edata = 0);
      void Version(XML_VERSION_INFO*);
      int MemoryUsage();
		void CompressMemory();
		bool IntegrityTest();
		int Compare(XML*);

	  XML(XML& x);
	  XML& operator =(XML&);
	  ~XML();

//      static void Kill(char* tf);
	  static int DoMatch(char *text, char *p, bool IsCaseSensitive = false);
	  static bool VMatching(char *text, char *p, bool IsCaseSensitive = false);
	  static bool TestMatch(const char* item1,const char* comp,const char* item2);
	  static Z<char>* ReadToZ(const char*,class XMLTransform* eclass = 0,class XMLTransformData* edata = 0);
	  static int Convert2HexCharsToNumber(int c1, int c2);
	  static XMLElement* Paste();

	  int ParseStatus(int* = 0);
	  void SaveOnClose(bool);
	  int Load(const char* data,int LoadMode = 0,class XMLTransform* eclass = 0,class XMLTransformData* edata = 0);
	  int LoadText(char*);
	  int LoadText(wchar_t*);
	  static int PartialLoad(const char* file,const char* map);
	  static XMLElement * PartialElement(const char* file,const char* map);
	  int Save(const char* file = 0,int SaveMode = 1,int TargetMode = 0,class XMLTransform* eclass = 0,class XMLTransformData* edata = 0); // Default, do not encode already encoded
     void Export(FILE* fp,int SaveMode,int TargetMode = 0,XMLHeader *hdr = 0,class XMLTransform* eclass = 0,class XMLTransformData* edata = 0);
	  void SetExportFormatting(XMLEXPORTFORMAT* xf);

     void Lock(bool);
     void SetRootElement(XMLElement*);
	  XMLElement* RemoveRootElementAndKeep();

     XMLElement* GetRootElement();
     XMLHeader* GetHeader();
	  void SetHeader(XMLHeader* h);
#ifdef XML_OPTIONAL_IMPORTDB
     static XMLElement* ImportDB(IMPORTDBPARAMS* );
#endif
#ifdef XML_OPTIONAL_IMPORTRKEY
     static XMLElement* ImportRKey(IMPORTRKEYDATA*);
#endif
      static int XMLEncode(const char* src,char* trg);
      static int XMLDecode(const char* src,char* trg);
      int XMLGetValue(const char* section,char* attr,char* put,int maxlen);
      void XMLSetValue(const char* section,char* attr,char* put);

      // Query functions
      int XMLQuery(const char* rootsection,const char* expression,XMLElement** rv,unsigned int deep = 0xFFFFFFFF);

   private:

      void Init();
      void Clear();

      int iParseStatus; // 0 Valid , 1 Error but recovered, 2 fatal error
      int iParseStatusPos;
      char* f;          // filename
      XMLHeader* hdr;   // header (one)
      XMLElement* root; // root element (one)
      bool SOnClose;
      // For Windows


   };

#ifdef __linux__ /*LINUX*/
		typedef int (*fcmp) (const void *, const void *);
#endif

class LIBEXP XMLElement
   {
   public:

      // constructors/destructor
      XMLElement(XMLElement* par,const char* el,int Type = 0);

		//XMLElement& operator =(XMLElement&);
		~XMLElement();

		XMLElement* operator[](int);
		void SetElementParam(unsigned long p);
		unsigned long GetElementParam();
      void Reparse(const char*el,int Type = 0);
      int GetDeep();
      int AddElement(XMLElement*);
		int AddElement(char*);
      int InsertElement(unsigned int,XMLElement*);
      int FindElement(XMLElement*);
      int FindElement(char* n);
      XMLElement* FindElementZ(XMLElement*);
      XMLElement* FindElementZ(char* n,bool ForceCreate = false,char* el = 0);
		int RemoveElement(unsigned int i);
		int RemoveElementAndKeep(unsigned int i,XMLElement** el);
		int RemoveAllElements();
		int DeleteUnloadedElementFile(int i);
		int UnloadElement(unsigned int i);
		int ReloadElement(unsigned int i);
		int ReloadAllElements();
		int MoveElement(unsigned int i,unsigned int y);
#ifdef __linux__ /*LINUX*/
      void SortElements(fcmp);
      void SortVariables(fcmp);
      friend int XMLElementfcmp(const void *, const void *);
      friend int XMLVariablefcmp(const void *, const void *);
#else
      void SortElements(int (_USERENTRY *fcmp)(const void *, const void *));
      void SortVariables(int (_USERENTRY *fcmp)(const void *, const void *));
      friend int _USERENTRY XMLElementfcmp(const void *, const void *);
      friend int _USERENTRY XMLVariablefcmp(const void *, const void *);
#endif
      XMLElement* Duplicate(XMLElement* = 0);
		void Copy();
		int MemoryUsage();
		void CompressMemory();
		bool IntegrityTest();
		int Compare(XMLElement*);

		// XMLComment
		XMLComment** GetComments();
		unsigned int GetCommentsNum();
		int AddComment(XMLComment*,int InsertBeforeElement);
		int AddComment(char*,int);
		int RemoveComment(unsigned int i);
		int RemoveAllComments();

		// XMLCData
		XMLCData** GetCDatas();
		unsigned int GetCDatasNum();
		int AddCData(XMLCData*,int InsertBeforeElement);
		int AddCData(char*,int);
		int RemoveCData(unsigned int i);
		int RemoveAllCDatas();


      int AddVariable(XMLVariable*);
		int AddVariable(char*,char*);
#ifdef XML_OPTIONAL_MIME
		int AddBinaryVariable(char*,char*,int);
#endif
		int FindVariable(XMLVariable*);
      int FindVariable(char*  x);
      XMLVariable* FindVariableZ(XMLVariable*);
      XMLVariable* FindVariableZ(char* x,bool ForceCreate = false,char* defnew = 0);
		int RemoveVariable(unsigned int i);
		int RemoveVariableAndKeep(unsigned int i,XMLVariable** vr);
		int RemoveAllVariables();

		int AddContent(XMLContent* v,int InsertBeforeElement);
		int AddContent(char*,int);
		int RemoveContent(unsigned int i);
		void RemoveAllContents();
		XMLContent** GetContents();
		unsigned int GetContentsNum();

      XMLElement* GetElementInSection(const char*);
      int XMLQuery(const char* expression,XMLElement** rv,unsigned int deep = 0xFFFFFFFF);

      XMLElement* GetParent();

      XMLElement** GetChildren();
		unsigned int GetChildrenNum();
      unsigned int GetAllChildren(XMLElement**,unsigned int deep = 0xFFFFFFFF);
      unsigned int GetAllChildrenNum(unsigned int deep = 0xFFFFFFFF);

      XMLVariable** GetVariables();
      unsigned int GetVariableNum();
      void Export(FILE* fp,int ShowAll,int SaveMode,int TargetMode = 0,XMLHeader* hdr = 0,class XMLTransform* eclass = 0,class XMLTransformData* edata = 0);
		void SetExportFormatting(XMLEXPORTFORMAT* xf);

      void SetElementName(const char*);
      int GetElementName(char*,bool NoDecode = false);
	  char * GetElementName();
      int GetElementFullName(char*,bool NoDecode = false);
		int GetElementUniqueString(char*);

      int   GetType();
		static void Write16String(FILE* fp,char* s);

   private:

			XMLElement(XMLElement&);
			unsigned long param;
			char* el; // element name
			int type; // type, 0 element
			XMLElement* parent; // one
			XMLElement** children; // many
			XMLVariable** variables; // many
			XMLComment** comments; // many
			XMLContent** contents; // many;
			XMLCData** cdatas;
			unsigned int childrennum;
			unsigned int variablesnum;
			unsigned int commentsnum;
			unsigned int contentsnum;
			unsigned int cdatasnum;


      int SpaceForElement(unsigned int);
      int SpaceForVariable(unsigned int);
      int SpaceForComment(unsigned int);
      int SpaceForContent(unsigned int);
		int SpaceForCData(unsigned int);

      int TotalChildPointersAvailable;
      int TotalVariablePointersAvailable;
      int TotalCommentPointersAvailable;
      int TotalContentPointersAvailable;
      int TotalCDataPointersAvailable;
		XMLEXPORTFORMAT xfformat;

      static void printc(FILE* fp,XMLElement* root,int deep,int ShowAll,int SaveMode,int TargetMode);
      void SetParent(XMLElement*);


   };


class LIBEXP XMLVariable
	{
	public:

		XMLVariable(const char*,const char*,bool NoDecode = false);
		~XMLVariable();
	  XMLVariable(XMLVariable&);
	  XMLVariable& operator =(XMLVariable&);


		int MemoryUsage();
		void CompressMemory();
		bool IntegrityTest();
		int Compare(XMLVariable*);
		XMLElement* SetOwnerElement(XMLElement*);
		int GetName(char*,bool NoDecode = false);
		int GetValue(char*,bool NoDecode = false);
		int GetValueInt();
		
		//BittWare additions
		char* GetValueStr();
		char* GetNameStr();

		void SetName(const char*,bool NoDecode = false);
		void SetValue(const char*,bool NoDecode = false);
		void SetValueInt(int);
		XMLVariable* Duplicate();
		void Copy();

#ifdef XML_OPTIONAL_MIME
		int GetBinaryValue(char*);
		int SetBinaryValue(char*,int);
#endif

	private:

		void Clear();
		char* vn;
		char* vv;
	  XMLElement* owner;


	};


class LIBEXP XMLHeader
	{
	public:

		// constructors/destructor
      XMLHeader(const char* ht);
      operator char*();
      int MemoryUsage();
		void CompressMemory();
		bool IntegrityTest();
		int Compare(XMLHeader*);
		void SetEncoding(char*);
		XMLHeader* Duplicate();


	   XMLHeader(XMLHeader&);
	   XMLHeader& operator =(XMLHeader&);
      ~XMLHeader();

      // XMLComment
      XMLComment** GetComments();
		unsigned int GetCommentsNum();
      int AddComment(XMLComment*,int pos);
      int RemoveComment(unsigned int i);
      int RemoveAllComments();
      int SpaceForComment(unsigned int);

      void Export(FILE* fp,int SaveMode,int TargetMode = 0,class XMLTransform* eclass = 0,class XMLTransformData* edata = 0);

   private:

   	  void Clear();
      int TotalCommentPointersAvailable;
      char* hdr;
      unsigned int commentsnum;
		XMLComment** comments;
   };

class LIBEXP XMLComment
   {
   public:

	  // constructors/destructor
	  XMLComment(XMLElement* p,int ElementPosition,const char* ht);
	  operator char*();
	  void SetComment(const char* ht);
	  int MemoryUsage();
	  void CompressMemory();
	  bool IntegrityTest();
	  int Compare(XMLComment*);

	  XMLComment(XMLComment&);
	  XMLComment& operator =(XMLComment&);
	  ~XMLComment();

	  XMLComment* Duplicate();
	  void SetParent(XMLElement* p,int ep);
	  int GetEP();

	private:

	  XMLElement* parent;
	  char* c;
	  int ep; // Element Position (Before)
	};


class LIBEXP XMLContent
   {
   public:

	  // constructors/destructor
	  XMLContent(XMLElement* p,int ElementPosition,const char* ht,bool NoDecode = false);
	  int GetValue(char*,bool NoDecode = false);
	  void SetValue(const char*,bool NoDecode = false);
	  int MemoryUsage();
	  void CompressMemory();
	  bool IntegrityTest();
	  int Compare(XMLContent*);

		XMLContent(XMLContent&);
	  XMLContent& operator =(XMLContent&);
	  ~XMLContent();

	  XMLContent* Duplicate();
	  void SetParent(XMLElement* p,int ep);
	  int GetEP();

   private:

	  XMLElement* parent;
	  char* c;
	  int ep; // Element Position (Before)
   };

class LIBEXP XMLCData
   {
   public:

	  // constructors/destructor
	  XMLCData(XMLElement* p,int ElementPosition,const char* ht);
	  operator char*();
	  void SetCData(const char* ht);
	  int MemoryUsage();
	  void CompressMemory();
	  bool IntegrityTest();
	  int Compare(XMLCData*);

	  XMLCData(XMLCData&);
	  XMLCData& operator =(XMLCData&);
	  ~XMLCData();

	  XMLCData* Duplicate();
	  void SetParent(XMLElement* p,int ep);
	  int GetEP();

	private:

	  XMLElement* parent;
	  char* c;
	  int ep; // Element Position (Before)
	};



// public functions
int XMLGetString(const char* section,const char* Tattr,const char* defv,char*out,const int maxlen,const char* xml,XML* af = 0);
int     XMLGetInt(const char* item,const char* attr,const int defv,const char* xml,XML* af = 0);
#ifdef _WIN32
__int64 XMLGetInt64(const char* item,const char* attr,const __int64 defv,const char* xml,XML* af = 0);
#endif
float   XMLGetFloat(const char* item,const char* attr,const float defv,const char* xml,XML* af = 0);
int     XMLGetBinaryData(const char* item,const char* attr,const char* defv,char*out,const int maxlen,const char* xml,XML* af = 0);

int    XMLSetString(const char* section,const char* Tattr,char*out,const char* xml,XML* af = 0);
int    XMLSetInt(char* section,char* attr,int v,char* xml,XML* af = 0);
#ifdef _WIN32
int    XMLSetString(const char* section,const char* Tattr,wchar_t*out,const char* xml,XML* af = 0);
int    XMLSetInt64(char* section,char* attr,__int64 v,char* xml,XML* af = 0);
#endif
int    XMLSetFloat(char* section,char* attr,float v,char* xml,XML* af = 0);
int    XMLSetBinaryData(char* section,char* attr,char* data,int len,char* xml,XML* af = 0);

int XMLRenameElement(const char* section,const char* newname,const char* xml,XML* af = 0);

#ifndef __SYMBIAN32__
#ifdef XML_USE_STL
int    XMLGetAllVariables(char* section,vector<char*>* vnames,vector<char*>* vvalues,char*xml);
int    XMLGetAllItems(char* section,vector<char*>* vnames,char*xml);
#else
int    XMLGetAllVariables(char* section,char** vnames,char** vvalues,char*xml);
int    XMLGetAllItems(char* section,char** vnames,char*xml);
#endif
#endif



// XMLTransform class

class LIBEXP XMLTransformData
   {
   public:
      XMLTransformData() {}
   };

class LIBEXP XMLTransform
   {
   public:

      XMLTransform(XMLTransformData*) { }
      virtual ~XMLTransform() {}
      virtual int Encrypt(const char*src,int srclen,int srctype,char* dst,int dstlen,XMLTransformData* data = 0) = 0;
      virtual int Decrypt(const char*src,int srclen,int srctype,char* dst,int dstlen,XMLTransformData* data = 0) = 0;

   };

class LIBEXP XMLHelper
	{
	public:

	// static functions
		 static char* FindXMLClose(char* s);
		 static XMLElement* ParseElementTree(XMLHeader* hdr,XMLElement* parent,char* tree,char** EndValue,int& iParseStatus);
		 static void AddBlankVariable(XMLElement* parent,char *a2,int Pos);
		 static int pow(int P,int z);

			
		
	};



#endif // _XML_H


