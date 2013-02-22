// xml.cpp
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
#ifdef USE_EXTERNAL_Z
#include "z.h"
#endif

#include "xml.h"


#ifdef __WATCOMC__
#define _atoi64(x) atoll(x)
#endif


#ifndef __SYMBIAN32__
#ifndef __linux__
#pragma comment(lib,"wininet.lib")
#endif
#endif

#pragma warning (disable:4244)
#pragma warning (disable:4267)
#pragma warning (disable:4800)

#if defined(__linux__) || defined(WINCE) || defined(VXWORKS)
#define strcmpi(a,b) strcmp(a,b)
#endif

/*#ifdef WINCE
#define strcmpi(a,b) strcmp(a,b)
#endif*/


#ifndef XML_MAX_INIT_CHILDREN
#define XML_MAX_INIT_CHILDREN 20
#endif

#ifndef XML_MAX_INIT_VARIABLES
#define XML_MAX_INIT_VARIABLES 20
#endif

#ifndef XML_MAX_INIT_CONTENTS
#define XML_MAX_INIT_CONTENTS 4
#endif

#ifndef XML_MAX_INIT_COMMENTS
#define XML_MAX_INIT_COMMENTS 10
#endif

#ifndef XML_MAX_INIT_CDATAS
#define XML_MAX_INIT_CDATAS 10
#endif

#ifndef XML_MAX_INIT_COMMENTS_HEADER
#define XML_MAX_INIT_COMMENTS_HEADER 5
#endif

// Help functions    
#define MATCH_TRUE 1
#define MATCH_FALSE 0
#define MATCH_ABORT -1

#define NEGATE_CLASS
#define OPTIMIZE_JUST_STAR
#undef MATCH_TAR_PATTERN

// OPTI

/* Extra definitions

	XML_OPTIONAL_WIN32
	XML_OPTIONAL_IMPORTDB
	XML_OPTIONAL_MIME
	XML_OPTIONAL_IMPORTRKEY
*/

#ifdef XML_OPTIONAL_MIME
#include "mime.h"
#endif

#ifdef WINCE
int remove(char* f);
#endif


int XML :: DoMatch(char *text, char *p, bool IsCaseSensitive)
{
	// probably the MOST DIFFICULT FUNCTION in TurboIRC
   // Thanks to BitchX for copying this function
   //int last;
   int matched;
   //int reverse;
   int pT = 0;
   int pP = 0;

   for (; p[pP] != '\0'; pP++, pT++)
   {
      if (text[pT] == '\0' && p[pP] != '*')
         return MATCH_ABORT;
      switch (p[pP])
      {
//         case '\\': // Match with following char
//                pP++;
         // NO BREAK HERE

      default:
         if (IsCaseSensitive)
         {
            if (text[pT] != p[pP])
               return MATCH_FALSE;
            else
               continue;
         }
         if (toupper(text[pT]) != toupper(p[pP]))
//         if (DMtable[text[pT]] != DMtable[p[pP]])
            return MATCH_FALSE;
         continue;

      case '?':
         continue;

      case '*':
         if (p[pP] == '*')
            pP++;
         if (p[pP] == '\0')
            return MATCH_TRUE;
         while (text[pT])
         {
            matched = DoMatch(text + pT++, p + pP);
            if (matched != MATCH_FALSE)
               return matched;
         }
         return MATCH_ABORT;

      }
   }
#ifdef MATCH_TAR_PATTERN
   if (text[pT] == '/')
      return MATCH_TRUE;
#endif
   return (text[pT] == '\0');
}



// This will be called from the other funcs
bool XML :: VMatching(char *text, char *p, bool IsCaseSensitive)
{
#ifdef OPTIMIZE_JUST_STAR
   if (p[0] == '*' && p[1] == '\0')
      return MATCH_TRUE;
#endif
   return (DoMatch(text, p, IsCaseSensitive) == MATCH_TRUE);
}




// XML class

void XML :: Version(XML_VERSION_INFO* x)
   {
   x->VersionLow = (XML_VERSION & 0xFF);
   x->VersionHigh = (XML_VERSION >> 8);
	strcpy(x->RDate,XML_VERSION_REVISION_DATE);
   }
   
XML :: XML()
   {
   Init();
   }


XML :: XML(const char* file,int LoadMode,class XMLTransform* eclass,class XMLTransformData* edata)
   {
   Init();
   Load(file,LoadMode,eclass,edata);
   }


int XML :: LoadText(char* txt)
	{
	return Load(txt,1,0,0);
	}

int XML :: LoadText(wchar_t* txt)
	{
#ifdef _WIN32
	int udsize = wcslen(txt);
	int dsize = udsize*4 + 1000; // just for safety
	Z<char> utfbuff(dsize);

  // Conver the array
   WideCharToMultiByte(CP_UTF8,0,txt,-1,utfbuff,dsize,0,0);
	return LoadText(utfbuff);
#else
	return 0;
#endif
	}

void XML :: Init()
   {
   SOnClose = 0;
   hdr = 0;
   root = 0;
   f = 0;
   }

void XML :: Clear()
   {
   if (SOnClose)
   	Save();
   // root
   if (root)
      {
      root->RemoveAllElements();
      delete root;
      }
   root = 0;
      
   // hdr
   if (hdr)
      delete hdr;
   hdr = 0;

   // item
   if (f)
      delete[] f;
   f = 0;
   }


XMLElement* XML :: GetRootElement()
   {
   return root;
   }

void XML :: Lock(bool)
   {

   }

void XML :: SetRootElement(XMLElement* newroot)
   {
   delete root;
   root = 0;
   root = newroot;
   return;
   }

XMLElement* XML :: RemoveRootElementAndKeep()
	{
	XMLElement* x = root;
	root = new XMLElement(0,"<root/>");
	return x;
	}





XMLHeader* XML :: GetHeader()
   {
   return hdr;
   }

void XML :: SetHeader(XMLHeader* h)
   {
   if (hdr)
      delete hdr;
   hdr = 0;
   hdr = h;
   }

int XML :: XMLEncode(const char* src,char* trg)
   {
   if (!src)
      return 0;
   //*...
   int Y = strlen(src);

   int x = 0;
   for(int i = 0 ; i < Y ; i++)
      {
      if (src[i] == '&' && src[i + 1] != '#')
         {
         if (trg)
         	strcat(trg + x,"&amp;");
         x += 5;
         continue;
         }
      if (src[i] == '>')
         {
         if (trg)
	         strcat(trg + x,"&gt;");
         x += 4;
         continue;
         }
      if (src[i] == '<')
         {
         if (trg)
	         strcat(trg + x,"&lt;");
         x += 4;
         continue;
         }
      if (src[i] == '\"')
         {
         if (trg)
	         strcat(trg + x,"&quot;");
         x += 6;
         continue;
         }
      if (src[i] == '\'')
         {
         if (trg)
			 strcat(trg + x,"&apos;");
         x += 6;
         continue;
         }

      if (trg)
	      trg[x] = src[i];
      x++;
      }
   if (trg)
 	  trg[x] = 0;
   return x;
   }

#ifdef XML_OPTIONAL_IMPORTRKEY

XMLElement* XML :: ImportRKey(IMPORTRKEYDATA* d)
	{
	HKEY pK = d->pK;
	int mode = d->StorageType;

	XMLElement* x = new XMLElement(0,"<root />");

	// Reads pK (Assumes it is open) and imports ALL children !

	// if mode == 0 , native backup
	// no name prefix, variable B,I,Y,E,Z,N,Q,S
	//

	// Reads Values of pK and writes it to myself
	for(int i = 0 ; ; i++)
		{
		Z<char> tmp1(300);
		DWORD ts = 20000;
		DWORD ty = 0;
		DWORD si = 0;
		RegEnumValue(pK,i,tmp1,&ts,0,&ty,0,&si);
		Z<char> tmp2(si + 10);

		ts = 20000;
		if (RegEnumValue(pK,i,tmp1,&ts,0,&ty,(unsigned char*)tmp2.operator char *() + 2,&si) != ERROR_SUCCESS)
			break; // end of values

		// write
		if (ty == REG_BINARY)
			tmp2[0] = 'B';
		if (ty == REG_DWORD)
			tmp2[0] = 'I';
		if (ty == REG_DWORD_BIG_ENDIAN)
			tmp2[0] = 'Y';
		if (ty == REG_EXPAND_SZ)
			tmp2[0] = 'E';
		if (ty == REG_MULTI_SZ)
			tmp2[0] = 'Z';
		if (ty == REG_NONE)
			tmp2[0] = 'N';
		if (ty == REG_QWORD)
			tmp2[0] = 'Q';
		if (ty == REG_SZ)
			tmp2[0] = 'S';

		if (mode == 0)
			tmp2[1] = '_';

		XMLVariable* v = new XMLVariable(tmp1,tmp2);
		x->AddVariable(v);
		}

	// Now enum children keys and do the same
	for(int i = 0 ; ; i++)
		{
		Z<char> tmp1(300);
		Z<char> tmp2(300);
		DWORD si = 300;

		if (RegEnumKeyEx(pK,i,tmp1,&si,0,0,0,0) != ERROR_SUCCESS)
			break; // end of values

		sprintf(tmp2,"<%s />",tmp1);
		XMLElement* child = new XMLElement(x,tmp2);


		HKEY NewPK = 0;
		RegOpenKeyEx(pK,tmp1,0,KEY_ALL_ACCESS,&NewPK);
		if (NewPK)
			{
			IMPORTRKEYDATA d2 = {0};
			d2.pK = NewPK;
			d2.StorageType = mode;
			ImportRKey(&d2);
			x->AddElement(child);
			RegCloseKey(NewPK);
			}
		}

	return x;
	}
#endif


#ifdef XML_OPTIONAL_IMPORTDB
#include <exdisp.h>
#include <urlmon.h>
#include <shlobj.h>
//#include <adoid.h>
#include <adoint.h>

#define ADOGUID(name, l) extern "C" const GUID name = \
{l, 0, 0x10, 0x80,0,0,0xAA,0,0x6D,0x2E,0xA4}
ADOGUID(CLSID_CADOConnection,0x00000514);
ADOGUID(IID_IADOConnection,0x00000550);

XMLElement* XML :: ImportDB(IMPORTDBPARAMS* db)
   {
   // Imports an MDB database with ADO
   XMLElement* r = 0;
   Z<char> str(1000);
   Z<wchar_t> wstr(1000);

   ADOConnection* aC = 0;
	HRESULT hr;
	BSTR b1 = 0,b2 = 0,b3 = 0;

	// Open Database
	CoCreateInstance(CLSID_CADOConnection,0,CLSCTX_ALL,IID_IADOConnection,(void**)&aC);
	if (!aC)
		goto E_0;

	if (db->provstr == 0)
		sprintf(str,"Provider=Microsoft.Jet.OLEDB.4.0;Data Source=%s",db->dbname);
	else
		sprintf(str,db->provstr,db->dbname);

	MultiByteToWideChar(CP_ACP,0,str,-1,wstr,1000);


	b1 = SysAllocString(L"");
	b2 = SysAllocString(L"");
	b3 = SysAllocString(wstr);
	hr = aC->Open(b3,b1,b2,0);
	SysFreeString(b1);
	SysFreeString(b2);
	SysFreeString(b3);

	if (hr)
      goto E_1;

   r = new XMLElement(0,"<db />");

   // Loop for all tables
   for(int iTable = 0 ; iTable < db->nTables ; iTable++)
      {
      ADORecordset* aR = 0;
	   VARIANT* RecordsAffected = 0;

	   sprintf(str,"SELECT * FROM %s",db->Tables[iTable].name);
      MultiByteToWideChar(CP_ACP,0,str,-1,wstr,1000);
		b1 = SysAllocString(wstr);
	   hr = aC->Execute(b1,RecordsAffected,0,&aR);
		SysFreeString(b1);
		
	   if (hr)
		   continue;

      // Add this table to r
      sprintf(str,"<%s />",db->Tables[iTable].name);
      XMLElement* rT = new XMLElement(r,str);
      r->AddElement(rT);

	   aR->MoveFirst();
      long TotalRecords = 0;
	   aR->get_RecordCount(&TotalRecords);

	   for(unsigned int y = 0 ; y < (unsigned)TotalRecords ; y++)
         {
         // We are looping the items now
	      short IsEOF;
	      aR->get_EOF(&IsEOF);
	      if (IsEOF == VARIANT_TRUE)
	      	break;


	   	long RecordCount = 0;
		   ADOFields* aFS = 0;
		   aR->get_Fields(&aFS);
		   aFS->get_Count(&RecordCount);

         // Add this item
         sprintf(str,"<%s />",db->Tables[iTable].itemname);
         XMLElement* rItem = new XMLElement(rT,str);
         rT->AddElement(rItem);

		   for(int iVariables = 0 ; iVariables < RecordCount ; iVariables++)
            {
            // We are loopting the variables now, get only what we actually need
		      ADOField* aF = 0;
		      VARIANT vt;
		      vt.vt= VT_I4;
		      vt.intVal = iVariables;

			   aFS->get_Item(vt,&aF);
		      if (!aF)
		      	continue;

		   	wchar_t* Name;
		      aF->get_Name(&Name);

		   	VARIANT Value;
		      aF->get_Value(&Value);

            if (Value.vt == VT_BSTR)
				   WideCharToMultiByte(CP_UTF8, 0, Value.bstrVal, -1, str, 1000,0,0);
            else
            if (Value.vt == VT_I4)
				   sprintf(str,"%u",Value.lVal);

            // Should we add this variable to the rItem ?
            int SL = wcslen(Name);
            Z<char> nam(SL*2 + 100);
            WideCharToMultiByte(CP_UTF8, 0, Name, -1, nam, SL*2 + 100,0,0);
            int IsToAdd = -1;
            for(int yAdds = 0; yAdds < db->Tables[iTable].nVariables ; yAdds++)
               {
               if (strcmpi(db->Tables[iTable].Variables[yAdds],nam) == 0)
                  {
                  IsToAdd = yAdds;
                  break;
                  }
               }
            if (IsToAdd != -1)
               {
               char* thename = nam;
               if (db->Tables[iTable].ReplaceVariables[IsToAdd])
                  thename = db->Tables[iTable].ReplaceVariables[IsToAdd];
               XMLVariable* var = new XMLVariable(thename,str);
               rItem->AddVariable(var);
               }


   	      if (Name)
		         SysFreeString(Name);
   			if (Value.vt == VT_BSTR)
	      	   SysFreeString(Value.bstrVal);
		      aF->Release();
            }

         aFS->Release();
	      aR->MoveNext();
         }
      }

   E_1:
   aC->Release();

   E_0:
   return r;
   }
#endif


int XMLHelper :: pow(int P,int z)
   {
   if (z == 0)
      return 1;
   int x = P;
   for(int i = 1 ; i < z ; i++)
      x*= P;
   return x;
   }

int XML :: XMLDecode(const char* src,char* trg)
   {
   int Y = strlen(src);
   if (!trg)
      return Y;

   int x = 0;
   for(int i = 0 ; i < Y ; )
      {
      char* fo = strchr((char*)src + i,'&');
      if (!fo)
         {
         // end of &s
         strcpy(trg + x,src + i);
         x = strlen(trg);
         break;
         }

      if (fo)
         {
         int untilfo = fo - (src + i);
         strncpy(trg + x,src + i,untilfo);
         i += untilfo;
         x += untilfo;
         }

      if (src[i] == '&' && src[i + 1] == '#' && tolower(src[i + 2]) == 'x')
         {
         i += 3;
         int dig[10] = {0};
         int y = 0;

         while ((src[i] >= 0x30 && src[i] <= 0x39) || (src[i] >= 'a' && src[i] <= 'f') || (src[i] >= 'A' && src[i] <= 'F'))
            {
            char C = src[i];
            if (C >= 0x30 && C <= 0x39)
               C -= 0x30;
            else
            if (C >= 0x41 && C <= 0x46)
               C -= 55;
            else
            if (C >= 0x61 && C <= 0x66)
               C -= 87;

            dig[y] = C;
            y++;
            i++;
            }

         unsigned long N = 0;
         for(int z = (y - 1) ; z >= 0 ; z--)
            {
				N += dig[z] * XMLHelper :: pow(16,(y - 1) - z);
            }

         // Convert result to UTF-8
         char d1[100] = {0};
#ifdef _WIN32
         wchar_t d2[100] = {0};
         swprintf(d2,L"%c",(wchar_t)N);
         WideCharToMultiByte(CP_UTF8,0,d2,-1,d1,100,0,0);
#endif
         strcat(trg + x,d1);
         x += strlen(d1);
         i++;
         continue;
         }
      if (src[i] == '&' && src[i + 1] == '#')
         {
         i += 2;
         int dig[10] = {0};
         int y = 0;

         while (src[i] >= 0x30 && src[i] <= 0x39)
            {
            dig[y] = src[i] - 0x30;
            y++;
            i++;
            }

         unsigned long N = 0;
         for(int z = (y - 1) ; z >= 0 ; z--)
            {
            N += dig[z] * XMLHelper :: pow(10,(y - 1) - z);
            }

         // Convert result to UTF-8
         char d1[100] = {0};
#ifdef _WIN32
         wchar_t d2[100] = {0};
         swprintf(d2,L"%c",(wchar_t)N);
         WideCharToMultiByte(CP_UTF8,0,d2,-1,d1,100,0,0);
#endif
         strcat(trg + x,d1);
         x += strlen(d1);
         i++;
         continue;
         }


      if (src[i] == '&')
         {
         if (strncmp(src + i + 1,"amp;",4) == 0)
            {
            i += 5;
            trg[x] = '&';
            x++;
            }
         else
         if (strncmp(src + i + 1,"quot;",5) == 0)
            {
            i += 6;
            trg[x] = '\"';
            x++;
            }
         else
         if (strncmp(src + i + 1,"apos;",5) == 0)
            {
            i += 6;
            trg[x] = '\'';
            x++;
            }
         else
         if (strncmp(src + i + 1,"lt;",3) == 0)
            {
            i += 4;
            trg[x] = '<';
            x++;
            }
         else
         if (strncmp(src + i + 1,"gt;",3) == 0)
            {
            i += 4;
            trg[x] = '>';
            x++;
            }
         else
            x++; // ignore invalid symbol
         continue;
         }

      trg[x] = src[i];
      i++;
      x++;
      }

   trg[x] = 0;
   return strlen(trg);
   }


