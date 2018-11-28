#include "BuildProto.h"

#include <algorithm>
#include <stdio.h>


//std::vector<CClassDef> loVecClassDef;

std::vector<std::string> g_vecIncludeAdd;

std::vector<std::string> g_vecDefFunc;

std::vector<std::string> g_vecDefPackNo;

std::vector<std::string> g_vecDefPackNo_Num;

const char* lpHeadFile = "#ifndef __%s_H__\n"\
                         "#define __%s_H__\n"\
                         "#include <unistd.h>\n"\
                         "#include <stdint.h>\n"\
                         "#include \"../Pack.h\"\n" \
                         "#include \"HandleCustomPackMgr.h\"\n" \
                         "#include \"DefPackNo.h\"\n" \
                         "#include \"../Utils.h\"\n\n\n"
                         ;


const char* lpClassHead ="class %s: public IPacket\n"\
                         "{\n "\
                         "private:\n"\
                         "   TOnCustomPackHandle _OnCustomPackHandle;\n"\
                         "public:\n"
                         ;
 //  "       memset(this, 0, sizeof(*this));\n"
const char* lpClassInit ="   %s()\n "\
                         "   {\n"\
                         "       packTypeId =%s;\n"\
                         "       _OnCustomPackHandle = std::bind(&HandleCustomPackMgr::On%sHandle, HandleCustomPackMgr::GetInstance(), std::placeholders::_1, std::placeholders::_2);\n"\
                         "   };\n"
                         ;

const char* lpFunction = "  void Pack(char* szBuf, uint32_t& aiLen);\n" \
                         "   void UnPack(char* szBuf, uint32_t aiLen);\n" \
                         "   void OnHandle(TcpConnPtr& apConn, uint16_t aipackTypeId, char* szBuf, uint32_t aiLen);\n"\
                         "};\n\n";


const char* lpCppHead = "#include \"Pack_%s.h\"\n"\
                        "#include \"../NetBytes.h\"\n"\
                        "#include <iostream>\n\n\n\n"
                        ;


const char* lpHandle = "void Pack_%s::OnHandle(TcpConnPtr& apConn, uint16_t aipackTypeId, char* szBuf, uint32_t aiLen)\n"\
                       "{\n"\
                       "   UnPack(szBuf, aiLen);\n"\
                       "   if (aipackTypeId != packTypeId)\n"\
                       "   {\n"\
                       "      LOG_ERR(\" recv packTypeId=%%d, unpack packTypeId=%%d\", aipackTypeId, packTypeId);\n"\
                       "      return;\n"\
                       "   }\n"\
                       "   // here write code ...\n"\
                       "   if(_OnCustomPackHandle)\n"\
                       "        _OnCustomPackHandle(apConn, this);\n"\
                       "}\n\n";


const char* lpCustomHandleDef = "   void On%sHandle(TcpConnPtr& apConn, IPacket* apPack);\n";
const char* lpCustomHandleBody = "void HandleCustomPackMgr::On%sHandle(TcpConnPtr& apConn, IPacket* apPack)\n"\
                                 "{\n"\
                                 "   %s* lpPack = dynamic_cast<%s*>(apPack); \n"         \
                                 "   if(lpPack == NULL)\n"\
                                 "           return;\n"\
                                 "    //here add code ...\n"\
                                 "}\n";






void ParseVarName(char* szBuf, std::string& sVarName)
{
    int liPos = strlen(szBuf) - 1;
    char* lpEnd = 0;
    char* lpBegin = 0;

    char* lp = szBuf + liPos;

    while (lp != 0)
    {
        if (lpEnd == 0)
        {
            if (!(*lp == ';' || *lp == ' ' || *lp == '\n'))
                lpEnd = lp;
        }
        else if (*lp == ' ')
            break;

        lp--;
    }
    lpBegin = lp + 1;

    sVarName.assign(lpBegin, lpEnd + 1);
}

