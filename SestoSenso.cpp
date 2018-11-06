//
//  nexdome.cpp
//  NexDome X2 plugin
//
//  Created by Rodolphe Pineau on 6/11/2016.


#include "SestoSenso.h"
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <memory.h>
#include <string.h>
#include <time.h>
#ifdef SB_MAC_BUILD
#include <unistd.h>
#endif


CSestoSenso::CSestoSenso()
{
    m_nTargetPos = 0;
    m_nMaxPosLimit = 0;
    m_nMaxPosLimit = 2097152;
    m_bAbborted = false;
    m_pSerx = NULL;

#ifdef SESTO_DEBUG
#if defined(SB_WIN_BUILD)
    m_sLogfilePath = getenv("HOMEDRIVE");
    m_sLogfilePath += getenv("HOMEPATH");
    m_sLogfilePath += "\\SestoSensoLog.txt";
#elif defined(SB_LINUX_BUILD)
    m_sLogfilePath = getenv("HOME");
    m_sLogfilePath += "/SestoSensoLog.txt";
#elif defined(SB_MAC_BUILD)
    m_sLogfilePath = getenv("HOME");
    m_sLogfilePath += "/SestoSensoLog.txt";
#endif
    Logfile = fopen(m_sLogfilePath.c_str(), "w");
#endif

#if defined SESTO_DEBUG && SESTO_DEBUG >= 2
    ltime = time(NULL);
    timestamp = asctime(localtime(&ltime));
    timestamp[strlen(timestamp) - 1] = 0;
    fprintf(Logfile, "[%s] [CSestoSenso] Constructor Called\n", timestamp);
    fflush(Logfile);
#endif

}

CSestoSenso::~CSestoSenso()
{
#ifdef	SESTO_DEBUG
	// Close LogFile
	if (Logfile)
        fclose(Logfile);
#endif
}

int CSestoSenso::Connect(const char *pszPort)
{
    int nErr = SENSO_OK;

    if(!m_pSerx)
        return ERR_COMMNOLINK;

#ifdef SESTO_DEBUG
	ltime = time(NULL);
	timestamp = asctime(localtime(&ltime));
	timestamp[strlen(timestamp) - 1] = 0;
	fprintf(Logfile, "[%s] [CSestoSenso::Connect] Called %s\n", timestamp, pszPort);
	fflush(Logfile);
#endif

    // 9600 8N1
    if(m_pSerx->open(pszPort, 9600, SerXInterface::B_NOPARITY, "-DTR_CONTROL 1") == 0)
        m_bIsConnected = true;
    else
        m_bIsConnected = false;

    if(!m_bIsConnected)
        return ERR_COMMNOLINK;

#ifdef SESTO_DEBUG
	ltime = time(NULL);
	timestamp = asctime(localtime(&ltime));
	timestamp[strlen(timestamp) - 1] = 0;
	fprintf(Logfile, "[%s] [CSestoSenso::Connect] connected to %s\n", timestamp, pszPort);
	fflush(Logfile);
#endif
	
    nErr = getFirmwareVersion(m_szFirmwareVersion, SERIAL_BUFFER_SIZE);
    if(nErr)
        nErr = ERR_COMMNOLINK;

    nErr = getCurrentValues();

    return nErr;
}

void CSestoSenso::Disconnect()
{
    int nErr = SENSO_OK;
    char szResp[SERIAL_BUFFER_SIZE];

    // save params and position on disconnect
    nErr = SestoSensoCommand("#PS!", szResp, SERIAL_BUFFER_SIZE);
    // nErr = SestoSensoCommand("#PC!", szResp, SERIAL_BUFFER_SIZE);

    // set to free as we're no longer controlling the focuser.
    nErr = SestoSensoCommand("#MF!", szResp, SERIAL_BUFFER_SIZE);

    if(m_bIsConnected && m_pSerx)
        m_pSerx->close();
 
	m_bIsConnected = false;
}

