#pragma comment(lib, "netapi32.lib")
#include <stdio.h>
#include <assert.h>
#include <windows.h> 
#include <lm.h>
#include <napi.h>
#include <stdlib.h>

int get_user_priv_level(std::string user_name)
{
   LPUSER_INFO_2 pBuf = NULL;
   LPUSER_INFO_2 pTmpBuf;
   DWORD dwLevel = 2;
   DWORD dwPrefMaxLen = MAX_PREFERRED_LENGTH;
   DWORD dwEntriesRead = 0;
   DWORD dwTotalEntries = 0;
   DWORD dwResumeHandle = 0;
   DWORD i;
   DWORD dwTotalCount = 0;
   NET_API_STATUS nStatus;
   LPTSTR pszServerName = NULL;

   do {
      nStatus = NetUserEnum((LPCWSTR) pszServerName,
                            dwLevel,
                            FILTER_NORMAL_ACCOUNT, 
                            (LPBYTE*)&pBuf,
                            dwPrefMaxLen,
                            &dwEntriesRead,
                            &dwTotalEntries,
                            &dwResumeHandle);
      if ((nStatus == NERR_Success) || (nStatus == ERROR_MORE_DATA))
      {
         if ((pTmpBuf = pBuf) != NULL)
         {
            for (i = 0; (i < dwEntriesRead); i++)
            {
               assert(pTmpBuf != NULL);

               if (pTmpBuf == NULL)
               {
                  fprintf(stderr, "An access violation has occurred\n");
                  break;
               }

               char buffer[500];
               wcstombs(buffer, pTmpBuf->usri2_name, 500);

               if (strcmp(buffer, user_name.c_str()) == 0) {
                  return pTmpBuf->usri2_priv;
               }

               pTmpBuf++;
               dwTotalCount++;
            }
         }
      } else fprintf(stderr, "A system error has occurred: %d\n", nStatus);

      if (pBuf != NULL)
      {
         NetApiBufferFree(pBuf);
         pBuf = NULL;
      }
   }

   while (nStatus == ERROR_MORE_DATA);

   if (pBuf != NULL)
      NetApiBufferFree(pBuf);

   return -1;
}

class MainAddon : public Napi::Addon<MainAddon> {
 public:
  MainAddon(Napi::Env env, Napi::Object exports) {
    DefineAddon(exports,
                {InstanceMethod("get_user_priv", &MainAddon::GetUserPriv, napi_enumerable)});
  }

 private:
  Napi::Value GetUserPriv(const Napi::CallbackInfo& info) {
   Napi::Env env = info.Env();
   std::string str = info[0].ToString().Utf8Value();
   int user_priv_level = get_user_priv_level(str);
   return Napi::Number::New(info.Env(), user_priv_level);
  }
};


NODE_API_ADDON(MainAddon)