void ParsePackTypeId(char* szBuf, std::string& sVarName)
{
    char* lp = szBuf;
    bool lbBegin = false;
    char* lpStart = 0;
    char* lpEnd= lp + strlen(szBuf);

    int i = 0;
    while (lp < lpEnd)
    {
        if(!lbBegin)
        {
            if(*lp == '=')
                lbBegin = true;
        }
        else
        {
            if(lpStart == 0)
            {
                while(lp!=0)
                {
                    if(*lp != ' ')
                    {
                        lpStart = lp;
                        break;
                    }
                    lp++;
                }
            }
            else{
                 if(*lp== ' '|| *lp == ';' || *lp == '\n')
                 {
                     sVarName.assign(lpStart, lp);
                     break;
                 }
            }
       }
        lp++;
    }
}


void ParseCrlf(char* szBuf, std::string& sVarName)
{
    while (*szBuf == ' ') szBuf++;
    char* lpBegin = szBuf;

    while (*szBuf != '\n')
        szBuf++;

    char* lpEnd = szBuf;

    sVarName.assign(lpBegin, lpEnd);
}

FILE* OpenFile(std::string sPath, std::string sFileName)
{

    std::string lsFileName = sPath + sFileName;
    FILE* lpFile = fopen(lsFileName.c_str(), "r");
    return lpFile;
}

bool FindStr(std::vector<std::string>& lsVecFunc,  std::string& lsFuncName )
{
    for(int i = 0; i < (int)lsVecFunc.size(); i++)
    {
        if(strstr(lsVecFunc[i].c_str(), lsFuncName.c_str()))
            return true;
    }
    return false;
}

//加 HandleCustomPackMgr 头文件，接口函数
void  AppendNewFunc(FILE* lpFile_w,  std::vector<CClassDef>& loVecClassDef,  std::vector<std::string>& lsVecFunc)
{
    for(int i = 0; i < (int)loVecClassDef.size(); i++)
    {
        std::string& lsFuncName = loVecClassDef[i].sHandleName;
        if(!FindStr(lsVecFunc,  lsFuncName))
        {
            loVecClassDef[i].bAdd = true;
            char szBuf[1024]= {0};
            std::string lsName = "Pack_";
            lsName += loVecClassDef[i].sName;

            int liLen =  sprintf(szBuf, lpCustomHandleDef, lsName.c_str());
            fwrite(szBuf, 1, liLen, lpFile_w);
        }
    }
}

//加Pack_   头文件
void    AddPackInclude(std::vector<std::string>& loVecInclude,    FILE* lpFile_w)
{
    if(g_vecIncludeAdd.size() == 0)
        return;

    for(int i = 0; i < (int)g_vecIncludeAdd.size(); i++)
    {
        if(! FindStr(loVecInclude, g_vecIncludeAdd[i]))
        {
            char szBuf[128]= {0};
            int liLen = sprintf(szBuf, "#include \"%s\"\n", g_vecIncludeAdd[i].c_str());
            fwrite(szBuf, 1, liLen, lpFile_w);

          //  printf("----add %s\n", szBuf);
        }
    }
}

/*
    IPacket* GetPackObj(uint32_t packTypeId)
{
    switch(packTypeId)
   {
        DEF_PACK(TickHeart)
        DEF_PACK(Msg)


   }
   return NULL;
}
*/