#pragma mark move commands
int CSestoSenso::haltFocuser()
{
    int nErr;
    char szResp[SERIAL_BUFFER_SIZE];

	if(!m_bIsConnected)
		return ERR_COMMNOLINK;

    // abort
    nErr = SestoSensoCommand("#MA!", szResp, SERIAL_BUFFER_SIZE);
    if(nErr)
        return nErr;
    if(!strstr(szResp, "ok")) {
        return ERR_CMDFAILED;
    }

    m_bAbborted = true;
	
	return nErr;
}

int CSestoSenso::gotoPosition(int nPos)
{
    int nErr;
    char szCmd[SERIAL_BUFFER_SIZE];
    char szResp[SERIAL_BUFFER_SIZE];

	if(!m_bIsConnected)
		return ERR_COMMNOLINK;

    if ( nPos < m_nMinPosLimit || nPos > m_nMaxPosLimit)
        return ERR_LIMITSEXCEEDED;

#if defined SESTO_DEBUG && SESTO_DEBUG >= 2
    ltime = time(NULL);
    timestamp = asctime(localtime(&ltime));
    timestamp[strlen(timestamp) - 1] = 0;
    fprintf(Logfile, "[%s] [CSestoSenso::gotoPosition] moving to %d\n", timestamp, nPos);
    fflush(Logfile);
#endif

    sprintf(szCmd,"#GT%d!", nPos);
    nErr = SestoSensoCommand(szCmd, szResp, SERIAL_BUFFER_SIZE, 5); // this returns 5 results for some reason
    if(nErr)
        return nErr;
    if(!strstr(szResp, "ok")) {
        return ERR_CMDFAILED;
    }

    m_nTargetPos = nPos;

    return nErr;
}

int CSestoSenso::moveFastInward(void)
{
    int nErr;
    char szResp[SERIAL_BUFFER_SIZE];

    if(!m_bIsConnected)
        return ERR_COMMNOLINK;

    // abort
    nErr = SestoSensoCommand("#FI!", szResp, SERIAL_BUFFER_SIZE);
    if(nErr)
        return nErr;
    if(!strstr(szResp, "ok")) {
        return ERR_CMDFAILED;
    }

    m_bAbborted = true;

    return nErr;
}

int CSestoSenso::moveSLowInward(void)
{
    int nErr;
    char szResp[SERIAL_BUFFER_SIZE];

    if(!m_bIsConnected)
        return ERR_COMMNOLINK;

    // abort
    nErr = SestoSensoCommand("#SI!", szResp, SERIAL_BUFFER_SIZE);
    if(nErr)
        return nErr;
    if(!strstr(szResp, "ok")) {
        return ERR_CMDFAILED;
    }

    m_bAbborted = true;

    return nErr;
}


int CSestoSenso::moveFastOutward(void)
{
    int nErr;
    char szResp[SERIAL_BUFFER_SIZE];

    if(!m_bIsConnected)
        return ERR_COMMNOLINK;

    // abort
    nErr = SestoSensoCommand("#FO!", szResp, SERIAL_BUFFER_SIZE);
    if(nErr)
        return nErr;
    if(!strstr(szResp, "ok")) {
        return ERR_CMDFAILED;
    }

    m_bAbborted = true;

    return nErr;
}

int CSestoSenso::moveSLowOutward(void)
{
    int nErr;
    char szResp[SERIAL_BUFFER_SIZE];

    if(!m_bIsConnected)
        return ERR_COMMNOLINK;

    // abort
    nErr = SestoSensoCommand("#SO!", szResp, SERIAL_BUFFER_SIZE);
    if(nErr)
        return nErr;
    if(!strstr(szResp, "ok")) {
        return ERR_CMDFAILED;
    }

    m_bAbborted = true;

    return nErr;
}



