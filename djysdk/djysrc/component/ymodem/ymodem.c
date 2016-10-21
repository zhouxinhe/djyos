//----------------------------------------------------
// Copyright (c) 2014, SHENZHEN PENGRUI SOFT CO LTD. All rights reserved.

// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:

// 1. Redistributions of source code must retain the above copyright notice,
//    this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------
// Copyright (c) 2014 ����Ȩ�����������������޹�˾���С�����Ȩ�˱���һ��Ȩ����
//
// �����Ȩ�����ʹ���߷������������������£�����ʹ����ʹ�ü���ɢ����
// ������װԭʼ�뼰����λ��ִ����ʽ��Ȩ�������۴˰�װ�Ƿ񾭸�����Ȼ��
//
// 1. ���ڱ�����Դ�������ɢ�������뱣�������İ�Ȩ���桢�������б�����
//    ������������������
// 2. ���ڱ��׼�����λ��ִ����ʽ����ɢ���������������ļ��Լ�������������
//    ��ɢ����װ�е�ý�鷽ʽ����������֮��Ȩ���桢�������б����Լ�����
//    ������������

// �����������������Ǳ�������Ȩ�������Լ�����������״��"as is"���ṩ��
// ��������װ�����κ���ʾ��Ĭʾ֮�������Σ������������ھ��������Լ��ض�Ŀ
// �ĵ�������ΪĬʾ�Ե�������Ȩ�����˼�������֮�����ߣ������κ�������
// ���۳�����κ��������塢���۴�����Ϊ���Լ��ϵ���޹�ʧ������������Υ
// Լ֮��Ȩ��������ʧ������ԭ��ȣ����𣬶����κ���ʹ�ñ�������װ��������
// �κ�ֱ���ԡ�����ԡ�ż���ԡ������ԡ��ͷ��Ի��κν�����𺦣�����������
// �������Ʒ������֮���á�ʹ����ʧ��������ʧ��������ʧ��ҵ���жϵȵȣ���
// �����κ����Σ����ڸ���ʹ���ѻ���ǰ��֪���ܻ���ɴ����𺦵���������Ȼ��
//-----------------------------------------------------------------------------
//����ģ��:ymodemģ��
//����:  ����.
//�汾��V2.0.0
//�ļ�����:ymodemģ�飬����ymodemЭ���ϴ��������ļ�
//����˵��:
//�޶���ʷ:
//2. ...
//1. ����: 2013-10-06
//   ����:  ������.
//   �°汾��: V1.0.0
//   �޸�˵��: ԭʼ�汾
//------------------------------------------------------

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "os.h"
#include "shell.h"
#include "driver.h"
#include "verify.h"

#include "ymodem.h"
#include <cfg/ymodemcfg.h>

//#define CN_MAX_PACKNUM    32            //����������������������������Ҫд���ļ�
//#define CN_YMODEM_FILEBUF_SIZE   ((CN_MAX_PACKNUM + 1)*1024) //���建������С���ɺ�����̬����

#define CN_YMODEM_PKGBUF_SIZE         	(1029)
#define CN_YMODEM_NAME_LENGTH  			(256)

struct ShellCmdTab const ymodem_cmd_table[] =
{
    {
        "download",
		Ymodem_DownloadFile,
        "�����ļ�",
        "�����ʽ: download"
    },
    {
        "upload",
		Ymodem_UploadFile,
        "�ϴ��ļ�",
        "�����ʽ: upload �ļ���"
    }
};

static struct ShellCmdRsc tg_ymodem_cmd_rsc
                        [sizeof(ymodem_cmd_table)/sizeof(struct ShellCmdTab)];
static struct DjyDevice *s_ptYmodemDevice;