void  AppendDefFunc(FILE* lpFile_w, std::vector<std::string>& lsVecFunc)
{
    for(int i = 0; i < (int)g_vecDefFunc.size(); i++)
    {
        std::string& lsName =  g_vecDefFunc[i];
        if(!FindStr(lsVecFunc,  lsName))
        {
            char szBuf[512]= {0};
            int liLen =  sprintf(szBuf, "   %s\n", lsName.c_str());
            fwrite(szBuf, 1, liLen, lpFile_w);
        }
    }
}
//关联 包类型与 包对象
void RegNewPack(std::string sDestPath)
{
    std::string lsRoutePack = sDestPath +  "RoutePack.cpp";
    printf("build %s ...\n",lsRoutePack.c_str());
    FILE* lpFile = fopen(lsRoutePack.c_str(), "r");

    std::string lsRoutePack_w = sDestPath +  "RoutePack.cpp_tmp";
    FILE* lpFile_w = fopen(lsRoutePack_w.c_str(), "w+");

    std::vector<std::string> lsVecDefFunc;

   int liStep = 0;
    while (!feof(lpFile))
    {
        char szBuf[1024] = { 0 };
        if (NULL != fgets(szBuf, sizeof(szBuf), lpFile))
        {
        //    printf("%s\n", szBuf);

            if(liStep != 5)
            {
                if(liStep == 0)
                {
                    if(strstr(szBuf, "GetPackObj"))
                        liStep = 1;
                }
                else if(liStep == 1)
                {
                    if(strstr(szBuf, "switch"))
                        liStep = 2;
                }
                else if(liStep == 2)
                {
                    if(strstr(szBuf, "{"))
                        liStep = 3;
                }
                else if(liStep == 3)
                {
                    if(strstr(szBuf, "}"))
                    {
                        liStep = 4;
                        continue;
                    }
                    else
                    {
                        char* lpEnd = strstr(szBuf, "//");
                        if(lpEnd )
                        {
                            std::string str(szBuf, lpEnd);
                            if(!str.empty())
                                   lsVecDefFunc.push_back(str);
                        }
                        else if (szBuf[0] != '\n')
                            lsVecDefFunc.push_back(szBuf);
                    }
                }
                else if(liStep == 4)
                {
             //       printf("-------step %d\n", 4);
                    AppendDefFunc(lpFile_w, lsVecDefFunc);
                    fwrite("}\n", 1,  2, lpFile_w);
                    liStep = 5;
                }
            }
            fwrite(szBuf, 1,  strlen(szBuf), lpFile_w);
        }
    }
    fclose(lpFile);
    fclose(lpFile_w);
    lsVecDefFunc.clear();

    std::string lsBakRoutePack = sDestPath +  "RoutePack.h_bak";
    rename(lsRoutePack.c_str(), lsBakRoutePack.c_str());
    rename(lsRoutePack_w.c_str(), lsRoutePack.c_str());
}
/*
enum
{
    DEF_FUNC_TickHeart,
    DEF_FUNC_Msg,

};
*/

void AppendPackNo(FILE* lpFile_w, std::vector<std::string>& loVecPackNo)
{
    if(g_vecDefPackNo_Num.size() == 0)
        return ;

    for(int i = 0; i < (int)g_vecDefPackNo_Num.size(); i++)
    {
           std::string & lsPackNo = g_vecDefPackNo[i];
           if(!FindStr(loVecPackNo, lsPackNo))
           {
               char szBuf[256] ={0};
               int liLen = sprintf(szBuf, "   %s,\n", g_vecDefPackNo_Num[i].c_str());
               fwrite(szBuf, 1, liLen, lpFile_w);
           }
    }
}
//定义 包类型
void EditPackNo(std::string& sDestPath)
{
    std::string lsDefPackNo = sDestPath +  "DefPackNo.h";
    printf("build %s ...\n",lsDefPackNo.c_str());
    FILE* lpFile = fopen(lsDefPackNo.c_str(), "r");

    std::string lsDefPackNo_w = sDestPath +  "DefPackNo.h_tmp";
    FILE* lpFile_w = fopen(lsDefPackNo_w.c_str(), "w+");

    std::vector<std::string> lsVecDefPackNo;
    int liStep = 0;
    while (!feof(lpFile))
    {
        char szBuf[1024] = { 0 };
        if (NULL != fgets(szBuf, sizeof(szBuf), lpFile))
        {
            if(liStep != 4)
            {
                if(liStep == 0)
                {
                    if(strstr(szBuf, "enum"))
                        liStep =1;
                }
                else if(liStep == 1)
                {
                    if(strstr(szBuf, "{"))
                        liStep = 2;
                }
                else if(liStep == 2)
                {
                    lsVecDefPackNo.push_back(szBuf);
                    if(strstr(szBuf, "}"))
                    {
                        liStep = 3;
                        continue;
                    }
                }
                else if(liStep == 3)
                {
                    AppendPackNo(lpFile_w, lsVecDefPackNo);
                    fwrite("};\n", 1,  3, lpFile_w);
                    liStep = 4;
                }
            }
            fwrite(szBuf, 1,  strlen(szBuf), lpFile_w);
        }
    }

    fclose(lpFile);
    fclose(lpFile_w);
    std::string lsBakDefPackNo = sDestPath +  "DefPackNo.h_bak";
    rename(lsDefPackNo.c_str(), lsBakDefPackNo.c_str());
    rename(lsDefPackNo_w.c_str(), lsDefPackNo.c_str());
}