int CSestoSenso::moveRelativeToPosision(int nSteps)
{
    int nErr;

    if(!m_bIsConnected)
		return ERR_COMMNOLINK;

#if defined SESTO_DEBUG && SESTO_DEBUG >= 2
    ltime = time(NULL);
    timestamp = asctime(localtime(&ltime));
    timestamp[strlen(timestamp) - 1] = 0;
    fprintf(Logfile, "[%s] [CSestoSenso::moveRelativeToPosision] moving by %d steps\n", timestamp, nSteps);
    fflush(Logfile);
#endif

    m_nTargetPos = m_nCurPos + nSteps;
    nErr = gotoPosition(m_nTargetPos);

    return nErr;
}

#pragma mark command complete functions

int CSestoSenso::isGoToComplete(bool &bComplete)
{
    int nErr = SENSO_OK;
	
	if(!m_bIsConnected)
		return ERR_COMMNOLINK;

    getPosition(m_nCurPos);
	if(m_bAbborted) {
		bComplete = true;
		m_nTargetPos = m_nCurPos;
		m_bAbborted = false;
	}
    else if(m_nCurPos == m_nTargetPos)
        bComplete = true;
    else
        bComplete = false;
    return nErr;
}


#pragma mark getters and setters

int CSestoSenso::getFirmwareVersion(char *pszVersion, int nStrMaxLen)
{
    int nErr = SENSO_OK;
    char szResp[SERIAL_BUFFER_SIZE];
	
	if(!m_bIsConnected)
		return ERR_COMMNOLINK;

    if(!m_bIsConnected)
        return NOT_CONNECTED;

    nErr = SestoSensoCommand("#QF!", szResp, SERIAL_BUFFER_SIZE);
    if(nErr)
        return nErr;

    strncpy(pszVersion, szResp, nStrMaxLen);
    return nErr;
}


int CSestoSenso::getDeviceName(char *pzName, int nStrMaxLen)
{
    int nErr = SENSO_OK;
    char szResp[SERIAL_BUFFER_SIZE];
    std::vector<std::string> vFieldsData;
    std::vector<std::string> vNameField;

    if(!m_bIsConnected)
        return ERR_COMMNOLINK;

    if(!m_bIsConnected)
        return NOT_CONNECTED;

    nErr = SestoSensoCommand("#QN!", szResp, SERIAL_BUFFER_SIZE);
    if(nErr)
        return nErr;

    nErr = parseFields(szResp, vFieldsData, ';');
    if(nErr)
        return nErr;
    if(vFieldsData.size()==2) { // name is in 2nd field
        nErr = parseFields(vFieldsData[1].c_str(), vNameField, '!');
        if(nErr)
            return nErr;
    }
    strncpy(pzName, vNameField[0].c_str(), nStrMaxLen);
    return nErr;
}

int CSestoSenso::getTemperature(double &dTemperature)
{
    int nErr = SENSO_OK;
    char szResp[SERIAL_BUFFER_SIZE];
	
	if(!m_bIsConnected)
		return ERR_COMMNOLINK;

    nErr = SestoSensoCommand("#QT!", szResp, SERIAL_BUFFER_SIZE);
    if(nErr)
        return nErr;

    // convert response
    dTemperature = atof(szResp);

    return nErr;
}

int CSestoSenso::getPosition(int &nPosition)
{
    int nErr = SENSO_OK;
    char szResp[SERIAL_BUFFER_SIZE];
	
	if(!m_bIsConnected)
		return ERR_COMMNOLINK;

    nErr = SestoSensoCommand("#QP!", szResp, SERIAL_BUFFER_SIZE);
    if(nErr)
        return nErr;

    // convert response
    nPosition = atoi(szResp);
    m_nCurPos = nPosition;
    return nErr;
}