static tagYmodem *pYmodem = NULL;
//----ymodemģ�ͳ�ʼ��---------------------------------------
//����: ymodem�����ļ�ģ��ӿں�������module_trim�е���
//����: ��
//����: ��
//-----------------------------------------------------------
ptu32_t ModuleInstall_Ymodem(struct DjyDevice *para)
{
    if(para == NULL)
        s_ptYmodemDevice = ToDev(stdin);
    else
        s_ptYmodemDevice = para;

    pYmodem = (tagYmodem *)malloc(sizeof(tagYmodem));
    if(NULL != pYmodem)
    {
    	pYmodem->FileName = (char *)malloc(CN_YMODEM_NAME_LENGTH);
    	if(NULL == pYmodem->FileName)
    	{
    		free(pYmodem);
    		return false;
    	}
    	pYmodem->pYmodemMutex = Lock_MutexCreate("YMODEM_MUTEX");
    	if(NULL != pYmodem->pYmodemMutex)
    	{
    		pYmodem->Path = NULL;
			if(Sh_InstallCmd(ymodem_cmd_table,tg_ymodem_cmd_rsc,
					sizeof(ymodem_cmd_table)/sizeof(struct ShellCmdTab)))
				return true;
    	}
    	else
    	{
    		free(pYmodem->FileName);
    		free(pYmodem);
    	}
    }
    return false;
}

// ============================================================================
// ���ܣ�����ymodem�����ļ��ľ���·�����ú���ѡ���Ե��ã�
//      1.��Ӧ����Ҫ�������·�����򲻵��øú�����ͨ��������ʽ����shell���޸�·��;
//      2.���øú�����ͨ��Ymodem���ص�·���򲻿ɱ任;
//      3.����˵����������iboot����������·������䣬���Path = "/iboot"
// ������Path��ͨ��Ymodem��ŵľ���·��
// ���أ�true,�ɹ�������ʧ��
// ============================================================================
bool_t Ymodem_PathSet(const char *Path)
{
	bool_t Ret = false;

	if( (pYmodem != NULL) && (NULL != Path) )
	{
		Lock_MutexPend(pYmodem->pYmodemMutex,CN_TIMEOUT_FOREVER);
		pYmodem->Path = Path;

		Lock_MutexPost(pYmodem->pYmodemMutex);
		Ret = true;
	}
	return Ret;
}

//----��ȡ��������-------------------------------------------
//������__Ymodem_Gets����ȡ���ֽ�����
//     __Ymodem_Get����ȡһ��������
//     __Ymodem_Puts��������ֽ�����
//     __Ymodem_Put������һ�ֽ�����
//���ܣ�YMODEM���ݽӿ�ͨ����ͨ��������ͨ����ȡ�ʹ�������
//������
//���أ�
//-----------------------------------------------------------
static u32 __Ymodem_Gets(u8 *buf,u32 len, u32 timeout)
{
   return Driver_ReadDevice(s_ptYmodemDevice,buf,len,0,timeout);
}
static u32 __Ymodem_Get(u8 *buf)
{
   return __Ymodem_Gets(buf,1,gYmodemPkgTimeOut);
}

static u32 __Ymodem_Puts(u8 *buf,u32 len,u32 timeout)
{
    return Driver_WriteDevice(s_ptYmodemDevice,buf,len,0,
                                CN_BLOCK_BUFFER,timeout);
}
static u32 __Ymodem_Put(u8 Char)
{
    u8 ch = Char;
    return __Ymodem_Puts(&ch,1,500*mS);
}

//----��ȡ����----------------------------------------------
//����: ��buf����ȡһ���ɿո���н����������ĵ��ʣ�next����
//      ������һ�������׵�ַ�����û����һ�����ʣ���next=NULL��
//����: buf�����������ַ���
//      next��������һ������ָ��
//����: ��ȡ�ĵ���ָ�룬�ѽ����ʺ���ķָ������ɴ�������'\0'
//-----------------------------------------------------------
//todo:shell.c��Sh_GetWord������˺�����ͬ��
static char *__Ymodem_GetWord(char *buf,char **next)
{
    uint32_t i=0;
    *next = NULL;
    if(buf == NULL)
        return NULL;
    while(1)
    {
        if((buf[i] == ' ') || (buf[i] == 0))
        {
            buf[i] = '\0';
            break;
        }
        i++;
    }
    i++;
    while(buf[i] != 0)
    {
        if(buf[i]!=' ')
        {
            if((buf[i] == '\n') || (buf[i] == '\r'))
                *next = NULL;
            else
                *next = &buf[i];
            break;
        }
        i++;
    }
    return buf;
}
//------ymodemȡ������---------------------------------------
//���ܣ�ymodemģ��֪ͨ�Զ�ȡ�����δ���
//��������
//���أ���
//-----------------------------------------------------------
static void __Ymodem_CancelTrans(void)
{
    __Ymodem_Put(CN_YMODEM_CAN);
    __Ymodem_Put(CN_YMODEM_CAN);
    __Ymodem_Put(CN_YMODEM_CAN);
}