int XML :: XMLGetValue(const char* section2,char* attr2,char* put2,int maxlen)
   {
   int y1 = XMLEncode(section2,0);
   int y2 = XMLEncode(attr2,0);

   Z<char> section(y1 + 10);
   Z<char> attr(y2 + 10);

   XMLEncode(section2,section);
   XMLEncode(attr2,attr);

   if (y1 == 0) // root
      {
      int k = root->FindVariable(attr);
      if (k == -1)
         return 0;

      XMLVariable* v = root->GetVariables()[k];

      int Sug = v->GetValue(0);
      Z<char> value(Sug + 10);
      v->GetValue(value);
      int Y = strlen(value);
      if (Y > maxlen)
         return Y;

      strcpy(put2,value);
      return Y;
      }



   // section is a\b\c\d...
   XMLElement* r = root;
	char* a2 = section.operator char *();

   for( ; ; )
      {
      char* a1 = strchr(a2,'\\');
      if (a1)
         *a1 = 0;

      int y = r->FindElement(a2);
      if (y == -1)
         {
         	if (a1)
         		*a1 = '\\';
         	return 0;
         }

      r = r->GetChildren()[y];
      if (!a1) // was last
         break;

      *a1 = '\\';
      a2 = a1 + 1;
      }

   // element with this variable is found !
   int k = r->FindVariable(attr);
   if (k == -1)
      return 0;

   XMLVariable* v = r->GetVariables()[k];

   int Sug = v->GetValue(0);
   Z<char> value(Sug + 10);
   v->GetValue(value);
   int Y = strlen(value);
   if (Y > maxlen)
      return Y;

   strcpy(put2,value);
   return Y;
   }

void XML :: XMLSetValue(const char* section2,char* attr,char* put)
   {
   // section is a\b\c\d...
   XMLElement* r = root;
   XMLElement* rr = root;
   Z<char> section(strlen(section2) + 10);
   strcpy(section,section2);
	char* a2 = section.operator char *();

   // Also set to delete values
   // if put is NULL, delete the specified attribute
   // if attr is NULL, delete the specified section (and all subsections!)

   if (!section || strlen(section) == 0)
      {
      int k = root->FindVariable(attr);
      if (k == -1)
         {
         XMLVariable* x = new XMLVariable(attr,"",0);
         root->AddVariable(x);
         k = root->FindVariable(attr);
         }

      if (put == 0)
         {
         // Delete this attribute
         root->RemoveVariable(k);
         }
      else
         {
         root->GetVariables()[k]->SetValue(put);
         }
      return;
      }

   int y = 0;
   for( ; ; )
      {
      char* a1 = strchr(a2,'\\');
      if (a1)
         *a1 = 0;

      y = r->FindElement(a2);
      if (y == -1)
         {
         // Create this element
         XMLElement* n = new XMLElement(r,a2);
         r->AddElement(n);
         y = r->FindElement(n);
         }

      rr = r;
      r = rr->GetChildren()[y];
      if (!a1) // was last
         break;

      *a1 = '\\';
      a2 = a1 + 1;
      }

   // element with this variable is found/created!
   if (attr == 0)
      {
      // DELETE this element AND all sub-elements!!
      rr->RemoveElement(y);
      return;
      }


   int k = r->FindVariable(attr);
   if (k == -1)
      {
      XMLVariable* x = new XMLVariable(attr,"",0);
      r->AddVariable(x);
      k = r->FindVariable(attr);
      }


   if (put == 0)
      {
      // Delete this attribute
      r->RemoveVariable(k);
      }
   else
      {
      r->GetVariables()[k]->SetValue(put);
      }
   }




char* XMLHelper :: FindXMLClose(char* s)
   {
   // For Each <!-- we must find a -->
   // For Each <?   we must find a ?>
   // For each <> , we must find a </>
	// For each <![CDATA[ we mst find a ]]>
   // For each < /> , its all ok :)

   int d = 0;
   char* a2 = s;
   bool IsComment = false;
	bool IsCData = false;

   for(;;)
      {
      char* a1 = strchr(a2,'<');
      if (!a1) // duh
         return 0;

      if (*(a1 + 1) == '/')
         {
         a2 = strchr(a1,'>') + 1;
         d--;
         if (!d)
            return a2;

         continue;
         }

      if ((*(a1 + 1) == '!' && strlen(a1) > 2 && *(a1 + 2) == '-' && *(a1 + 3) == '-') || *(a1 + 1) == '?')
         IsComment = true;

		if (*(a1 + 1) == '!' && strlen(a1) > 8 && strncmp(a1 + 1,"![CDATA[",8) == 0)
			IsCData = true;

      bool Nest = 0;
      for(;;)
         {
			// Bugfix
			if (IsCData && (*(a1) != ']' || *(a1 + 1) != ']' || *(a1 + 2) != '>'))
				{
				a1++;
				continue;
				}
			if (IsCData)
				{
				a1 += 2;
				break;
				}

			if (*a1 != '/' && *a1 != '>')
            {
            if (*a1 == '\"')
               Nest = !Nest;

            a1++;
            continue;
            }
         if (*a1 == '/' && Nest)
            {
            a1++;
            continue;
            }

			// Also continue if / and no comment/no cdata
			if (*a1 == '/' && (IsComment || IsCData))
				{
				a1++;
				continue;
				}

			// Also continue if > and cdata with no ]]
			if (*a1 == '>' && IsCData && (*(a1 - 1) != ']' || *(a1 - 2) != ']'))
				{
				a1++;
				continue;
				}


			break;
         }

      d++;

      if ((*a1 == '/' || IsComment) && !IsCData) // nice, it closes
         {
         IsComment = false;
         a2 = a1 + 1;
         d--;
         if (d == 0)
            return a2; // finish !
         continue;
         }

		if (*a1 == '>' && IsCData && *(a1 - 1) == ']' && *(a1 - 2) == ']')
			{
			IsCData = false;
			a2 = a1 + 1;

         d--;
         if (d == 0)
            return a2; // finish !
         continue;
			}

      a2 = a1 + 1;
      }
   }   

void XMLHelper :: AddBlankVariable(XMLElement* parent,char *a2,int Pos)
   {
   int Y = strlen(a2);
   if (Y == 0 || parent == 0)
      return;

//   Z<char> vrb(Y + 1000);

/*   int y = 0;

   bool Ex = false;
   bool AddedSpace = false;


   for(int i = 0 ; i < Y ; i++)
      {
      char X = a2[i];

      if (Ex == false || AddedSpace == true)
         {
         if (X == '\r' || X == '\t' || X == '\n' || X == ' ')
            continue;
         }
      if (Ex == true)
         {
         if (X == '\r' || X == '\t' || X == '\n' || X == ' ')
            {
            vrb[y++] = ' ';
            AddedSpace = true;
            continue;
            }
		 }
      vrb[y++] = X;
      Ex = true;
      AddedSpace = false;
      }
*/
   char* a1 = a2;
   while(*a1 == ' ' || *a1 == '\t' || *a1 == '\n' || *a1 == '\r')
      a1++;
   int Z = strlen(a1);
   if (Z == 0)
      return;
   int PZ = Z;

   while(a1[PZ - 1] == '\t' || a1[PZ - 1] == '\r' || a1[PZ - 1] == '\n' || a1[PZ - 1] == ' ')
      PZ--;
      
   if (PZ == 0)
      return;

   char CC = a1[PZ];
   a1[PZ] = 0;

   // Add this vrb
/*   XMLVariable* x = new XMLVariable("*",a1,0);
   parent->AddVariable(x);
   a1[PZ] = CC;
*/
   XMLContent* x = new XMLContent(parent,Pos,a1,true);
   parent->AddContent(x,Pos);
   a1[PZ] = CC;
   }

XMLElement* XMLHelper :: ParseElementTree(XMLHeader* hdr,XMLElement* parent,char* tree,char** EndValue,int& iParseStatus)
   {
   char *a1,*a2,*a3,*a4,*a5;//,*a6;
   char c1,c2;//,c3,c4,c5,c6;

   XMLElement* root = 0;

   bool IsRootCommentSecond = false;

   a2 = tree;
   for(;;)
      {
      // find
      a3 = strchr(a2,'<');
      if (!a3)
         {
		 int Pos = parent ? parent->GetChildrenNum() : 0;
		 XMLHelper :: AddBlankVariable(parent,a2,Pos);
		 break; // end/error
		 }


		// Bugfix: See if a3 is cdata
		bool IsCData = false;
		if (strncmp(a3,"<![CDATA[",8) == 0)
			IsCData = true;
	  // Between a3 and a2, add everything which isn't \r\n,space,tabs
	  *a3 = 0;
	  int PosV = parent ? parent->GetChildrenNum() : 0;
	  XMLHelper :: AddBlankVariable(parent,a2,PosV);
      *a3 = '<';

		if (IsCData == true)
			a4 = strstr(a3,"]]>");
		else
			a4 = strchr(a3,'>');
      if (!a4)
         break; // end/error
		if (IsCData)
			a4 += 2; // move to '>'

      if ((*(a3 + 1) == '!' && strlen(a3 + 1) > 2 && *(a3 + 2) == '-' && *(a3 + 3) == '-' ) || *(a3 + 1) == '?') // comment/markup
         {
         c2 = *a4;
         *a4 = 0;
         if (parent)
            {
            //XMLElement* c = new XMLElement(parent,a3 + 1,1);
            //parent->AddElement(c);
            int Pos = parent->GetChildrenNum();
            Z<char> com(strlen(a3) + 100);
            strncpy(com,a3 + 4,strlen(a3 + 4) - 2);
            XMLComment* c = new XMLComment(parent,Pos,com);
            parent->AddComment(c,Pos);
            }
         else // It is a root comment
            {
            int Pos = IsRootCommentSecond;
            Z<char> com(strlen(a3) + 100);
            strncpy(com,a3 + 4,strlen(a3 + 4) - 2);
            XMLComment* c = new XMLComment(0,Pos,com);
            hdr->AddComment(c,Pos);
            }
         *a4 = c2;
         a2 = a4 + 1;
         continue;
         }

      if ((*(a3 + 1) == '!' && strlen(a3 + 1) > 8 && strncmp(a3 + 1,"![CDATA[",8) == 0)) // cdata
         {
         c2 = *a4;
         *a4 = 0;

			int Pos = parent->GetChildrenNum();
         Z<char> com(strlen(a3) + 100);
         strncpy(com,a3 + 9,strlen(a3 + 9) - 2);
         XMLCData* c = new XMLCData(parent,Pos,com);
         parent->AddCData(c,Pos);

			*a4 = c2;
         a2 = a4 + 1;
         continue;
         }


      if (*(a3 + 1) == '/') // bye bye from this element
         {
         if (parent && root && parent->FindElement(root) == -1)
            parent->AddElement(root);
         a2 = a4 + 1;
         continue;
         }


      IsRootCommentSecond = true;
      // It is an opening element
      // If < /> , all ok, Add to current and continue
      // If < > , then find relative at end, and recursive

      if (*(a4 - 1) == '/')
         {
         // Yes it is this element alone
         c2 = *a4;
         *a4 = 0;
         XMLElement* c = new XMLElement(parent,a3 + 1,0);
         if (parent)
            parent->AddElement(c);
         else
            ;// invalid. Can't have a < /> root

         *a4 = c2;
         a2 = a4 + 1;
         if (!root)
            root = c;
         continue;
         }



      // Now it is an < > entry
      // Find this one at end, strchr <
      a5 = XMLHelper :: FindXMLClose(a3);
      if (!a5)
         {
         // ERROR in the FILE
         iParseStatus = 2;
         return root;
         }

		a5--; // fixes a bug when next element is rightly after closing
      while(*a5 != '<')
         a5--;
      a1 = a5;
      c1 = *a1;
      *a1 = 0;

      // Create element a3
      c2 = *(a4 + 1);
      *(a4 + 1) = 0;
      root = new XMLElement(parent,a3,0);
      *(a4 + 1) = c2;
      char* eV = 0;
      XMLHelper :: ParseElementTree(hdr,root,a4 + 1,&eV,iParseStatus);



		char* sa2 = a2;
      *a1 = c1;
      a2 = a1;
      if (eV)
         a2 = eV;
		if (a2 == sa2)
			break; // whops ? error! . Break to avoid fatal loop
      continue;
      }

   return root;
   }

int XMLElement :: RemoveAllElements()
   {
   for(int i = childrennum - 1 ; i >= 0 ; i--)
      {
		if (children[i] == 0)
			{
			// Unload 
			DeleteUnloadedElementFile(i);
			}

      delete children[i];
      children[i] = 0;
      }
   childrennum = 0;
   return 0;
   }

void XMLElement :: SetElementParam(unsigned long p)
   {
   param = p;
   }

unsigned long XMLElement :: GetElementParam()
   {
   return param;
   }


int XMLElement :: DeleteUnloadedElementFile(int i)
	{
	// Find Unique STR
	int si = GetElementUniqueString(0);
	Z<char> us(si);
	GetElementUniqueString(us);
	if (us[strlen(us) - 1] == '-')
		us[strlen(us) - 1] = 0;
	// Add this element
	if (strlen(us))
		sprintf(us + strlen(us),"-%u",i);
	else
		sprintf(us + strlen(us),"%u",i);
	// Extension
	strcat(us,".xmltmp");

	return remove(us);
	}

int XMLElement :: RemoveElement(unsigned int i)
   {
   if (i >= childrennum)
      return childrennum;

	if (children[i] == 0)
		{
		// Unloaded already, just delete the file
		DeleteUnloadedElementFile(i);
		}

   delete children[i];
   children[i] = 0;

   for(unsigned int k = i ; k < childrennum ; k++)
      children[k] = children[k + 1];

   children[childrennum - 1] = 0;
   return --childrennum;
   }

int XMLElement :: RemoveElementAndKeep(unsigned int i,XMLElement** el)
   {
   if (el) 
		*el = 0;   

   if (i >= childrennum)
	  return childrennum;

	if (children[i] == 0) // unloaded
		ReloadElement(i);

   //delete children[i];
   if (el) 
		*el = children[i];   
   children[i] = 0;

   for(unsigned int k = i ; k < childrennum ; k++)
	  children[k] = children[k + 1];

   children[childrennum - 1] = 0;
   return --childrennum;
   }

int XMLElement :: UnloadElement(unsigned int i)
	{
	XMLElement* e = children[i];
	if (!e)
		return 1; // already unloaded

	e->ReloadAllElements();

	// Find Unique STR
	int si = GetElementUniqueString(0);
	Z<char> us(si);
	GetElementUniqueString(us);
	if (us[strlen(us) - 1] == '-')
		us[strlen(us) - 1] = 0;
	// Add this element
	if (strlen(us))
		sprintf(us + strlen(us),"-%u",i);
	else
		sprintf(us + strlen(us),"%u",i);
	// Extension
	strcat(us,".xmltmp");



	FILE* fp = fopen(us,"rb");
	if (fp)
		{
		// file exists !
		fclose(fp);
		return 0;
		}

	fp = fopen(us,"wb");
	if (!fp)
		{
		// Failed !
		return 0;
		}

	e->Export(fp,1,0);
	fclose(fp);

	// Delete this element, but do not remove it.
   delete children[i];
   children[i] = 0;

	return 1;
	}