int CSestoSenso::syncMotorPosition(const int &nPos)
{
    int nErr = SENSO_OK;
    char szCmd[SERIAL_BUFFER_SIZE];
    char szResp[SERIAL_BUFFER_SIZE];

	if(!m_bIsConnected)
		return ERR_COMMNOLINK;

    snprintf(szCmd, SERIAL_BUFFER_SIZE, "#SP%d!", nPos);
    nErr = SestoSensoCommand(szCmd, szResp, SERIAL_BUFFER_SIZE);
    if(nErr)
        return nErr;
    if(!strstr(szResp, "ok"))
        nErr = ERR_CMDFAILED;

    m_nCurPos = nPos;

    return nErr;
}


int CSestoSenso::getMaxPosLimit(int &nLimit)
{
    int nErr = SENSO_OK;
    char szResp[SERIAL_BUFFER_SIZE];
    std::vector<std::string> vFieldsData;

    if(!m_bIsConnected)
        return ERR_COMMNOLINK;

    nErr = SestoSensoCommand("#QM!", szResp, SERIAL_BUFFER_SIZE);
    if(nErr)
        return nErr;

    // convert response
    nErr = parseFields(szResp, vFieldsData, ';');
    if(nErr)
        return nErr;
    if(vFieldsData.size()>=2) {
        nLimit = atoi(vFieldsData[1].c_str());
        m_nMaxPosLimit = nLimit;
    }
    else
        nErr = ERR_CMDFAILED;

    return nErr;
}

int CSestoSenso::setMaxPosLimit(const int &nLimit)
{
    int nErr = SENSO_OK;
    char szCmd[SERIAL_BUFFER_SIZE];
    char szResp[SERIAL_BUFFER_SIZE];

    if(!m_bIsConnected)
        return ERR_COMMNOLINK;

    snprintf(szCmd, SERIAL_BUFFER_SIZE, "#SM;%d!", nLimit);
    nErr = SestoSensoCommand(szCmd, szResp, SERIAL_BUFFER_SIZE);
    if(nErr)
        return nErr;
    if(!strstr(szResp, "ok"))
        nErr = ERR_CMDFAILED;
    else
        m_nMaxPosLimit = nLimit;

    return nErr;
}

int CSestoSenso::getMinPosLimit(int &nLimit)
{
    int nErr = SENSO_OK;
    char szResp[SERIAL_BUFFER_SIZE];
    std::vector<std::string> vFieldsData;

    nLimit = m_nMinPosLimit;

    if(!m_bIsConnected)
        return ERR_COMMNOLINK;

    nErr = SestoSensoCommand("#Qm!", szResp, SERIAL_BUFFER_SIZE);
    if(nErr)
        return nErr;
    // convert response
    nErr = parseFields(szResp, vFieldsData, ';');
    if(nErr)
        return nErr;
    if(vFieldsData.size()>=2) {
        nLimit = atoi(vFieldsData[1].c_str());
        m_nMinPosLimit = nLimit;
    }
    else
        nErr = ERR_CMDFAILED;

    return nErr;
}

int CSestoSenso::setMinPosLimit(const int &nLimit)
{
    int nErr = SENSO_OK;
    char szCmd[SERIAL_BUFFER_SIZE];
    char szResp[SERIAL_BUFFER_SIZE];


    if(!m_bIsConnected)
        return ERR_COMMNOLINK;

    snprintf(szCmd, SERIAL_BUFFER_SIZE, "#Sm;%d!", nLimit);
    nErr = SestoSensoCommand(szCmd, szResp, SERIAL_BUFFER_SIZE);
    if(nErr)
        return nErr;
    if(!strstr(szResp, "ok"))
        nErr = ERR_CMDFAILED;
    else
        m_nMinPosLimit = nLimit;

    return nErr;
}

int CSestoSenso::setCurrentPosAsMax()
{
    int nErr = SENSO_OK;
    char szResp[SERIAL_BUFFER_SIZE];


    if(!m_bIsConnected)
        return ERR_COMMNOLINK;

    nErr = SestoSensoCommand("#SM!", szResp, SERIAL_BUFFER_SIZE);
    if(nErr)
        return nErr;
    if(!strstr(szResp, "ok"))
        nErr = ERR_CMDFAILED;

    return nErr;
}