//------У��ymodem���ݰ�-------------------------------------
//���ܣ�ymodemģ��У����յ������ݰ�
//������buf�����ݰ�����
//      pack_len: ymodem���ݰ�����
//���أ�true:  ���ݰ�У����ȷ
//      false: ���ݰ�У�����
//----------------------------------------------------------
static bool_t __Ymodem_PackCheck(u8* buf, u32 pack_len)
{
    u16 checksum,check;
    if((buf[1] + buf[2]) != 0xff)               //У�����������
    {
        __Ymodem_Put(CN_YMODEM_NAK);                    //Ӧ��nak�������ط�
        return false;
    }
    checksum = crc16(buf+3, pack_len);
    if(CN_CFG_BYTE_ORDER == CN_CFG_LITTLE_ENDIAN)
    {
        check = (buf[pack_len+3]<<8) + buf[pack_len+4];
    }
    else
    {
        check = *(u16 *)&buf[pack_len+3];
    }
    if(checksum != check)    //CRCУ�����
    {
        __Ymodem_Put(CN_YMODEM_NAK);                    //Ӧ��nak�������ط�
        return false;
    }

    return true;
}

static void __Ymodem_WriteCrc16(u8 *package, u32 pack_len)
{
	u16 checksum;

	checksum = crc16(package+3, pack_len);
	if(CN_CFG_BYTE_ORDER == CN_CFG_LITTLE_ENDIAN)
	{
		package[3+pack_len] = (checksum >> 8) & 0xFF;
		package[3+pack_len + 1] = (checksum) & 0xFF;

	}
	else
	{
		package[3+pack_len] = (checksum) & 0xFF;
		package[3+pack_len + 1] = (checksum >> 8) & 0xFF;
	}
}

// ��ȡ������YMODEM���ݰ����������ݰ���ACK��CAN�Ȱ�
static YMRESULT __Ymodem_GetPkg(tagYmodem *ym)
{
	YMRESULT Ret = YMODEM_OK;
	u32 bytes;

	if(ym->PkgBufCnt == 0)
	{
		if(1 != __Ymodem_Get(ym->PkgBuf) )
			Ret = YMODEM_TIMEOUT;
	}

	switch(ym->PkgBuf[0])
	{
	case CN_YMODEM_SOH:
		ym->PkgSize = CN_YMODEM_SOH_SIZE;
		break;
	case CN_YMODEM_STX:
		ym->PkgSize = CN_YMODEM_STX_SIZE;
		break;
	case CN_YMODEM_ACK:
	case CN_YMODEM_NAK:
	case CN_YMODEM_C:
	case CN_YMODEM_CAN:
	case CN_YMODEM_EOT:
		ym->PkgSize = 1;
		break;
	default:
		ym->PkgSize = 1;
		Ret = YMODEM_UNKNOW_ERR;
		break;
	}

	if(ym->PkgSize > 1)
	{
		bytes = __Ymodem_Gets(ym->PkgBuf + 1,ym->PkgSize + 4,gYmodemPkgTimeOut);
		if(bytes != ym->PkgSize + 4)
			Ret = YMODEM_TIMEOUT;
	}

	return Ret;
}

static bool_t __Ymodem_FilePathMerge(char *dst,const char *path,char *name)
{
	u32 PathLen,NameLen;

	if(NULL != name)
	{
		PathLen = strlen(path);
		NameLen = strlen(name);
		if( NULL != path )
		{
			if(PathLen + NameLen + 1 < CN_YMODEM_NAME_LENGTH)
			{
				strcpy(dst,path);
				dst[PathLen] = '/';
				strcpy(dst + PathLen + 1,name);
				dst[PathLen + NameLen + 1] = '\0';
				return true;
			}
		}
		else
		{
			strcpy(dst,name);
			dst[NameLen] = '\0';
			return true;
		}
	}
	return false;
}
static bool_t __Ymodem_InfoPkg(tagYmodem *ym)
{
	char *NextParam,*strFileSize,*FileName;

	FileName = __Ymodem_GetWord((char*)&ym->PkgBuf[3],&NextParam);
	strFileSize = __Ymodem_GetWord(NextParam,&NextParam);

	if(__Ymodem_FilePathMerge(ym->FileName,ym->Path,FileName))
	{
		ym->FileSize = strtol(strFileSize, (char **)NULL, 0);
		ym->FileOps = YMODEM_FILE_OPEN;
		ym->Status = CN_YMODEM_SOH;
		ym->PkgNo = 1;
		return true;
	}
	return false;
}