int XMLElement :: ReloadElement(unsigned int i)
	{
	if (children[i])
		return 1; // Item is already here

	// Find Unique STR
	int si = GetElementUniqueString(0);
	Z<char> us(si);
	GetElementUniqueString(us);
	if (us[strlen(us) - 1] == '-')
		us[strlen(us) - 1] = 0;
	// Add this element
	if (strlen(us))
		sprintf(us + strlen(us),"-%u",i);
	else
		sprintf(us + strlen(us),"%u",i);
	// Extension
	strcat(us,".xmltmp");

	FILE* fp = fopen(us,"rb");
	if (!fp)
		{
		// file failed !
		return 0;
		}
	fclose(fp);
	XML fx(us);
   int K = fx.ParseStatus();
   if (K == 2) // Fatal error
		return 0; 

	XMLElement* r = fx.RemoveRootElementAndKeep();

	// Reload element
	children[i] = r;
	r->SetParent(this);

	remove(us);
	return 1;
	}

int XMLElement :: ReloadAllElements()
	{
	for(unsigned int i = 0 ; i < childrennum ; i++)
		{
		if (children[i] == 0)
			ReloadElement(i);
		}

	return 0;
	}


int XMLElement :: MoveElement(unsigned int i,unsigned int y)
   {
   if (i >= childrennum || y >= childrennum)
      return childrennum;

   XMLElement* x = children[i];

   children[i] = 0;
   for(unsigned int k = i ; k < childrennum ; k++)
      children[k] = children[k + 1];

   return InsertElement(y,x);
   }

int XMLElement :: InsertElement(unsigned int y,XMLElement* x)
   {
   // leave from 0 to y
   // move from y + 1 to childrennum + 1
   // save
   // childrennum++;
   if (y >= childrennum)
      return AddElement(x);

   SpaceForElement(1);

   memmove((void*)(children + y + 1),(void*)(children + y),(childrennum - y)*sizeof(XMLElement*));
   children[y] = x;
   x->SetParent(this);
   childrennum++;
   return y;
   }

int XMLElement :: RemoveAllVariables()
   {
   for(int i = variablesnum - 1 ; i >= 0 ; i--)
	  {
	  delete variables[i];
	  variables[i] = 0;
	  }
   variablesnum = 0;
   return 0;
   }


int XMLElement :: RemoveVariable(unsigned int i)
   {
   if (i >= variablesnum)
	  return variablesnum;

   delete variables[i];
   variables[i] = 0;

   for(unsigned int k = i ; k < variablesnum ; k++)
	  variables[k] = variables[k + 1];

   variables[variablesnum - 1] = 0;
   return --variablesnum;
   }

int XMLElement :: RemoveVariableAndKeep(unsigned int i,XMLVariable** vr)
   {
   if (vr) 
	{
	*vr = 0;   
	}
   if (i >= variablesnum)
	  return variablesnum;

   //delete variables[i];
   if (vr) 
	{
	*vr = variables[i];   
	}
   variables[i] = 0;

   for(unsigned int k = i ; k < variablesnum ; k++)
	  variables[k] = variables[k + 1];

   variables[variablesnum - 1] = 0;
   return --variablesnum;
   }

void XML :: Export(FILE* fp,int SaveMode,int TargetMode,XMLHeader *hdr,class XMLTransform* eclass,class XMLTransformData* edata)
   {
   // Export all elements
   root->Export(fp,1,SaveMode,TargetMode,hdr,eclass,edata);
   }

void XML :: SetExportFormatting(XMLEXPORTFORMAT* xf)
	{
	root->SetExportFormatting(xf);
	}

XMLElement* XMLElement :: GetElementInSection(const char* section2)
   {
   // From section, get the element we need
   XMLElement* r = this;
   if (strcmp(section2,"") == 0)
   	return this;

	Z<char> section(strlen(section2) + 1);
	strcpy(section,section2);
   
	char* a2 = section.operator char *();

   for( ; ; )
      {
      char* a1 = strchr(a2,'\\');
      if (a1)
         *a1 = 0;

      int y = r->FindElement(a2);
      if (y == -1)
         {
         	if (a1)
         		*a1 = '\\';
         	return 0;
         }

      r = r->GetChildren()[y];
      if (!a1) // was last
         break;

      *a1 = '\\';
      a2 = a1 + 1;
      }


   return r;
   }


void XMLElement :: Write16String(FILE* fp,char* s)
	{
#ifdef _WIN32
	int sl = strlen(s)*2 + 100;
	Z<wchar_t> ws(sl);
	MultiByteToWideChar(CP_UTF8,0,s,-1,ws,sl);
	fwrite(ws.operator wchar_t*(),1,wcslen(ws)*2,fp);
#endif
	}

void XMLElement :: printc(FILE* fp,XMLElement* root,int deep,int ShowAll,int SaveMode,int TargetMode)
   {
   if (!root)
	  return;

	root->ReloadAllElements();

   char* sp = (char*)fp;
	if (TargetMode == 1)
		sp += strlen(sp);
	unsigned int spi = 0;
#ifdef _WIN32
   HKEY pKey = (HKEY)fp;
   HKEY pKey2 = 0;
#endif


	/*
		Targetmodes

		0 	- Export to a FILE*
		1 	- Export to memory
		2  - Export to a registry key (Win32)
		3  - Export to a FILE* , utf-16

	*/

	char DelimiterChar[100] = {0};
	if (root->xfformat.UseSpace)
		{
		for(int i = 0 ; i < root->xfformat.nId ; i++)
			strcat(DelimiterChar," ");
		}
	else
		{
		for(int i = 0 ; i < root->xfformat.nId ; i++)
			strcat(DelimiterChar,"\t");
		}
	//* Use it later


   int Sug = root->GetElementName(0,SaveMode);
   Z<char> b(Sug + deep + 100);
   for(int i = 0 ; i < deep ; i++)
      //strcat(b,"\t");
		strcat(b,DelimiterChar);

   strcat(b,"<");
   root->GetElementName(b.operator char*() + strlen(b),SaveMode);
   if (TargetMode == 1)
		{
		spi = sprintf(sp,"%s",b.operator char*());
		sp += spi;
		}
   else
   if (TargetMode == 2)
	  {
#ifdef _WIN32
#ifndef WINCE
	  XMLElement* par = root->GetParent();
	  int bP = 0;
	  if (par)
		{
		bP = par->FindElement(root);
		}
	  sprintf(b,"E%u",bP);
	  root->GetElementName(b.operator char *() + strlen(b),SaveMode);
	  DWORD dw = 0;
	  RegCreateKeyExA(pKey,b,0,0,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,0,&pKey2,&dw);
#endif
#endif
	  }
   else
   if (TargetMode == 3)
		Write16String(fp,b.operator char*());
   else
	  fprintf(fp,"%s",b.operator char*());
  // DJM3
   // printf("%s",b.operator char*());

   int iY = root->GetVariableNum();
   int iC = root->GetChildrenNum();

   // print variables if they exist
//   XMLVariable* SaveAstVariable = 0;
   if (iY)
	  {
	  for(int i = 0 ; i < iY ; i++)
		 {
		 XMLVariable* v = root->GetVariables()[i];
		 int s1 = v->GetName(0,SaveMode);
		 int s2 = v->GetValue(0,SaveMode);

		 Z<char> Name(s1 + 10);
		 Z<char> Value(s2 + 10);

		 v->GetName(Name,SaveMode);
		 v->GetValue(Value,SaveMode);

/*		 if (strcmp(Name,"*") == 0)
			SaveAstVariable = v;
		 else*/
			{
			if (TargetMode == 1)
			   {
			   spi = sprintf(sp," %s=",Name.operator char*());
				sp += spi;
			   spi = sprintf(sp,"\"%s\"",Value.operator char*());
				sp += spi;
			   }
			else
			if (TargetMode == 2)
			   {
#ifdef _WIN32
#ifndef WINCE
				// create a value
			   Z<char> VName(strlen(Name) + 10);
			   sprintf(VName,"V%s",Name.operator char*());
			   RegSetValueExA(pKey2,VName,0,REG_SZ,(const BYTE*)Value.operator char *(),strlen(Value) + 1);
#endif
#endif
			   }
			else
			if (TargetMode == 3)
			   {
				Z<char> xy(strlen(Name)*2 + 100 + strlen(Value)*2);
				sprintf(xy," %s=\"%s\"",Name.operator char*(),Value.operator char*());
				Write16String(fp,xy);
				}
			else // TM == 0
			   {
			   fprintf(fp," %s=",Name.operator char*());
			   fprintf(fp,"\"%s\"",Value.operator char*());
			   }
			}
		 }
	  }

   // cdatas, comments, contents may be between children
   int TotalCDatas = root->GetCDatasNum();
   int NextCData = 0;

	int TotalComments = root->GetCommentsNum();
   int NextComment = 0;

   int TotalContents = root->GetContentsNum();
   int NextContent = 0;

   // children ?
   // close now if no children/contents/comments
	if ((!iC || ShowAll == 0) && /*SaveAstVariable == 0 &&*/ TotalContents == 0 && TotalComments == 0 && TotalCDatas == 0)
	  {
	  if (TargetMode == 1)
		  {
		  spi = sprintf(sp,"/>\r\n");
		  sp += spi;
		  }
	  else
	  if (TargetMode == 2)
		 ; // Nothing :)
	  else
	  if (TargetMode == 3)
		 fwrite(L"/>\r\n",1,4,fp);
	  else // 0
		 fprintf(fp,"/>\r\n");
	  return;
	  }
   if (TargetMode == 1)
		{
		spi = sprintf(sp,">\r\n");
//		spi = sprintf(sp,">\r\n",b.operator char*());
	   sp += spi;
		}
   else
		{
		// Write \r\n only if ElementBreak
		if (root->xfformat.ElementsNoBreak == false || TotalContents != 1 || TotalComments || TotalCDatas || iC)
			{
		   if (TargetMode == 2)
				; // Nothing :)
			else
			if (TargetMode == 3)
				fwrite(L">\r\n",1,3,fp);
			else
				fprintf(fp,">\r\n");
//				fprintf(fp,">\r\n",b.operator char*());
			}
		else
			{
		   if (TargetMode == 2)
				; // Nothing :)
			else
			if (TargetMode == 3)
				fwrite(L">",1,1,fp);
			else
				fprintf(fp,">");
//				fprintf(fp,">",b.operator char*());
			}
		}


   if (ShowAll)
	  {
	  for(int i = 0 ; i < iC ; i++)
		 {
		 if (TotalComments && (NextComment < TotalComments))
			{
			while ((NextComment < TotalComments) && root->GetComments()[NextComment]->GetEP() <= i)
			   {
			   // print that comment now
			   char* t = root->GetComments()[NextComment]->operator char *();
			   Z<char> b(strlen(t) + deep + 200);
			   for(int i = 0 ; i < (deep + 1) ; i++)
//				  strcat(b,"\t");
				  strcat(b,DelimiterChar);
			   strcat(b,"<!--");
			   strcat(b,t);
			   strcat(b,"-->\r\n");

			   if (TargetMode == 1)
					{
					spi = sprintf(sp,"%s",b.operator char*());
		  		   sp += spi;
					}
			   else
			   if (TargetMode == 2)
				  {
#ifdef _WIN32
#ifndef WINCE
				  // Create a comment
				  Z<char> VName(20);
				  sprintf(VName,"C%u",NextComment);
				  RegSetValueExA(pKey2,VName,0,REG_SZ,(const BYTE*)t,strlen(t) + 1);
#endif
#endif
				  }
			   else
				if (TargetMode == 3)
				  Write16String(fp,b.operator char*());
			   else // 0
				   fprintf(fp,"%s",b.operator char*());

            NextComment++;
				}
			 }

		 if (TotalContents && (NextContent < TotalContents))
			{
			while ((NextContent < TotalContents) && root->GetContents()[NextContent]->GetEP() <= i)
			   {
			   // print that content now
			   //char* t = root->GetContents()[NextContent]->operator char *();
			   int vx = root->GetContents()[NextContent]->GetValue(0);

			   Z<char> b(vx + deep + 200);
				if (root->xfformat.ElementsNoBreak == false)
					{
				   for(int i = 0 ; i < (deep + 1) ; i++)
					//strcat(b,"\t");
					strcat(b,DelimiterChar);
					//strcat(b,t);
					}
				root->GetContents()[NextContent]->GetValue(b.operator char*() + strlen(b),SaveMode);
				if (root->xfformat.ElementsNoBreak == false)
					strcat(b,"\r\n");
			// DJM3
			   if (TargetMode == 1)
					{
					spi = sprintf(sp,"%s",b.operator char*());
					sp += spi;
					}
			   else
			   if (TargetMode == 2)
				  {
#ifdef _WIN32
#ifndef WINCE
				  // Create a content
				  Z<char> VName(20);
				  sprintf(VName,"D%u",NextContent);
				  root->GetContents()[NextContent]->GetValue(b,SaveMode);
				  RegSetValueExA(pKey2,VName,0,REG_SZ,(const BYTE*)b.operator char *(),strlen(b) + 1);
#endif
#endif
				  }
			   else
				if (TargetMode == 3)
				  Write16String(fp,b.operator char*());
				else // 0
				  fprintf(fp,"%s",b.operator char*());

			   NextContent++;
			   }
		 if (TotalCDatas && (NextCData < TotalCDatas))
			{
			while ((NextCData < TotalCDatas) && root->GetCDatas()[NextCData]->GetEP() <= i)
			   {
			   // print that CData now
			   char* t = root->GetCDatas()[NextCData]->operator char *();
			   Z<char> b(strlen(t) + deep + 200);
			   for(int i = 0 ; i < (deep + 1) ; i++)
//				  strcat(b,"\t");
				  strcat(b,DelimiterChar);
			   strcat(b,"<![CDATA[");
			   strcat(b,t);
			   strcat(b,"]]>\r\n");

			   if (TargetMode == 1)
					{
					spi = sprintf(sp,"%s",b.operator char*());
					sp += spi;
					}
			   else
			   if (TargetMode == 2)
				  {
#ifdef _WIN32
#ifndef WINCE
				  // Create a cdata
				  Z<char> VName(20);
				  sprintf(VName,"D%u",NextCData);
				  RegSetValueExA(pKey2,VName,0,REG_SZ,(const BYTE*)t,strlen(t) + 1);
#endif
#endif
				  }
			   else
				if (TargetMode == 3)
				  Write16String(fp,b.operator char*());
			   else // 0
				  fprintf(fp,"%s",b.operator char*());

               NextCData++;
               }
            }
			}

			
#ifdef _WIN32
		 if (TargetMode == 2)
			printc((FILE*)pKey2,root->GetChildren()[i],deep + 1,ShowAll,SaveMode,TargetMode);
		 else
#endif		 	
			 {
			 printc(fp,root->GetChildren()[i],deep + 1,ShowAll,SaveMode,TargetMode);
			 if (TargetMode == 1)
				sp = (char*)fp + strlen((char*)fp);
			 }
		 }
	  }

   // Check if there are still comments
   if (TotalComments && (NextComment < TotalComments))
	  {
	  while (NextComment < TotalComments)
		 {
		 // print that comment now
		 char* t = root->GetComments()[NextComment]->operator char *();
		 Z<char> b(strlen(t) + deep + 200);
		 for(int i = 0 ; i < (deep + 1) ; i++)
			//strcat(b,"\t");
  		   strcat(b,DelimiterChar);
		 strcat(b,"<!--");
		 strcat(b,t);
		 strcat(b,"-->\r\n");

		 if (TargetMode == 1)
			 {
			 spi = sprintf(sp,"%s",b.operator char*());
			 sp += spi;
			 }
		 else
		 if (TargetMode == 2)
			{
#ifdef _WIN32
#ifndef WINCE
			// Create a comment
			Z<char> VName(20);
			sprintf(VName,"C%u",NextComment);
			RegSetValueExA(pKey2,VName,0,REG_SZ,(const BYTE*)t,strlen(t) + 1);
#endif
#endif
			}
	   else
		if (TargetMode == 3)
		  Write16String(fp,b.operator char*());
	   else // 0
			fprintf(fp,"%s",b.operator char*());

		 NextComment++;
		 }
	  }

   // Check if there are still cdatas
   if (TotalCDatas && (NextCData < TotalCDatas))
	  {
	  while (NextCData < TotalCDatas)
		 {
		 // print that CData now
		 char* t = root->GetCDatas()[NextCData]->operator char *();
		 Z<char> b(strlen(t) + deep + 200);
		 for(int i = 0 ; i < (deep + 1) ; i++)
			//strcat(b,"\t");
  		   strcat(b,DelimiterChar);
		 strcat(b,"<![CDATA[");
		 strcat(b,t);
		 strcat(b,"]]>\r\n");

		 if (TargetMode == 1)
			 {
			 spi = sprintf(sp,"%s",b.operator char*());
			 sp += spi;
			 }
		 else
		 if (TargetMode == 2)
			{
#ifdef _WIN32
#ifndef WINCE
			// Create a cdata
			Z<char> VName(20);
			sprintf(VName,"D%u",NextCData);
			RegSetValueExA(pKey2,VName,0,REG_SZ,(const BYTE*)t,strlen(t) + 1);
#endif
#endif
			}
		 else
		 if (TargetMode == 3)
		   Write16String(fp,b.operator char*());
	    else // 0
			fprintf(fp,"%s",b.operator char*());

		 NextCData++;
		 }
	  }


	// Check if there are still Contents
   if (TotalContents && (NextContent < TotalContents))
	  {
	  while (NextContent < TotalContents)
		 {
		 // print that content now
		 //char* t = root->GetContents()[NextContent]->operator char *();
		 int vx = root->GetContents()[NextContent]->GetValue(0);

		 Z<char> b(vx + deep + 200);
		 if (root->xfformat.ElementsNoBreak == false)
			 {
			 for(int i = 0 ; i < (deep + 1) ; i++)
				//strcat(b,"\t");
   			strcat(b,DelimiterChar);
			 }
		 //strcat(b,t);
		 root->GetContents()[NextContent]->GetValue(b.operator char*() + strlen(b),SaveMode);
		 if (root->xfformat.ElementsNoBreak == false)
			 strcat(b,"\r\n");

		 if (TargetMode == 1)
			 {
			 spi = sprintf(sp,"%s",b.operator char*());
			 sp += spi;
			 }
		 else
		 if (TargetMode == 2)
			{
#ifdef _WIN32
#ifndef WINCE
			// Create a content
			Z<char> VName(20);
			sprintf(VName,"D%u",NextContent);
			root->GetContents()[NextContent]->GetValue(b,SaveMode);
			RegSetValueExA(pKey2,VName,0,REG_SZ,(const BYTE*)b.operator char *(),strlen(b) + 1);
#endif
#endif
			}
 	    else
		 if (TargetMode == 3)
		  Write16String(fp,b.operator char*());
	    else // 0
			fprintf(fp,"%s",b.operator char*());

		 NextContent++;
		 }
	  }

   // ending
   strcpy(b,"");
	if (root->xfformat.ElementsNoBreak == false || iC || TotalCDatas || TotalComments || TotalContents != 1)
		{
	   for(int i = 0 ; i < deep ; i++)
			//strcat(b,"\t");
			strcat(b,DelimiterChar);
		}
   strcat(b,"</");
   root->GetElementName(b.operator char*() + strlen(b));
   strcat(b,">\r\n");

   if (TargetMode == 1)
		{
		spi = sprintf(sp,"%s",b.operator char*());
		sp += spi;
		}
   else
   if (TargetMode == 2)
	  {
	   // Nothing
#ifdef _WIN32
	  RegCloseKey(pKey2);
#endif
	  }
   else
	if (TargetMode == 3)
	  Write16String(fp,b.operator char*());
   else // 0
	  fprintf(fp,"%s",b.operator char*());
   }