// Current
void CSestoSenso::getHoldCurrent(int &nValue)
{
    nValue = m_SensoParams.nHoldCurrent;
}

void CSestoSenso::setHoldCurrent(const int &nValue)
{
    m_SensoParams.nHoldCurrent = nValue;
}

void CSestoSenso::getRunCurrent(int &nValue)
{
    nValue = m_SensoParams.nRunCurrent;
}

void CSestoSenso::setRunCurrent(const int &nValue)
{
    m_SensoParams.nRunCurrent = nValue;
}

void CSestoSenso::getAccCurrent(int &nValue)
{
    nValue = m_SensoParams.nAccCurrent;
}

void CSestoSenso::setAccCurrent(const int &nValue)
{
    m_SensoParams.nAccCurrent = nValue;
}

void CSestoSenso::getDecCurrent(int &nValue)
{
    nValue = m_SensoParams.nDecCurrent;
}

void CSestoSenso::setDecCurrent(const int &nValue)
{
    m_SensoParams.nDecCurrent = nValue;
}

// Sppeds
void CSestoSenso::getRunSpeed(int &nValue)
{
    nValue = m_SensoParams.nRunSpeed;
}

void CSestoSenso::setRunSpeed(const int &nValue)
{
    m_SensoParams.nRunSpeed = nValue;
}

void CSestoSenso::getAccSpeed(int &nValue)
{
    nValue = m_SensoParams.nAccSpeed;
}

void CSestoSenso::setAccSpeed(const int &nValue)
{
    m_SensoParams.nAccSpeed = nValue;
}

void CSestoSenso::getDecSpeed(int &nValue)
{
    nValue = m_SensoParams.nDecSpeed;
}

void CSestoSenso::setDecSpeed(const int &nValue)
{
    m_SensoParams.nDecSpeed = nValue;
}

int CSestoSenso::saveParams(void)
{
    int nErr = SENSO_OK;

    nErr = setCurrentValues();
    nErr |= setSpeedValues();

    return nErr;
}

int CSestoSenso::saveParamsToMemory(void)
{
    int nErr = SENSO_OK;
    char szResp[SERIAL_BUFFER_SIZE];

    // save params to memory
    nErr = SestoSensoCommand("#PS!", szResp, SERIAL_BUFFER_SIZE);
    if(nErr)
        return nErr;

    if(!strstr(szResp, "ok")) {
        return ERR_CMDFAILED;
    }

    return nErr;
}

int CSestoSenso::resetToDefault(void)
{
    int nErr = SENSO_OK;
    char szResp[SERIAL_BUFFER_SIZE];

    // reset to default
    nErr = SestoSensoCommand("#PD!", szResp, SERIAL_BUFFER_SIZE);
    if(nErr)
        return nErr;

    if(!strstr(szResp, "ok")) {
        return ERR_CMDFAILED;
    }
    nErr = getCurrentValues();
    nErr |= getSpeedValues();
    return nErr;
}

int CSestoSenso::setLockMode(const bool &bLock)
{
    int nErr = SENSO_OK;
    char szResp[SERIAL_BUFFER_SIZE];

    if(bLock){
        // set to free as we're no longer controlling the focuser.
        nErr = SestoSensoCommand("#MF!", szResp, SERIAL_BUFFER_SIZE);

    }
    else {
        // set to free as we're no longer controlling the focuser.
        nErr = SestoSensoCommand("#MF!", szResp, SERIAL_BUFFER_SIZE);
    }

    return nErr;
}


#pragma mark command and response functions

