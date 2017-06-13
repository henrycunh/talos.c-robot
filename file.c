TFileIOResult nIoResult;
TFileHandle hFileHandle;
int nFileSize = 2000;

bool setup=false;

//Initialize the datalog
void DL_Init(char* sFileName)
{
   Delete(sFileName, nIoResult);
   hFileHandle = 0;
   OpenWrite(  hFileHandle, nIoResult, sFileName, nFileSize);
   setup=true;
}

//Insert Time/Data Pairs
void DL_Insert(int time, int data)
{
   if(setup)
   {
      char myresults[20];
      sprintf(myresults,"%f,%d,\n", time, data);
      WriteString(hFileHandle, nIoResult, myresults);
   }
}

//Close datalog
void DL_Close()
{
   if(setup)
   {
      Close(hFileHandle, nIoResult);
   }
}