void XMLElement :: SetExportFormatting(XMLEXPORTFORMAT* xf)
	{
	if (xf)
		memcpy(&xfformat,xf,sizeof(XMLEXPORTFORMAT));
	if (xfformat.nId > 50)
		xfformat.nId = 50;
	for(unsigned int i = 0 ; i < GetChildrenNum() ; i++)
		GetChildren()[i]->SetExportFormatting(xf);
	}

void XMLElement :: Export(FILE* fp,int ShowAll,int SaveMode,int TargetMode,XMLHeader* hdr,class XMLTransform* eclass,class XMLTransformData* edata)
   {
   // Export this element
	ReloadAllElements();
   if (eclass == 0)
      {
      if (hdr)
         hdr->Export(fp,0,TargetMode,eclass,edata);
      printc(fp,this,0,ShowAll,SaveMode,TargetMode);
      if (hdr)
         hdr->Export(fp,1,TargetMode,eclass,edata);
      }
   else
      {
      //* save to another fp, then encrypt with eclass to this fp
      int S = MemoryUsage();
      Z<char> ram(S);
      int NewTargetMode = 1;
      if (hdr)
         hdr->Export((FILE*)ram.operator char *(),0,NewTargetMode,eclass,edata);
      printc((FILE*)ram.operator char *(),this,0,ShowAll,SaveMode,NewTargetMode);
      if (hdr)
         hdr->Export((FILE*)ram.operator char *(),1,NewTargetMode,eclass,edata);


      Z<char> yy(S + 100);

      // convert
      //eclass->Prepare(edata);
      S = strlen(ram);
      int nS = eclass->Encrypt(ram.operator char *(),S,0,yy.operator char *(),S + 100,0);

      // Write
      if (TargetMode == 0)
         fwrite(yy.operator char* (),1,nS,fp);
      else
      if (TargetMode == 1)
         memcpy((char*)fp,yy.operator char *(),nS);


/*      char* tf = ".\\a.tmp";
      FILE* fpn = fopen(tf,"wb");
      if (!fpn)
         return;

      // save
      if (hdr)
         hdr->Export(fp,0,TargetMode,eclass,edata);
      printc(fpn,this,0,ShowAll,SaveMode,TargetMode);
      if (hdr)
         hdr->Export(fp,1,TargetMode,eclass,edata);

      int S = ftell(fpn);
      fclose(fpn);

      // read a.tmp again
      Z<char>* y = XML :: ReadToZ(tf);
      XML :: Kill(tf);

      Z<char> yy(S + 100);

      // convert
      eclass->Prepare(edata);
      int nS = eclass->Encrypt((*y).operator char *(),S,0,yy.operator char *(),S + 100,0);

      fwrite(yy.operator char* (),1,nS,fp);
      delete y;
*/
      }
   }


int _USERENTRY  XMLElementfcmp(const void * a, const void * b)
   {
   XMLElement* x1 = *(XMLElement**)a;
   XMLElement* x2 = *(XMLElement**)b;

   // compare names
   int z1 = x1->GetElementName(0);
   int z2 = x2->GetElementName(0);

   Z<char> s1(z1 + 10);
   Z<char> s2(z2 + 10);
   x1->GetElementName(s1);
   x2->GetElementName(s2);

   return strcmpi(s1,s2);
   }

int _USERENTRY  XMLVariablefcmp(const void * a, const void * b)
   {
   XMLVariable* x1 = *(XMLVariable**)a;
   XMLVariable* x2 = *(XMLVariable**)b;

   // compare names
   int z1 = x1->GetName(0);
   int z2 = x2->GetName(0);

   Z<char> s1(z1 + 10);
   Z<char> s2(z2 + 10);
   x1->GetName(s1);
   x2->GetName(s2);

   return strcmpi(s1,s2);
   }

void XMLElement :: SortElements(int (_USERENTRY *fcmp)(const void *, const void *))
   {
   // to all elements
   XMLElement** x =  GetChildren();
   int y = GetChildrenNum();
   if (!fcmp)
      qsort(x,y,sizeof(XMLElement*),XMLElementfcmp);
   else
      qsort(x,y,sizeof(XMLElement*),fcmp);
   }

void XMLElement :: SortVariables(int (_USERENTRY *fcmp)(const void *, const void *))
   {
   // to all Variables
   XMLVariable** x =  GetVariables();
   int y = GetVariableNum();
   if (!fcmp)
      qsort(x,y,sizeof(XMLVariable*),XMLVariablefcmp);
   else
      qsort(x,y,sizeof(XMLVariable*),fcmp);
   }

// Memory usage funcs
int XML :: MemoryUsage()
   {
   return GetRootElement()->MemoryUsage() + GetHeader()->MemoryUsage();
   }

void XML :: CompressMemory()
	{
	GetRootElement()->CompressMemory();
	GetHeader()->CompressMemory();
	}

bool XML :: IntegrityTest()
	{
	return (GetHeader()->IntegrityTest() && GetRootElement()->IntegrityTest());
	}

int XML :: Compare(XML*x)
	{
	// 2 XML = equals if headers & root elements compare ok
	int a1 = (GetRootElement()->Compare(x->GetRootElement()));
	int a2 = (GetHeader()->Compare(x->GetHeader()));
	return !(a1 == 0 && a2 == 0);
	}

int XMLHeader :: MemoryUsage()
   {
   int m = 0;

   // Our size
   m += sizeof(*this);

   // Comments
   for(unsigned int i = 0 ; i < commentsnum ; i++)
      {
      m += GetComments()[i]->MemoryUsage();
      }

   // number of comment pointers
   m += TotalCommentPointersAvailable*4;

   // Text
   if (hdr)
      m += strlen(hdr);

   return m;
   }

void XMLHeader :: CompressMemory()
	{
	// Remove wasted space by comments
	int P = commentsnum;
	if (P == 0)
		P = 1;

	XMLComment** oldp = new XMLComment*[P];
	if (commentsnum)
		memcpy(oldp,comments,commentsnum*sizeof(XMLComment*));

	TotalCommentPointersAvailable = P;
	delete[] comments;
	comments = oldp;
	}

bool XMLHeader :: IntegrityTest()
	{
	if (!hdr)
		return false;
#ifdef _WIN32
#ifndef WINCE
	if (IsBadStringPtrA(hdr,-1))
		return false;

	// Comments pointer
	if (IsBadReadPtr(comments,sizeof(XMLComment*)*commentsnum))
		return false;
#endif
#endif

	// Check comment
	for(unsigned int i = 0 ; i < commentsnum ; i++)
		{
#ifdef _WIN32
#ifndef WINCE
		if (IsBadReadPtr(GetComments()[i],sizeof(XMLComment*)))
			return false;
#endif
#endif
		if (!GetComments()[i]->IntegrityTest())
			return false;
		}
	return true;
	}

int XMLHeader :: Compare(XMLHeader* x)
	{
	// 2 Headers compare ok <=> Same text, same # comments, comments compare ok
	if (strcmp(hdr,x->hdr) != 0)
		return 1; // fail header

	unsigned int Y = GetCommentsNum();
	if (Y != x->GetCommentsNum())
		return 1;// differnet comment num

	for(unsigned int i = 0 ; i < Y ; i++)
		{
		if (GetComments()[i]->Compare(x->GetComments()[i]) == 1)
			return 1; // different comment
		}
	return 0; // OK!
	}


int XMLComment :: MemoryUsage()
   {
   int m = 0;

   // Our size
   m += sizeof(*this);

   // Comment size
   if (c)
      m += strlen(c);

   return m;
   }

void XMLComment :: CompressMemory()
	{
	}

bool XMLComment :: IntegrityTest()
	{
	// check parent,c

	if (!c)
		return false;
#ifdef _WIN32
#ifndef WINCE
	if (IsBadStringPtrA(c,-1))
		return false;
#endif
#endif

	if (parent)
		{
		// Check pointer
#ifdef _WIN32
#ifndef WINCE
		if (IsBadReadPtr(parent,sizeof(XMLElement*)))
			return false;
#endif
#endif
		}

	return true;
	}

int XMLComment :: Compare(XMLComment* x)
	{
	// Compare OK <=> Same Text
	if (strcmp(c,x->c) != 0)
		return 1;


	return 0;
	}


XMLComment* XMLComment :: Duplicate()
	{
   // returns a copy of myself
   return new XMLComment(parent,ep,c);
   }


int XMLContent :: MemoryUsage()
   {
   int m = 0;

   // Our size
   m += sizeof(*this);

   // Comment size
   if (c)
	  m += strlen(c);

   return m;
   }

void XMLContent :: CompressMemory()
	{
	}

bool XMLContent :: IntegrityTest()
	{
	// check parent,c

	if (!c)
		return false;
#ifdef _WIN32
#ifndef WINCE
	if (IsBadStringPtrA(c,-1))
		return false;
#endif
#endif

	if (parent)
		{
#ifdef _WIN32
#ifndef WINCE
		if (IsBadReadPtr(parent,sizeof(XMLElement*)))
			return false;
#endif
#endif
		}

	return true;
	}

int XMLContent :: Compare(XMLContent* x)
	{
	// Contents OK <=> Same text
	if (strcmp(c,x->c) != 0)
		return 1;
	return 0;
	}

XMLContent* XMLContent :: Duplicate()
	{
   // returns a copy of myself

   int s2 = GetValue(0);
   Z<char> x2(s2 + 100);
   GetValue(x2);

   return new XMLContent(parent,ep,x2);
   }


int XMLCData :: MemoryUsage()
   {
   int m = 0;

   // Our size
   m += sizeof(*this);

   // Comment size
   if (c)
      m += strlen(c);

   return m;
   }

void XMLCData :: CompressMemory()
	{
	}

bool XMLCData :: IntegrityTest()
	{
	// check parent,c

	if (!c)
		return false;
#ifdef _WIN32
#ifndef WINCE
	if (IsBadStringPtrA(c,-1))
		return false;
#endif
#endif

	if (parent)
		{
		// Check pointer
#ifdef _WIN32
#ifndef WINCE
		if (IsBadReadPtr(parent,sizeof(XMLElement*)))
			return false;
#endif
#endif
		}

	return true;
	}

int XMLCData :: Compare(XMLCData* x)
	{
	// Compare OK <=> Same Text
	if (strcmp(c,x->c) != 0)
		return 1;


	return 0;
	}


XMLCData* XMLCData :: Duplicate()
	{
   // returns a copy of myself
   return new XMLCData(parent,ep,c);
   }





int XMLVariable :: MemoryUsage()
   {
   int m = 0;

   // Our size
   m += sizeof(*this);

   // Variable size
   m += GetName(0);
   m += GetValue(0);

   return m;
   }

void XMLVariable :: CompressMemory()
	{
	}

bool XMLVariable :: IntegrityTest()
	{
	// check vv,vn,owner
	if (!vn || !vv)
		return false;
#ifdef _WIN32
#ifndef WINCE
	if (IsBadStringPtrA(vn,-1))
		return false;
	if (IsBadStringPtrA(vv,-1))
		return false;
#endif
#endif
	if (owner)
		{
#ifdef _WIN32
#ifndef WINCE
		if (IsBadReadPtr(owner,sizeof(XMLElement*)))
			return false;
#endif
#endif
		}
	return true;
	}

int XMLVariable :: Compare(XMLVariable* x)
	{
	// Contents OK <=> Same value & name
	if (strcmp(vn,x->vn) != 0)
		return 1;
	if (strcmp(vv,x->vv) != 0)
		return 1;
	return 0;
	}


int XMLElement :: MemoryUsage()
   {
   int m = 0;
   // returns # of bytes used by this element's data

   // Our size
   m += sizeof(*this);

   // Variables of this
   for(unsigned int i = 0 ; i < variablesnum ; i++)
      {
      m += GetVariables()[i]->MemoryUsage();
      }

   // Comments of this
   for(unsigned int i = 0 ; i < commentsnum ; i++)
	  {
	  m += GetComments()[i]->MemoryUsage();
	  }

   // Contents of this
   for(unsigned int i = 0 ; i < contentsnum ; i++)
	  {
	  m += GetContents()[i]->MemoryUsage();
	  }

   // CDatas of this
   for(unsigned int i = 0 ; i < cdatasnum ; i++)
	  {
	  m += GetCDatas()[i]->MemoryUsage();
	  }

   // Elements of this
   for(unsigned int i = 0 ; i < childrennum ; i++)
      {
		if (GetChildren()[i])
	      m += GetChildren()[i]->MemoryUsage();
      }

   // number of children pointers
   m += TotalChildPointersAvailable*4;

   // number of variable pointers
   m += TotalVariablePointersAvailable*4;

   // number of comment pointers
   m += TotalCommentPointersAvailable*4;

   // number of content pointers
   m += TotalContentPointersAvailable*4;

	// number of cdata pointers
   m += TotalCDataPointersAvailable*4;

	// Element name
   m += GetElementName(0);
   return m;
   }