static bool_t __Ymodem_IsZeroPkg(tagYmodem *ym)
{
	u8 i;

	if( (ym->PkgBuf[1] == 0x00) && (ym->PkgBuf[2] == 0xFF) )
	{
		for(i = 0; i < 128; i++)
		{
			if(ym->PkgBuf[3+i] != 0x00)
			{
				return false;
			}
		}
	}

	return true;
}


static YMRESULT __Ymodem_FileOps(tagYmodem *ym, u8 flag)
{
	YMRESULT Ret = YMODEM_OK;
    struct stat FpInfo;
	u32 FileOpsLen;

	//���ļ����в���
	switch(ym->FileOps)
	{
	case YMODEM_FILE_OPEN:
		if(flag)
		{
			ym->File = fopen(ym->FileName,"w+");		//���ļ����������򴴽�
			__Ymodem_Put(CN_YMODEM_ACK);
			__Ymodem_Put(CN_YMODEM_C);
		}
		else
		{
			ym->File = fopen(ym->FileName,"r");		//���ļ����������򴴽�
		}
		if(ym->File == NULL)
		{
			Ret = YMODEM_FILE_ERR;
		}
		break;
	case YMODEM_FILE_WRITE:
        memcpy(ym->FileBuf + ym->FileBufCnt,ym->PkgBuf + 3,ym->PkgSize);
        ym->FileBufCnt += ym->PkgSize;
        if((ym->FileBufCnt >= ym->FileSize - ym->FileCnt) ||
        		(ym->FileBufCnt >= gYmodemBufPkgNum*1024)) //����㹻��д��flash
        {
            if(ym->FileBufCnt >= ym->FileSize - ym->FileCnt)                                   //�жϱ���д���С
            	FileOpsLen = ym->FileSize - ym->FileCnt;
            else
            	FileOpsLen = ym->FileBufCnt;
            ym->FileBufCnt = 0;
            if(FileOpsLen != fwrite(ym->FileBuf,FileOpsLen,1,ym->File))
            {
            	Ret = YMODEM_FILE_ERR;
            }
            ym->FileCnt += FileOpsLen;
        }
        __Ymodem_Put(CN_YMODEM_ACK);
		break;
	case YMODEM_FILE_READ:
		memset(ym->PkgBuf,0x00,CN_YMODEM_STX_SIZE);
		FileOpsLen = (ym->FileSize - ym->FileCnt > CN_YMODEM_STX_SIZE) ?
				(CN_YMODEM_STX_SIZE) : ym->FileSize - ym->FileCnt;
		ym->PkgSize = CN_YMODEM_STX_SIZE;
		ym->PkgBuf[0] = CN_YMODEM_STX;
		ym->PkgBuf[1] = ym->PkgNo - 1;
		ym->PkgBuf[2] = 0xFF - ym->PkgBuf[1];

		if(FileOpsLen != fread(ym->PkgBuf + 3,1,FileOpsLen,ym->File))
		{
			Ret = YMODEM_FILE_ERR;
		}
        __Ymodem_WriteCrc16(ym->PkgBuf,ym->PkgSize);
        ym->FileCnt += ym->PkgSize;
		break;
	case YMODEM_FILE_STAT:
		if(0 == stat(ym->FileName,&FpInfo))
		{
			ym->FileSize = (u32)FpInfo.st_size;
		}
		else
		{
			Ret = YMODEM_FILE_ERR;
		}
		break;
	case YMODEM_FILE_CLOSE:
		fclose(ym->File);
		break;
	default:
		break;
	}
	ym->FileOps = YMODEM_FILE_NOOPS;
	return Ret;
}

