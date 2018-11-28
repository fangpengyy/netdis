 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <dirent.h>
 #include <unistd.h>
 #include <vector>
#include <string>
#include "BuildProto.h"




std::vector<std::string> gProtoFile;


int readFileList(char *basePath)
{
     DIR *dir;
     struct dirent *ptr;
     if ((dir=opendir(basePath)) == NULL)
     {
         perror("Open dir error...");
         exit(1);
     }

     while ((ptr=readdir(dir)) != NULL)
     {
         if(strcmp(ptr->d_name,".")==0 || strcmp(ptr->d_name,"..")==0)    ///current dir OR parrent dir
             continue;
         else if(ptr->d_type == 8)    ///file
         {
            printf("d_name:%s/%s\n",basePath,ptr->d_name);
            gProtoFile.push_back(ptr->d_name);
         }
     }

     closedir(dir);

    return 0;
}


int main (int argc, char* argv[])
{
   printf("usage: proto protoSrcDir destDir\n");

   if(0 != readFileList(argv[1]))
   {
      printf("readFileList err %s\n", argv[1]);
      return -1;
   }
   if(argc < 3)
   {
        printf("err: param count 3\n");
       return 0;
   }

   std::string lsSrcPath = argv[1];
   lsSrcPath += "/";
   std::string lsDestPath = argv[2];
   lsDestPath += "/";

   for(int i = 0; i < (int)gProtoFile.size(); i++)
   {
       printf("--------------\n\n");
 
       FILE* lpFile = OpenFile(lsSrcPath.c_str(), gProtoFile[i]);
       if(lpFile)
       {
             ParseFile(lpFile, gProtoFile[i], lsDestPath);
             fclose(lpFile);
         
       }
   }
   printf("\n========================\n");
   BuildInclude(lsDestPath);

   return 0;
}