void XMLElement :: CompressMemory()
	{
	{
	// Remove wasted space by comments
	int PC = commentsnum;
	if (PC == 0)
		PC = 1;

	XMLComment** oldpc = new XMLComment*[PC];
	if (commentsnum)
		memcpy(oldpc,comments,commentsnum*sizeof(XMLComment*));

	TotalCommentPointersAvailable = PC;
	delete[] comments;
	comments = oldpc;
	}

	{
	// Remove wasted space by variables
	int PV = variablesnum;
	if (PV == 0)
		PV = 1;

	XMLVariable** oldpv = new XMLVariable*[PV];
	if (variablesnum)
		memcpy(oldpv,variables,variablesnum*sizeof(XMLVariable*));

	TotalVariablePointersAvailable = PV;
	delete[] variables;
	variables = oldpv;
	}

	{
	// Remove wasted space by children
	int PE = childrennum;
	if (PE == 0)
		PE = 1;

	XMLElement** oldpv = new XMLElement*[PE];
	if (childrennum)
		memcpy(oldpv,children,childrennum*sizeof(XMLElement*));

	TotalChildPointersAvailable = PE;
	delete[] children;
	children = oldpv;
	}

	// Do the same for all Contents
	for(unsigned int i = 0 ; i < contentsnum ; i++)
	  {
	  contents[i]->CompressMemory();
	  }

	// Do the same for all Comments
	for(unsigned int i = 0 ; i < commentsnum ; i++)
	  {
	  comments[i]->CompressMemory();
	  }

	// Do the same for all CDatas
	for(unsigned int i = 0 ; i < cdatasnum ; i++)
	  {
	  cdatas[i]->CompressMemory();
	  }

	// Do the same for all Variables
	for(unsigned int i = 0 ; i < variablesnum ; i++)
	  {
	  variables[i]->CompressMemory();
	  }

	// Do the same for all child elements
	for(unsigned int i = 0 ; i < childrennum ; i++)
		{
		if (children[i])
			children[i]->CompressMemory();
		}

	}

bool XMLElement :: IntegrityTest()
	{
	// The main meat IntegrityTest

	/*

		Check

		name
		parent
		childen
		contents
		variables
		comments

		char* el; // element name
		XMLElement* parent; // one
		XMLElement** children; // many
		XMLVariable** variables; // many
		XMLComment** comments; // many
		XMLContent** contents; // many;

	*/

#ifdef _WIN32
#ifndef WINCE
	// parent pointer
	if (parent && IsBadReadPtr(parent,sizeof(XMLElement*)))
		return false;
	if (IsBadStringPtrA(el,-1))
		return false;
#endif
#endif


#ifdef _WIN32
#ifndef WINCE
	// Comments pointer
	if (IsBadReadPtr(comments,sizeof(XMLComment*)*commentsnum))
		return false;
#endif
#endif

	// Check comment
	for(unsigned int i = 0 ; i < commentsnum ; i++)
		{
#ifdef _WIN32
#ifndef WINCE
		if (IsBadReadPtr(GetComments()[i],sizeof(XMLComment*)))
			return false;
#endif
#endif
		if (!GetComments()[i]->IntegrityTest())
			return false;
		}

#ifdef _WIN32
#ifndef WINCE
	// Contents pointer
	if (IsBadReadPtr(contents,sizeof(XMLContent*)*contentsnum))
		return false;
#endif
#endif

	// Check content
	for(unsigned int i = 0 ; i < contentsnum ; i++)
		{
#ifdef _WIN32
#ifndef WINCE
		if (IsBadReadPtr(GetContents()[i],sizeof(XMLContent*)))
			return false;
#endif
#endif
		if (!GetContents()[i]->IntegrityTest())
			return false;
		}


#ifdef _WIN32
#ifndef WINCE
	// Variables pointer
	if (IsBadReadPtr(variables,sizeof(XMLVariable*)*variablesnum))
		return false;
#endif
#endif

	// Check comment
	for(unsigned int i = 0 ; i < variablesnum ; i++)
		{
#ifdef _WIN32
#ifndef WINCE
		if (IsBadReadPtr(GetVariables()[i],sizeof(XMLVariable*)))
			return false;
#endif
#endif
		if (!GetVariables()[i]->IntegrityTest())
			return false;
		}

#ifdef _WIN32
#ifndef WINCE
	// Children pointer
	if (IsBadReadPtr(children,sizeof(XMLElement*)*childrennum))
		return false;
#endif
#endif

	// Check children
	for(unsigned int i = 0 ; i < childrennum ; i++)
		{
#ifdef _WIN32
#ifndef WINCE
		if (IsBadReadPtr(GetChildren()[i],sizeof(XMLElement*)))
			return false;
#endif
#endif
		if (!GetChildren()[i]->IntegrityTest())
			return false;
		}



	return true;
	}


int XMLElement :: Compare(XMLElement* x)
	{
	/*
		XMLElements match if

			Have same element name

			Have same # of variables,and they match
			Have same # of comments, and they match
			Have same # of contents, and they match
			Have same # of children, and they match
	*/

	// Test element name
	if (strcmp(el,x->el) != 0)
		return 1;

	// Test Variables
	unsigned int nV = GetVariableNum();
	if (nV != x->GetVariableNum())
		return 1;
	for(unsigned int i = 0 ; i < nV ; i++)
		{
		if (GetVariables()[i]->Compare(x->GetVariables()[i]) != 0)
			return 1;
		}

	// Test Comments
	unsigned int nC = GetCommentsNum();
	if (nC != x->GetCommentsNum())
		return 1;
	for(unsigned int i = 0 ; i < nC ; i++)
		{
		if (GetComments()[i]->Compare(x->GetComments()[i]) != 0)
			return 1;
		}

	// Test CDatas
	unsigned int nD = GetCDatasNum();
	if (nD != x->GetCDatasNum())
		return 1;
	for(unsigned int i = 0 ; i < nD ; i++)
		{
		if (GetCDatas()[i]->Compare(x->GetCDatas()[i]) != 0)
			return 1;
		}

	// Test Contents
	unsigned int nT = GetContentsNum();
	if (nT != x->GetContentsNum())
		return 1;
	for(unsigned int i = 0 ; i < nT ; i++)
		{
		if (GetContents()[i]->Compare(x->GetContents()[i]) != 0)
			return 1;
		}

	// Test Children Elements
	unsigned int nE = GetChildrenNum();
	if (nE != x->GetChildrenNum())
		return 1;
	for(unsigned int i = 0 ; i < nE ; i++)
		{
		if (!GetChildren()[i] || !x->GetChildren()[i])
			continue;
		if (GetChildren()[i]->Compare(x->GetChildren()[i]) != 0)
			return 1;
		}

	return 0; // MATCH!
	}


void XMLElement :: Copy()
		{
      // Copies this element to clipboard as a text
#ifdef _WIN32
      int M = MemoryUsage();
      Z<char> d(M);
      Export((FILE*)d.operator char *(),1,1,1);
      int S = strlen(d);

      OpenClipboard(0);
      EmptyClipboard();

      HGLOBAL hG =
         GlobalAlloc(GMEM_MOVEABLE, S + 10);
      void *pp = GlobalLock(hG);
      //lstrcpyA((char *)pp, d.operator char *());
		strcpy((char*)pp,d.operator char *());
      GlobalUnlock(hG);
      SetClipboardData(CF_TEXT, hG);
      CloseClipboard();
#endif
		}

XMLElement* XML :: Paste()
   {
#ifdef _WIN32
      OpenClipboard(0);

      HGLOBAL hG =
         GetClipboardData(CF_TEXT);
      if (!hG)
         {
         CloseClipboard();
         return 0;
         }


      void *pp = GlobalLock(hG);
      int S = strlen((char*)pp);
      Z<char> d(S + 100);
      strcpy(d,(char*)pp);

      GlobalUnlock(hG);
      CloseClipboard();

      // d has the data, size S
      XML* xm = new XML();
      xm->Load(d,1,0,0);
      int K = xm->ParseStatus();
      if (K == 2) // Fatal error
         {
         delete xm;
         return 0;
         }
      XMLElement* r = xm->GetRootElement()->Duplicate(0);
      delete xm;
      return r;
#else
	return 0;
#endif
		}

XMLElement* XMLElement :: Duplicate(XMLElement* par)
	{
   // Creates a new XML element, excact copy of myself
   /*
   	Formula
      dup all variables for this element
      dup all contents  for this element
      dup all comments  for this element
		dup all cdatas    for this element
      dup all elements  in a loop

   */

	ReloadAllElements();

   int z1 = GetElementName(0);
   Z<char> en(z1 + 10);
   GetElementName(en);

   XMLElement* nX = new XMLElement(par,en);

   // Add All Variables
   int y = GetVariableNum();
   for(int i = 0 ; i < y ; i++)
		{
		nX->AddVariable(GetVariables()[i]->Duplicate());
		}
   
   // Add All Contents
   y = GetContentsNum();
   for(int i = 0 ; i < y ; i++)
	  {
	  nX->AddContent(GetContents()[i]->Duplicate(),GetContents()[i]->GetEP());
	  }

   // Add All Comments
   y = GetCommentsNum();
   for(int i = 0 ; i < y ; i++)
	  {
	  nX->AddComment(GetComments()[i]->Duplicate(),GetComments()[i]->GetEP());
	  }

   // Add All Cdatas
   y = GetCDatasNum();
   for(int i = 0 ; i < y ; i++)
	  {
	  nX->AddCData(GetCDatas()[i]->Duplicate(),GetCDatas()[i]->GetEP());
	  }

   // Recurse to add all child elements
   int c = GetChildrenNum();
   for(int i = 0 ; i < c ; i++)
   	{
      nX->AddElement(GetChildren()[i]->Duplicate(nX));
      }

   return nX;
   }

void XML :: SaveOnClose(bool S)
	{
   SOnClose = S;
   }   

int XML :: Save(const char* file,int SaveMode,int TargetMode,class XMLTransform* eclass,class XMLTransformData* edata)
   {
   if (TargetMode == 1)
      {
      if (!file)
         return 0;

      // TargetMode == 1, save to memory buffer
      Export((FILE*)file,SaveMode,1,hdr,eclass,edata);
      return 1;
      }
   if (TargetMode == 2)
      {
      return 0; // We can't save to registry from XML :: Save.
      }



   if (!file)
      file = f;
   if (!file)
      return 0;

   // write this file
   // Header, and all elements
   FILE* fp = fopen(file,"wb");
   if (!fp)
      return 0;

	if (TargetMode == 3)
		{
		// Write BOM
		fwrite("\xFF\xFE",1,2,fp); 

		// Hdr utf-16
		if (hdr)
			hdr->SetEncoding("UTF-16");
		}
	if (TargetMode == 0)
		{
		if (hdr)
			hdr->SetEncoding("UTF-8");
		}


   // Show
   Export(fp,SaveMode,TargetMode,hdr,eclass,edata);

   fclose(fp);
   return 1;
   }

void XMLElement :: SetElementName(const char* x)
   {
   if (el)
      delete[] el;
   el = 0;

   int Sug = XML :: XMLEncode(x,0);
   el = new char[Sug +10];
   memset(el,0,Sug + 10);
   XML :: XMLEncode(x,el);
   }

char * XMLElement :: GetElementName()
{
	return el;
}

int XMLElement :: GetElementName(char* x,bool NoDecode)
	{
	if (!x)
		{
		if (NoDecode)
			return strlen(el);
		else
			return XML :: XMLDecode(el,0);
		}

	if (NoDecode)
		strcpy(x,el);
	else
		XML :: XMLDecode(el,x);
	return strlen(x);
	}

int XMLElement :: GetElementFullName(char* x,bool NoDecode)
	{
	Z<char> fel(5000); // full element name store here
   if (parent == 0) // this is the root element
      {
      return 0;
      }
   else
      {
      parent->GetElementFullName(fel,NoDecode);
      if (strlen(fel))
         strcat(fel,"\\");
      strcat(fel,el);
      }
      
   if (!x)
      {
      if (NoDecode)
         return strlen(fel);
      else
         return XML :: XMLDecode(fel,0);
      }

   if (NoDecode)
      strcpy(x,fel);
   else
      XML :: XMLDecode(fel,x);
   return strlen(x);
   }

int XMLElement :: GetElementUniqueString(char* x)
	{
	int d = GetDeep();
	if (!x)
		return (d*4) + 10;

	if (parent)
		parent->GetElementUniqueString(x);
	
	// strcat to x our position
	if (parent)
		{
		int iid = parent->FindElement(this);
		sprintf(x + strlen(x),"%i-",iid);
		}
	return strlen(x);
	}

int XMLElement :: GetType()
   {
   return type;
   }

int XMLElement :: FindElement(XMLElement* x)
   {
   for(unsigned int i = 0 ; i < childrennum ; i++)
      {
      if (children[i] == x)
         return i;
      }
   return -1;
   }

int XMLElement :: FindElement(char* n)
   {
   for(unsigned int i = 0 ; i < childrennum ; i++)
      {
		if (!children[i])
			continue;
      int Sug = children[i]->GetElementName(0);
      Z<char> Name(Sug + 10);
      children[i]->GetElementName(Name);
      if (strcmp(Name,n) == 0)
         return i;
      }
   return -1;
   }

XMLElement* XMLElement:: FindElementZ(XMLElement* x)
   {
   for(unsigned int i = 0 ; i < childrennum ; i++)
      {
      if (children[i] == x)
         return children[i];
      }
   return 0;
   }

XMLElement* XMLElement:: FindElementZ(char* n,bool ForceCreate,char* el)
   {
   for(unsigned int i = 0 ; i < childrennum ; i++)
      {
		if (!children[i])
			continue;
      int Sug = children[i]->GetElementName(0);
      Z<char> Name(Sug + 10);
      children[i]->GetElementName(Name);
      if (strcmp(Name,n) == 0)
         return children[i];
      }
   if (ForceCreate == 0)
	   return 0;

   // Create New Element and add
   // Force to create a new variable
   XMLElement* vv = new XMLElement(this,el ? el : n,0);
   AddElement(vv);
   return FindElementZ(vv);
   }

int XMLElement :: FindVariable(XMLVariable* x)
   {
   for(unsigned int i = 0 ; i < variablesnum ; i++)
      {
      if (variables[i] == x)
         return i;
      }
   return -1;
   }

XMLVariable* XMLElement :: FindVariableZ(XMLVariable* x)
   {
   for(unsigned int i = 0 ; i < variablesnum ; i++)
      {
      if (variables[i] == x)
         return variables[i];
      }
  return 0;
  }

int XMLElement :: FindVariable(char*  x)
   {
   for(unsigned int i = 0 ; i < variablesnum ; i++)
		{
      int Sug = variables[i]->GetName(0);
      Z<char> Name(Sug + 10);
      variables[i]->GetName(Name);
      if (strcmp(Name,x) == 0)
         return i;
      }
   return -1;
   }

XMLVariable* XMLElement :: FindVariableZ(char*  x,bool ForceCreate,char* defnew)
   {
   for(unsigned int i = 0 ; i < variablesnum ; i++)
      {
      int Sug = variables[i]->GetName(0);
      Z<char> Name(Sug + 10);
      variables[i]->GetName(Name);
      if (strcmp(Name,x) == 0)
         return variables[i];
      }
   if (ForceCreate == 0)
	   return 0;

   // Force to create a new variable
   XMLVariable* vv = new XMLVariable(x,defnew,0);
   AddVariable(vv);
   return FindVariableZ(x,0);
   }

int XML :: PartialLoad(const char* file,const char* map)
	{
	if (!map)
		return 0;
/*
	FILE* fp = fopen(file,"rb");
	if (!fp)
		return 0;

	// Read 1MB portions of the file
	Z<char> bx(1048580);
	char* b = bx;
	int CurrentDeep = 0;
	
	unsigned long PositionStarting = -1;
	unsigned long PositionEnding = -1;

	for(;;)
		{
		int Y = fread(b,1,1048576,fp);
		if (Y == 0)
			break;

		bool IsQuote = false;
		bool IsHdr = false;
		bool IsClosingWithSlash = false;

		// Find < and /> or >
		for(int i = 0 ; i < Y ; i++)
			{
			if (b[i] == '<' && IsQuote == false)
				{
				if (b[i + 1] == '?')
					IsHdr = true;
				else
				if (b[i + 1] == '/')
					{
					IsClosingWithSlash = true;
					continue;
					}
				else
					{
					CurrentDeep++;
					if (CurrentDeep == d->deep && PositionStarting  == -1)
						PositionStarting = i;
					IsHdr = false;
					}
				}

			if (b[i] == '\"')
				IsQuote = !IsQuote;

			if (b[i] == '>' && b[i - 1] == '/')
				{
				CurrentDeep--;
				if (CurrentDeep < d->deep && PositionEnding == -1)
					{
					PositionEnding = i;
					break; // found positions !
					}
				}

			if (b[i] == '>' && IsClosingWithSlash)
				{
				CurrentDeep--;
				if (CurrentDeep < d->deep && PositionEnding == -1)
					{
					PositionEnding = i;
					break;
					}
				IsClosingWithSlash = false;
				}

			if (b[i] == '>' && IsHdr)
				{
				IsHdr = false;
				}

			if (CurrentDeep < 0)
				CurrentDeep = 0;

			}
		}

	// PositionStarting,PositionEnding are found (else error)
	if (PositionStarting == -1 || PositionEnding == -1)
		return 0; // Load failed


	// Time to parse "map"
	// Which is something like "el1\\el2\\el3" ...
	// Named elements to search

*/








  // fclose(fp);
	return 0;// 
	}