static YMRESULT __Ymodem_ReceiveProcess(tagYmodem *ym)
{
	YMRESULT Ret = YMODEM_OK;
	s64 CurrentTime;

	while(1)
	{
		Ret = __Ymodem_GetPkg(ym);
		if(Ret != YMODEM_OK)
		{
			__Ymodem_CancelTrans();
			break;
		}
		CurrentTime = DjyGetSysTime();					//�ܳ�ʱ����
		if(CurrentTime - ym->StartTime >= ym->TimeOut)
		{
			Ret = YMODEM_TIMEOUT;
			__Ymodem_CancelTrans();
			break;
		}
		//���ݰ�����
		switch(ym->PkgBuf[0])
		{
		case CN_YMODEM_SOH:
			if(!__Ymodem_PackCheck(ym->PkgBuf,ym->PkgSize))	//need retry load
				break;
			if(ym->Status == ENUM_YMODEM_STA_INFO)			//info pkg
			{
				__Ymodem_InfoPkg(ym);
			}
			else if(ym->Status == CN_YMODEM_EOT)
			{
				if(__Ymodem_IsZeroPkg(ym))
				{
					__Ymodem_Put(CN_YMODEM_ACK);//ȫ��������д������
					goto YMODEM_RECVEXIT;
				}
				else
				{
					__Ymodem_InfoPkg(ym);		//��������һ���ļ�
				}
			}
			else							//�յ�128�ֽڴ�С�����ݰ�
			{
	            if(ym->PkgBuf[1] == (ym->PkgNo & 0xff))
	            {
	            	ym->PkgNo ++;
	            	ym->FileOps = YMODEM_FILE_WRITE;
	            }
	            else
	            {
	            	__Ymodem_Put(CN_YMODEM_NAK);	//���Ŵ������ش�
	            }
	            ym->Status = CN_YMODEM_SOH;
			}
			break;
		case CN_YMODEM_STX:
			if(!__Ymodem_PackCheck(ym->PkgBuf,ym->PkgSize))	//need retry load
				break;
            if(ym->PkgBuf[1] == (ym->PkgNo & 0xff))
            {
            	ym->PkgNo ++;
            	ym->FileOps = YMODEM_FILE_WRITE;
            }
            else
            {
            	__Ymodem_Put(CN_YMODEM_NAK);//���Ŵ������ش�
            }
            ym->Status = CN_YMODEM_STX;
			break;
		case CN_YMODEM_EOT:
	        if( (ym->Status == CN_YMODEM_SOH) || (ym->Status == CN_YMODEM_STX))	//	��һ��EOT
	        {
	        	__Ymodem_Put(CN_YMODEM_NAK);                         //���յ����������ظ�ACK
	        	ym->Status = CN_YMODEM_EOT;
	        }
	        else if(ym->Status == CN_YMODEM_EOT)
	        {
	        	__Ymodem_Put(CN_YMODEM_ACK);                         //���յ����������ظ�ACK
	        	__Ymodem_Put(CN_YMODEM_C);                           //���յ����������ظ�C
	        }
			break;
		case CN_YMODEM_CAN:
        	Ret = YMODEM_CAN_TRANS;
        	goto YMODEM_RECVEXIT;
			break;
		default:
        	Ret = YMODEM_UNKNOW_ERR;
			break;
		}
		ym->PkgBufCnt = 0;

		Ret = __Ymodem_FileOps(ym,1);
		if(Ret != YMODEM_OK)			//������� 1.case 2.file ops
		{
			__Ymodem_CancelTrans();
			break;
		}
	}
YMODEM_RECVEXIT:
	ym->FileOps = YMODEM_FILE_CLOSE;
	__Ymodem_FileOps(ym,1);					//close file
	return Ret;
}