void ParseFile(FILE* apFile, std::string sFileName, std::string sDestPath)
{
    //定义struct 接口类型
   std::vector<CClassDef>   loVecClassDef;

    bool lbBegin = false;

   CClassDef loClassDef;

    while (!feof(apFile))
    {
        char szBuf[2048] = { 0 };
        if (NULL != fgets(szBuf, sizeof(szBuf), apFile))
        {
            if (szBuf[0] == '\n' || szBuf[0] == '#')
                continue;
            if (szBuf[0] == '/' && szBuf[1] == '/')
                continue;

            if (!lbBegin)
            {
                char* lpBuf = strstr(szBuf, "Stru_");
                if (lpBuf)
                {
                    //CClassDef loClassDef;
                     ParseCrlf(lpBuf + 5, loClassDef.sName);

                    loVecClassDef.push_back(loClassDef);
                    std::string  lsDef ="DEF_PACK(";
                    lsDef += loClassDef.sName +")";
                    g_vecDefFunc.push_back(lsDef);

               //     std::string lsPackNo = "DEF_FUNC_" + loClassDef.sName;
               //     loVecClassDef[loVecClassDef.size() - 1].sPackTypeId = lsPackNo;
               //     g_vecDefPackNo.push_back(lsPackNo);

                    lbBegin = true;
                }
            }
            else
            {
                if (strstr(szBuf, "};"))
                {
                    lbBegin = false;
                    static int liCount = 0;
                    printf("parse  class pack index=%d\n", ++liCount);
                }
                else
                {
                    if (strstr(szBuf, "{"))
                        continue;

                    int liIndex = loVecClassDef.size() - 1;

                    if(strstr(szBuf, "="))
                    {
                        std::string lsPackTypeId;
                        ParsePackTypeId(szBuf,  lsPackTypeId);
                        loVecClassDef[liIndex].sPackTypeId_Num = lsPackTypeId;

                         std::string lsPackNo = "DEF_FUNC_" + loClassDef.sName;
                        loVecClassDef[loVecClassDef.size() - 1].sPackTypeId = lsPackNo;
                        g_vecDefPackNo.push_back(lsPackNo);

                        g_vecDefPackNo_Num.push_back( lsPackNo +"=" + lsPackTypeId);
                    }
                    else
                    {
                        loVecClassDef[liIndex].vecVarDef.push_back(szBuf);

                        std::string lsVarName;
                        ParseVarName(szBuf, lsVarName);
                        loVecClassDef[liIndex].vecVarName.push_back(lsVarName);
                    }
                }
            }
        }
    } //while

//覆盖 Pack_ 的 h \ cpp 文件 ===================================

    //生成Pack头文件
    int liPos = sFileName.find("Proto_");
    liPos += 6;
    int liPos2 = sFileName.find(".", liPos);

    std::string lsFile = sFileName.substr(liPos, liPos2 - liPos);
    std::string lsHFile = "Pack_" + lsFile + ".h";
    std::string lgFile = lsFile;

    std::string lsFileName = sDestPath + lsHFile;
    FILE* lpFile = fopen(lsFileName.c_str(), "w+");

    char szBuf[4096] = { 0 };
    std::transform(lsFile.begin(), lsFile.end(), lsFile.begin(), ::toupper);

    int liLen = sprintf(szBuf, lpHeadFile, lsFile.c_str(), lsFile.c_str());
    fwrite(szBuf, 1, liLen, lpFile);


    for (int i = 0; i < (int)loVecClassDef.size(); i++)
    {
        std::string lsName = "Pack_";
        lsName += loVecClassDef[i].sName;

        char szHandle[218]= {0};
        sprintf(szHandle,  "On%sHandle", lsName.c_str());
        loVecClassDef[i].sHandleName = szHandle;

        int liLen = sprintf(szBuf, lpClassHead, lsName.c_str());
        fwrite(szBuf, 1, liLen, lpFile);

        for (int k = 0; k < (int)loVecClassDef[i].vecVarDef.size(); k++)
        {
            std::string& lsVar = loVecClassDef[i].vecVarDef[k];
            int liLen = sprintf(szBuf, "  %s", lsVar.c_str());
            fwrite(szBuf, 1, liLen, lpFile);
        }

        const char* lpPublic = "public:\n";
        fwrite(lpPublic, 1, strlen(lpPublic), lpFile);

        liLen = sprintf(szBuf, lpClassInit,  lsName.c_str(), loVecClassDef[i].sPackTypeId.c_str(), lsName.c_str());
        fwrite(szBuf, 1, liLen, lpFile);

        liLen = sprintf(szBuf, "   virtual ~%s(){};\n", lsName.c_str());
        fwrite(szBuf, 1, liLen, lpFile);

        fwrite(lpFunction, 1, strlen(lpFunction), lpFile);

        printf("create class pack head , %s , index=%d\n", lsName.c_str(), i + 1);
    }
    fwrite("\n#endif", 1, 7, lpFile);

    fclose(lpFile);

    //生成Pack,Cpp文件-------------------------------------------------------------

    std::string lsCppFile = "Pack_" + lgFile + ".cpp";
    lsCppFile = sDestPath + lsCppFile;
    lpFile = fopen(lsCppFile.c_str(), "w+");

    liLen = sprintf(szBuf, lpCppHead, lgFile.c_str());
    fwrite(szBuf, 1, liLen, lpFile);

    for (int i = 0; i < (int)loVecClassDef.size(); i++)
    {
        liLen = sprintf(szBuf, "void Pack_%s::Pack(char* szBuf, uint32_t& aiLen)\n{\n   NetBytes loBytes(szBuf, aiLen);\n",
                        loVecClassDef[i].sName.c_str());

        fwrite(szBuf, 1, liLen, lpFile);

        std::string loBytes = "   loBytes";
        for (int k = 0; k < (int)loVecClassDef[i].vecVarName.size(); k++)
        {
            loBytes += "<<" + loVecClassDef[i].vecVarName[k];
            //   printf("%s\n", loVecClassDef[i].vecVarName[k].c_str());
        }

        loBytes += ";\n";
        fwrite(loBytes.c_str(), 1, (int)loBytes.size(), lpFile);
        //printf("val = %d varname len=  %d\n", (int)loVecClassDef[i].vecVarName.size(),  (int)loBytes.size());
        liLen = sprintf(szBuf, "   aiLen = loBytes.GetDataLen();\n}\n\n");
        //printf("-3--- %d\n", liLen);
        fwrite(szBuf, 1, liLen, lpFile);

        //UnPack
        liLen = sprintf(szBuf, "void Pack_%s::UnPack(char* szBuf, uint32_t aiLen)\n{\n   NetBytes loBytes(szBuf, aiLen);\n",
                        loVecClassDef[i].sName.c_str());
        fwrite(szBuf, 1, liLen, lpFile);

        loBytes = "   loBytes";
        for (int k = 0; k < (int)loVecClassDef[i].vecVarName.size(); k++)
            loBytes += ">>" + loVecClassDef[i].vecVarName[k];

        loBytes += ";\n}\n\n";
        //   printf("%s",loBytes.c_str());

        fwrite(loBytes.c_str(), 1, (int)loBytes.size(), lpFile);

        //OnHandle
        liLen = sprintf(szBuf, lpHandle, loVecClassDef[i].sName.c_str());
        fwrite(szBuf, 1, liLen, lpFile);

        fwrite("\n\n", 1, 2, lpFile);

        printf("create class pack  body, %s, index=%d\n", loVecClassDef[i].sName.c_str(), i + 1);
    }
    fclose(lpFile);

   //===================================================

// HandleCustomPackMgr , 修改 函数接口**********************************************

    std::string lsCustomHandle = sDestPath +  "HandleCustomPackMgr.h";
    printf("build %s ...\n",lsCustomHandle.c_str());

    lpFile = fopen(lsCustomHandle.c_str(), "r");

    std::string lsCustomHandle_w = sDestPath +  "HandleCustomPackMgr.h_tmp";
    FILE* lpFile_w = fopen(lsCustomHandle_w.c_str(), "w+");

    std::vector <std::string> lsVecFunc;
    bool lbJoin = false;
    bool lbAddFunc= false;
    while (!feof(lpFile))
    {
        char szBuf[2048] = { 0 };
        if (NULL != fgets(szBuf, sizeof(szBuf), lpFile))
        {
             if(!lbAddFunc && strstr(szBuf, "public:"))
            {
                lbAddFunc = true;
            }

            if(!lbJoin &&  strstr(szBuf, "private:"))
            {
                lbJoin = true;
                lbAddFunc = false;
                AppendNewFunc(lpFile_w, loVecClassDef, lsVecFunc);
            }
            if(lbAddFunc)
                lsVecFunc.push_back(szBuf);

            fwrite(szBuf, 1,  strlen(szBuf), lpFile_w);
        }
    }
    fclose(lpFile);
    fclose(lpFile_w);
    lsVecFunc.clear();

    std::string lsBakCustomHandle = sDestPath +  "HandleCustomPackMgr.h_bak";
    rename(lsCustomHandle.c_str(), lsBakCustomHandle.c_str());
    rename(lsCustomHandle_w.c_str(), lsCustomHandle.c_str());

    //
    // HandleCustomPackMgr. cpp **************************************************************
    //修改
    std::string lsPackMgrCpp = sDestPath +  "HandleCustomPackMgr.cpp";
    FILE* lpFileMgr = fopen(lsPackMgrCpp.c_str(), "r");

    std::vector<std::string> loVecFuncHandle;
    if(lpFileMgr)
    {
           char szBuf[2048]= {0};
           while (!feof(lpFileMgr))
         {
                 if (NULL != fgets(szBuf, sizeof(szBuf), lpFileMgr))
                {
                    if(strstr(szBuf, "void"))
                        loVecFuncHandle.push_back(szBuf);
                }
         }
        fclose(lpFileMgr);
    }

    std::string lsCustomHandle_cpp = sDestPath +  "HandleCustomPackMgr.cpp";
    printf("build %s ...\n",lsCustomHandle_cpp.c_str());

    lpFile_w = fopen(lsCustomHandle_cpp.c_str(), "ab+");

    for(int i = 0; i < (int)loVecClassDef.size(); i++)
    {
      //  if(loVecClassDef[i].bAdd)
       // {
            std::string lsName = "Pack_";
            lsName += loVecClassDef[i].sName;
            printf(" add handle-- %s\n", lsName.c_str());

            char szFunc[512]= {0};
            sprintf(szFunc, "HandleCustomPackMgr::On%sHandle", lsName.c_str());
            std::string lsFunc = szFunc;

            if(!FindStr(loVecFuncHandle, lsFunc) )
            {
                int liLen =  sprintf(szBuf, lpCustomHandleBody,  lsName.c_str(), lsName.c_str(), lsName.c_str());
                fwrite(szBuf, 1, liLen, lpFile_w);
            }
       // }
    }
    fclose(lpFile_w);
   loVecFuncHandle.clear();

    std::string lsName = "Pack_";
    int k = sFileName.find("_");
    lsName += sFileName.substr(k + 1);

    g_vecIncludeAdd.push_back(lsName);

    printf("complete %s\n", sFileName.c_str());
}