XMLElement * XML :: PartialElement(const char* file,const char* map)
	{
	XML x;
	if (x.PartialLoad(file,map) == 1)
		return x.GetRootElement()->Duplicate();
	return 0;
   }


int XML :: Load(const char* file,int LoadMode,XMLTransform* eclass,class XMLTransformData* edata)
	{
   Clear();
   Z<char>* y = 0;
   iParseStatus = 0;

   if (LoadMode == 0) // local xml file
      {
      f = new char[strlen(file) + 1];
      strcpy(f,file);
      // parse this file
      y = ReadToZ(file,eclass,edata);
      if (!y)
         {
         // It is an empty XML file.
         // Create the initial data/header

         hdr = new XMLHeader("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\" ?>");
         root = new XMLElement(0,"root",0);

         return 1;
         }
      }
   else
   if (LoadMode == 1) // memory buffer
      {
      f = 0;
      if (!file || strlen(file) == 0)
         {
         // It is an empty XML file.
         // Create the initial data/header

         hdr = new XMLHeader("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\" ?>");
         root = new XMLElement(0,"root",0);

         return 1;
         }
      else
         {
         int S = strlen(file) + 100;
         y = new Z<char>(S);
         strcpy(y->operator char *(),file);
         }
      }
   else
   if (LoadMode == 2) // url
      {
      f = 0;
#ifdef _WIN32
#ifndef __SYMBIAN32__
#ifdef XML_OPTIONAL_WIN32
      HINTERNET hI = 0,hRead = 0;
      hI = InternetOpen(_T("XML Library"),INTERNET_OPEN_TYPE_PRECONFIG,0,0,0);
      Z<char> fx(1000);
      GetTempFileNameA(".","xml",0,fx);
      if (hI)
	      {
         hRead = InternetOpenUrlA(hI,file,0,0,0,0);
         if (hRead)
            {
            // Get this file
            Z<char> Buff(1010);

            int err = 0;
            unsigned int TotalTransferred = 0;

            for (;;)
               {
               DWORD n;

               bool F = InternetReadFile(hRead,Buff,1000,&n);
          	   if (F == false)
         		 {
                  err = 2;
                  break;
         		 }
               if (n == 0)
                  {
                  // End of file !
                  err = 0;
                  break;
                  }
         	   TotalTransferred += n;

               HANDLE hF = CreateFileA(fx,GENERIC_WRITE,0,0,OPEN_ALWAYS,0,0);
               SetFilePointer(hF,0,0,FILE_END);
               DWORD Actual = 0;
         	   WriteFile(hF,Buff,n,&Actual,0);
               FlushFileBuffers(hF);
               CloseHandle(hF);
               }
            if (err == 0)
               {
               // read that file now
               f = new char[strlen(fx) + 1];
               strcpy(f,fx);
               // parse this file
               y = ReadToZ(fx,eclass,edata);
               }
            InternetCloseHandle(hRead);
            }
         InternetCloseHandle(hI);
         remove(fx);
         if (!y)
            {
            //It is an empty XML file.
            // Create the initial data/header
            hdr = new XMLHeader("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\" ?>");
            root = new XMLElement(0,"root",0);
            return 1;
            }
         }
#endif
#endif
#endif
      }

   // Read file in y and create all XML data
   char* d = (*y).operator char*();

   // read hdr
   char* a2 = 0;
   char c1;
   char* a1 = strstr(d,"?>");
   if (!a1)
      {
      if (f)
         delete[] f;
      f = 0;
      iParseStatus = 1;
      hdr = new XMLHeader("\xEF\xBB\xBF<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\" ?>");
//      root = new XMLElement(0,"root",0);
//      delete y;
//      return 1;
      a1 = d;
      a2 = a1;
      }
   else
      {
      a1 += 2;
      c1 = *a1;
      *a1 = 0;
      hdr = new XMLHeader(d);
      *a1 = c1;
      a2 = a1;
      }

   if (eclass)
      {
      // Delete f if was an encrypted opening
      if (f)
         delete[] f;
      f = 0;
      }

   /*
    Parse each < >
    Possible values

    <!-- -->   comment
    <>         element
    </>        end element
    <? ?>      markup

   */


   a1 = strchr(a2,'<');
   if (a1)
      root = XMLHelper :: ParseElementTree(hdr,0,a1,0,iParseStatus);
   else
      {
      if (f)
         delete[] f;
      f = 0;
      iParseStatus = 1;
      root = new XMLElement(0,"<root>");
      }

   // Print all elements of this

   delete y;
   return 1;
   }

int XML :: ParseStatus(int* v)
   {
   if (v)
      *v = iParseStatusPos;
   return iParseStatus;
   }   

XML :: ~XML()
   {
   Clear();
   }

XML :: XML(XML& xml)
	{
	Clear();
	Init();
	operator =(xml);
	}

XML& XML :: operator =(XML& xml)
	{
	Clear();
	Init();
	hdr = xml.GetHeader()->Duplicate();
	root = xml.GetRootElement()->Duplicate();
	iParseStatus = 0;
	f = 0;
	SOnClose = false;
	return *this;
	}


// XMLElement class
void XMLElement :: Reparse(const char* elm2,int Type)
   {
   RemoveAllVariables();
   RemoveAllElements();
   RemoveAllComments();

	Z<char> elm(strlen(elm2) + 1);
	strcpy(elm,elm2);

   if (Type == 1)
      {
      el = new char[strlen(elm) + 1];
      strcpy(el,elm);
      return;
      }

   el = new char[strlen(elm) + 10];
   memset(el,0,strlen(elm) + 10);

   int x = 0;
   int i = 0;
   for( ; ; i++)
      {
      if (elm[i] == '<')
         continue;
      if (elm[i] == ' ' || elm[i] == '/' ||  elm[i] == '\t' || elm[i] == '>' || elm[i] == '\"' || elm[i] == 0)
         break;
      el[x++] = elm[i];
      }

   int Sug = XML :: XMLEncode(el,0);
   char* ael = new char[Sug + 10];
   memset(ael,0,Sug +10);
   XML :: XMLEncode(el,ael);

   delete[] el;
   el = ael;



   // must be variable ?
	char* a1 = (char*)elm.operator char *() + i;
   for(;;)
      {
      // seek vars
      while(*a1 == ' ' || *a1 == '\t')
         a1++;

      if (*a1 == '>' || *a1 == '/')
         return;

      char* a2 = strchr(a1,'=');
      if (!a2)
         return;

      *a2 = 0;
      char* vvn = a1;
      a1 = a2 + 1;
		// 0x132 fix for white space after =
		while(*a1 != '\"')
			a1++;

      if (*a1 == '\"')
         a1++;
      char* a3 = strchr(a1,'\"');
  		if (!a3)
			   return;
      *a3 = 0;

      XMLVariable* v = new XMLVariable(vvn,a1,true);
      *a2 = '=';
      *a3 = '\"';
      AddVariable(v);
      a1 = a3 + 1;
      }

   }
   
int XMLElement :: GetDeep()
		{
		int d = 0;
		XMLElement* t = this;
		while(t->GetParent() != 0)
			{
			t = t->GetParent();
			d++;
			}
		return d;
		}

XMLElement :: XMLElement(XMLElement* par,const char* elm,int Type)
   {
   // parent
   parent = par;

   // children
   children = new XMLElement*[XML_MAX_INIT_CHILDREN];
   memset(children,0,sizeof(XMLElement*)*XML_MAX_INIT_CHILDREN);
   TotalChildPointersAvailable = XML_MAX_INIT_CHILDREN;
   childrennum = 0;

   // type
   type = Type;

   // variables
   variables = new XMLVariable*[XML_MAX_INIT_VARIABLES];
   memset(variables,0,sizeof(XMLVariable*)*XML_MAX_INIT_VARIABLES);
   TotalVariablePointersAvailable = XML_MAX_INIT_VARIABLES;
   variablesnum = 0;

   // contents
   contents = new XMLContent*[XML_MAX_INIT_CONTENTS];
   memset(contents,0,sizeof(XMLContent*)*XML_MAX_INIT_CONTENTS);
   TotalContentPointersAvailable = XML_MAX_INIT_CONTENTS;
   contentsnum = 0;

   // comments
   comments = new XMLComment*[XML_MAX_INIT_COMMENTS];
   memset(comments,0,sizeof(XMLComment*)*XML_MAX_INIT_COMMENTS);
   TotalCommentPointersAvailable = XML_MAX_INIT_COMMENTS;
   commentsnum = 0;

   // cdatas
   cdatas = new XMLCData*[XML_MAX_INIT_CDATAS];
   memset(cdatas,0,sizeof(XMLCData*)*XML_MAX_INIT_CDATAS);
   TotalCDataPointersAvailable = XML_MAX_INIT_CDATAS;
   cdatasnum = 0;

	// Set default format
	xfformat.nId = 1;
	xfformat.UseSpace = false;
	//xfformat.ElementsNoBreak = false; // DJM3
	xfformat.ElementsNoBreak = true; // DJM3

	// param 0
	param = 0;

   Reparse(elm,Type);
   }

XMLElement :: ~XMLElement()
   {
   RemoveAllVariables();
   RemoveAllElements();
   RemoveAllComments();
   RemoveAllContents();
	RemoveAllCDatas();

   if (variables)
	  delete[] variables;
   variables = 0;
   variablesnum = 0;

   if (children)
	  delete[] children;
   children = 0;
   childrennum = 0;

   if (comments)
	  delete[] comments;
   comments = 0;
   commentsnum = 0;

   if (contents)
	  delete[] contents;
   contents = 0;
   contentsnum = 0;

   if (cdatas)
	  delete[] cdatas;
   cdatas = 0;
   cdatasnum = 0;

	// element
   if (el)
      delete[] el;
   el = 0;
   }



XMLElement* XMLElement :: GetParent()
   {
   return parent;
   }

void XMLElement :: SetParent(XMLElement* Parent)
   {
   parent = Parent;
   }

XMLElement** XMLElement :: GetChildren()
   {
   return children;
	}

XMLElement* XMLElement :: operator [](int i)
	{
	return GetChildren()[i];
	}


unsigned int XMLElement :: GetChildrenNum()
   {
   return childrennum;
   }

XMLVariable** XMLElement :: GetVariables()
   {
   return variables;
   }

unsigned int XMLElement :: GetVariableNum()
   {
   return variablesnum;
   }

int XMLElement :: SpaceForElement(unsigned int i)
   {
   if ((TotalChildPointersAvailable - childrennum) >= i)
      return (TotalChildPointersAvailable - childrennum);

   // make more space
   Z<XMLElement*> oldp(childrennum);
   memcpy(oldp,children,childrennum*sizeof(XMLElement*));

   TotalChildPointersAvailable += XML_MAX_INIT_CHILDREN;

   delete[] children;
   children = new XMLElement*[TotalChildPointersAvailable];
   memcpy(children,oldp,childrennum*sizeof(XMLElement*));
   return (TotalChildPointersAvailable - childrennum);
   }

int XMLElement :: SpaceForVariable(unsigned int i)
   {
   if ((TotalVariablePointersAvailable - variablesnum) >= i)
      return (TotalVariablePointersAvailable - variablesnum);

   Z<XMLVariable*> oldp(variablesnum);
   memcpy(oldp,variables,variablesnum*sizeof(XMLVariable*));

   TotalVariablePointersAvailable += XML_MAX_INIT_VARIABLES;

   delete[] variables;
   variables = new XMLVariable*[TotalVariablePointersAvailable];
   memcpy(variables,oldp,variablesnum*sizeof(XMLVariable*));
   return (TotalVariablePointersAvailable - variablesnum);
   }

int XMLElement :: SpaceForComment(unsigned int i)
   {
   if ((TotalCommentPointersAvailable - commentsnum) >= i)
      return (TotalCommentPointersAvailable - commentsnum);

   Z<XMLComment*> oldp(commentsnum);
   memcpy(oldp,comments,commentsnum*sizeof(XMLComment*));

   TotalCommentPointersAvailable += XML_MAX_INIT_COMMENTS;

   delete[] comments;
   comments = new XMLComment*[TotalCommentPointersAvailable];
   memcpy(comments,oldp,commentsnum*sizeof(XMLComment*));
   return (TotalCommentPointersAvailable - commentsnum);
   }

int XMLHeader :: SpaceForComment(unsigned int i)
   {
   if ((TotalCommentPointersAvailable - commentsnum) >= i)
      return (TotalCommentPointersAvailable - commentsnum);

   Z<XMLComment*> oldp(commentsnum);
   memcpy(oldp,comments,commentsnum*sizeof(XMLComment*));

   TotalCommentPointersAvailable += XML_MAX_INIT_COMMENTS;

   delete[] comments;
   comments = new XMLComment*[TotalCommentPointersAvailable];
   memcpy(comments,oldp,commentsnum*sizeof(XMLComment*));
   return (TotalCommentPointersAvailable - commentsnum);
   }


int XMLElement :: SpaceForCData(unsigned int i)
   {
   if ((TotalCDataPointersAvailable - cdatasnum) >= i)
      return (TotalCDataPointersAvailable - commentsnum);

   Z<XMLCData*> oldp(cdatasnum);
   memcpy(oldp,cdatas,cdatasnum*sizeof(XMLCData*));

   TotalCDataPointersAvailable += XML_MAX_INIT_CDATAS;

   delete[] cdatas;
   cdatas = new XMLCData*[TotalCDataPointersAvailable];
   memcpy(cdatas,oldp,cdatasnum*sizeof(XMLCData*));
   return (TotalCDataPointersAvailable - cdatasnum);
   }


int XMLElement :: SpaceForContent(unsigned int i)
   {
   if ((TotalContentPointersAvailable - contentsnum) >= i)
	  return (TotalContentPointersAvailable - contentsnum);

   Z<char*> oldp(contentsnum);
   memcpy(oldp,contents,contentsnum*sizeof(char*));

   TotalContentPointersAvailable += XML_MAX_INIT_CONTENTS;

   delete[] contents;
   contents = new XMLContent*[TotalContentPointersAvailable];
   memcpy(contents,oldp,contentsnum*sizeof(XMLContent*));
   return (TotalContentPointersAvailable - contentsnum);
   }


int XMLElement :: AddElement(XMLElement* child)
   {
   SpaceForElement(1);
   children[childrennum++] = child;
   child->SetParent(this);
   return childrennum;
   }

int XMLElement :: AddElement(char* t)
	{
	XMLElement* x = new XMLElement(this,t);
	return AddElement(x);
	}

int XMLElement :: AddVariable(XMLVariable* v)
   {
   SpaceForVariable(1);
   variables[variablesnum++] = v;
   v->SetOwnerElement(this);
   return variablesnum;
   }

int XMLElement :: AddVariable(char* vn,char* vv)
	{
	XMLVariable* x = new XMLVariable(vn,vv);
	return AddVariable(x);
	}

#ifdef XML_OPTIONAL_MIME
int XMLElement :: AddBinaryVariable(char* vn,char* vv,int S)
	{
	XMLVariable* x = new XMLVariable(vn,"");
	x->SetBinaryValue(vv,S);
	return AddVariable(x);
	}
#endif

int XMLElement :: AddComment(XMLComment* v,int InsertBeforeElement)
   {
   SpaceForComment(1);
   comments[commentsnum++] = v;
   v->SetParent(this,InsertBeforeElement);
   return commentsnum;
   }

int XMLElement :: AddComment(char*t,int InsertBeforeElement)
	{
	XMLComment* x = new XMLComment(this,InsertBeforeElement,t);
	return AddComment(x,InsertBeforeElement);
	}


unsigned int XMLElement :: GetCommentsNum()
   {
   return commentsnum;
   }

XMLComment** XMLElement :: GetComments()
   {
   return comments;
   }


int XMLElement :: RemoveAllComments()
   {
   for(int i = commentsnum - 1 ; i >= 0 ; i--)
      {
      delete comments[i];
      comments[i] = 0;
      }
   commentsnum = 0;
   return 0;
   }


int XMLElement :: RemoveComment(unsigned int i)
   {
   if (i >= commentsnum)
      return commentsnum;

   delete comments[i];
   comments[i] = 0;

   for(unsigned int k = i ; k < commentsnum ; k++)
      comments[k] = comments[k + 1];

   comments[commentsnum - 1] = 0;
   return --commentsnum;
   }


int XMLElement :: AddCData(XMLCData* v,int InsertBeforeElement)
   {
   SpaceForCData(1);
   cdatas[cdatasnum++] = v;
   v->SetParent(this,InsertBeforeElement);
   return cdatasnum;
   }

int XMLElement :: AddCData(char*t,int InsertBeforeElement)
	{
	XMLCData* x = new XMLCData(this,InsertBeforeElement,t);
	return AddCData(x,InsertBeforeElement);
	}