// ============================================================================
// ���ܣ�Ymodem�����ļ����޲�������Ϊ�ļ����Ѿ���ymodemЭ������������
// �������ޣ���ͨ��shell���õ�ǰ·�������ߵ���Ymodem_PathSet����·��
// ���أ�true,���سɹ�������ʧ��
// ============================================================================
bool_t Ymodem_DownloadFile(char *Param)
{
	YMRESULT Ret = YMODEM_OK;
    u32 CntOver = 0;

    if(NULL == pYmodem)
    {
    	Ret = YMODEM_MEM_ERR;
    	goto YMODEM_EXIT;
    }
    Lock_MutexPend(pYmodem->pYmodemMutex,CN_TIMEOUT_FOREVER);

    pYmodem->File 	= NULL;
    pYmodem->FileOps  = YMODEM_FILE_NOOPS;
    pYmodem->FileSize = 0;
    pYmodem->FileCnt  = 0;
    pYmodem->PkgNo    = 0;
    pYmodem->PkgSize  = CN_YMODEM_SOH_SIZE;
    pYmodem->TimeOut  = gYmodemTotalTimeOut;
    pYmodem->PkgBufCnt = 0;
    pYmodem->FileBufCnt = 0;
    pYmodem->Status   = ENUM_YMODEM_STA_INFO;
    pYmodem->FileBuf  = (u8*)malloc((gYmodemBufPkgNum + 1)*1024);
    pYmodem->PkgBuf   = (u8*)malloc(CN_YMODEM_PKGBUF_SIZE);

    if( (NULL == pYmodem->FileBuf) || (NULL == pYmodem->PkgBuf)  )
    {
    	Ret = YMODEM_MEM_ERR;
    	goto YMODEM_EXIT;
    }

    __Ymodem_Put(CN_YMODEM_C);

    //�ȴ������������ݣ���ʱ����
    printf("���ص���ʱ��     ");
    while(!__Ymodem_Gets(pYmodem->PkgBuf,1,1000*mS))
    {
        if (CntOver++ < 60)
        {
        	__Ymodem_Put(CN_YMODEM_C); //��ʱ�����·���C
            printf("\b\b\b\b\b%2dS ",60-CntOver);
            continue;
        }
        else
        {
        	Ret = YMODEM_TIMEOUT;
        	goto YMODEM_EXIT;
        }
    }

    pYmodem->PkgBufCnt = 1;
    pYmodem->StartTime = DjyGetSysTime();
    Ret = __Ymodem_ReceiveProcess(pYmodem);

YMODEM_EXIT:
	if( NULL != pYmodem->FileBuf)
		free(pYmodem->FileBuf);
	if( NULL != pYmodem->PkgBuf)
		free(pYmodem->PkgBuf);

	Lock_MutexPost(pYmodem->pYmodemMutex);
	if(Ret != YMODEM_OK)						//��ӡ�����Ϣ
	{
		if(Ret == YMODEM_PARAM_ERR)
		{
			printf("\r\nYMODEM PARAMETER ERR !\r\n");
		}
		else if(Ret == YMODEM_FILE_ERR)
		{
			printf("\r\nYMODEM FILE OPERATION ERR !\r\n");
		}
		else if(Ret == YMODEM_TIMEOUT)
		{
			printf("\r\nYMODEM OPERATION TIMEOUT ERR !\r\n");
		}
		else if(Ret == YMODEM_MEM_ERR)
		{
			printf("\r\nYMODEM NOT ENOUGH MEMORY !\r\n");
		}
		else if(Ret == YMODEM_CAN_TRANS)
		{
			printf("\r\nYMODEM BE CANCELED !\r\n");
		}
		else
		{
			printf("\r\nYMODEM UNKNOW ERR !\r\n");
		}
		return false;
	}
	else
	{
		printf("\r\nYMODEM SUCCESSED !\r\n");
		return true;
	}
}

