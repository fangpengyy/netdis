#ifndef __BUILDPROTO_H__
#define __BUILDPROTO_H__

#include <string.h>
#include <stdio.h>
#include <string>
#include <vector>




class CClassDef
{
public:
    CClassDef()
    {
        bAdd = false;
    }
	std::string sName;
	std::string sPackTypeId;
	std::string sPackTypeId_Num;
	std::string sHandleName;
	std::vector<std::string> vecVarDef;
	std::vector<std::string> vecVarName;
    bool bAdd;
};

extern std::vector<std::string> g_vecIncludeAdd;
extern void BuildInclude(std::string& sDestPath);

extern FILE* OpenFile(std::string sPath, std::string sFileName);
extern void ParseFile(FILE* apFile, std::string sFileName, std::string sDestPath);


#endif