int CSestoSenso::SestoSensoCommand(const char *pszszCmd, char *pszResult, int nResultMaxLen, int nbResults)
{
    int nErr = SENSO_OK;
    char szResp[SERIAL_BUFFER_SIZE];
    unsigned long  ulBytesWrite;
    std::string sResult;
    int i = 0;

	if(!m_bIsConnected)
		return ERR_COMMNOLINK;

    m_pSerx->purgeTxRx();
#if defined SESTO_DEBUG && SESTO_DEBUG >= 3
	ltime = time(NULL);
	timestamp = asctime(localtime(&ltime));
	timestamp[strlen(timestamp) - 1] = 0;
	fprintf(Logfile, "[%s] CSestoSenso::SestoSensoCommand Sending %s\n", timestamp, pszszCmd);
	fflush(Logfile);
#endif
    nErr = m_pSerx->writeFile((void *)pszszCmd, strlen(pszszCmd), ulBytesWrite);
    m_pSerx->flushTx();

    if(nErr){
        return nErr;
    }

    if(pszResult) {
        for(i = 0; i<nbResults; i++) {
            // read response
            nErr = readResponse(szResp, SERIAL_BUFFER_SIZE);
            if(nErr){
#if defined SESTO_DEBUG && SESTO_DEBUG >= 2
                ltime = time(NULL);
                timestamp = asctime(localtime(&ltime));
                timestamp[strlen(timestamp) - 1] = 0;
                fprintf(Logfile, "[%s] CSestoSenso::SestoSensoCommand response Error : %d\n", timestamp, nErr);
                fflush(Logfile);
#endif
                return nErr;
            }
#if defined SESTO_DEBUG && SESTO_DEBUG >= 3
            ltime = time(NULL);
            timestamp = asctime(localtime(&ltime));
            timestamp[strlen(timestamp) - 1] = 0;
            fprintf(Logfile, "[%s] CSestoSenso::SestoSensoCommand response \"%s\"\n", timestamp, szResp);
            fflush(Logfile);
#endif
            sResult += szResp;
        }

        // copy response(s) to result string
        strncpy(pszResult, sResult.c_str(),nResultMaxLen);
    }

    strncpy(pszResult, sResult.c_str(), nResultMaxLen);
    return nErr;
}

int CSestoSenso::readResponse(char *pszRespBuffer, int nBufferLen)
{
    int nErr = SENSO_OK;
    unsigned long ulBytesRead = 0;
    unsigned long ulTotalBytesRead = 0;
    char *pszBufPtr;
	
	if(!m_bIsConnected)
		return ERR_COMMNOLINK;

    memset(pszRespBuffer, 0, (size_t) nBufferLen);
    pszBufPtr = pszRespBuffer;

    do {
        nErr = m_pSerx->readFile(pszBufPtr, 1, ulBytesRead, MAX_TIMEOUT);
        if(nErr) {
#if defined SESTO_DEBUG && SESTO_DEBUG >= 2
            ltime = time(NULL);
            timestamp = asctime(localtime(&ltime));
            timestamp[strlen(timestamp) - 1] = 0;
            fprintf(Logfile, "[%s] [CSestoSenso::readResponse] readFile Error.\n\n", timestamp);
            fflush(Logfile);
#endif
            return nErr;
        }

        if (ulBytesRead !=1) {// timeout
#if defined SESTO_DEBUG && SESTO_DEBUG >= 2
			ltime = time(NULL);
			timestamp = asctime(localtime(&ltime));
			timestamp[strlen(timestamp) - 1] = 0;
			fprintf(Logfile, "[%s] CSestoSenso::readResponse timeout\n", timestamp);
			fflush(Logfile);
#endif
            nErr = ERR_NORESPONSE;
            break;
        }
        ulTotalBytesRead += ulBytesRead;
        // special case
        if (*pszBufPtr == '!') {
            pszBufPtr++;
            break;
        }
    } while (*pszBufPtr++ != '\r' && ulTotalBytesRead < nBufferLen );

    if(ulTotalBytesRead)
        *(pszBufPtr-1) = 0; //remove the \n or the !

    return nErr;
}