unsigned int XMLElement :: GetCDatasNum()
   {
   return cdatasnum;
   }

XMLCData** XMLElement :: GetCDatas()
   {
   return cdatas;
   }


int XMLElement :: RemoveAllCDatas()
   {
   for(int i = cdatasnum - 1 ; i >= 0 ; i--)
      {
      delete cdatas[i];
      cdatas[i] = 0;
      }
   cdatasnum = 0;
   return 0;
   }


int XMLElement :: RemoveCData(unsigned int i)
   {
   if (i >= cdatasnum)
      return cdatasnum;

   delete cdatas[i];
   cdatas[i] = 0;

   for(unsigned int k = i ; k < cdatasnum ; k++)
      cdatas[k] = cdatas[k + 1];

   cdatas[cdatasnum - 1] = 0;
   return --cdatasnum;
   }


// Content class
int XMLElement :: AddContent(XMLContent* v,int InsertBeforeElement)
	{
	SpaceForContent(1);
	contents[contentsnum++] = v;
	v->SetParent(this,InsertBeforeElement);
	return contentsnum;
	}

int XMLElement :: AddContent(char* t,int InsertBeforeElement)
	{
	XMLContent* x = new XMLContent(this,InsertBeforeElement,t);
	return AddContent(x,InsertBeforeElement);
	}

int XMLElement :: RemoveContent(unsigned int i)
	{
	if (i >= contentsnum)
	  return contentsnum;

    delete contents[i];
	contents[i] = 0;

	for(unsigned int k = i ; k < contentsnum ; k++)
	  contents[k] = contents[k + 1];

	contents[contentsnum - 1] = 0;
	return --contentsnum;
	}

void XMLElement :: RemoveAllContents()
	{
   for(int i = contentsnum - 1 ; i >= 0 ; i--)
	  {
	  delete contents[i];
	  contents[i] = 0;
	  }
	contentsnum = 0;
	}

XMLContent** XMLElement :: GetContents()
	{
	return contents;
	}

unsigned int XMLElement :: GetContentsNum()
	{
    return contentsnum;
	}


unsigned int XMLElement :: GetAllChildren(XMLElement** x,unsigned int deep)
   {
   int C = 0;

   for(unsigned int i = 0 ; i < childrennum && deep != 0 ; i++)
      {
		if (!children[i])
			continue;
      C += children[i]->GetAllChildren(x + C,deep == 0xFFFFFFFF ? deep : (deep - 1));
      x[C++] = children[i];
      }

   return C;
   }

unsigned int XMLElement :: GetAllChildrenNum(unsigned int deep)
   {
   int C = 0;
   for(unsigned int i = 0 ; i < childrennum && deep != 0 ; i++)
      {
		if (!children[i])
			continue;
      C += children[i]->GetAllChildrenNum(deep == 0xFFFFFFFF ? deep : (deep - 1));
      }
   C += childrennum;
   return C;
   }


// XMLComment class
XMLComment :: XMLComment(XMLElement* p,int ElementPosition,const char* ht)
	{
	parent = p;
	ep = ElementPosition;
	c = new char[strlen(ht) + 1];
	strcpy(c,ht);
	}

XMLComment :: XMLComment(XMLComment& h)
	{
	 operator =(h);
	}

XMLComment& XMLComment :: operator =(XMLComment& h)
	{
	if (c)
		delete[] c;
	c = 0;

	parent = h.parent;
	ep = h.GetEP();
	char* ht = h.operator char*();
	c = new char[strlen(ht) + 1];
	strcpy(c,ht);
	return *this;
	}


XMLComment :: ~XMLComment()
   {
   if (c)
      delete[] c;
   c = 0;
   }

XMLComment :: operator char*()
   {
   return c;
   }


void XMLComment :: SetComment(const char* ht)
   {
   if (c)
      delete[] c;
   c = new char[strlen(ht) + 1];
   strcpy(c,ht);
   }

void XMLComment :: SetParent(XMLElement* p,int epp)
   {
   parent = p;
   ep = epp;
   }

int XMLComment :: GetEP()
   {
   return ep;
   }


// XMLCData class
XMLCData :: XMLCData(XMLElement* p,int ElementPosition,const char* ht)
	{
	parent = p;
	ep = ElementPosition;
	c = new char[strlen(ht) + 1];
	strcpy(c,ht);
	}

XMLCData :: XMLCData(XMLCData& h)
	{
	 operator =(h);
	}

XMLCData& XMLCData :: operator =(XMLCData& h)
	{
	if (c)
		delete[] c;
	c = 0;

	parent = h.parent;
	ep = h.GetEP();
	char* ht = h.operator char*();
	c = new char[strlen(ht) + 1];
	strcpy(c,ht);
	return *this;
	}


XMLCData :: ~XMLCData()
   {
   if (c)
      delete[] c;
   c = 0;
   }

XMLCData :: operator char*()
   {
   return c;
   }


void XMLCData :: SetCData(const char* ht)
   {
   if (c)
      delete[] c;
   c = new char[strlen(ht) + 1];
   strcpy(c,ht);
   }

void XMLCData :: SetParent(XMLElement* p,int epp)
   {
   parent = p;
   ep = epp;
   }

int XMLCData :: GetEP()
   {
   return ep;
   }


// XMLContent class
XMLContent :: XMLContent(XMLElement* p,int ElementPosition,const char* ht,bool NoDecode)
   {
   parent = p;
   ep = ElementPosition;
   c = 0;
   SetValue(ht,NoDecode);
   }

XMLContent :: XMLContent(XMLContent& h)
	{
	 operator =(h);
	}

XMLContent& XMLContent :: operator =(XMLContent& h)
	{
	if (c)
	  delete[] c;
	c = 0;

	parent = h.parent;
	ep = h.GetEP();

	int k = h.GetValue(0,true);
	Z<char> vv(k + 10);
	h.GetValue(vv,true);
	SetValue(vv,true);
	return *this;
	}


XMLContent :: ~XMLContent()
   {
   if (c)
	  delete[] c;
   c = 0;
   }

int XMLContent :: GetValue(char* x,bool NoDecode)
	{
	if (!x)
	  {
	  if (NoDecode)
		 return strlen(c);
	  else
		 return XML :: XMLDecode(c,0);
	  }


   if (NoDecode)
	  strcpy(x,c);
   else
	  XML :: XMLDecode(c,x);
   return strlen(x);
	}


void XMLContent :: SetValue(const char* VV,bool NoDecode)
   {
   if (c)
	  delete[] c;

   int Sug = XML :: XMLEncode(VV,0);

   c = new char[Sug + 10];
   memset(c,0,Sug + 10);

   if (NoDecode)
	  strcpy(c,VV);
   else
	  XML :: XMLEncode(VV,c);
   }

void XMLContent :: SetParent(XMLElement* p,int epp)
   {
   parent = p;
   ep = epp;
   }

int XMLContent :: GetEP()
   {
   return ep;
   }



// XMLHeader class
XMLHeader :: XMLHeader(const char* ht)
   {
   hdr = new char[strlen(ht) + 1];
   strcpy(hdr,ht);

   // comments
   comments = new XMLComment*[XML_MAX_INIT_COMMENTS_HEADER];
   memset(comments,0,sizeof(XMLComment*)*XML_MAX_INIT_COMMENTS_HEADER);
   TotalCommentPointersAvailable = XML_MAX_INIT_COMMENTS_HEADER;
   commentsnum = 0;
   }
   
XMLHeader :: XMLHeader(XMLHeader& h)
	{
	 operator =(h);
	}

void XMLHeader :: Clear()
	{
	if (hdr)
	  delete[] hdr;
	hdr = 0;

	RemoveAllComments();
	if (comments)
	  delete[] comments;
	comments = 0;
    commentsnum = 0;
	}

XMLHeader& XMLHeader :: operator =(XMLHeader& h)
	{
	Clear();

	char*ht = h.operator char*();
	hdr = new char[strlen(ht) + 1];
	strcpy(hdr,ht);

	// comments
	comments = new XMLComment*[XML_MAX_INIT_COMMENTS_HEADER];
	memset(comments,0,sizeof(XMLComment*)*XML_MAX_INIT_COMMENTS_HEADER);
	TotalCommentPointersAvailable = XML_MAX_INIT_COMMENTS_HEADER;
	commentsnum = 0;

	// Add comments from h
	int yC = h.GetCommentsNum();
	for(int i = 0 ; i < yC ; i++)
		{
		AddComment(h.GetComments()[i]->Duplicate(),h.GetComments()[i]->GetEP());
		}

	return *this;
	}
  

XMLHeader :: ~XMLHeader()
   {
   Clear();
   }

XMLHeader :: operator char*()
   {
   return hdr;
   }   

void XMLHeader :: SetEncoding(char* e)
	{
	// 
	Z<char> nt(1000);
	sprintf(nt,"<?xml version=\"1.0\" encoding=\"%s\" standalone=\"yes\" ?>",e);
	int sl = strlen(nt) + 1;
	delete[] hdr;
	hdr = new char[sl];
	memset(hdr,0,sl);
	strcpy(hdr,nt);
	}

XMLHeader* XMLHeader :: Duplicate()
	{
	XMLHeader* hz =  new XMLHeader(hdr);
	
   // Add All Comments
   int y = GetCommentsNum();
   for(int i = 0 ; i < y ; i++)
	  {
	  hz->AddComment(GetComments()[i]->Duplicate(),GetComments()[i]->GetEP());
	  }

	return hz;
	}

int XMLHeader :: AddComment(XMLComment* v,int pos)
   {
   SpaceForComment(1);
   comments[commentsnum++] = v;
   v->SetParent(0,pos);
   return commentsnum;
   }



unsigned int XMLHeader :: GetCommentsNum()
   {
   return commentsnum;
   }

XMLComment** XMLHeader :: GetComments()
   {
   return comments;
   }


int XMLHeader :: RemoveAllComments()
   {
   for(int i = commentsnum - 1 ; i >= 0 ; i--)
      {
      delete comments[i];
      comments[i] = 0;
      }
   commentsnum = 0;
   return 0;
   }


int XMLHeader :: RemoveComment(unsigned int i)
   {
   if (i >= commentsnum)
      return commentsnum;

   delete comments[i];
   comments[i] = 0;

   for(unsigned int k = i ; k < commentsnum ; k++)
      comments[k] = comments[k + 1];

   comments[commentsnum - 1] = 0;
   return --commentsnum;
   }


void XMLHeader :: Export(FILE* fp,int SaveMode,int TargetMode,class XMLTransform* eclass,class XMLTransformData* edata)
   {
   if (TargetMode == 1)
      {
      if (SaveMode == 0)
         sprintf((char*)fp,"%s\r\n",hdr);
	  for(unsigned int i = 0 ; i < commentsnum ; i++)
         {
         if (comments[i]->GetEP() == SaveMode)
            sprintf((char*)fp,"<!--%s-->\r\n",comments[i]->operator char *());
         }
      return;
      }

   if (TargetMode == 2)
      {
      return; //*

      }

   if (TargetMode == 0) // UTF-8
		{
	   if (SaveMode == 0)
			fprintf(fp,"%s\r\n",hdr);
		for(unsigned int i = 0 ; i < commentsnum ; i++)
	      {
			if (comments[i]->GetEP() == SaveMode)
	         fprintf(fp,"<!--%s-->\r\n",comments[i]->operator char *());
			}
		}
	if (TargetMode == 3) // UTF-16
		{
	   if (SaveMode == 0)
			{
			Z<char> txt(strlen(hdr)*2 + 100);
			sprintf(txt,"%s\r\n",hdr);
			XMLElement :: Write16String(fp,txt);
			}
			
		for(unsigned int i = 0 ; i < commentsnum ; i++)
	      {
			if (comments[i]->GetEP() == SaveMode)
				{
				char* c = comments[i]->operator char *();
				Z<char> txt(strlen(c)*2 + 100);
	         sprintf(txt,"<!--%s-->\r\n",c);
				XMLElement :: Write16String(fp,txt);
				}
			}
		}
   }



// XMLVariable class
void XMLVariable :: SetName(const char* VN,bool NoDecode)
   {
   if (vn)
      delete[] vn;

   int Sug = XML :: XMLEncode(VN,0);

   vn = new char[Sug + 10];
   memset(vn,0,Sug + 10);
   if (NoDecode)
      strcpy(vn,VN);
   else
      XML :: XMLEncode(VN,vn);
	
	// 0x132 fix for white space at the end of the variable
	while(vn[strlen(vn) - 1] == ' ')
		vn[strlen(vn) - 1] = 0;
   }

void XMLVariable :: SetValue(const char* VV,bool NoDecode)
   {
   if (vv)
      delete[] vv;

   int Sug = XML :: XMLEncode(VV,0);

   vv = new char[Sug + 10];
   memset(vv,0,Sug + 10);

   if (NoDecode)
      strcpy(vv,VV);
   else
      XML :: XMLEncode(VV,vv);
   }

XMLVariable :: XMLVariable(const char* VN,const char* VV,bool NoDecode)
   {
   vn = 0;
   vv = 0;
   owner = 0;
   SetName(VN,NoDecode);
   SetValue(VV,NoDecode);
   }

XMLElement* XMLVariable :: SetOwnerElement(XMLElement* o)
   {
   XMLElement* oldowner = owner;
   owner = o;
   return oldowner;
   }

void XMLVariable :: Clear()
	{
	if (vn)
	  delete[] vn;
	vn = 0;

	if (vv)
	  delete[] vv;
	vv = 0;
	}

XMLVariable :: ~XMLVariable()
   {
   Clear();
   }

XMLVariable :: XMLVariable(XMLVariable& h)
	{
	 operator =(h);
	}

XMLVariable& XMLVariable :: operator =(XMLVariable& h)
	{
	Clear();

	owner = h.owner;

	int n = h.GetName(0,true);
	Z<char> nn(n + 10);
	h.GetName(nn,true);
	SetName(nn,true);


	int k = h.GetValue(0,true);
	Z<char> vv(k + 10);
	h.GetValue(vv,true);
	SetValue(vv,true);
	return *this;
	}


int XMLVariable :: GetName(char* x,bool NoDecode)
   {
   if (!x)
      {
      if (NoDecode)
         return strlen(vn);
      else
         return XML :: XMLDecode(vn,0);
      }
      
   if (NoDecode)
      strcpy(x,vn);
   else
      XML :: XMLDecode(vn,x);
   return strlen(x);
   }

int XMLVariable :: GetValue(char* x,bool NoDecode)
   {
   if (!x)
      {
      if (NoDecode)
         return strlen(vv);
      else
         return XML :: XMLDecode(vv,0);
      }


   if (NoDecode)
      strcpy(x,vv);
   else
      XML :: XMLDecode(vv,x);
   return strlen(x);
   }

//BittWare additions
char * XMLVariable :: GetValueStr()
{
	return vv;
}
char * XMLVariable :: GetNameStr()
{
	return vn;
}


int XMLVariable :: GetValueInt()
	{
	int p = GetValue(0);
	Z<char> d(p + 10);
	GetValue(d);
	return atoi(d);
	}

void XMLVariable :: SetValueInt(int V)
	{
	char t[50] = {0};
	sprintf(t,"%u",V);
	SetValue(t);
	}

void XMLVariable :: Copy()
	{
	}

XMLVariable* XMLVariable :: Duplicate()
	{
   // returns a copy of myself
   int s1 = GetName(0);
   int s2 = GetValue(0);
   Z<char> x1(s1 + 100);
   Z<char> x2(s2 + 100);
   GetName(x1);
   GetValue(x2);

   return new XMLVariable(x1,x2,0);
   }

	
#ifdef XML_OPTIONAL_MIME
int XMLVariable :: SetBinaryValue(char* data,int len)
	{
	// Sets value using MIME
	char* a = new char[len*5 + 1000];
	memset(a,0,len*5 + 1000);

	char *oo;
	MimeCoder<char*,char*>* e;
	e = new Base64Encoder<char*,char*>;
	oo = e->Filter(a, data, data + len);
	oo = e->Finish(oo);
	*oo = 0;                  // Put a zero to the end
	delete e;

	int I = strlen(a);
	SetValue(a,false);

	delete[] a;
	return I;
	}

int XMLVariable :: GetBinaryValue(char* data)
	{
	int aL = GetValue(0);
	if (!aL)
		return 0;

	Z<char> bdata(aL + 100);
	GetValue(bdata);

	Z<char> oout(aL + 1000);

	char *oo;
	MimeCoder<char*,char*>* e;
	e = new Base64Decoder<char*,char*>;
	oo = e->Filter(oout, bdata.operator char*(), bdata.operator char*() + aL);
	oo = e->Finish(oo);
	*oo = 0;                  // Put a zero to the end
	delete e;

	int S = oo - oout.operator char*();
	if (!data)
		{
		return S;
		}
	else
		{
		memcpy(data,oout,S);
		return S;
		}
	}