static YMRESULT __Ymodem_SendProcess(tagYmodem *ym)
{
	YMRESULT Ret = YMODEM_OK;
	s64 CurrentTime;
	u8 Cmd[8];
	u32 temp;
	char *FileName;

	while(1)
	{
		if(ym->PkgBufCnt == 1)
		{
			Cmd[0] = ym->PkgBuf[0];
		}
		else if(!__Ymodem_Get(Cmd))
		{
			Ret = YMODEM_TIMEOUT;
			__Ymodem_CancelTrans();
			break;
		}
		CurrentTime = DjyGetSysTime();				//�ܳ�ʱ����
		if(CurrentTime - ym->StartTime >= ym->TimeOut)
		{
			Ret = YMODEM_TIMEOUT;
			__Ymodem_CancelTrans();
			break;
		}
		//��ʼ�������ݰ�
		switch(Cmd[0])
		{
		case CN_YMODEM_C:
			if(ym->Status == ENUM_YMODEM_STA_INFO )			//д�װ�
			{
				FileName = strrchr(ym->FileName,'\\');
				if(NULL == FileName)
					FileName = ym->FileName;
				temp = strlen(FileName);
				ym->PkgSize = CN_YMODEM_SOH_SIZE;
                memset(ym->PkgBuf,0x00,ym->PkgSize + 5);
                ym->PkgBuf[0] = CN_YMODEM_SOH;
                ym->PkgBuf[1] = 0x00;
                ym->PkgBuf[2] = 0xFF;
                memcpy(ym->PkgBuf + 3,FileName,temp);
                ym->PkgBuf[temp + 3] = 0x00;
                itoa(ym->FileSize,(char*)(ym->PkgBuf + temp + 4),10);
                __Ymodem_WriteCrc16(ym->PkgBuf,ym->PkgSize);
                ym->Status = CN_YMODEM_C;		//��ʼ������
                ym->PkgNo = 1;
			}
            else if(ym->PkgNo == 1)		//���͵�һ��
            {
            	ym->Status = ENUM_YMODEM_STA_SOH;
            	ym->FileOps = YMODEM_FILE_READ;
            	ym->PkgNo ++;
            }
            else if(ym->Status == CN_YMODEM_EOT)
            {
            	ym->PkgSize = CN_YMODEM_SOH_SIZE;
                memset(ym->PkgBuf,0x00,ym->PkgSize + 5);
                ym->PkgBuf[0] = CN_YMODEM_SOH;
                ym->PkgBuf[1] = 0x00;
                ym->PkgBuf[2] = 0xFF;
                __Ymodem_WriteCrc16(ym->PkgBuf,ym->PkgSize);
            	ym->Status = ENUM_YMODEM_STA_ZERO;
            }
			break;
		case CN_YMODEM_NAK:						//�ط����ݰ�
			break;
		case CN_YMODEM_CAN:
			Ret = YMODEM_CAN_TRANS;
			break;
		case CN_YMODEM_ACK:
            if( (ym->PkgNo == 1) )				//�ոշ������װ���Ϣ��
            {
            	continue;
            }
            else if(ym->Status == ENUM_YMODEM_STA_ZERO)	//�������
            {
            	Ret = YMODEM_OK;
            	goto YMODEM_SENDEXIT;
            }
            else if(ym->FileCnt >= ym->FileSize)
            {
                __Ymodem_Put(CN_YMODEM_EOT);
                ym->Status = CN_YMODEM_EOT;
                continue;
            }
            else
            {
            	ym->PkgNo ++;
            	ym->Status = ENUM_YMODEM_STA_STX;
            	ym->FileOps = YMODEM_FILE_READ;
            }
			break;
		default:
			Ret = YMODEM_UNKNOW_ERR;
			break;
		}

		ym->PkgBufCnt = 0;
		Ret = __Ymodem_FileOps(ym,0);
		if(Ret == YMODEM_OK)
		{
			temp = __Ymodem_Puts(ym->PkgBuf,ym->PkgSize + 5,CN_TIMEOUT_FOREVER);
			if(temp != ym->PkgSize + 5)
			{
				Ret = YMODEM_MEDIA_ERR;
			}
		}
		if(Ret != YMODEM_OK)		//�������1.case 2.file 3.__Ymodem_Puts
		{
			__Ymodem_CancelTrans();
			break;
		}
	}
YMODEM_SENDEXIT:
	return Ret;
}

