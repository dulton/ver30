//
// THIS FILE IS PART OF THE Openzoep SOFTWARE SOURCE CODE.
// USE, DISTRIBUTION AND REPRODUCTION OF THIS LIBRARY SOURCE IS
// GOVERNED BY A GPL-STYLE SOURCE LICENSE INCLUDED WITH THIS SOURCE
// IN 'COPYING'. PLEASE READ THESE TERMS BEFORE DISTRIBUTING.
//
// THE Openzoep SOURCE CODE IS (C) COPYRIGHT 2004-2006
// by the Openzoep.Org Foundation http://www.openzoep.org/
//


#include <string.h>
#include <stdio.h>

//#include "global.h"
#include "md5.h"
#include "digcalc.h"

void CvtHex(
    IN HASH Bin,
    OUT HASHHEX Hex
)
{
    unsigned short i;
    unsigned char j;

    for (i = 0; i < HASHLEN; i++)
    {
        j = (Bin[i] >> 4) & 0xf;
        if (j <= 9)
            Hex[i*2] = (j + '0');
        else
            Hex[i*2] = (j + 'a' - 10);
        j = Bin[i] & 0xf;
        if (j <= 9)
            Hex[i*2+1] = (j + '0');
        else
            Hex[i*2+1] = (j + 'a' - 10);
    };
    Hex[HASHHEXLEN] = '\0';
};

/* calculate H(A1) as per spec */
void DigestCalcHA1(
    IN char * pszAlg,
    IN char * pszUserName,
    IN char * pszRealm,
    IN char * pszPassword,
    IN char * pszNonce,
    IN char * pszCNonce,
    OUT HASHHEX SessionKey
)
{

    MD5_CTX md5;

    MD5Init(&md5, 0);
    MD5Update(&md5, (unsigned char*)pszUserName, strlen(pszUserName));
    MD5Update(&md5, (unsigned char*)":", 1);
    MD5Update(&md5, (unsigned char*)pszRealm, strlen(pszRealm));
    MD5Update(&md5, (unsigned char*)":", 1);
    MD5Update(&md5, (unsigned char*)pszPassword, strlen(pszPassword));
    MD5Final(&md5);

    CvtHex((char*)md5.digest, SessionKey);

    if (strcasecmp(pszAlg, MD_5_SESS_ALGORITHM) == 0)
    {
        MD5_CTX md5;

        MD5Init(&md5, 0);
        MD5Update(&md5, (unsigned char*) SessionKey, HASHHEXLEN);
        MD5Update(&md5, (unsigned char*)":", 1);
        MD5Update(&md5, (unsigned char*)pszNonce, strlen(pszNonce));
        MD5Update(&md5, (unsigned char*)":", 1);
        MD5Update(&md5, (unsigned char*)pszCNonce, strlen(pszCNonce));
        MD5Final(&md5);

        CvtHex((char*)md5.digest, SessionKey);
    }
};

/* calculate request-digest/response-digest as per HTTP Digest spec */
void DigestCalcResponse(
    IN HASHHEX HA1,           /* H(A1) */
    IN char * pszNonce,       /* nonce from server */
    IN char * pszNonceCount,  /* 8 hex digits */
    IN char * pszCNonce,      /* client nonce */
    IN char * pszQop,         /* qop-value: "", "auth", "auth-int" */
    IN char * pszMethod,      /* method from the request */
    IN char * pszDigestUri,   /* requested URL */
    IN HASHHEX HEntity,       /* H(entity body) if qop="auth-int" */
    OUT HASHHEX Response      /* request-digest or response-digest */
)
{
    // calculate H2
    HASHHEX H2;
    MD5_CTX md5;

    MD5Init(&md5, 0);
    MD5Update(&md5, (unsigned char*)pszMethod, strlen(pszMethod));
    MD5Update(&md5, (unsigned char*)":", 1);
    MD5Update(&md5, (unsigned char*)pszDigestUri, strlen(pszDigestUri));

    if (strcasecmp(pszQop, QOP_AUTH_INT) == 0 )
    {
        MD5Update(&md5, (unsigned char*)":", 1);
        MD5Update(&md5, (unsigned char*)HEntity, HASHHEXLEN);
    }

    MD5Final(&md5);
    CvtHex((char*)md5.digest, H2);

    // calculate response
    MD5Init(&md5, 0);
    MD5Update(&md5, (unsigned char*)HA1, HASHHEXLEN);
    MD5Update(&md5, (unsigned char*)":", 1);
    MD5Update(&md5, (unsigned char*)pszNonce, strlen(pszNonce));

    if (*pszQop != '\0')
    {
        MD5Update(&md5, (unsigned char*)":", 1);
        MD5Update(&md5, (unsigned char*)pszNonceCount, strlen(pszNonceCount));
        MD5Update(&md5, (unsigned char*)":", 1);
        MD5Update(&md5, (unsigned char*)pszCNonce, strlen(pszCNonce));
        MD5Update(&md5, (unsigned char*)":", 1);
        MD5Update(&md5, (unsigned char*)pszQop, strlen(pszQop));
    }

    MD5Update(&md5, (unsigned char*)":", 1);
    MD5Update(&md5, (unsigned char*)H2, HASHHEXLEN);
    MD5Final(&md5);

    CvtHex((char*)md5.digest, Response);
};