#endif


bool XML :: TestMatch(const char* item1,const char* comp,const char* item2)
   {
   Z<char> iitem2(1000);
   if (item2[0] == '\"')
      {
      strcpy(iitem2,item2 + 1);
      if (strlen(iitem2))
      	iitem2[strlen(iitem2) - 1] = 0;

      if (strcmp(comp,"==") == 0)
         return VMatching((char*)item1,(char*)iitem2);
      if (strcmp(comp,"!=") == 0)
         return !VMatching((char*)item1,(char*)iitem2);
      }
   else // integer
      {
		// Fix: Check if items are integers
		if (atoi(item1) == 0 && item1[0] != '0')
			return false;
		if (atoi(item2) == 0 && item2[0] != '0')
			return false;


      if (strcmp(comp,"==") == 0)
         return (atoi(item1) == atoi(item2));

      if (strcmp(comp,"!=") == 0)
         return (atoi(item1) != atoi(item2));

      if (strcmp(comp,">=") == 0)
		 return (atoi(item1) >= atoi(item2));

      if (strcmp(comp,"<=") == 0)
         return (atoi(item1) <= atoi(item2));

      if (strcmp(comp,"<") == 0)
         return (atoi(item1) < atoi(item2));

      if (strcmp(comp,">") == 0)
         return (atoi(item1) > atoi(item2));
      }


   return true;
   }

int XML :: XMLQuery(const char* rootsection,const char* expression,XMLElement** rv,unsigned int deep)
   {
   XMLElement* r = root->GetElementInSection(rootsection);
   if (!r)
      return 0;
   return r->XMLQuery(expression,rv,deep);
   }

int XMLElement :: XMLQuery(const char* expression2,XMLElement** rv,unsigned int deep)
   {
	Z<char> expression(strlen(expression2) + 1);
	strcpy(expression,expression2);
   // Executes query based on expression of variables
   /*

      Valid expressions

      <item1> <comparator> <item2>[<expr> ...]

      <item1> is a variable name, or * if any variable can match or ? if it is to match
              the element name or ! if it is to match the full element name 
				  or ~ to match the content name

      <item2> can be either integers, or strings, or strings with ? and *
      <comparator> can be
            ==
            !=
            >=
            >
            <
            <=

      MUST HAVE SPACES
      EXAMPLES

      v == "xa*"
      a >= 5


   */

   XMLElement* r = this;

   // r is the base section
   // get all its elements
   int C = r->GetAllChildrenNum(deep);
   Z<XMLElement*> allelements(C + 1);
   r->GetAllChildren(allelements,deep);
   Z<int> positives(C);
   for(int i = 0 ; i < C ; i++)
      positives[i] = 1;

	char* a = expression.operator char *();
   for( ;; )
      {
      // Parse the expression

      // Get item 1
      char* a1 = strchr(a,' ');
      if (!a1)
         break;
      Z<char> item1(300);
      *a1 = 0;
      strcpy(item1,a);
      *a1 = ' ';
      a = a1 + 1;

      // Get comparator
      a1 = strchr(a,' ');
      if (!a1)
         break;
      Z<char> comp(100);
      *a1 = 0;
      strcpy(comp,a);
      *a1 = ' ';
      a = a1 + 1;

      // Get item 2
		if (*a == '\"')
			{
			a1 = strchr(a + 1,'\"');
			if (a1)
				a1++;
			}
		else
	      a1 = strchr(a,' ');
      Z<char> item2(300);
      if (a1)
         *a1 = 0;
      strcpy(item2,a);
      if (a1)
         {
         *a1 = ' ';
         a = a1 + 1;
         }


      // Test this expression against all elements
      for(int y = 0 ; y < C ; y++)
         {
         Z<char> ItemToMatch(1000);

         if (strcmp(item1,"?") == 0)
            {
            allelements[y]->GetElementName(ItemToMatch,0);
            if (XML :: TestMatch(ItemToMatch,comp,item2) == 0)
               positives[y] = 0;
            }
         else
         if (strcmp(item1,"!") == 0)
            {
            allelements[y]->GetElementFullName(ItemToMatch,0);
            if (XML :: TestMatch(ItemToMatch,comp,item2) == 0)
               positives[y] = 0;
            }
         else
		 if (strncmp(item1,"~",1) == 0)
			{
			unsigned int iC = atoi(item1.operator char*() + 1);
			if (allelements[y]->GetContentsNum() > iC)
				allelements[y]->GetContents()[iC]->GetValue(ItemToMatch);

			if (XML :: TestMatch(ItemToMatch,comp,item2) == 0)
			   positives[y] = 0;
			}
		 else
			{
			int V = allelements[y]->FindVariable(item1);
			if (V == -1)
			   strcpy(ItemToMatch,"");
			else
			   allelements[y]->GetVariables()[V]->GetValue(ItemToMatch,0);
			if (XML :: TestMatch(ItemToMatch,comp,item2) == 0)
			   positives[y] = 0;
			}
         }

      // Testing finished.
      if (!a1)
         break;


      }

   int N = 0;
   for(int i = 0 ; i < C ; i++)
      {
      if (positives[i])
         {
		   if (rv)
	         rv[N] = allelements[i];
         N++;
         }
      }
   return N;
   }



// Global functions
Z<char>* XML :: ReadToZ(const char* file,XMLTransform* eclass,class XMLTransformData* edata)
   {
   FILE* fp = fopen(file,"rb");
   if (!fp)
      return 0;

   fseek(fp,0,SEEK_END);
   int S = ftell(fp);
   fseek(fp,0,SEEK_SET);

	Z<char>* y = 0;
   if (eclass == 0)
      {
      y = new Z<char>(S + 32);
      fread((*y).operator char *(),1,S,fp);
      fclose(fp);
      }
   else
      {
      Z<char> yy(S + 32);
      y = new Z<char>(S + 32);
      fread(yy.operator char *(),1,S,fp);
      fclose(fp);

      //eclass->Prepare(edata);
      eclass->Decrypt(yy.operator char *(),S,0,(*y).operator char *(),S,0);
      }

	// Check if this XML we loaded is a unicode XML
	// In such case, we must convert to UTF-8
#ifdef _WIN32
	unsigned char* yy = (unsigned char*)(*y).operator char *();
	if (yy[0] == 0xFF && yy[1] == 0xFE)
		{
		// Whops, unicode XML file loaded
		wchar_t* wd = (wchar_t*)(*y).operator char *();


      Z<char>* nyy = new Z<char>(S*4 + 32);
		WideCharToMultiByte(CP_UTF8,0,wd,-1,(*nyy),S*4 + 32,0,0);
		delete y;
		y = nyy;
		}
#endif

	return y;
   }


int XMLGetString(const char* section,const char* Tattr,const char* defv,char*out,const int maxlen,const char* xml,XML* af)
   {
   int Z = 0;
   if (!af)
   	{
   	XML f(xml);
   	Z = f.XMLGetValue((char*)section,(char*)Tattr,out,maxlen);
   	}
   else
   	{
   	Z = af->XMLGetValue((char*)section,(char*)Tattr,out,maxlen);
   	}

   if (Z)
      return Z;
   strcpy(out,defv);
   return strlen(defv);
   }

int XMLSetString(const char* section,const char* Tattr,char*out,const char* xml,XML* af)
   {
   if (!af)
   	{
   	XML f(xml);
	   f.XMLSetValue((char*)section,(char*)Tattr,out);
      f.Save();
		}
	else
		{
	   af->XMLSetValue((char*)section,(char*)Tattr,out);
		}
   return 1;
   }

#ifndef __SYMBIAN32__
#ifdef _WIN32
int XMLSetString(const char* section,const char* Tattr,wchar_t*out,const char* xml,XML* af)
   {
   // Convert to UTF-8
   int S = wcslen(out);
   Z<char> ut(S*2 + 1000);
   WideCharToMultiByte(CP_UTF8,0,out,-1,ut,S*2 + 1000,0,0);
   return XMLSetString(section,Tattr,ut,xml,af);
   }
#endif
#endif



int XMLRenameElement(const char* section,const char* newname,const char* xml,XML* af)
   {
   bool C = false;
   if (!af)
      {
      C = true;
      af = new XML(xml);
      }

   XMLElement* r = af->GetRootElement();
   XMLElement* e = r->GetElementInSection(section);
   if (!e)
      return 0; // no items under this one

   e->SetElementName(newname);

   if (C)
      {
      delete af;
      af = 0;
      }
   return 1;
   }


int    XMLGetInt(const char* item,const char* attr,const int defv,const char* xml,XML* af)
   {
   Z<char> i(100);
   Z<char> id(100);
   sprintf(id,"%u",defv);
   XMLGetString(item,attr,id,i,100,xml,af);
   return atoi(i);
   }

#ifdef _WIN32
__int64    XMLGetInt64(const char* item,const char* attr,const __int64 defv,const char* xml,XML* af)
   {
   Z<char> i(100);
   Z<char> id(100);
   sprintf(id,"%Lu",defv);
   XMLGetString(item,attr,id,i,100,xml,af);
   return _atoi64(i);
   }
#endif

float XMLGetFloat(const char* item,const char* attr,const float defv,const char* xml,XML* af)
   {
   Z<char> a1(30);
   sprintf(a1,"%f",defv);

   Z<char> a2(30);
   XMLGetString(item,attr,a1,a2,30,xml,af);

   return atof(a2);
   }

#ifdef XML_OPTIONAL_MIME
int XMLGetBinaryData(const char* item,const char* attr,const char* defv,char*out,const int maxlen,const char* xml,XML* af)
   {
   Z<char> bdata(maxlen*5 + 1000);
   int len = XMLGetString(item,attr,defv,bdata,maxlen*5 + 1000,xml,af);

   if (!len)
   		return 0;

   Z<char> oout(5000);

   char *oo;
   MimeCoder<char*,char*>* e;
   e = new Base64Decoder<char*,char*>;
   oo = e->Filter(oout, bdata.operator char*(), bdata.operator char*() + len);
   oo = e->Finish(oo);
   *oo = 0;                  // Put a zero to the end
   delete e;

   int S = oo - oout.operator char*();
   if (S > maxlen)
      {
      memcpy(out,oout,maxlen);
      return maxlen;
      }
   else
      {
	  memcpy(out,oout,S);
	  return S;
	  }
   }
#endif

int    XMLSetInt(char* section,char* attr,int v,char* xml,XML* af)
	{
   char a[20] = {0};
   sprintf(a,"%u",v);
   return XMLSetString(section,attr,a,xml,af);
   }

#ifdef _WIN32
int    XMLSetInt64(char* section,char* attr,__int64 v,char* xml,XML* af)
	{
   char a[40] = {0};
   sprintf(a,"%Lu",v);
   return XMLSetString(section,attr,a,xml,af);
   }
#endif

 int    XMLSetFloat(char* section,char* attr,float v,char* xml,XML* af)
	{
   char a[20] = {0};
   sprintf(a,"%f",v);
   return XMLSetString(section,attr,a,xml,af);
   }

#ifdef XML_OPTIONAL_MIME
 int    XMLSetBinaryData(char* section,char* attr,char* data,int len,char* xml,XML* af)
	{
	char* a = new char[len*5 + 1000];
	memset(a,0,len*5 + 1000);

	char *oo;
	MimeCoder<char*,char*>* e;
	e = new Base64Encoder<char*,char*>;
	oo = e->Filter(a, data, data + len);
	oo = e->Finish(oo);
	*oo = 0;                  // Put a zero to the end
	delete e;


	int I = XMLSetString(section,attr,a,xml,af);
	delete[] a;
	return I;
	}
#endif

// vector based things
#ifndef __SYMBIAN32__

#ifdef XML_USE_STL
int XMLGetAllVariables(char* section,vector<char*>* vnames,vector<char*>* vvalues,char*xml)
   {
   XML f(xml);

   XMLElement* r = f.GetRootElement();
   XMLElement* e = r->GetElementInSection(section);
   if (!e)
	  return 0; // no items under this one

   int Y = e->GetVariableNum();
   for(int i = 0 ; i < Y ; i++)
	  {
	  int yS = e->GetVariables()[i]->GetName(0);
	  char* d = new char[yS + 10];
	  memset(d,0,yS + 10);
	  e->GetVariables()[i]->GetName(d);
	  vnames->insert(vnames->end(),d);

	  yS = e->GetVariables()[i]->GetValue(0);
	  char* d2 = new char[yS + 10];
	  memset(d2,0,yS + 10);
	  e->GetVariables()[i]->GetValue(d2);
    vvalues->insert(vvalues->end(),d2);
    }
   return Y;
   }

int XMLGetAllItems(char* section,vector<char*>* vnames,char*xml)
   {
   XML f(xml);

   XMLElement* r = f.GetRootElement();
   XMLElement* e = r->GetElementInSection(section);
   if (!e)
	  return 0; // no items under this one

   int Y = e->GetChildrenNum();
   for(int i = 0 ; i < Y ; i++)
	  {
	  int yS =  e->GetChildren()[i]->GetElementName(0);
	  char* d = new char[yS + 10];
	  memset(d,0,yS + 10);
	  e->GetChildren()[i]->GetElementName(d);
	  vnames->insert(vnames->end(),d);
	  }
   return Y;
   }
#else // NO VECTOR

int XMLGetAllVariables(char* section,char** vnames,char** vvalues,char*xml)
   {
   XML f(xml);

   XMLElement* r = f.GetRootElement();
   XMLElement* e = r->GetElementInSection(section);
   if (!e)
	  return 0; // no items under this one

   int Y = e->GetVariableNum();
   for(int i = 0 ; i < Y ; i++)
	  {
	  int yS = e->GetVariables()[i]->GetName(0);
	  char* d = new char[yS + 10];
	  memset(d,0,yS + 10);
	  e->GetVariables()[i]->GetName(d);
	  vnames[i] = d;

	  yS = e->GetVariables()[i]->GetValue(0);
	  char* d2 = new char[yS + 10];
	  memset(d2,0,yS + 10);
	  e->GetVariables()[i]->GetValue(d2);
    vvalues[i] = d2;
    }
   return Y;
   }

int XMLGetAllItems(char* section,char** vnames,char*xml)
   {
   XML f(xml);

   XMLElement* r = f.GetRootElement();
   XMLElement* e = r->GetElementInSection(section);
   if (!e)
	  return 0; // no items under this one

   int Y = e->GetChildrenNum();
   for(int i = 0 ; i < Y ; i++)
	  {
	  int yS =  e->GetChildren()[i]->GetElementName(0);
	  char* d = new char[yS + 10];
	  memset(d,0,yS + 10);
	  e->GetChildren()[i]->GetElementName(d);
	  vnames[i] = d;
	  }
   return Y;
   }


#endif
#endif


// Win32 Ansi/Unicode wrappers

// Win32 ANSI string
#ifdef _WIN32
#ifndef __SYMBIAN32__

int    XMLSetStringA(const char* item,const char* attr,const char*v,const char* xml)
   {
   int Y = strlen(v)*2 + 1000;
   Z<char> vv(Y);
   Z<wchar_t> ww(Y);

   MultiByteToWideChar(CP_ACP,0,v,strlen(v),ww,Y);
   WideCharToMultiByte(CP_UTF8,0,ww,wcslen(ww),vv,Y,0,0);
   return XMLSetString(item,attr,vv,xml);
   }

int    XMLSetStringW(const char* item,const char* attr,const wchar_t*v,const char* xml)
   {
   int Y = wcslen(v)*2 + 1000;
   Z<char> vv(Y);

   WideCharToMultiByte(CP_UTF8,0,v,wcslen(v),vv,Y,0,0);
   return XMLSetString(item,attr,vv,xml);
   }

int XMLGetStringA(const char* section,const char* Tattr,const char* defv,char*out,const int maxlen,const char* xml,XML* aF,int CP)
   {
   int S = maxlen*2 + 1000;
   Z<char> a1(S);

   int S2 = strlen(defv) + 10;
   Z<char> a2(S2);
   strcpy(a2,defv);

   int Y = XMLGetString(section,Tattr,defv,a1,S,xml,aF);
   if (strcmp(a2,a1) == 0)
      return Y; // default was returned

   // An uTf-8 string was returned
   Z<wchar_t> uv(S);
   MultiByteToWideChar(CP_UTF8,0,a1,-1,uv,S);
   return WideCharToMultiByte(CP,0,uv,-1,out,maxlen,0,0);
   }


int XMLGetStringW(const char* section,const char* Tattr,const wchar_t* defv,wchar_t*out,const int maxlen,const char* xml,XML* aF,int CP)
   {
   //
	return 0;
   }

#endif
#endif