// ============================================================================
// ���ܣ�Ymodem����API�������ߴ����ļ�����Ϊ��������ȡ�ļ�
// �������ļ���·�����ļ�����
//      1.��Ymodem_PathSet������ʱ��ʹ�þ���·����
//      2.����ʹ�õ������·������ͨ��shell�ȷ�ʽ�ı�·����
// ���أ�true,���سɹ�������ʧ��
// ============================================================================
bool_t Ymodem_UploadFile(char *Param)
{
	YMRESULT Ret = YMODEM_OK;
    u32 CntOver = 0;
    char *NextParam,*FileName;

    if(NULL == pYmodem)
    {
    	Ret = YMODEM_MEM_ERR;
    	goto YMODEM_EXIT;
    }
    Lock_MutexPend(pYmodem->pYmodemMutex,CN_TIMEOUT_FOREVER);

    FileName = (char *)Sh_GetWord(Param, &NextParam);
    if(NULL == FileName)
    {
    	Ret = YMODEM_PARAM_ERR;
    	goto YMODEM_EXIT;
    }
    __Ymodem_FilePathMerge(pYmodem->FileName,pYmodem->Path,FileName);
    pYmodem->File 	= NULL;
    pYmodem->FileOps  = YMODEM_FILE_OPEN;
    pYmodem->FileSize = 0;
    pYmodem->FileCnt  = 0;
    pYmodem->PkgNo    = 0;
    pYmodem->PkgSize  = CN_YMODEM_SOH_SIZE;
    pYmodem->TimeOut  = gYmodemTotalTimeOut;
    pYmodem->PkgBufCnt = 0;
    pYmodem->FileBufCnt = 0;
    pYmodem->Status   = ENUM_YMODEM_STA_INFO;
    pYmodem->FileBuf  = NULL;
    pYmodem->PkgBuf   = (u8*)malloc(CN_YMODEM_PKGBUF_SIZE);

    if( (NULL == pYmodem->PkgBuf)  )
    {
    	Ret = YMODEM_MEM_ERR;
    	goto YMODEM_EXIT;
    }
    Ret = __Ymodem_FileOps(pYmodem,0);			//open the file,must be exist
    if(Ret != YMODEM_OK)
    {
    	goto YMODEM_EXIT;
    }
    pYmodem->FileOps  = YMODEM_FILE_STAT;
    Ret = __Ymodem_FileOps(pYmodem,0);			//GET THE FILE SIZE
    if(Ret != YMODEM_OK)
    {
    	goto YMODEM_EXIT;
    }

    printf("���ص���ʱ��     ");
    while(!__Ymodem_Gets(pYmodem->PkgBuf,1,1000*mS))//�ȴ������������ݣ���ʱ����
    {
        if (CntOver++ < 60)
        {
            printf("\b\b\b\b%2d s",60-CntOver);
            continue;
        }
        else
        {
        	Ret = YMODEM_TIMEOUT;
        	goto YMODEM_EXIT;
        }
    }

    pYmodem->PkgBufCnt = 1;
    pYmodem->StartTime = DjyGetSysTime();
    Ret = __Ymodem_SendProcess(pYmodem);

    pYmodem->FileOps = YMODEM_FILE_CLOSE;		//�ر��ļ�
    __Ymodem_FileOps(pYmodem,0);

YMODEM_EXIT:
	if( NULL != pYmodem->FileBuf)
		free(pYmodem->FileBuf);
	if( NULL != pYmodem->PkgBuf)
		free(pYmodem->PkgBuf);

	Lock_MutexPost(pYmodem->pYmodemMutex);
	if(Ret != YMODEM_OK)						//��ӡ�����Ϣ
	{
		if(Ret == YMODEM_PARAM_ERR)
		{
			printf("\r\nYMODEM PARAMETER ERR, PLEASE ENTER FILE NAME !\r\n");
		}
		else if(Ret == YMODEM_FILE_ERR)
		{
			printf("\r\nYMODEM FILE OPERATION ERR !\r\n");
		}
		else if(Ret == YMODEM_TIMEOUT)
		{
			printf("\r\nYMODEM OPERATION TIMEOUT ERR !\r\n");
		}
		else if(Ret == YMODEM_MEM_ERR)
		{
			printf("\r\nYMODEM NOT ENOUGH MEMORY !\r\n");
		}
		else if(Ret == YMODEM_CAN_TRANS)
		{
			printf("\r\nYMODEM BE CANCELED !\r\n");
		}
		else
		{
			printf("YMODEM UNKNOW ERR !\r\n");
		}
		return false;
	}
	else
	{
		printf("\r\nYMODEM SUCCESSED !\r\n");
		return true;
	}

	return true;
}