int CSestoSenso::parseFields(const char *pszIn, std::vector<std::string> &svFields, char cSeparator)
{
    int nErr = SENSO_OK;
    std::string sSegment;
    std::stringstream ssTmp(pszIn);

    svFields.clear();
    // split the string into vector elements
    while(std::getline(ssTmp, sSegment, cSeparator))
    {
        svFields.push_back(sSegment);
    }

    if(svFields.size()==0) {
        nErr = ERR_BADFORMAT;
    }
    return nErr;
}

#pragma mark parameters command

int CSestoSenso::getCurrentValues(void)
{
    int nErr = SENSO_OK;
    char szResp[SERIAL_BUFFER_SIZE];
    std::vector<std::string> vFieldsData;

    if(!m_bIsConnected)
        return ERR_COMMNOLINK;

    if(!m_bIsConnected)
        return NOT_CONNECTED;

    nErr = SestoSensoCommand("#GC!", szResp, SERIAL_BUFFER_SIZE);
    if(nErr)
        return nErr;

    nErr = parseFields(szResp, vFieldsData, ';');
    if(nErr)
        return nErr;
    if(vFieldsData.size()>=5) {
        m_SensoParams.nHoldCurrent = atoi(vFieldsData[1].c_str());
        m_SensoParams.nRunCurrent = atoi(vFieldsData[2].c_str());
        m_SensoParams.nAccCurrent = atoi(vFieldsData[3].c_str());
        m_SensoParams.nDecCurrent = atoi(vFieldsData[4].c_str());
    }
    return nErr;
}

int CSestoSenso::getSpeedValues(void)
{
    int nErr = SENSO_OK;
    char szResp[SERIAL_BUFFER_SIZE];
    std::vector<std::string> vFieldsData;

    if(!m_bIsConnected)
        return ERR_COMMNOLINK;

    if(!m_bIsConnected)
        return NOT_CONNECTED;

    nErr = SestoSensoCommand("#GS!", szResp, SERIAL_BUFFER_SIZE);
    if(nErr)
        return nErr;

    nErr = parseFields(szResp, vFieldsData, ';');
    if(nErr)
        return nErr;
    if(vFieldsData.size()>=4) {
        m_SensoParams.nAccSpeed = atoi(vFieldsData[1].c_str());
        m_SensoParams.nRunSpeed = atoi(vFieldsData[2].c_str());
        m_SensoParams.nDecSpeed = atoi(vFieldsData[3].c_str());
    }
    return nErr;
}

int CSestoSenso::setCurrentValues(void)
{
    int nErr = SENSO_OK;
    char szResp[SERIAL_BUFFER_SIZE];
    char szCmd[SERIAL_BUFFER_SIZE];

    snprintf(szCmd, SERIAL_BUFFER_SIZE, "#SC;%d;%d;%d;%d!", m_SensoParams.nHoldCurrent,
                                                            m_SensoParams.nRunCurrent,
                                                            m_SensoParams.nAccCurrent,
                                                            m_SensoParams.nDecCurrent );
    nErr = SestoSensoCommand(szCmd, szResp, SERIAL_BUFFER_SIZE);
    if(nErr)
        return nErr;

    if(!strstr(szResp, "ok"))
        nErr = ERR_CMDFAILED;

    return nErr;
}

int CSestoSenso::setSpeedValues(void)
{
    int nErr = SENSO_OK;
    char szResp[SERIAL_BUFFER_SIZE];
    char szCmd[SERIAL_BUFFER_SIZE];
    //
#pragma mark    // TODO : FIX WITH CORRECT COMMAND
    //
    snprintf(szCmd, SERIAL_BUFFER_SIZE, "#SS;%d;%d;%d!",  m_SensoParams.nAccSpeed, m_SensoParams.nRunSpeed, m_SensoParams.nDecSpeed );
    nErr = SestoSensoCommand(szCmd, szResp, SERIAL_BUFFER_SIZE);
    if(nErr)
        return nErr;

    if(!strstr(szResp, "ok"))
        nErr = ERR_CMDFAILED;

    return nErr;
}