void BuildInclude(std::string& sDestPath)
{
    std::string lsPackInclude = sDestPath +  "Pack_Include.h";
    printf("build %s ...\n",lsPackInclude.c_str());

    FILE* lpFile = fopen(lsPackInclude.c_str(), "r");

    std::string lsPackInclude_w = sDestPath +  "Pack_Include.h_tmp";
    FILE* lpFile_w = fopen(lsPackInclude_w.c_str(), "w+");

    std::vector<std::string> loVecInclude;
    char szBuf[1024]= {0};
    while (!feof(lpFile))
    {
        if (NULL != fgets(szBuf, sizeof(szBuf), lpFile))
        {
            if(szBuf[0] == '/')
                continue;

            if(strstr(szBuf, "#include"))
                loVecInclude.push_back(szBuf);
            else  if(strstr(szBuf, "#endif"))
            {
                AddPackInclude(loVecInclude,   lpFile_w);

            }
            fwrite(szBuf, 1, strlen(szBuf), lpFile_w);
        }
    }
    fclose(lpFile);
    fclose(lpFile_w);

    std::string lsBakPackInclude = sDestPath +  "Pack_Include.h_bak";
    rename(lsPackInclude.c_str(), lsBakPackInclude.c_str());
    rename(lsPackInclude_w.c_str(), lsPackInclude.c_str());

    g_vecIncludeAdd.clear();

    printf("RegNewPack...\n");
    RegNewPack(sDestPath);

    printf("add PackNo...\n");
    EditPackNo(sDestPath);
}
