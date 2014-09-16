/* Module:  skypekit.cpp
   Purpose: Simple wrapper for SkypeKit to SKYPE API to maintain compatibility with Skype-Plugins
            Derived from imo2skypeapi.c
   Author:  leecher
   Date:    21.11.2011
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#define strcasecmp stricmp
#define strncasecmp strnicmp
#define vsnprintf _vsnprintf
#define Delay(x) Sleep(x*100)
#define mutex_t CRITICAL_SECTION
#define LockMutex(x) EnterCriticalSection (&x)
#define UnlockMutex(x) LeaveCriticalSection(&x)
#define InitMutex(x) InitializeCriticalSection(&x)
#define ExitMutex(x) DeleteCriticalSection(&x)
#else
#include <unistd.h>
#include <pthread.h>
#define Delay(x) sleep(x)
#define mutex_t pthread_mutex_t
#define LockMutex(x) pthread_mutex_lock(&x)
#define UnlockMutex(x) pthread_mutex_unlock(&x)
#define InitMutex(x)  pthread_mutex_init(&x, NULL);
#define ExitMutex(x) 
#endif
extern "C" {
#include "imo2skypeapi.h"
}
#include "skype-embedded_2.h"

#define PROTVERSION 5	// Better set to 3 to be sure, but as we partly support multichat, can use 5

#define SKYPEKIT_KEY \
"-----BEGIN RSA PRIVATE KEY-----\nMIIBOwIBAAJBANLJhPHhITqQbPklG3ibCVxwGMRfp/v4XqhfdQHdcVfHap6NQ5Wo\nk/4xIA+ui35/MmNartNuC+BdZ1tMuVCPFZcCAwEAAQJAEJ2N+zsR0Xn8/Q6twa4G\n6OB1M1WO+k+ztnX/1SvNeWu8D6GImtupLTYgjZcHufykj09jiHmjHx8u8ZZB/o1N\n" \
"MQIhAPW+eyZo7ay3lMz1V01WVjNKK9QSn1MJlb06h/LuYv9FAiEA25WPedKgVyCW\nSmUwbPw8fnTcpqDWE3yTO3vKcebqMSsCIBF3UmVue8YU3jybC3NxuXq3wNm34R8T\nxVLHwDXh/6NJAiEAl2oHGGLz64BuAfjKrqwz7qMYr9HCLIe/YsoWq/olzScCIQDi\nD2lWusoe2/nEqfDVVWGWlyJ7yOmqaVm/iNUN9B2N2g==" \
"-----END RSA PRIVATE KEY-----\n-----BEGIN CERTIFICATE-----\nMIIB0zCCAX2gAwIBAgIJAI/M7BYjwB+uMA0GCSqGSIb3DQEBBQUAMEUxCzAJBgNV\nBAYTAkFVMRMwEQYDVQQIDApTb21lLVN0YXRlMSEwHwYDVQQKDBhJbnRlcm5ldCBX\naWRnaXRzIFB0eSBMdGQwHhcNMTIwOTEyMjE1MjAyWhcNMTUwOTEyMjE1MjAyWjBF" \
"MQswCQYDVQQGEwJBVTETMBEGA1UECAwKU29tZS1TdGF0ZTEhMB8GA1UECgwYSW50\nZXJuZXQgV2lkZ2l0cyBQdHkgTHRkMFwwDQYJKoZIhvcNAQEBBQADSwAwSAJBANLJ\nhPHhITqQbPklG3ibCVxwGMRfp/v4XqhfdQHdcVfHap6NQ5Wok/4xIA+ui35/MmNa\nrtNuC+BdZ1tMuVCPFZcCAwEAAaNQME4wHQYDVR0OBBYEFJvKs8RfJaXTH08W+SGv" \
"zQyKn0H8MB8GA1UdIwQYMBaAFJvKs8RfJaXTH08W+SGvzQyKn0H8MAwGA1UdEwQF\nMAMBAf8wDQYJKoZIhvcNAQEFBQADQQBJlffJHybjDGxRMqaRmDhX0+6v02TUKZsW\nr5QuVbpQhH6u+0UgcW0jp9QwpxoPTLTWGXEWBBBurxFwiCBhkQ+V\n-----END CERTIFICATE-----"

static void Send(IMOSAPI *pInst, const char *pszMsg, ...);
static const char *MapAvailability(Contact::AVAILABILITY avail);
static const char *MapSendingStatus(Message::SENDING_STATUS sstat);
static const char *MapRank(Participant::RANK rank);
static SEString RemoveHtml(SEString data);

class MySkype;

class MyContact : public Contact
{
public:
  typedef DRef<MyContact, Contact> Ref;
  typedef DRefs<MyContact, Contact> Refs;
  MyContact(unsigned int oid, MySkype* root);

  void OnChange(int prop);
  bool DoChange(int prop, bool bError = true);
private:
  MySkype *root;
};

class MyContactGroup : public ContactGroup
{
public:
  typedef DRef<MyContactGroup, ContactGroup> Ref;
  typedef DRefs<MyContactGroup, ContactGroup> Refs;
  MyContactGroup(unsigned int oid, MySkype* root);

  MyContact::Refs contactList;
  virtual void OnChange( const ContactRef& contact );
private:
  MySkype *root;

};

class MyAccount : public Account
{
public:
    typedef DRef<MyAccount, Account> Ref;
    MyAccount(unsigned int oid, MySkype* root);
    void OnChange(int prop);
	bool DoChange(int prop, bool bError = true);
	bool loggedIn, loggedOut;
private:
	MySkype *root;
};


class MyParticipant : public Participant
{
public:
	typedef DRef<MyParticipant, Participant> Ref;
	typedef DRefs<MyParticipant, Participant> Refs;
	MyParticipant(unsigned int oid, MySkype* root);
	void OnChange(int prop);
private:
	bool DoChange(int prop);
	MySkype *root;
};

class MyConversation : public Conversation
{
public:
	typedef DRef<MyConversation, Conversation> Ref;
	typedef DRefs<MyConversation, Conversation> Refs;
	MyConversation(unsigned int oid, MySkype* root);
	void OnChange(int prop);
	bool DoChange(int prop, bool bError = true);
private:
	MySkype *root;
};

class MyContactSearch : public ContactSearch
{ 
public:
	typedef DRef<MyContactSearch, ContactSearch> Ref;
	typedef DRefs<MyContactSearch, ContactSearch> Refs;

	MyContactSearch(unsigned int oid, MySkype* root);

	void OnChange(int prop);
	char *pszCmdID;
private:
	bool DoChange(int prop);
	MySkype *root;
};

class MySkype : public Skype
{
public:
  MySkype(IMOSAPI *pPInst) : Skype() { pInst = pPInst; };

  Account*      newAccount(int oid);
  ContactGroup* newContactGroup(int oid);
  Contact*      newContact(int oid);
  Participant*  newParticipant(int oid);
  Conversation* newConversation(int oid);
  ContactSearch*newContactSearch(int oid);
    virtual void OnMessage(
        const Message::Ref& message,
        const bool& changesInboxTimestamp,
        const Message::Ref& supersedesHistoryMessage,
        const Conversation::Ref& conversation); 

  void RefreshContactList();
  IMOSAPI *pInst;
  MyContactGroup::Ref skypeNamesContactGroup;
  MyAccount::Ref account;
  MyContactSearch::Ref search;
};


static void HandleMessage(IMOSAPI *pInst, char *pszMsg);

static const char *m_pszSex[] = {"UNKNOWN", "MALE", "FEMALE"};

struct _tagIMOSAPI
{
    char *pszPass;
	char *pszUser;
    char *pszLogBuf;
	char *pszClientName;
    int cbBuf;
    int iProtocol;
    int iLoginStat;
    IMO2SCB Callback;
	void *pUser;
	FILE *fpLog;
    IMO2SCFG stCfg;
	int iShuttingDown;
	char *pszCmdID;
	char *pszWindowState;

	MySkype* s;
	mutex_t mutex;
};

// -----------------------------------------------------------------------------
// Interface
// -----------------------------------------------------------------------------

IMOSAPI *Imo2S_Init(IMO2SCB Callback, void *pUser, IMO2SCFG stCfg)
{
	IMOSAPI *pInst = (IMOSAPI*)calloc(1, sizeof(IMOSAPI));

	if (!pInst) return NULL;
	if (!(pInst->pszLogBuf = (char*)malloc(pInst->cbBuf=512)))
	{
		Imo2S_Exit(pInst);
		return NULL;
	}
	pInst->Callback = Callback;
	pInst->pUser = pUser;
	pInst->iProtocol = PROTVERSION;
	pInst->pszWindowState = "NORMAL";

	pInst->s = new MySkype(pInst);
	InitMutex(pInst->mutex);
	if (pInst->s->init(SKYPEKIT_KEY, stCfg.szHost, stCfg.sPort, 0, 1) != TransportInterface::OK)
	{
		Imo2S_Exit(pInst);
		return NULL;
	}
	pInst->s->start(); 
	return pInst;
}

// -----------------------------------------------------------------------------

void Imo2S_SetLog (IMOSAPI *pInst, FILE *fpLog)
{
	pInst->fpLog = fpLog;
}

// -----------------------------------------------------------------------------

void Imo2S_Exit (IMOSAPI *pInst)
{
	if (!pInst) return;
	pInst->iShuttingDown = 1;
	if (pInst->fpLog) fprintf (pInst->fpLog, "Imo2S_Exit()\n");
	if (pInst->iLoginStat == 1) Imo2S_Logout(pInst);
	if (pInst->pszPass) free (pInst->pszPass);
	if (pInst->pszLogBuf) free(pInst->pszLogBuf);
	if (pInst->pszClientName) free(pInst->pszClientName);

	pInst->s->stop();
	delete pInst->s;
	ExitMutex(pInst->mutex);

	free (pInst);
}

// -----------------------------------------------------------------------------

int Imo2S_Login (IMOSAPI *pInst, char *pszUser, char *pszPass, char **ppszError)
{
	// In case this module is passing in the original values...
	char *pszLocalUser, *pszLocalPass;

	if (pInst->fpLog) fprintf (pInst->fpLog, "Imo2S_Login(%s, ****)\n", pszUser);
	if (pInst->iLoginStat == 1) return pInst->iLoginStat;
	pszLocalUser = strdup(pszUser);
	if (pInst->pszUser) free (pInst->pszUser);
	pInst->pszUser = pszLocalUser;
	pszLocalPass = strdup(pszPass);
	if (pInst->pszPass) free (pInst->pszPass);
	pInst->pszPass = pszLocalPass;
	Send(pInst, "CONNSTATUS CONNECTING");

	if (pInst->s->GetAccount(pInst->pszUser, pInst->s->account))
	{
        Account::STATUS LoginStatus;
		pInst->s->account->LoginWithPassword(pInst->pszPass, false, true);
		while ( (!pInst->s->account->loggedIn) ) { Delay(1); }; 
		pInst->s->account->GetPropStatus(LoginStatus);
        if (LoginStatus == Account::LOGGED_IN)
		{
			// Populate contact list
			MyContactGroup::Ref skypeNamesContactGroup;
			unsigned int i;
			char szMsg[]="SEARCH FRIENDS";

			Send(pInst, "CONNSTATUS ONLINE");
			pInst->iLoginStat = 1;
			skypeNamesContactGroup = skypeNamesContactGroup;
			pInst->s->GetHardwiredContactGroup(ContactGroup::ALL_BUDDIES, skypeNamesContactGroup);
			skypeNamesContactGroup->GetContacts(skypeNamesContactGroup->contactList);
			fetch(skypeNamesContactGroup->contactList);
			for (i = 0; i < skypeNamesContactGroup->contactList.size(); i++)
			{
				SEString contactName, moodText;
				
				Contact::AVAILABILITY avail;

				if (!skypeNamesContactGroup->contactList[i]->GetPropSkypename(contactName))
					continue;
				
				if (skypeNamesContactGroup->contactList[i]->GetPropAvailability(avail))
				{
					Send (pInst, "USER %s ONLINESTATUS %s", (const char*)contactName, MapAvailability(avail));
					switch (avail)
					{
					case Contact::PENDINGAUTH:
						Send (pInst, "USER %s BUDDYSTATUS 2", (const char*)contactName);
						break;
					case Contact::BLOCKED:
					case Contact::BLOCKED_SKYPEOUT:
						Send (pInst, "USER %s ISBLOCKED TRUE", (const char*)contactName);
						break;
					}
				}
				if (skypeNamesContactGroup->contactList[i]->GetPropMoodText(moodText))
					Send (pInst, "USER %s MOOD_TEXT %s", (const char*)contactName, 
						(const char*)moodText);
			}; 
			pInst->s->RefreshContactList();

			// SEARCH FRIENDS
			HandleMessage (pInst, szMsg);
			return 1;
		}
	}

	if (ppszError) *ppszError = "Account does not exist";
	Send(pInst, "CONNSTATUS OFFLINE");
	return pInst->iLoginStat;
}

// -----------------------------------------------------------------------------

void Imo2S_Logout(IMOSAPI *pInst)
{
	if (pInst->fpLog) fprintf (pInst->fpLog, "Imo2S_Logout()\n");
	pInst->s->account->Logout(false);
	while (!pInst->s->account->loggedOut) { Delay(1); };
	pInst->iLoginStat = 0;
	pInst->s->account->loggedIn = false;
	pInst->s->account->loggedOut = false;
	Send (pInst, "CONNSTATUS OFFLINE");
}

// -----------------------------------------------------------------------------

int Imo2S_Send (IMOSAPI *pInst, char *pszMsg)
{
	char *pszDup = (char*)calloc(1,strlen(pszMsg)+2);
	char *pszRealMsg = pszMsg;

	// Needed so that there always is padding 0 byte at end, therefore no strdup
	strcpy(pszDup, pszMsg);
	if (*pszRealMsg=='#')
	{
		char *p;
		if (p = strchr (pszRealMsg, ' ')) pszRealMsg=p+1;
	}
	if (pInst->fpLog) fprintf (pInst->fpLog, "Imo2S_Send(%s)\n", pszMsg);
	if (strlen(pszRealMsg)>15 && strncasecmp (pszRealMsg, "SET ", 4)== 0 &&
		(strncasecmp (pszRealMsg+4, "USERSTATUS", 10)==0 ||
		 strncasecmp (pszRealMsg+4, "CONNSTATUS", 10)==0))
	{
		if (pInst->iLoginStat == 0)
		{
			if (pInst->pszUser && pInst->pszPass && strncasecmp (pszRealMsg+15, "OFFLINE", 7))
			{
				Imo2S_Login(pInst, pInst->pszUser, pInst->pszPass, NULL);
			}
		}
		else
		{
			LockMutex(pInst->mutex);
			HandleMessage (pInst, pszDup);
			UnlockMutex(pInst->mutex);
			/* if (strncasecmp (pszRealMsg+15, "OFFLINE", 7) == 0)
				Imo2S_Logout(pInst); */
			pInst->pszCmdID = NULL;
			free (pszDup);
			return 0;
		}
	}
	if (pInst->iLoginStat != 1) return -1;
	LockMutex(pInst->mutex);
	HandleMessage(pInst, pszDup);
	pInst->pszCmdID = NULL;
	free (pszDup);
	UnlockMutex(pInst->mutex);
	return 0;
}

// -----------------------------------------------------------------------------
// Static
// -----------------------------------------------------------------------------

// There are no separate events for different account properties -
// this callback will fire for all property changes.

void MySkype::RefreshContactList()
{
	// Keep references to all contacts to that events get fired
	GetHardwiredContactGroup(ContactGroup::ALL_KNOWN_CONTACTS , skypeNamesContactGroup);
	skypeNamesContactGroup->GetContacts(skypeNamesContactGroup->contactList);
	fetch(skypeNamesContactGroup->contactList);
}

void MyAccount::OnChange(int prop)
{
	LockMutex(root->pInst->mutex);
	DoChange(prop);
	UnlockMutex(root->pInst->mutex);
}

bool MyAccount::DoChange(int prop, bool bError)
{
	bool bRet = false;

	switch (prop)
	{
    case Account::P_STATUS:
    {
        Account::STATUS LoginStatus;
		SEString SkypeName;

        if (GetPropStatus(LoginStatus))
		{
			switch (LoginStatus)
			{
			case Account::LOGGED_IN:
			{
				char szMsg[] = "GET USERSTATUS";
				::Send (root->pInst, "CONNSTATUS ONLINE");
				HandleMessage(root->pInst, szMsg);
				GetPropSkypename(SkypeName);
				::Send (root->pInst, "CURRENTUSERHANDLE %s", (const char*)SkypeName);
				loggedIn = true;
				break;
			}
			case Account::LOGGED_OUT:
			{
				char szMsg[] = "GET USERSTATUS";
				HandleMessage(root->pInst, szMsg);
				::Send (root->pInst, "CONNSTATUS OFFLINE");
				loggedOut = true;
				break;
			}
			case Account::CONNECTING_TO_P2P:
			case Account::CONNECTING_TO_SERVER :
				::Send (root->pInst, "CONNSTATUS CONNECTING");
				break;
			}
			return true;
		}
		break;
    }
	case Account::P_SKYPEOUT_BALANCE_CURRENCY:
	{
		SEString str;

		if (bRet = GetPropSkypeoutBalanceCurrency(str))
			::Send (root->pInst, "PROFILE %s %s", "PSTN_BALANCE_CURRENCY", (const char*)str);
		break;
	}
	case Account::P_SKYPEOUT_BALANCE:
	{
		unsigned int i;

		if (bRet = GetPropSkypeoutBalance(i))
			::Send (root->pInst, "PROFILE %s %d", "PSTN_BALANCE", i);
		break;
	}
	case Account::P_FULLNAME:
	{
		SEString str;

		if (bRet = GetPropFullname(str))
			::Send (root->pInst, "PROFILE %s %s", "FULLNAME", (const char*)str);
		break;
	}
	case Account::P_BIRTHDAY:
	{
		unsigned int i = 0;

		GetPropBirthday(i);
		::Send (root->pInst, "PROFILE %s %d", "BIRTHDAY", i);
		return true;
	}
	case Account::P_GENDER:
	{
		unsigned int i;

		if (bRet = GetPropGender(i) && i<3)
			::Send (root->pInst, "PROFILE %s %s", "SEX", ::m_pszSex[i]);
		break;
	}
	case Account::P_LANGUAGES:
	{
		SEString str;

		if (bRet = GetPropLanguages(str))
			::Send (root->pInst, "PROFILE %s %s", "LANGUAGES", (const char*)str);
		break;
	}
	case Account::P_COUNTRY:
	{
		SEString str;

		if (bRet = GetPropCountry(str))
			::Send (root->pInst, "PROFILE %s %s", "COUNTRY", (const char*)str);
		break;
	}
	case Account::P_PROVINCE:
	{
		SEString str;

		if (bRet = GetPropProvince(str))
			::Send (root->pInst, "PROFILE %s %s", "PROVINCE", (const char*)str);
		break;
	}
	case Account::P_CITY:
	{
		SEString str;

		if (bRet = GetPropProvince(str))
			::Send (root->pInst, "PROFILE %s %s", "CITY", (const char*)str);
		break;
	}
	case Account::P_PHONE_HOME :
	{
		SEString str;

		if (bRet = GetPropPhoneHome(str))
			::Send (root->pInst, "PROFILE %s %s", "PHONE_HOME", (const char*)str);
		break;
	}
	case Account::P_PHONE_OFFICE:
	{
		SEString str;

		if (bRet = GetPropPhoneOffice(str))
			::Send (root->pInst, "PROFILE %s %s", "PHONE_OFFICE", (const char*)str);
		break;
	}
	case Account::P_PHONE_MOBILE :
	{
		SEString str;

		if (bRet = GetPropPhoneMobile(str))
			::Send (root->pInst, "PROFILE %s %s", "PHONE_MOBILE", (const char*)str);
		break;
	}
	case Account::P_HOMEPAGE :
	{
		SEString str;

		if (bRet = GetPropHomepage(str))
			::Send (root->pInst, "PROFILE %s %s", "HOMEPAGE", (const char*)str);
		break;
	}
	case Account::P_ABOUT:
	{
		SEString str;

		if (bRet = GetPropAbout(str))
			::Send (root->pInst, "PROFILE %s %s", "ABOUT", (const char*)str);
		break;
	}
	case Account::P_MOOD_TEXT:
	{
		SEString str;

		if (bRet = GetPropMoodText(str))
			::Send (root->pInst, "PROFILE %s %s", "MOOD_TEXT", (const char*)str);
		break;
	}
	case Account::P_TIMEZONE:
	{
		unsigned int i;

		if (bRet = GetPropTimezone(i))
			::Send (root->pInst, "PROFILE %s %d", "TIMEZONE", i);
		break;
	}
	case Account::P_OFFLINE_CALLFORWARD:
	{
		SEString str;

		if (bRet = GetPropMoodText(str))
			::Send (root->pInst, "PROFILE %s %s", "CALL_FORWARD_RULES", (const char*)str);
		break;
	}
	default:
		bError = false;
	}
	if (bError && !bRet)
		::Send (root->pInst, "ERROR 9901 Internal error");
	return bRet;
}

// -----------------------------------------------------------------------------

void MyContactSearch::OnChange(int prop)
{
	LockMutex(root->pInst->mutex);
	DoChange(prop);
	UnlockMutex(root->pInst->mutex);
}

bool MyContactSearch::DoChange(int prop)
{
    if (prop == P_CONTACT_SEARCH_STATUS)
    {
        MyContactSearch::STATUS status;
        this->GetPropContactSearchStatus(status);
        if (status == FINISHED || status == FAILED)
        {
			SEString users;
			ContactRefs co;
			char *pszBak;

			if (GetResults(co))
			{
				int i, nCount = co.size();

				for (i=0; i<nCount; i++)
				{
					SEString id;

					if (co[i]->GetIdentity(id))
					{
						if (!users.isNull()) users+=", ";
						users+=id;
					}
				}
			}

			if (pszCmdID)
			{
				pszBak = this->root->pInst->pszCmdID;
				this->root->pInst->pszCmdID = pszCmdID;
			}
			::Send (this->root->pInst, "USERS %s", (const char*)users);
			if (pszCmdID)
			{
				this->root->pInst->pszCmdID = pszBak;
				this->pszCmdID = NULL;
			}
			Release();
			return true;
        }
    }
	return false;
} 

// -----------------------------------------------------------------------------

void MyContact::OnChange(int prop)
{
	LockMutex(root->pInst->mutex);
	DoChange(prop, false);
	UnlockMutex(root->pInst->mutex);
}

bool MyContact::DoChange(int prop, bool bError)
{
	SEString name;
	bool bRet = false;

	if (GetPropSkypename(name))
	{
		if (!bError)
		{
			SEString handle;

			// If the OnChange event gives us info about ourself, this is not interesting
			root->account->GetPropSkypename (handle);
			if (handle == name) 
				return false;
		}
		switch (prop)
		{
		case Contact::P_SKYPENAME:	// Doesn't make much sense, but for GET requests in Handler...
			::Send (root->pInst, "%s %s %s %s", "USER", (const char*)name, "HANDLE", (const char*)name);
			break;

		case Contact::P_FULLNAME:
		{
			SEString fullname;

			if (bRet = GetPropFullname(fullname))
				::Send (root->pInst, "%s %s %s %s", "USER", (const char*)name, "FULLNAME", (const char*)fullname);
			break;
		}
		case Contact::P_BIRTHDAY:
		{
			uint birthday;

			if (bRet = GetPropBirthday(birthday))
				::Send (root->pInst, "%s %s %s %d", "USER", (const char*)name, "BIRTHDAY", birthday);
			break;
		}
		case Contact::P_GENDER:
		{
			uint gender;

			if (bRet = GetPropGender(gender))
				::Send (root->pInst, "%s %s %s %d", "USER", (const char*)name, "SEX", m_pszSex[gender]);
			break;
		}
		case Contact::P_LANGUAGES:	
		{
			SEString languages;
			char *pszLang, *pLang;

			// FIXME, currently not supported correctly
			// We have to get the language name from the abbreviation here somehow
			// In WIN32, GetLocaleInfo with LOCALE_SABBREVLANGNAME and LOCALE_SENGLANGUAGE may be used.
			// We must distinguish between protocol 4 and below
			if (bRet = GetPropLanguages(languages))
			{
				pszLang = strdup((const char*)languages);
				pLang = strtok(pszLang, " ");
				::Send (root->pInst, "%s %s %s %s", "USER", (const char*)name, "LANGUAGE", pLang);
				free (pszLang);
			}
			break;
		}
		case Contact::P_COUNTRY:	
		{
			SEString country;

			// FIXME, currently not supported correctly
			// We have to get the language name from the abbreviation here somehow
			// In WIN32, GetLocaleInfo with LOCALE_SABBREVLANGNAME and LOCALE_SENGLANGUAGE may be used.
			// We must distinguish between protocol 4 and below
			if (bRet = GetPropCountry(country))
				::Send (root->pInst, "%s %s %s %s", "USER", (const char*)name, "COUNTRY", (const char*)country);
			break;
		}
		case Contact::P_PROVINCE:
		{
			SEString province;

			if (bRet = GetPropProvince(province))
				::Send (root->pInst, "%s %s %s %s", "USER", (const char*)name, "PROVINCE", (const char*)province);
			break;
		}
		case Contact::P_CITY:
		{
			SEString city;

			if (bRet = GetPropCity(city))
				::Send (root->pInst, "%s %s %s %s", "USER", (const char*)name, "CITY", (const char*)city);
			break;
		}
		case Contact::P_PHONE_HOME:
		{
			SEString phone;

			if (bRet = GetPropPhoneHome(phone))
				::Send (root->pInst, "%s %s %s %s", "USER", (const char*)name, "PHONE_HOME", (const char*)phone);
			break;
		}
		case Contact::P_PHONE_OFFICE:
		{
			SEString phone;

			if (bRet = GetPropPhoneOffice(phone))
				::Send (root->pInst, "%s %s %s %s", "USER", (const char*)name, "PHONE_OFFICE", (const char*)phone);
			break;
		}
		case Contact::P_PHONE_MOBILE:
		{
			SEString phone;

			if (bRet = GetPropPhoneMobile(phone))
				::Send (root->pInst, "%s %s %s %s", "USER", (const char*)name, "PHONE_MOBILE", (const char*)phone);
			break;
		}
		case Contact::P_HOMEPAGE:
		{
			SEString homepage;

			if (bRet = GetPropHomepage(homepage))
				::Send (root->pInst, "%s %s %s %s", "USER", (const char*)name, "HOMEPAGE", (const char*)homepage);
			break;
		}
		case Contact::P_ABOUT:
		{
			SEString about;

			if (bRet = GetPropAbout(about))
				::Send (root->pInst, "%s %s %s %s", "USER", (const char*)name, "ABOUT", (const char*)about);
			break;
		}
		case Contact::P_GIVEN_AUTHLEVEL:
		{
			AUTHLEVEL authlevel;

			if (GetPropGivenAuthlevel(authlevel))
			{
				switch (authlevel)
				{
				case AUTHORIZED_BY_ME:
					::Send (root->pInst, "%s %s %s %s", "USER", (const char*)name, "ISAUTHORIZED", "TRUE");
					return true;
				case BLOCKED_BY_ME:
					::Send (root->pInst, "%s %s %s %s", "USER", (const char*)name, "ISBLOCKED", "TRUE");
					return true;
				}
			}
			break;
		}
		case Contact::P_LASTONLINE_TIMESTAMP:
		{
			uint laston;

			if (bRet = GetPropLastonlineTimestamp(laston))
				::Send (root->pInst, "%s %s %s %d", "USER", (const char*)name, "LASTONLINETIMESTAMP", laston);
			break;
		}
		case Contact::P_AVAILABILITY:
		{
			Contact::AVAILABILITY availability;

			if (bRet = GetPropAvailability(availability))
				::Send (root->pInst, "%s %s %s %s", "USER", (const char*)name, "ONLINESTATUS", MapAvailability(availability));
			break;
		}
		case Contact::P_RECEIVED_AUTHREQUEST:
		{
			SEString authtxt;
			bool bRes;

			if (bError || (IsMemberOfHardwiredGroup(ContactGroup::CONTACTS_WAITING_MY_AUTHORIZATION, bRes) && bRes))
			{
				if (bRet = GetPropReceivedAuthrequest(authtxt))
					::Send (root->pInst, "%s %s %s %s", "USER", (const char*)name, "RECEIVEDAUTHREQUEST", (const char*)::RemoveHtml(authtxt));
			}
			break;
		}
		case Contact::P_MOOD_TEXT:
		{
			SEString moodText;

			if (bRet = GetPropMoodText (moodText))
				::Send (root->pInst, "%s %s %s %s", "USER", (const char*)name, "MOOD_TEXT", (const char*)moodText);
			break;
		}
		case Contact::P_RICH_MOOD_TEXT:
		{
			SEString moodText;

			if (bRet = GetPropRichMoodText (moodText))
				::Send (root->pInst, "%s %s %s %s", "USER", (const char*)name, "RICH_MOOD_TEXT", (const char*)moodText);
			break;
		}
		case Contact::P_TIMEZONE:
		{
			uint timezone;

			if (bRet = GetPropTimezone (timezone))
				::Send (root->pInst, "%s %s %s %d", "USER", (const char*)name, "TIMEZONE", timezone);
			break;
		}
		case Contact::P_NROF_AUTHED_BUDDIES:
		{
			uint nrauthed;

			if (bRet = GetPropNrofAuthedBuddies (nrauthed))
				::Send (root->pInst, "%s %s %s %d", "USER", (const char*)name, "NROF_AUTHED_BUDDIES", nrauthed);
			break;
		}
		case Contact::P_DISPLAYNAME:
		{
			SEString displayName;

			if (bRet = GetPropDisplayname (displayName))
				::Send (root->pInst, "%s %s %s %s", "USER", (const char*)name, "DISPLAYNAME", (const char*)displayName);
			break;
		}
		case Contact::P_AUTHREQUEST_COUNT:
		{
			char szMsg[256];

			sprintf (szMsg, "GET USER %s BUDDYSTATUS", (const char*)name);
			HandleMessage(root->pInst, szMsg);
			break;
		}
		/*
		case Contact::P_AUTHREQ_TIMESTAMP:
		{
			DoChange(Contact::P_RECEIVED_AUTHREQUEST, false);
			break;
		}
		*/
		default:
			bError = false;
		}
	}
	if (bError && !bRet)
		::Send (root->pInst, "ERROR 9901 Internal error");
	return bRet;
}

// -----------------------------------------------------------------------------


void MyContactGroup::OnChange(const ContactRef& contact)
{
  ContactGroup::TYPE groupType;
  this->GetPropType(groupType);

  if (groupType == ContactGroup::SKYPE_BUDDIES)
  {
    SEString contactName;
    contact->GetPropDisplayname(contactName);

    if (!contactList.contains(contact))
    {
      contactList.append(contact);
      contact.fetch();
    }
    else
    {
      contactList.remove_val(contact);
    };
  };
}

// -----------------------------------------------------------------------------

void MyParticipant::OnChange(int prop)
{
	LockMutex(root->pInst->mutex);
	DoChange(prop);
	UnlockMutex(root->pInst->mutex);
}

bool MyParticipant::DoChange(int prop)
{
	switch (prop)
	{
	case Participant::P_TEXT_STATUS:
		{
			Participant::TEXT_STATUS status;
			SEString name;

			if (GetPropTextStatus(status) && GetPropIdentity(name))
			{
				::Send(root->pInst, "APPLICATION libpurple_typing DATAGRAM %s:1 %s", (const char*)name,
					Participant::WRITING || Participant::WRITING_AS_ANGRY?"PURPLE_TYPING":"PURPLE_NOT_TYPING");
				return true;
			}
			break;
		}
	case Participant::P_RANK:
		{
			Participant::RANK rank;

			if (GetPropRank(rank))
			{
				::Send(root->pInst, "CHATMEMBER %d ROLE %s", getOID(), ::MapRank(rank));
				return true;
			}
			break;
		}
	}
	return false;
}

// -----------------------------------------------------------------------------

void MyConversation::OnChange(int prop)
{
	LockMutex(root->pInst->mutex);
	DoChange(prop, false);
	UnlockMutex(root->pInst->mutex);
}


bool MyConversation::DoChange(int prop, bool bError)
{
	SEString name;
	bool bRet = false;

	if (GetPropIdentity(name))
	{
		switch (prop)
		{
		case Conversation::P_IDENTITY:
			::Send (root->pInst, "CHAT %s %s %s", (const char*)name, "NAME", (const char*)name);
			return true;
		case Conversation::P_TYPE:
		{
			Conversation::TYPE type;
			char *pszTyp = "DIALOG";

			if (bRet = GetPropType(type))
			{
				switch (type)
				{
				case Conversation::DIALOG: pszTyp="DIALOG"; break;
				case Conversation::TERMINATED_CONFERENCE:
				case Conversation::CONFERENCE: pszTyp="MULTICHAT"; break;
				}
				::Send (root->pInst, "CHAT %s %s %s", (const char*)name, "TYPE", pszTyp);
			}
			break;
		}
		/* P_LIVE currently missing as no call support */
		case Conversation::P_IS_BOOKMARKED:
		{
			bool bBookmarked = false;

			if (bRet = GetPropIsBookmarked(bBookmarked))
				::Send (root->pInst, "CHAT %s %s %s", (const char*)name, "BOOKMARKED", bBookmarked?"TRUE":"FALSE");
			break;
		}
		case Conversation::P_DISPLAYNAME:
		{
			SEString descr;

			if (bRet = GetPropDisplayname(descr))
				::Send(root->pInst, "CHAT %s %s %s", (const char*)name, "FRIENDLYNAME", (const char*)descr);
			break;
		}
		case Conversation::P_LAST_ACTIVITY_TIMESTAMP:
		{
			unsigned int ts;

			if (bRet = GetPropLastActivityTimestamp(ts))
				::Send (root->pInst, "CHAT %s %s %d", (const char*)name, "ACTIVITY_TIMESTAMP", ts);
			break;
		}
		case Conversation::P_CREATION_TIMESTAMP:
		{
			unsigned int ts;

			if (bRet = GetPropCreationTimestamp(ts))
				::Send (root->pInst, "CHAT %s %s %d", (const char*)name, "TIMESTAMP", ts);
			break;
		}
		case Conversation::P_META_TOPIC:
		{
			SEString topic;

			if (bRet = GetPropMetaTopic(topic))
				::Send (root->pInst, "CHAT %s %s %s", (const char*)name, "TOPIC", (const char*)::RemoveHtml(topic));
			break;
		}
		case Conversation::P_PASSWORDHINT:
		{
			SEString name;

			if (bRet = GetPropPasswordhint(name))
				::Send (root->pInst, "CHAT %s %s %s", (const char*)name, "PASSWORDHINT", (const char*)name);
			break;
		}
		case Conversation::P_META_GUIDELINES:
		{
			SEString name;

			if (bRet = GetPropMetaGuidelines(name))
				::Send (root->pInst, "CHAT %s %s %s", (const char*)name, "GUIDELINES", (const char*)name);
			break;
		}
		case Conversation::P_MY_STATUS:
		{
			Conversation::MY_STATUS status;

			if (bRet = GetPropMyStatus(status))
			{
				char *pszStatus="";

				switch (status)
				{
				case Conversation::CONNECTING: pszStatus="CONNECTING"; break;
				case Conversation::RETRY_CONNECTING: pszStatus="RETRY_CONNECTING"; break;
				case Conversation::QUEUED_TO_ENTER: pszStatus="QUEUED_BECAUSE_CHAT_IS_FULL"; break;
				case Conversation::APPLICANT: pszStatus="WAITING_REMOTE_ACCEPT"; break;
				case Conversation::APPLICATION_DENIED: pszStatus="APPLICATION_DENIED"; break;
				case Conversation::INVALID_ACCESS_TOKEN: pszStatus="PASSWORD_REQUIRED"; break;
				case Conversation::CONSUMER: pszStatus="SUBSCRIBED"; break;
				case Conversation::RETIRED_FORCEFULLY: 
					{
						ParticipantRefs participantList;

						pszStatus="KICKED"; 
						if (GetParticipants(participantList, Conversation::MYSELF) &&
							participantList.size()>0)
						{
							Participant::RANK rank;

							if (participantList[0]->GetPropRank(rank) && rank==Participant::OUTLAW)
								pszStatus="BANNED";
						}
						break;
					}
				case Conversation::RETIRED_VOLUNTARILY: pszStatus="UNSUBSCRIBED"; break;
				}
				::Send (root->pInst, "CHAT %s %s %s", (const char*)name, "MYSTATUS", pszStatus);
			}
			break;
		}
		default:
			bError = false;
		}
	}
	if (bError && !bRet)
		::Send (root->pInst, "ERROR 9901 Internal error");
	return bRet;
}

/* Initialize Callbacks */
Account*      MySkype::newAccount(int oid) {return new MyAccount(oid, this);}
ContactGroup* MySkype::newContactGroup(int oid) {return new MyContactGroup(oid, this);}
Contact*      MySkype::newContact(int oid) {return new MyContact(oid, this);}
Participant*  MySkype::newParticipant(int oid)  {return new MyParticipant(oid, this);}
Conversation* MySkype::newConversation(int oid)  {return new MyConversation(oid, this);}
ContactSearch*MySkype::newContactSearch(int oid)  {return new MyContactSearch(oid, this);}

MyContact::MyContact(unsigned int oid, MySkype* root) : Contact(oid, root) { this->root = root; };
MyContactGroup::MyContactGroup(unsigned int oid, MySkype* root) : ContactGroup(oid, root) { this->root = root; };
MyAccount::MyAccount(unsigned int oid, MySkype* root) : Account(oid, root) { this->root = root; loggedIn=false; loggedOut=false; };
MyParticipant::MyParticipant(unsigned int oid, MySkype* root) : Participant(oid, root) { this->root = root; }
MyConversation::MyConversation(unsigned int oid, MySkype* root) : Conversation(oid, root) { this->root = root; }
MyContactSearch::MyContactSearch(unsigned int oid, MySkype* root) : ContactSearch(oid, root) { this->root = root; this->pszCmdID = NULL; }



// -----------------------------------------------------------------------------

void MySkype::OnMessage(
        const Message::Ref& message,
        const bool& changesInboxTimestamp,
        const Message::Ref& supersedesHistoryMessage,
        const Conversation::Ref& conversation)
{
	int MessageType = message->GetProp(Message::P_TYPE).toInt();

	switch (MessageType)
	{
	case Message::POSTED_TEXT:
	case Message::POSTED_EMOTE:
	case Message::SET_METADATA:
	case Message::SPAWNED_CONFERENCE:
	case Message::POSTED_CONTACTS:
	case Message::SET_RANK:
	case Message::BLOCKED:
	case Message::ADDED_APPLICANTS:
	case Message::RETIRED:
	case Message::RETIRED_OTHERS:
	case Message::ADDED_CONSUMERS:

	{
		SEIntList Props;
		SEString Author;

		message->GetPropAuthor(Author);
		if ((MessageType == Message::POSTED_TEXT || MessageType == Message::POSTED_EMOTE) && 
			!strcmp((const char*)Author, pInst->pszUser))
		{
			Message::SENDING_STATUS status;

			message->GetPropSendingStatus(status);
			/* It seems like if the status is still SENDING, we don't get another
			 * event if it is SENT, therefore we have to pretend that it is SENT?? */
			if (status == Message::SENDING) status = Message::SENT;
			Send(pInst, "%s %d %s %s", pInst->iProtocol>=3?"CHATMESSAGE":"MESSAGE", 
				message->getOID(), "STATUS", MapSendingStatus(status));
		
		}
		else
		{
			Send(pInst, "%s %d %s %s", pInst->iProtocol>=3?"CHATMESSAGE":"MESSAGE", 
				message->getOID(), "STATUS", "RECEIVED");
		}
		break;
	}
	}
}; 

// -----------------------------------------------------------------------------

static const char *MapAvailability(Contact::AVAILABILITY avail)
{
	switch (avail)
	{
	case Contact::ONLINE:
	case Contact::ONLINE_FROM_MOBILE:
		return "ONLINE";
	case Contact::OFFLINE:
	case Contact::OFFLINE_BUT_VM_ABLE:
	case Contact::OFFLINE_BUT_CF_ABLE:
	case Contact::PENDINGAUTH:
	case Contact::BLOCKED:
	case Contact::BLOCKED_SKYPEOUT:
		return"OFFLINE";
	case Contact::AWAY:
	case Contact::AWAY_FROM_MOBILE:
		return "AWAY";
	case Contact::NOT_AVAILABLE:
	case Contact::NOT_AVAILABLE_FROM_MOBILE:
		return "NA";
	case Contact::DO_NOT_DISTURB:
	case Contact::DO_NOT_DISTURB_FROM_MOBILE:
		return "DND";
	case Contact::SKYPE_ME:
	case Contact::SKYPE_ME_FROM_MOBILE:
		return "SKYPEME";
	case Contact::SKYPEOUT:
		return "SKYPEOUT";
	case Contact::INVISIBLE:
		return "INVISIBLE";
	}
	return "UNKNOWN";
}

// -----------------------------------------------------------------------------

static const char *MapSendingStatus(Message::SENDING_STATUS sstat)
{
	switch (sstat)
	{
	case Message::SENDING: return "SENDING";
	case Message::SENT: return "SENT";
	case Message::FAILED_TO_SEND: return "FAILED";
	}
	return "";
}

// -----------------------------------------------------------------------------

static const char *MapRank(Participant::RANK rank)
{

	switch (rank)
	{
	case Participant::CREATOR: return "CREATOR";
	case Participant::ADMIN: return "MASTER";
	case Participant::SPEAKER: return "HELPER";
	case Participant::WRITER: return "USER";
	case Participant::SPECTATOR: return "LISTENER";
	case Participant::APPLICANT: return "APPLICANT";
	}
	return "";
}

// -----------------------------------------------------------------------------

struct HtmlEntity
{
	const char *entity;
	const char *symbol;
};

const HtmlEntity htmlEntities[]={
	{"nbsp",	" "},
	{"amp",		"&"},
	{"quot",	"\""},
	{"lt",		"<"},
	{"gt",		">"},
	{"apos",	"'"},
	{"copy",	"©"},
	// TODO: add more
};

static SEString RemoveHtml(SEString data)
{
	SEString new_string = "";
	char cbuf[2]={0};

	if (strstr((const char*)data, "\x1b\xe3\xac\x8d\x1d"))
		data = "CONVERSATION MEMBERS:" + data.substr(5, data.length() - 5);

	for (int i = 0; i < data.length(); i++)
	{
		if (data[i] == '<' && data[i+1] != ' ')
		{
			if ((i = data.find(i, '>')) == -1)
				break;

			continue;
		}

		if (data[i] == '&') {
			int begin = i;
			i = data.find(i, ';');
			if (i == -1) {
				i = begin;
			} else {
				SEString entity = data.substr(begin+1, i-begin-1);

				bool found = false;
				for (int j=0; j<sizeof(htmlEntities)/sizeof(htmlEntities[0]); j++)
				{
					if (!stricmp((const char*)entity, htmlEntities[j].entity)) {
						new_string += htmlEntities[j].symbol;
						found = true;
						break;
					}
				}

				if (found)
					continue;
				else
					i = begin;
			}
		}

		cbuf[0]=data[i];
		new_string += cbuf;
	}

	return new_string;
}

// -----------------------------------------------------------------------------

#ifdef WIN32
static int Dispatcher_Start(IMOSAPI *pInst)
{
	if (pInst->fpLog) fprintf (pInst->fpLog, "Imo2S::Dispatcher_Start()\n");
	return 1;
}

static int Dispatcher_Stop(IMOSAPI *pInst)
{
	if (pInst->fpLog)
	{
		fprintf (pInst->fpLog, "Imo2S::Dispatcher_Stop()\n");
		fflush(pInst->fpLog);
		pInst->fpLog = NULL;
	}
	return 1;
}

#else
static int Dispatcher_Start(IMOSAPI *pInst)
{
	if (pInst->fpLog) fprintf (pInst->fpLog, "Imo2S::Dispatcher_Start()\n");
	return 1;
}

static int Dispatcher_Stop(IMOSAPI *pInst)
{
	if (pInst->fpLog) fprintf (pInst->fpLog, "Imo2S::Dispatcher_Stop()\n");
	return 1;
}
#endif

// -----------------------------------------------------------------------------

static void Send(IMOSAPI *pInst, const char *pszMsg, ...)
{
	va_list ap;
	int iLen, iLenCmdID;
	char *pszLogBuf = pInst->pszLogBuf;
	int cbBuf = pInst->cbBuf;

	iLenCmdID = pInst->pszCmdID?strlen(pInst->pszCmdID)+1:0;
	do
	{
		cbBuf = pInst->cbBuf - iLenCmdID;
		pszLogBuf = pInst->pszLogBuf + iLenCmdID;
		va_start(ap, pszMsg);
		iLen = vsnprintf (pszLogBuf, cbBuf, pszMsg, ap);
		va_end(ap);
#ifndef WIN32
		if (iLen>=cbBuf) iLen=-1;
#endif
		if (iLen == -1)
		{
			char *pNewBuf;
			
			if (!(pNewBuf = (char*)realloc(pInst->pszLogBuf, pInst->cbBuf*2)))
			{
				break;
			}
			pInst->cbBuf*=2;
			pInst->pszLogBuf = pNewBuf;
		}
	} while (iLen == -1);
	if (pInst->pszCmdID && iLenCmdID>1)
	{
		memcpy (pInst->pszLogBuf, pInst->pszCmdID, iLenCmdID);
		pInst->pszLogBuf[iLenCmdID-1]=' ';
	}

//printf ("%s\n", szBuf);
	pInst->Callback(pInst->pszLogBuf, pInst->pUser);
}

// -----------------------------------------------------------------------------

static void HandleSEARCH(IMOSAPI *pInst)
{
	char *pszCmd;

	if (!(pszCmd = strtok(NULL, " ")))
	{
		Send (pInst, "ERROR 2 Invalid command");
		return;
	}
	
	if (strcasecmp(pszCmd, "FRIENDS") == 0 || strcasecmp(pszCmd, "USERSWAITINGMYAUTHORIZATION") == 0)
	{
		ContactGroup::TYPE type;
		ContactGroupRef cg;
		SEString contacts;

		switch (toupper(pszCmd[0]))
		{
		case 'F': type=ContactGroup::ALL_BUDDIES; break;
		case 'U': type=ContactGroup::CONTACTS_WAITING_MY_AUTHORIZATION; break;
		}
		if (pInst->s->GetHardwiredContactGroup(type, cg))
		{
			ContactRefs cr;

			if (cg->GetContacts(cr))
			{
				int i, nCount = cr.size();
				SEString name;

				for (i=0; i<nCount; i++)
				{
					if (!contacts.isNull()) contacts+=", ";
					if (cr[i]->GetIdentity(name)) contacts+=name;
				}
			}
		}
		Send (pInst, "USERS %s", (const char*)contacts);
	}
	else if (strcasecmp(pszCmd, "MISSEDMESSAGES") == 0 || strcasecmp(pszCmd, "MISSEDCHATMESSAGES") == 0)
	{
		ConversationRefs cv;
		SEString messages;

#if 0	/* There are often messages already read in this list, so I better leave it empty for now */
		if (pInst->s->GetConversationList(cv))
		{
			int i, nCount=cv.size();

			for (i=0; i<nCount; i++)
			{
				MessageRefs ct, uc;

				if (cv[i]->GetLastMessages(ct, uc))
				{
					int j, nCountj=uc.size();
					char szID[16];
					Message::CONSUMPTION_STATUS cstat;

					for (j=0; j<nCountj; j++)
					{
						uc[j]->GetPropConsumptionStatus(cstat);
						if (cstat!=Message::CONSUMED)
						{
							if (!messages.isNull()) messages+=", ";
							sprintf (szID, "%d", uc[j]->getOID());
							messages+=szID;
						}
					}
				}
			}
		}
#endif
		Send (pInst, "MESSAGES %s", (const char*)messages);
	}
	else if (strcasecmp(pszCmd, "USERS") == 0)
	{
		bool valid;
		MyContactSearch::Ref search;

		if (!(pszCmd = strtok(NULL, " ")))
		{
			Send (pInst, "ERROR 4 Empty target not allowed");
			return;
		}

		if (pInst->s->CreateBasicContactSearch(pszCmd, search))
		{
			search->pszCmdID = pInst->pszCmdID;
			if (search->IsValid(valid) && valid && search->Submit())
			{
				pInst->s->search = search;
				return;
			}
		}
		Send (pInst, "ERROR 9901 Internal error");
	}
	else if (strcasecmp(pszCmd, "ACTIVECALLS") == 0)
	{
		char szCalls[512];
		int i, nCount, iOffs=0;

		iOffs = sprintf (szCalls, "CALLS");
		/* TODO: Not implemented */
		Send (pInst, szCalls);
	}
	else if (strcasecmp(pszCmd, "BOOKMARKEDCHATS") == 0 || strcasecmp(pszCmd, "CHATS") == 0 ||
		strcasecmp(pszCmd, "ACTIVECHATS") == 0 || strcasecmp(pszCmd, "RECENTCHATS") == 0)
	{
		ConversationRefs cv;
		Conversation::LIST_TYPE type;
		SEString chats;

		switch (toupper(pszCmd[0]))
		{
		case 'B': type = Conversation::BOOKMARKED_CONVERSATIONS; break;
		case 'C': type = Conversation::REALLY_ALL_CONVERSATIONS; break;
		case 'R':
		case 'A': type = Conversation::ALL_CONVERSATIONS; break;
		}
		if (pInst->s->GetConversationList(cv, type))
		{
			int i, nCount=cv.size();
			SEString chat;

			for (i=0; i<nCount; i++)
			{
				if (cv[i]->GetPropIdentity(chat))
				{
					if (!chats.isNull()) chats+=", ";
					chats+=chat;
				}
			}
		}
		Send (pInst, "CHATS %s", (const char*)chats);
	}
	else if (strcasecmp(pszCmd, "FILETRANSFERS") == 0 ||
			 strcasecmp(pszCmd, "ACTIVEFILETRANSFERS") == 0)
	{
		MessageRefs mr;
		SEString files;

		if (pInst->s->GetMessageListByType(Message::POSTED_FILES, false, mr))
		{
			int i, nCount=mr.size();

			for (i=0; i<nCount; i++)
			{
				TransferRefs tr;

				if (mr[i]->GetTransfers(tr))
				{
					int j, nCntTf=tr.size();
					Transfer::STATUS status;
					char szOID[32];

					for (j=0; j<nCntTf; j++)
					{
						if (pszCmd[13] && (!tr[j]->GetPropStatus(status) ||
							status == Transfer::COMPLETED || status == Transfer::CANCELLED ||
							status == Transfer::FAILED))
							continue;

						if (!files.isNull()) files+=", ";
						sprintf(szOID, "%d", tr[j]->getOID());
						files+=szOID;
					}
				}
			}
		}
		Send (pInst, "%s %s", pszCmd, (const char*)files);
	}
	else
	{
		Send(pInst, "ERROR 10 Invalid propery");
	}
	return;
}

// -----------------------------------------------------------------------------

static void HandleGET(IMOSAPI *pInst)
{
	char *pszCmd;

	if (!(pszCmd = strtok(NULL, " ")))
	{
		Send (pInst, "ERROR 7 Invalid property");
		return;
	}

	if (strcasecmp(pszCmd, "USER") == 0)
	{
		MyContact::Ref user;
		bool bFoundContact = false;
		char *pszName;

		if (pszCmd = strtok(NULL, " "))
		{
			bFoundContact = pInst->s->GetContact(pszCmd, user);
		}

		if (!bFoundContact)			
		{
			Send (pInst, "ERROR 26 Invalid user handle");
			return;
		}
		pszName = pszCmd;
		if (!(pszCmd = strtok(NULL, " ")))
		{
			Send (pInst, "ERROR 10 Invalid property");
			return;
		}

		if (!strcasecmp (pszCmd, "HANDLE"))
			user->DoChange(Contact::P_SKYPENAME);
		else if (!strcasecmp (pszCmd, "FULLNAME"))
			user->DoChange(Contact::P_FULLNAME);
		else if (!strcasecmp (pszCmd, "DISPLAYNAME"))
			user->DoChange(Contact::P_DISPLAYNAME);
		else if (!strcasecmp (pszCmd, "BIRTHDAY"))
			user->DoChange(Contact::P_BIRTHDAY);
		else if (!strcasecmp (pszCmd, "SEX"))
			user->DoChange(Contact::P_GENDER);
		else if (!strcasecmp (pszCmd, "LANGUAGE"))
			user->DoChange(Contact::P_LANGUAGES);
		else if (!strcasecmp (pszCmd, "COUNTRY"))
			user->DoChange(Contact::P_COUNTRY);
		else if (!strcasecmp (pszCmd, "PROVINCE"))
			user->DoChange(Contact::P_PROVINCE);
		else if (!strcasecmp (pszCmd, "CITY"))
			user->DoChange(Contact::P_CITY);
		else if (!strcasecmp (pszCmd, "PHONE_HOME"))
			user->DoChange(Contact::P_PHONE_HOME);
		else if (!strcasecmp (pszCmd, "PHONE_OFFICE"))
			user->DoChange(Contact::P_PHONE_OFFICE);
		else if (!strcasecmp (pszCmd, "PHONE_MOBILE"))
			user->DoChange(Contact::P_PHONE_MOBILE);
		else if (!strcasecmp (pszCmd, "HOMEPAGE"))
			user->DoChange(Contact::P_HOMEPAGE);
		else if (!strcasecmp (pszCmd, "ABOUT"))
			user->DoChange(Contact::P_ABOUT);
		else if (!strcasecmp (pszCmd, "HASCALLEQUIPMENT"))
			Send (pInst, "USER %s %s TRUE", pszName, pszCmd);
		else if (!strcasecmp (pszCmd, "IS_VIDEO_CAPABLE"))
		{
			bool bRes = false;

			user->HasCapability(Contact::CAPABILITY_VIDEO, bRes);
			Send (pInst, "USER %s %s %s", pszName, pszCmd, bRes?"FALSE":"TRUE");
		}
		else if (!strcasecmp (pszCmd, "IS_VOICEMAIL_CAPABLE"))
		{
			bool bRes = false;

			user->HasCapability(Contact::CAPABILITY_VOICEMAIL, bRes);
			Send (pInst, "USER %s %s %s", pszName, pszCmd, bRes?"FALSE":"TRUE");
		}
		else if (!strcasecmp (pszCmd, "BUDDYSTATUS"))
		{
			bool bRes = false;
			int status = 3;

			if (user->IsMemberOfHardwiredGroup(ContactGroup::SKYPE_BUDDIES, bRes) && bRes) status = 3;
			else
			{
				if (user->IsMemberOfHardwiredGroup(ContactGroup::CONTACTS_WAITING_MY_AUTHORIZATION, bRes) && bRes) status = 2;
				else
				{
					if (user->IsMemberOfHardwiredGroup(ContactGroup::UNKNOWN_OR_PENDINGAUTH_BUDDIES, bRes) && bRes) status = 0;
					else
					{
						// FIXME: How to find deleted contacts...?
						if (user->IsMemberOfHardwiredGroup(ContactGroup::CONTACTS_BLOCKED_BY_ME, bRes) && bRes) status = 1;
					}
				}
			}
			Send (pInst, "USER %s %s %d", pszName, pszCmd, status);
		}
		else if (!strcasecmp (pszCmd, "ISAUTHORIZED"))
		{
			Contact::AUTHLEVEL authlevel;

			user->GetPropGivenAuthlevel(authlevel);
			Send (pInst, "USER %s %s %s", pszName, pszCmd, 
				authlevel==Contact::AUTHORIZED_BY_ME?"TRUE":"FALSE");
		}
		else if (!strcasecmp (pszCmd, "ISBLOCKED"))
		{
			Contact::AUTHLEVEL authlevel;

			user->GetPropGivenAuthlevel(authlevel);
			Send (pInst, "USER %s %s %s", pszName, pszCmd, 
				authlevel==Contact::BLOCKED_BY_ME?"TRUE":"FALSE");
		}
		else if (!strcasecmp (pszCmd, "ONLINESTATUS"))
		{
			Contact::AVAILABILITY avail;

			if (user->GetPropAvailability(avail))
				Send (pInst, "USER %s %s %s", pszName, pszCmd, MapAvailability(avail));
			else
				Send(pInst, "ERROR 10 Invalid propery");
		}
		else if (!strcasecmp (pszCmd, "LASTONLINETIMESTAMP"))
			user->DoChange(Contact::P_LASTONLINE_TIMESTAMP);
		else if (!strcasecmp (pszCmd, "CAN_LEAVE_VM"))
		{
			bool bRes = false;

			user->HasCapability(Contact::CAPABILITY_CAN_BE_SENT_VM, bRes);
			Send (pInst, "USER %s %s %s", pszName, pszCmd, bRes?"TRUE":"FALSE");
		}
		else if (!strcasecmp (pszCmd, "RECEIVEDAUTHREQUEST"))
			user->DoChange(Contact::P_RECEIVED_AUTHREQUEST);
		else if (!strcasecmp (pszCmd, "MOOD_TEXT"))
			user->DoChange(Contact::P_MOOD_TEXT);
		else if (!strcasecmp (pszCmd, "RICH_MOOD_TEXT"))
			user->DoChange(Contact::P_RICH_MOOD_TEXT);
		else if (!strcasecmp (pszCmd, "TIMEZONE"))
			user->DoChange(Contact::P_TIMEZONE);
		else if (!strcasecmp (pszCmd, "NROF_AUTHED_BUDDIES"))
			user->DoChange(Contact::P_NROF_AUTHED_BUDDIES);
		else if (!strcasecmp (pszCmd, "AVATAR"))
		{
			char *pszFile;
			unsigned int dwLength;
			FILE *fp;
			bool bPresent = false;
			Sid::Binary avatar;

			if (!(pszFile = strtok(NULL, " ")) || strcasecmp (pszFile, "1"))
			{
				Send (pInst, "ERROR 116 GET invalid ID");
				return;
			}
			if (!(pszFile = strtok(NULL, "\n")))
			{
				Send (pInst, "ERROR 7 GET: invalid WHAT");
				return;
			}
			if (fp=fopen(pszFile, "r"))
			{
				fseek (fp, 0, SEEK_END);
				if (ftell(fp))
				{
					fclose(fp);
					Send (pInst, "ERROR 124 GET Destination file is not empty");
					return;
				}
				fclose(fp);
			}
			if (!user->GetAvatar (bPresent, avatar))
			{
				Send (pInst, "ERROR 122 GET Unable to load avatar");
				return;
			}
			if (!(fp=fopen(pszFile, 
#ifdef WIN32
				"wb"
#else
				"w"
#endif
				)))
			{
				Send (pInst, "ERROR 121 GET File path doesn't exist");
				return;
			}
			fwrite (avatar.data(), avatar.size(), 1, fp);
			fclose (fp);
			Send (pInst, "USER %s AVATAR 1 %s", pszName, pszFile);
		}
		else
		{
			// SPEEDDIAL            - N/A?
			// ALIASES              - N/A?
			// IS_CF_ACTIVE         -> check P_AVAILABILITY for OFFLINE_BUT_CF_ABLE ??
			Send(pInst, "ERROR 10 Invalid propery");
		}
		return;
	}
	else
	if (strcasecmp(pszCmd, "CURRENTUSERHANDLE") == 0)
	{
		SEString handle;

		if (pInst->s->account->GetPropSkypename (handle))
			Send(pInst, "CURRENTUSERHANDLE %s", (const char*)handle);
		else
			Send(pInst, "ERROR 10 Invalid propery");
	}
	else
	if (strcasecmp(pszCmd, "USERSTATUS") == 0)
	{
		Contact::AVAILABILITY avail;

		pInst->s->account->GetPropAvailability(avail);
		Send (pInst, "USERSTATUS %s", MapAvailability(avail));
	}
	else
	if (strcasecmp(pszCmd, "MESSAGE") == 0 || strcasecmp(pszCmd, "CHATMESSAGE") == 0)
	{
		MessageRef message;
		char *pszMessage = pszCmd, *pszMsgId;

		if (!(pszMsgId = strtok(NULL, " ")) || !(message = MessageRef(atol(pszMsgId)))->getOID())
		{
			Send (pInst, "ERROR 14 Invalid message id");
			return;
		}
		if (!(pszCmd = strtok(NULL, " ")))
		{
			Send (pInst, "ERROR 10 Invalid property");
			return;
		}
		if (!strcasecmp (pszCmd, "TIMESTAMP"))
		{
			uint ts;

			if (message->GetPropTimestamp(ts))
			{
				Send (pInst, "%s %s %s %ld", pszMessage, pszMsgId, pszCmd, ts);
				return;
			}
		}
		else if (!strcasecmp (pszCmd, "PARTNER_HANDLE") || !strcasecmp (pszCmd, "FROM_HANDLE"))
		{
			SEString author;

			if (message->GetPropAuthor(author))
			{
				Send (pInst, "%s %s %s %s", pszMessage, pszMsgId, pszCmd, (const char*)author);
				return;
			}
		}
		else if (!strcasecmp (pszCmd, "PARTNER_DISPNAME") || !strcasecmp (pszCmd, "FROM_DISPNAME"))
		{
			SEString author;

			if (message->GetPropAuthorDisplayname(author))
			{
				Send (pInst, "%s %s %s %s", pszMessage, pszMsgId, pszCmd, (const char*)author);
				return;
			}
		}
		else if (!strcasecmp (pszCmd, "USERS"))
		{
			SEString users;

			if (message->GetPropIdentities(users))
			{
				Send (pInst, "%s %s %s %s", pszMessage, pszMsgId, pszCmd, (const char*)users);
				return;
			}
		}
		else if (!strcasecmp (pszCmd, "TYPE"))
		{
			Message::TYPE type;

			if (message->GetPropType(type))
			{
				char *pszType = "UNKNOWN";

				switch (type)
				{
				default:
				case Message::SET_METADATA:
					{
						unsigned int paramkey;

						if (message->GetPropParamKey(paramkey))
						{
							switch (paramkey)
							{
							case Message::SET_META_TOPIC: pszType="SETTOPIC"; break;
							case Message::SET_META_GUIDELINES: pszType="SETGUIDELINES"; break;
							case Message::SET_META_PICTURE: pszType="SETPICTURE"; break;
							}
						}
						break;
					}
				case Message::SPAWNED_CONFERENCE: pszType="CREATEDCHATWITH"; break;
				case Message::POSTED_CONTACTS: pszType="POSTEDCONTACTS"; break;
				case Message::SET_RANK: pszType="SETROLE"; break;
				case Message::BLOCKED: pszType="KICKBANNED"; break;
				case Message::ADDED_APPLICANTS: pszType="JOINEDASAPPLICANT"; break;
				case Message::POSTED_TEXT: pszType="TEXT"; break;
				case Message::RETIRED: pszType="LEFT"; break;
				case Message::RETIRED_OTHERS: pszType="KICKED"; break;
				case Message::ADDED_CONSUMERS: pszType="ADDEDMEMBERS"; break;
				}
				Send (pInst, "%s %s %s %s", pszMessage, pszMsgId, pszCmd, pszType);
				return;
			}
		}
		else if (!strcasecmp (pszCmd, "STATUS"))
		{
			Message::SENDING_STATUS sstat;
			Message::CONSUMPTION_STATUS cstat;

			if (message->GetPropSendingStatus(sstat))
			{
				Send (pInst, "%s %s %s %s", pszMessage, pszMsgId, pszCmd, MapSendingStatus(sstat));
				return;
			}
			else if (message->GetPropConsumptionStatus(cstat))
			{
				Send (pInst, "%s %s %s %s", pszMessage, pszMsgId, pszCmd, cstat==Message::CONSUMED?"READ":"RECEIVED");
				return;
			}
		}
		else if (!strcasecmp (pszCmd, "FAILUREREASON"))
		{
			Send (pInst, "%s %s %s 1", pszMessage, pszMsgId, pszCmd);
			return;
		}
		else if (!strcasecmp (pszCmd, "BODY"))
		{
			SEString body;

			message->GetPropBodyXml(body);
			Send (pInst, "%s %s %s %s", pszMessage, pszMsgId, pszCmd, (const char*)RemoveHtml(body));
			return;
		}
		else if (!strcasecmp (pszCmd, "CHATNAME"))
		{
			SEString convo;

			message->GetPropConvoGuid(convo);
			Send (pInst, "%s %s %s %s", pszMessage, pszMsgId, pszCmd, (const char*)convo);
			return;
		}
		else if (!strcasecmp (pszCmd, "LEAVEREASON"))
		{
			Message::LEAVEREASON lr;

			if (message->GetPropLeavereason(lr))
			{
				char *pszReason = "USER_NOT_FOUND";
				switch (lr)
				{
				case Message::USER_INCAPABLE: pszReason="USER_INCAPABLE"; break;
				case Message::ADDER_MUST_BE_FRIEND: pszReason="ADDER_MUST_BE_FRIEND"; break;
				case Message::ADDER_MUST_BE_AUTHORIZED: pszReason="ADDER_MUST_BE_AUTHORIZED"; break;
				case Message::UNSUBSCRIBE: pszReason="UNSUBSCRIBE"; break;
				}
				Send (pInst, "%s %s %s %s", pszMessage, pszMsgId, pszCmd, pszReason);
				return;
			}
		}
		else if (!strcasecmp (pszCmd, "EDITED_BY"))
		{
			SEString editor;

			message->GetPropEditedBy(editor);
			Send (pInst, "%s %s %s %s", pszMessage, pszMsgId, pszCmd, (const char*)editor);
			return;
		}
		else if (!strcasecmp (pszCmd, "EDITED_TIMESTAMP"))
		{
			uint ts;

			message->GetPropEditTimestamp(ts);
			Send (pInst, "%s %s %s %ld", pszMessage, pszMsgId, pszCmd, ts);
			return;
		}
		Send (pInst, "ERROR 10 Invalid property / not implemented");
		return;		
	}
	else
	if (strcasecmp(pszCmd, "PRIVILEGE") == 0)
	{
		if (!(pszCmd = strtok(NULL, " ")) ||
			(strcasecmp (pszCmd, "SKYPEOUT") &&
			 strcasecmp (pszCmd, "SKYPEIN") &&
			 strcasecmp (pszCmd, "VOICEMAIL")))
		{
			Send (pInst, "ERROR 40 Unknown Privilege");
			return;
		}
		Send (pInst, "PRIVILEGE %s FALSE", pszCmd);
		return;
	}
	else
	if (strcasecmp(pszCmd, "CHAT") == 0)
	{
		char *pszChat;
		MyConversation::Ref convo;

		if (!(pszChat = strtok(NULL, " ")) || 
			!(pInst->s->GetConversationByIdentity(pszChat, convo)))
		{
			Send (pInst, "ERROR 14 Invalid message id");
			return;
		}
		if (!(pszCmd = strtok(NULL, " ")))
		{
			Send (pInst, "ERROR 10 Invalid property");
			return;
		}

		if (strcasecmp(pszCmd, "NAME") == 0)
		{
			if (convo->DoChange(Conversation::P_IDENTITY))
				return;
		}
		else if (strcasecmp(pszCmd, "TIMESTAMP") == 0)
		{
			if (convo->DoChange(Conversation::P_CREATION_TIMESTAMP))
				return;
		}
		else if (strcasecmp(pszCmd, "STATUS") == 0)
		{
			Conversation::TYPE type;
			char *pszTyp = "LEGACY_DIALOG";

			if (convo->GetPropType(type))
			{
				switch (type)
				{
				case Conversation::DIALOG: pszTyp="DIALOG"; break;
				case Conversation::TERMINATED_CONFERENCE:
				case Conversation::CONFERENCE: pszTyp="MULTI_SUBSCRIBED"; break;
				}
			}
			Send (pInst, "CHAT %s %s %s", pszChat, pszCmd, pszTyp);
			return;
		}
		else if (strcasecmp(pszCmd, "ADDER") == 0)
		{
			SEString user;
			ParticipantRefs participantList;

			if (convo->GetParticipants(participantList, Conversation::MYSELF) &&
				participantList.size()>0)
			{
				if (participantList[0]->GetPropAdder(user))
					Send (pInst, "CHAT %s %s %s", pszChat, pszCmd, (const char*)user);
				else
					Send (pInst, "CHAT %s %s %s", pszChat, pszCmd, pInst->pszUser);
			}
			return;
		}
		else if (strcasecmp(pszCmd, "TYPE") == 0)
		{
			if (convo->DoChange(Conversation::P_TYPE))
				return;
		}
		else if (strcasecmp(pszCmd, "TOPIC") == 0 || strcasecmp(pszCmd, "TOPICXML") == 0)
		{
			SEString topic;

			if (convo->GetPropMetaTopic(topic))
			{
				if (pszCmd[5] == 0) Send (pInst, "CHAT %s %s %s", pszChat, pszCmd, (const char*)RemoveHtml(topic));
				else Send (pInst, "CHAT %s %s %s", pszChat, pszCmd, (const char*)topic);
				return;
			}
		}
		else if (strcasecmp(pszCmd, "FRIENDLYNAME") == 0)
		{
			if (convo->DoChange(Conversation::P_DISPLAYNAME))
				return;
		}
		else if (strcasecmp(pszCmd, "ACTIVEMEMBERS") == 0 || strcasecmp(pszCmd, "MEMBERS") == 0 ||
			strcasecmp(pszCmd, "DIALOG_PARTNER") == 0 || strcasecmp(pszCmd, "APPLICANTS") == 0)
		{
			ParticipantRefs participantList;
			Conversation::PARTICIPANTFILTER filter;
			unsigned int i;
			SEString id, users;

			switch (toupper(pszCmd[0]))
			{
			case 'A': filter = pszCmd[1]=='C'?Conversation::ALL:Conversation::APPLICANTS; break;
			case 'M': filter = Conversation::CONSUMERS; break;
			case 'D': filter = Conversation::OTHER_CONSUMERS; break;
			}
			convo->GetParticipants(participantList, filter);
			for (i = 0; i < participantList.size(); ++i)
			{
				if (participantList[i]->GetPropIdentity(id))
				{
					if (!users.isNull()) users+=" ";
					users+=id;
				}
			}
			Send (pInst, "CHAT %s %s %s", pszChat, pszCmd, (const char*)users);
			return;
		}
		else if (strcasecmp(pszCmd, "RECENTCHATMESSAGES") == 0 || strcasecmp(pszCmd, "CHATMESSAGES") == 0)
		{
			MessageRefs contextMessages;
			MessageRefs unconsumedMessages;
			SEString msgs;
			unsigned int i, size;
			int messageType = 0;

			convo->GetLastMessages(contextMessages, unconsumedMessages);
			if (pszCmd[0]!='R')
			{
				size = contextMessages.size();
				for (i = 0; i < size; ++i) 
				{
					// Get the message type.
					messageType = contextMessages[i]->GetProp(Message::P_TYPE).toInt();

					if (messageType != Message::POSTED_EMOTE &&
						messageType != Message::POSTED_TEXT &&
						messageType != Message::POSTED_SMS)
						continue;
					if (!msgs.isNull()) msgs+=", ";
					msgs+=contextMessages[i]->getOID();
				}
			}

			size = unconsumedMessages.size();
			for (i = 0; i < size; ++i) {
				// Get the message type.
				messageType = unconsumedMessages[i]->GetProp(Message::P_TYPE).toInt();

				if (messageType != Message::POSTED_EMOTE &&
					messageType != Message::POSTED_TEXT &&
					messageType != Message::POSTED_SMS)
					continue;
				if (!msgs.isNull()) msgs+=", ";
				msgs+=contextMessages[i]->getOID();
			} 
			Send (pInst, "CHAT %s %s %s", pszChat, pszCmd, (const char*)msgs);
			return;
		}
		else if (strcasecmp(pszCmd, "MEMBEROBJECTS") == 0)
		{
			ParticipantRefs participantList;
			unsigned int i;
			SEString id, users;
			char szBuf[16];

			convo->GetParticipants(participantList, Conversation::ALL);
			for (i = 0; i < participantList.size(); ++i)
			{
				if (!users.isNull()) users+=", ";
				sprintf (szBuf, "%d", participantList[i]->getOID());
				users+=szBuf;
			}
			Send (pInst, "CHAT %s %s %s", pszChat, pszCmd, (const char*)users);
			return;
		}
		else if (strcasecmp(pszCmd, "PASSWORDHINT") == 0)
		{
			if (convo->DoChange(Conversation::P_PASSWORDHINT))
				return;
		}
		else if (strcasecmp(pszCmd, "BOOKMARKED") == 0)
		{
			if (convo->DoChange(Conversation::P_IS_BOOKMARKED))
				return;
		}
		else if (strcasecmp(pszCmd, "GUIDELINES") == 0)
		{
			if (convo->DoChange(Conversation::P_META_GUIDELINES))
				return;
		}
		else if (strcasecmp(pszCmd, "OPTIONS") == 0)
		{
			unsigned long flags = 8;
			bool bJoining;
			Participant::RANK rank;
			Conversation::ALLOWED_ACTIVITY allowed;

			if (convo->GetPropOptJoiningEnabled(bJoining) && bJoining)
				flags |= 1;
			if (convo->GetPropOptEntryLevelRank(rank))
			{
				switch (rank)
				{
				case Participant::APPLICANT: flags |= 2; break;
				case Participant::SPECTATOR: flags |= 4; break;
				}
			}
			if (convo->GetPropOptDiscloseHistory(bJoining) && bJoining)
				flags &= ~8;
			if (convo->GetPropOptAdminOnlyActivities(allowed))
			{
				if ((allowed & Conversation::SPEAK) || (allowed & Conversation::SPEAK_AND_WRITE))
					flags |= 16;
				if (allowed & Conversation::SET_META)
					flags |= 32;
			}
			Send (pInst, "CHAT %s %s %d", pszChat, pszCmd, flags);
			return;
		}
		else if (strcasecmp(pszCmd, "ACTIVITY_TIMESTAMP") == 0)
		{
			if (convo->DoChange(Conversation::P_LAST_ACTIVITY_TIMESTAMP))
				return;
		}
		else if (strcasecmp(pszCmd, "MYSTATUS") == 0)
		{
			if (convo->DoChange(Conversation::P_MY_STATUS))
				return;
		}
		else if (strcasecmp(pszCmd, "MYROLE") == 0)
		{
			ParticipantRefs participantList;

			if (convo->GetParticipants(participantList, Conversation::MYSELF) &&
				participantList.size()>0)
			{
				Participant::RANK rank;

				if (participantList[0]->GetPropRank(rank))
				{
					Send (pInst, "CHAT %s %s %s", pszChat, pszCmd, MapRank(rank));
					return;
				}
			}
		}
		else if (strcasecmp(pszCmd, "BLOB") == 0)
		{
			SEString blob;

			if (convo->GetJoinBlob(blob))
			{
				Send (pInst, "CHAT %s %s %s", pszChat, pszCmd, (const char*)blob);
				return;
			}
		}
		Send(pInst, "ERROR 7 Invalid property / not implemented");
		return;
	}
	else
	if (strcasecmp(pszCmd, "CHATMEMBER") == 0)
	{
		ParticipantRef ptcp;
		char *pszChat = pszCmd, *pszMsgId;

		if (!(pszMsgId = strtok(NULL, " ")) || !(ptcp = ParticipantRef(atol(pszMsgId)))->getOID())
		{
			Send (pInst, "ERROR 14 Invalid chatmember id");
			return;
		}
		if (!(pszCmd = strtok(NULL, " ")))
		{
			Send (pInst, "ERROR 10 Invalid property");
			return;
		}
		if (!strcasecmp (pszCmd, "CHATNAME"))
		{
			ConversationRef convo;

			if (ptcp->GetPropConvoId(convo))
			{
				SEString id;

				if (convo->GetPropIdentity(id))
				{
					Send (pInst, "%s %s %s %s", pszChat, pszMsgId, pszCmd, (const char*)id);
					return;
				}
			}
		}
		else if (!strcasecmp (pszCmd, "IDENTITY"))
		{
			SEString id;

			if (ptcp->GetPropIdentity(id))
			{
				Send (pInst, "%s %s %s %s", pszChat, pszMsgId, pszCmd, (const char*)id);
				return;
			}
		}
		else if (!strcasecmp (pszCmd, "ROLE"))
		{
			Participant::RANK rank;

			if (ptcp->GetPropRank(rank))
			{
				Send (pInst, "%s %s %s %s", pszChat, pszMsgId, pszCmd, MapRank(rank));
				return;
			}
		}
		else if (!strcasecmp (pszCmd, "IS_ACTIVE"))
		{
			/* Fixme: Dunno how this can be false in Skypekit? */
			Send (pInst, "%s %s %s TRUE", pszChat, pszMsgId, pszCmd);
			return;
		}
		Send(pInst, "ERROR 7 Invalid property / not implemented");
		return;
	}
	else
	if (strcasecmp(pszCmd, "PROFILE") == 0)
	{
		if (!(pszCmd = strtok(NULL, " ")))
		{
			Send (pInst, "ERROR 10 Invalid property");
			return;
		}

		if (!strcasecmp (pszCmd, "MOOD_TEXT") && 
			pInst->s->account->SetStrProperty(Account::P_MOOD_TEXT, pszCmd+strlen(pszCmd)+1))
			return;
			//Send (pInst, "PROFILE MOOD_TEXT %s", pszCmd+10);
		else if (!strcasecmp (pszCmd, "PSTN_BALANCE_CURRENCY"))
			pInst->s->account->DoChange(Account::P_SKYPEOUT_BALANCE_CURRENCY);
		else if (!strcasecmp (pszCmd, "FULLNAME"))
			pInst->s->account->DoChange(Account::P_FULLNAME);
		else if (!strcasecmp (pszCmd, "LANGUAGES"))
			pInst->s->account->DoChange(Account::P_LANGUAGES);
		else if (!strcasecmp (pszCmd, "COUNTRY"))
			pInst->s->account->DoChange(Account::P_COUNTRY);
		else if (!strcasecmp (pszCmd, "PROVINCE"))
			pInst->s->account->DoChange(Account::P_PROVINCE);
		else if (!strcasecmp (pszCmd, "CITY"))
			pInst->s->account->DoChange(Account::P_CITY);
		else if (!strcasecmp (pszCmd, "PHONE_HOME"))
			pInst->s->account->DoChange(Account::P_PHONE_HOME);
		else if (!strcasecmp (pszCmd, "PHONE_OFFICE"))
			pInst->s->account->DoChange(Account::P_PHONE_OFFICE);
		else if (!strcasecmp (pszCmd, "PHONE_MOBILE"))
			pInst->s->account->DoChange(Account::P_PHONE_MOBILE);
		else if (!strcasecmp (pszCmd, "HOMEPAGE"))
			pInst->s->account->DoChange(Account::P_HOMEPAGE);
		else if (!strcasecmp (pszCmd, "ABOUT"))
			pInst->s->account->DoChange(Account::P_ABOUT);
		else if (!strcasecmp (pszCmd, "MOOD_TEXT"))
			pInst->s->account->DoChange(Account::P_MOOD_TEXT);
		else if (!strcasecmp (pszCmd, "CALL_FORWARD_RULES"))
			pInst->s->account->DoChange(Account::P_OFFLINE_CALLFORWARD);
		else if (!strcasecmp (pszCmd, "PSTN_BALANCE"))
			pInst->s->account->DoChange(Account::P_SKYPEOUT_BALANCE);
		else if (!strcasecmp (pszCmd, "BIRTHDAY"))
			pInst->s->account->DoChange(Account::P_BIRTHDAY);
		else if (!strcasecmp (pszCmd, "TIMEZONE"))
			pInst->s->account->DoChange(Account::P_TIMEZONE);
		else if (!strcasecmp (pszCmd, "SEX"))
			pInst->s->account->DoChange(Account::P_GENDER);
		else
			Send (pInst, "ERROR 552 Invalid property");
		return;
	}
	else
	if (strcasecmp(pszCmd, "CALL") == 0)
	{
		Send (pInst, "ERROR 2 Not Implemented");
	}
	else
	if (strcasecmp(pszCmd, "SKYPEVERSION") == 0)
	{
		Send (pInst, "SKYPEVERSION 3.8.0.188"); // Fake
	}
	else
	if (strcasecmp(pszCmd, "WINDOWSTATE") == 0)
	{
		Send (pInst, "%s", pInst->pszWindowState);
	}
	else
	{
		Send(pInst, "ERROR 7 Invalid property / not implemented");
	}
	return;
}

// -----------------------------------------------------------------------------

static void HandleSET(IMOSAPI *pInst)
{
	char *pszCmd;

	if (!(pszCmd = strtok(NULL, " ")))
	{
		Send (pInst, "ERROR 7 Invalid property");
		return;
	}

	if (strcasecmp(pszCmd, "USER") == 0)
	{
		MyContact::Ref user;
		bool bFoundContact = false;
		char *pszName;

		if (pszCmd = strtok(NULL, " "))
		{
			bFoundContact = pInst->s->GetContact(pszCmd, user);
		}

		if (!bFoundContact)			
		{
			Send (pInst, "ERROR 26 Invalid user handle");
			return;
		}
		pszName = pszCmd;

		if (!(pszCmd = strtok(NULL, " ")))
		{
			Send (pInst, "ERROR 7 Invalid property");
			return;
		}

		if (strcasecmp(pszCmd, "BUDDYSTATUS") == 0)
		{
			int iStatus = -1;
			bool bStatus;
			char *pszAuthRq = NULL;

			if (pszCmd = strtok(NULL, " "))
				iStatus = atoi(pszCmd);

			switch (iStatus)
			{
			case 1:
				bStatus = false;
				break;
			case 2:
				bStatus = true;
				pszAuthRq = pszCmd+strlen(pszCmd)+1;
				break;
			default:
				Send (pInst, "ERROR 518 Invalid status given for BUDDYSTATUS");
				return;
			}
			if (!user->SetBuddyStatus(bStatus) || (pszAuthRq && !user->SendAuthRequest(pszAuthRq)))
			{
				Send (pInst, "ERROR 519 Updating BUDDYSTATUS failed");
				return;
			}
			Send (pInst, "%s %s %s %d", "USER", pszName, "BUDDYSTATUS", iStatus);
			return;
		}
		else
		if (strcasecmp(pszCmd, "ISBLOCKED") == 0)
		{
			bool bBlocked;
			if (pszCmd = strtok(NULL, " "))
			{
				if (!strcasecmp(pszCmd, "TRUE"))
				{
					bBlocked=true;
					user->SetBuddyStatus(false);
				}
				else if (!strcasecmp(pszCmd, "FALSE")) bBlocked=false;
				else pszCmd=NULL;
			}
			if (!pszCmd)
			{
				Send (pInst, "ERROR 516 Invalid value given to ISBLOCKED");
				return;
			}
			if (!user->SetBlocked(bBlocked))
				Send (pInst, "ERROR 517 Changing ISBLOCKED failed");
			else
				Send (pInst, "%s %s %s %s", "USER", pszName, "ISBLOCKED", pszCmd);
			return;
		}
		else
		if (strcasecmp(pszCmd, "ISAUTHORIZED") == 0)
		{
			// Documentation states to change it with Contact::GiveAuthlevel(), but its not implemented?!?
			Send (pInst, "%s %s %s %s", "USER", pszName, pszCmd, user->SetBuddyStatus(true)?"TRUE":"FALSE");
		}
		else
		if (strcasecmp(pszCmd, "DISPLAYNAME") == 0)
		{
			SEString displayName;
			
			pszCmd += strlen(pszCmd)+1;
			user->GiveDisplayName(pszCmd);
			if (user->GiveDisplayName(pszCmd) && GetPropDisplayname(displayName))
				Send (root->pInst, "%s %s %s %s", "USER", pszName, "DISPLAYNAME", (const char*)displayName);
		}
		else
		{
			Send (pInst, "ERROR 7 Not implemented");
		}
	}
	else
	if (strcasecmp(pszCmd, "USERSTATUS") == 0)
	{
		Contact::AVAILABILITY avail;

		if (!(pszCmd = strtok(NULL, " ")))
		{
			Send (pInst, "ERROR 28 Unknown userstatus");
			return;
		}

		if (strcasecmp(pszCmd, "ONLINE")==0) avail=Contact::ONLINE; else
		if (strcasecmp(pszCmd, "OFFLINE")==0) avail=Contact::OFFLINE; else
		if (strcasecmp(pszCmd, "AWAY")==0) avail=Contact::AWAY; else
		if (strcasecmp(pszCmd, "NA")==0) avail=Contact::NOT_AVAILABLE; else
		if (strcasecmp(pszCmd, "DND")==0) avail=Contact::DO_NOT_DISTURB; else
		if (strcasecmp(pszCmd, "SKYPEME")==0) avail=Contact::SKYPE_ME; else
		if (strcasecmp(pszCmd, "SKYPEOUT")==0) avail=Contact::SKYPEOUT; else
		if (strcasecmp(pszCmd, "INVISIBLE")==0) avail=Contact::INVISIBLE; else
		{
			Send (pInst, "ERROR 28 Unknown userstatus");
			return;
		}
		if (pInst->s->account->SetAvailability(avail))
			Send (pInst, "USERSTATUS %s", pszCmd);
		return;
	}
	else
	if (strcasecmp(pszCmd, "MESSAGE") == 0 || strcasecmp(pszCmd, "CHATMESSAGE") == 0)
	{
		MessageRef message;
		char *pszMessage = pszCmd, *pszMsgId;

		if (!(pszMsgId = strtok(NULL, " ")) || !(message = MessageRef(atol(pszMsgId)))->getOID())
		{
			Send (pInst, "ERROR 14 Invalid message id");
			return;
		}
		if (!(pszCmd = strtok(NULL, " ")))
		{
			Send (pInst, "ERROR 10 Invalid property");
			return;
		}
		if (!strcasecmp (pszCmd, "SEEN"))
		{
			/* 
			FIXME: CONSUMPTION_STATUS of message object cannot be set? 
			*/
			Send (pInst, "%s %s STATUS READ", pszMessage, pszMsgId);
			return;
		}
		else if (!strcasecmp (pszCmd, "BODY"))
		{
			pszCmd+=strlen(pszCmd)+1;
			if (!*pszCmd)
			{
				Send (pInst, "ERROR 43 Cannot send empty message");
				return;
			}
			if (!message->Edit(pszCmd))
				Send (pInst, "ERROR 9901 Internal error");
			return;
		}
		Send (pInst, "ERROR 10 Invalid property / not implemented");
		return;
	}
	else
	if (strcasecmp(pszCmd, "CALL") == 0)
	{
		Send (pInst, "ERROR 2 Not Implemented");
		return;
	}
	else
	if (strcasecmp(pszCmd, "PROFILE") == 0)
	{
		if (!(pszCmd = strtok(NULL, " ")))
		{
			Send (pInst, "ERROR 10 Invalid property");
			return;
		}

		if (!strcasecmp (pszCmd, "MOOD_TEXT") && 
			pInst->s->account->SetStrProperty(Account::P_MOOD_TEXT, pszCmd+strlen(pszCmd)+1))
			return;
			//Send (pInst, "PROFILE MOOD_TEXT %s", pszCmd+10);
		else if (!strcasecmp (pszCmd, "PSTN_BALANCE_CURRENCY") && 
			pInst->s->account->SetStrProperty(Account::P_SKYPEOUT_BALANCE_CURRENCY, pszCmd+strlen(pszCmd)+1))
			return;
		else if (!strcasecmp (pszCmd, "FULLNAME") && 
			pInst->s->account->SetStrProperty(Account::P_FULLNAME, pszCmd+strlen(pszCmd)+1))
			return;
		else if (!strcasecmp (pszCmd, "LANGUAGES") && 
			pInst->s->account->SetStrProperty(Account::P_LANGUAGES, pszCmd+strlen(pszCmd)+1))
			return;
		else if (!strcasecmp (pszCmd, "COUNTRY") && 
			pInst->s->account->SetStrProperty(Account::P_COUNTRY, pszCmd+strlen(pszCmd)+1))
			return;
		else if (!strcasecmp (pszCmd, "PROVINCE") && 
			pInst->s->account->SetStrProperty(Account::P_PROVINCE, pszCmd+strlen(pszCmd)+1))
			return;
		else if (!strcasecmp (pszCmd, "CITY") && 
			pInst->s->account->SetStrProperty(Account::P_CITY, pszCmd+strlen(pszCmd)+1))
			return;
		else if (!strcasecmp (pszCmd, "PHONE_HOME") && 
			pInst->s->account->SetStrProperty(Account::P_PHONE_HOME, pszCmd+strlen(pszCmd)+1))
			return;
		else if (!strcasecmp (pszCmd, "PHONE_OFFICE") && 
			pInst->s->account->SetStrProperty(Account::P_PHONE_OFFICE, pszCmd+strlen(pszCmd)+1))
			return;
		else if (!strcasecmp (pszCmd, "PHONE_MOBILE") && 
			pInst->s->account->SetStrProperty(Account::P_PHONE_MOBILE, pszCmd+strlen(pszCmd)+1))
			return;
		else if (!strcasecmp (pszCmd, "HOMEPAGE") && 
			pInst->s->account->SetStrProperty(Account::P_HOMEPAGE, pszCmd+strlen(pszCmd)+1))
			return;
		else if (!strcasecmp (pszCmd, "ABOUT") && 
			pInst->s->account->SetStrProperty(Account::P_ABOUT, pszCmd+strlen(pszCmd)+1))
			return;
		else if (!strcasecmp (pszCmd, "MOOD_TEXT") && 
			pInst->s->account->SetStrProperty(Account::P_MOOD_TEXT, pszCmd+strlen(pszCmd)+1))
			return;
		else if (!strcasecmp (pszCmd, "CALL_FORWARD_RULES") && 
			pInst->s->account->SetServersideStrProperty(Account::P_OFFLINE_CALLFORWARD, pszCmd+strlen(pszCmd)+1))
			return;
		else if (!strcasecmp (pszCmd, "PSTN_BALANCE") && 
			pInst->s->account->SetIntProperty(Account::P_SKYPEOUT_BALANCE, (unsigned int)strtoul(pszCmd+strlen(pszCmd)+1, NULL, 10)))
			return;
		else if (!strcasecmp (pszCmd, "BIRTHDAY") && 
			pInst->s->account->SetIntProperty(Account::P_BIRTHDAY, (unsigned int)strtoul(pszCmd+strlen(pszCmd)+1, NULL, 10)))
			return;
		else if (!strcasecmp (pszCmd, "TIMEZONE") && 
			pInst->s->account->SetIntProperty(Account::P_TIMEZONE, (unsigned int)strtoul(pszCmd+strlen(pszCmd)+1, NULL, 10)))
			return;
		else if (!strcasecmp (pszCmd, "SEX"))
		{
			unsigned int i;
			
			pszCmd+=strlen(pszCmd)+1;
			for (i=0; i<3; i++) if (strcasecmp(m_pszSex[i], pszCmd) == 0) break;
			if (i<3)
			{
				pInst->s->account->SetIntProperty(Account::P_GENDER, i);
				return;
			}
		}
		Send (pInst, "ERROR 552 Invalid property");
		return;
	}
	else
	if (strcasecmp(pszCmd, "WINDOWSTATE") == 0)
	{
		if ((pszCmd = strtok(NULL, " ")))
		{
			if (strcasecmp(pszCmd, "NORMAL") == 0) pInst->pszWindowState="NORMAL"; 
			else if (strcasecmp(pszCmd, "MINIMIZED") == 0) pInst->pszWindowState="MINIMIZED"; 
			else if (strcasecmp(pszCmd, "MAXIMIZED") == 0) pInst->pszWindowState="MAXIMIZED"; 
			else if (strcasecmp(pszCmd, "HIDDEN") == 0) pInst->pszWindowState="HIDDEN"; 
			else 
			{
				Send (pInst, "ERROR 10 Invalid property");
				return;
			}
			Send (pInst, "%s %s", "WINDOWSTATE", pInst->pszWindowState);
			pInst->pszCmdID = NULL;
			Send (pInst, "%s %s", "WINDOWSTATE", pInst->pszWindowState);
			return;
		}
		Send (pInst, "ERROR 10 Invalid property");
		return;
	}
	else
	{
		Send (pInst, "ERROR 7 Invalid property");
		return;
	}
	return;
}

// -----------------------------------------------------------------------------

static void HandleALTER(IMOSAPI *pInst)
{
	char *pszCmd;

	if (!(pszCmd = strtok(NULL, " ")))
		Send (pInst, "ERROR 526 ALTER: no object type given");
	else
	{
		if (!strcasecmp(pszCmd, "APPLICATION"))
		{
			if (!(pszCmd = strtok(NULL, " ")) || strcasecmp(pszCmd, "libpurple_typing") ||
				!(pszCmd = strtok(NULL, " ")))
				Send (pInst, "ERROR 545 ALTER: missing or invalid action");
			else
			{
				if (strcasecmp (pszCmd, "CONNECT") == 0)
				{
					MyContact::Ref  user;
					bool bFoundContact = false;

					if (pszCmd = strtok(NULL, " "))
						bFoundContact = pInst->s->GetContact(pszCmd, user);
					if (!bFoundContact)			
						Send (pInst, "ERROR 547 ALTER APPLICATION CONNECT: Invalid user handle");
					else
					{
						Send (pInst, "ALTER APPLICATION libpurple_typing CONNECT %s", pszCmd);
						Send (pInst, "APPLICATION CONNECTING %s", pszCmd);
						Send (pInst, "APPLICATION libpurple_typing STREAMS %s:1", pszCmd);
						// FIXME: Shouldn't we enumerate all STREAMS here? dunno...
						return;
					}
				} else
				if (strcasecmp (pszCmd, "DATAGRAM") == 0)
				{
					char *pSep;

					if (!(pszCmd = strtok(NULL, " ")) || !(pSep = strchr(pszCmd, ':')))
						Send (pInst, "ERROR 551 ALTER APPLICATION DATAGRAM: Missing or invalid stream identifier");
					else
					{
						MyContact::Ref user;
						Conversation::Ref conv;
						MessageRef message;
						char *pszUser;
						bool bFoundContact = false;

						*pSep=0;
						if (pInst->s->GetContact((pszUser = pszCmd), user))
						{
							SEStringList partnerNames;

							partnerNames.append(pszUser);
							if (pInst->s->GetConversationByParticipants(partnerNames, conv, true, false))
								bFoundContact = true;
						}

						if (!bFoundContact)
							Send (pInst, "ERROR 551 ALTER APPLICATION DATAGRAM: Missing or invalid stream identifier");
						else
						{
							*pSep=':';
							if (!(pszCmd = strtok(NULL, " ")))
								Send (pInst, "ERROR 541 APPLICATION: Operation failed");
							else
							{
								if (!strcmp (pszCmd, "PURPLE_TYPING"))
									conv->SetMyTextStatusTo(Participant::WRITING);
								else if (!strcmp (pszCmd, "PURPLE_TYPED") || !strcmp (pszCmd, "PURPLE_NOT_TYPING"))
									conv->SetMyTextStatusTo(Participant::READING);
								Send (pInst, "ALTER APPLICATION libpurple_typing DATAGRAM %s", pszUser);
								return;
							}
						}
					}
				}
			}
		}
		else if (!strcasecmp(pszCmd, "CHAT"))
		{
			char *pszChat;
			ConversationRef convo;

			if (!(pszChat = strtok(NULL, " ")) || 
				!(pInst->s->GetConversationByIdentity(pszChat, convo)))
			{
				Send (pInst, "ERROR 501 CHAT: No chat found for given chat");
				return;
			}
			if (!(pszCmd = strtok(NULL, " ")))
			{
				Send (pInst, "ERROR 10 Invalid property");
				return;
			}
			if (strcasecmp(pszCmd, "SETOPTIONS") == 0)
			{
				unsigned long flags;
				bool bJoining, bDiscloseHistory = true;
				Participant::RANK rank = Participant::SPEAKER;
				Conversation::ALLOWED_ACTIVITY allowed = Conversation::ADD_CONSUMERS;

				if (!(pszCmd = strtok(NULL, " ")))
				{
					Send (pInst, "ERROR 504 CHAT: action failed");
					return;
				}
				flags = atoi(pszCmd);

				bJoining = flags & 1;
				convo->SetOption(Conversation::P_OPT_JOINING_ENABLED, bJoining);
				if (flags & 2) rank = Participant::APPLICANT;
				if (flags & 4) rank = Participant::SPECTATOR;
				convo->SetOption(Conversation::P_OPT_ENTRY_LEVEL_RANK, rank);
				bDiscloseHistory = !(flags & 8);
				convo->SetOption(Conversation::P_OPT_DISCLOSE_HISTORY, bDiscloseHistory);
				if (flags & 16) allowed=Conversation::SPEAK_AND_WRITE;
				if (flags & 32) allowed=Conversation::SET_META;
				convo->SetOption(Conversation::P_OPT_ADMIN_ONLY_ACTIVITIES, allowed);
				Send (pInst, "CHAT %s OPTIONS %d", pszChat, flags);
				return;
			}
			else if (strcasecmp(pszCmd, "SETTOPIC") == 0 || strcasecmp(pszCmd, "SETTOPICXML") == 0)
			{
				bool bXml = pszCmd[8]!=0;
				char *pszTopic;

				pszTopic=pszCmd+strlen(pszCmd)+1;
				if (*pszTopic)
				{
					if (convo->SetTopic(pszTopic, bXml))
					{
						Send (pInst, "ALTER CHAT %s", pszCmd);
						return;
					}
				}
				Send (pInst, "ERROR 504 CHAT: action failed");
				return;
			}
			else if (strcasecmp(pszCmd, "ADDMEMBERS") == 0)
			{
				SEStringList needToAdd;

				while (pszCmd=strtok(NULL, ", "))
					needToAdd.append(pszCmd);
				if (convo->AddConsumers(needToAdd))
				{
					Send (pInst, "ALTER CHAT %s", "ADDMEMBERS");
					return;
				}
				Send (pInst, "ERROR 504 CHAT: action failed");
				return;
			}
			else if (strcasecmp(pszCmd, "LEAVE") == 0)
			{
				if (convo->RetireFrom())
				{
					Send (pInst, "ALTER CHAT %s", pszCmd);
					return;
				}
				else
				{
					// Only I'm left appearently, so delete the convo
					if (convo->Delete())
					{
						Send (pInst, "ALTER CHAT %s", pszCmd);
						return;
					}
				}

				Send (pInst, "ERROR 504 CHAT: action failed");
				return;
			}
			else if (strcasecmp(pszCmd, "BOOKMARK") == 0 || strcasecmp(pszCmd, "UNBOOKMARK") == 0)
			{
				bool bBookmark = toupper(pszCmd[0])=='B';

				if (convo->SetBookmark(bBookmark))
				{
					Send (pInst, "ALTER CHAT %s BOOKMARKED %s", pszChat, bBookmark?"TRUE":"FALSE");
					return;
				}
				Send (pInst, "ERROR 504 CHAT: action failed");
				return;
			}
			else if (strcasecmp(pszCmd, "SETPASSWORD") == 0)
			{
				char *pszHint;

				if ((pszCmd = strtok(NULL, " ")))
				{
					if (!(pszHint = strtok(NULL, " "))) pszHint="";
					if (convo->SetPassword(pszCmd, pszHint))
					{
						Send (pInst, "ALTER CHAT %s", "SETPASSWORD");
						return;
					}
				}
				Send (pInst, "ERROR 504 CHAT: action failed");
				return;
			}
			else if (strcasecmp(pszCmd, "ENTERPASSWORD") == 0)
			{
				if ((pszCmd = strtok(NULL, " ")))
				{
					if (convo->EnterPassword(pszCmd))
					{
						Send (pInst, "ALTER CHAT %s", "ENTERPASSWORD");
						return;
					}
				}
				Send (pInst, "ERROR 504 CHAT: action failed");
				return;
			}
			else if (strcasecmp(pszCmd, "JOIN") == 0)
			{
				if (!convo->Join())
					Send (pInst, "ERROR 504 CHAT: action failed");
				return;
			}
			else if (strcasecmp(pszCmd, "CLEARRECENTMESSAGES") == 0)
			{
				if (convo->SetConsumedHorizon(time(NULL)))
				{
					Send (pInst, "ALTER CHAT %s", pszCmd);
					return;
				}
				Send (pInst, "ERROR 504 CHAT: action failed");
				return;
			}
			else if (strcasecmp(pszCmd, "SETALERTSTRING") == 0)
			{
				char *pszTopic=pszCmd+strlen(pszCmd)+1;

				if (*pszTopic)
				{
					if (convo->SetAlertString(pszTopic))
					{
						Send (pInst, "ALTER CHAT %s", "SETALERTSTRING");
						return;
					}
				}
				Send (pInst, "ERROR 504 CHAT: action failed");
				return;
			}
			else if (strcasecmp(pszCmd, "KICK") == 0 || strcasecmp(pszCmd, "KICKBAN") == 0)
			{
				ParticipantRefs participantList;
				unsigned int i;
				SEString id;
				bool bBan = pszCmd[4]!=0;
				char *pszVerb=pszCmd;

				if (convo->GetParticipants(participantList, Conversation::ALL))
				{
					while (pszCmd=strtok(NULL, ", "))
					{
						for (i = 0; i < participantList.size(); ++i)
						{
							if (participantList[i]->GetPropIdentity(id))
							{
								if (strcasecmp((const char*)id, pszCmd) == 0)
								{
									if (bBan) participantList[i]->SetRankTo(Participant::OUTLAW);
									participantList[i]->Retire();
								}
							}
						}
					}
					Send (pInst, "ALTER CHAT %s", pszVerb);
					return;
				}
				Send (pInst, "ERROR 504 CHAT: action failed");
				return;
			}
			else if (strcasecmp(pszCmd, "SETGUIDELINES") == 0)
			{
				char *pszTopic=pszCmd+strlen(pszCmd)+1;

				if (*pszTopic)
				{
					if (convo->SetGuidelines(pszTopic, false))
					{
						Send (pInst, "ALTER CHAT %s", pszCmd);
						return;
					}
				}
				Send (pInst, "ERROR 504 CHAT: action failed");
				return;
			}
			else if (strcasecmp(pszCmd, "DISBAND") == 0)
			{
				if (convo->Delete())
				{
					Send (pInst, "ALTER CHAT %s", pszCmd);
					return;
				}
				Send (pInst, "ERROR 504 CHAT: action failed");
				return;
			}				
			
			Send(pInst, "ERROR 7 Invalid property / not implemented");
			return;
		}
		else if (!strcasecmp(pszCmd, "CHATMEMBER"))
		{
			ParticipantRef ptcp;
			char *pszChat = pszCmd, *pszMsgId;

			if (!(pszMsgId = strtok(NULL, " ")) || !(ptcp = ParticipantRef(atol(pszMsgId)))->getOID())
			{
				Send (pInst, "ERROR 14 Invalid chatmember id");
				return;
			}
			if (!(pszCmd = strtok(NULL, " ")))
			{
				Send (pInst, "ERROR 10 Invalid property");
				return;
			}
			if (!strcasecmp (pszCmd, "SETROLETO") || !strcasecmp (pszCmd, "CANSETROLETO"))
			{
				bool bCan = toupper(pszCmd[0])=='C';

				if ((pszCmd = strtok(NULL, " ")))
				{
					Participant::RANK rank;
					bool bRet = true;

					if (!strcasecmp (pszCmd, "CREATOR")) rank=Participant::CREATOR;
					else if (!strcasecmp (pszCmd, "MASTER")) rank=Participant::ADMIN;
					else if (!strcasecmp (pszCmd, "HELPER")) rank=Participant::SPEAKER;
					else if (!strcasecmp (pszCmd, "USER")) rank=Participant::WRITER;
					else if (!strcasecmp (pszCmd, "LISTENER")) rank=Participant::SPECTATOR;
					else if (!strcasecmp (pszCmd, "APPLICANT")) rank=Participant::APPLICANT;
					else bRet=false;
					if (bRet)
					{
						if (bCan)
						{
							bool bCanSet;

							if (ptcp->CanSetRankTo(rank, bCanSet))
							{
								Send(pInst, "ALTER CHATMEMBER CANSETROLETO %s", bCanSet?"TRUE":"FALSE");
								return;
							}
						}
						else
						if (ptcp->SetRankTo(rank))
						{
							Send(pInst, "ALTER CHATMEMBER SETROLETO");
							return;
						}
						Send (pInst, "ERROR 9901 Internal error");
						return;
					}
				}
				Send(pInst, "ERROR 609 CHATMEMBER: ALTER SETROLE/CANSETROLETO: Invalid role specified");
				return;
			}
			Send(pInst, "ERROR 7 Invalid property / not implemented");
			return;
		}
		Send (pInst, "ERROR 527 ALTER: unknown object type given");
	}
	return;
}

// -----------------------------------------------------------------------------

static void HandleMessage(IMOSAPI *pInst, char *pszMsg)
{
	char *pszCmd=strtok(pszMsg, " ");

	if (!pInst || !pszCmd || !pInst->s) return;
	if (*pszCmd=='#')
	{
		// This is a PROTOCOL 4 feature, but we will support it just in case...
		pInst->pszCmdID = pszCmd;
		if (!(pszCmd=strtok(NULL, " ")))
		{
			pInst->pszCmdID = NULL;
			return;
		}
	}
	else pInst->pszCmdID = NULL;

	if (strcasecmp(pszCmd, "PROTOCOL") == 0)
	{
		if (pszCmd = strtok(NULL, " "))
		{
			pInst->iProtocol = atoi(pszCmd);
			if (pInst->iProtocol>PROTVERSION) pInst->iProtocol=PROTVERSION;
		}

		Send (pInst, "PROTOCOL %d", pInst->iProtocol);
		return;
	}
	else
	if (strcasecmp(pszCmd, "PING") == 0)
	{
		Send (pInst, "PONG");
		return;
	}
	else
	if (strcasecmp(pszCmd, "SEARCH") == 0)
	{
		HandleSEARCH(pInst);
		return;
	}
	else
	if (strcasecmp(pszCmd, "GET") == 0)
	{
		HandleGET(pInst);
		return;
	}
	else
	if (strcasecmp(pszCmd, "SET") == 0)
	{
		HandleSET(pInst);
		return;
	}
	else
	if (strcasecmp(pszCmd, "MESSAGE") == 0 || strcasecmp(pszCmd, "CHATMESSAGE") == 0)
	{
		Conversation::Ref convo;
		MessageRef message;
		BOOL bChatMsg = pszCmd[7];

		if (bChatMsg)
		{
			if (!(pszCmd = strtok(NULL, " ")) || 
				!(pInst->s->GetConversationByIdentity(pszCmd, convo)))
			{
				Send (pInst, "ERROR 510 Invalid/unknown chat name given");
				return;
			}
		}
		else
		{
			char *pszUser;
			MyContact::Ref user;
			SEStringList partnerNames;

			if (!(pszCmd = strtok(NULL, " ")) || 
				!(pInst->s->GetContact((pszUser = pszCmd), user)))
			{
				Send (pInst, "ERROR 26 Invalid user handle");
				return;
			}

			partnerNames.append(pszUser);
			if (!pInst->s->GetConversationByParticipants(partnerNames, convo, true, false))
			{
				Send (pInst, "ERROR 9901 Internal error");
				return;
			}
		}

		pszCmd+=strlen(pszCmd)+1;
		if (!*pszCmd)
		{
			Send (pInst, "ERROR 43 Cannot send empty message");
			return;
		}

		if (convo->PostText(pszCmd, message))
		{
			Message::SENDING_STATUS status;

			message->GetPropSendingStatus(status);
			Send(pInst, "%s %d %s %s", pInst->iProtocol>=3?"CHATMESSAGE":"MESSAGE", 
				message->getOID(), "STATUS", MapSendingStatus(status));
			return;
		}
		Send (pInst, "ERROR 9901 Internal error");
		return;
	}
	else
	if (strcasecmp(pszCmd, "CHAT") == 0)
	{
		if (pszCmd = strtok(NULL, " "))
		{
			if (strcasecmp(pszCmd, "CREATE") == 0)
			{
				ConversationRef convo;

				if (pInst->s->CreateConference(convo))
				{
					SEStringList consumers;
					SEString name;
					char szStatus[256];

					while (pszCmd=strtok(NULL, ", "))
						consumers.append(pszCmd);
					if (consumers.size()>0) convo->AddConsumers(consumers);
					convo->GetPropIdentity(name);
					sprintf (szStatus, "GET CHAT %s STATUS", (const char*)name);
					HandleMessage (pInst, szStatus);
					return;
				}
				Send (pInst, "ERROR 9901 Internal error");
				return;
			}
		}
		Send (pInst, "ERROR 7 Invalid property");
		return;
	}
	else
	if (strcasecmp(pszCmd, "CALL") == 0)
	{
		MyContact::Ref user;
		bool bFoundContact = false;

		if (pszCmd = strtok(NULL, " "))
		{
			bFoundContact = pInst->s->GetContact(pszCmd, user);
		}

		if (!bFoundContact)			
		{
			Send (pInst, "ERROR 26 Invalid user handle");
			return;
		}

		/* FIXME: Not implemented! */
		Send (pInst, "ERROR 2 Not Implemented");
		return;
	}
	else
	if (strcasecmp(pszCmd, "OPEN") == 0)
	{
		return;
	}
	else
	if (strcasecmp(pszCmd, "CREATE") == 0)
	{
		if (!(pszCmd = strtok(NULL, " ")))
		{
			Send (pInst, "ERROR 536 CREATE: no object or type given");
			return;
		}
		if (strcasecmp(pszCmd, "APPLICATION") == 0)
		{
			if (!(pszCmd = strtok(NULL, " ")) || strcasecmp(pszCmd, "libpurple_typing"))
				Send (pInst, "ERROR 540 CREATE APPLICATION: Missing or invalid name");
			else
				Send (pInst, "CREATE APPLICATION libpurple_typing");		
		}
		else
			Send (pInst, "ERROR 537 CREATE: Unknown object type given");
		return;
	}
	else
	if (strcasecmp(pszCmd, "ALTER") == 0)
	{
		HandleALTER(pInst);
		return;
	}
	else
	if (strcasecmp(pszCmd, "DELETE") == 0)
	{
		if (!(pszCmd = strtok(NULL, " ")))
			Send (pInst, "ERROR 538 DELETE: no object or type given");
		else
		{
			if (!strcasecmp(pszCmd, "APPLICATION"))
			{
				if (!(pszCmd = strtok(NULL, " ")) || strcasecmp(pszCmd, "libpurple_typing") ||
					!(pszCmd = strtok(NULL, " ")))
					Send (pInst, "ERROR 542 DELETE APPLICATION : missing or invalid application name");
				else
					Send (pInst, "%s %s %s", "DELETE", "APPLICATION", pszCmd);
			}
			else
				Send (pInst, "ERROR 539 DELETE: Unknown object type given");
		}
		return;
	}
	else
	if (strcasecmp(pszCmd, "NAME") == 0)
	{
		if (pszCmd = strtok(NULL, " "))
		{
			if (pInst->pszClientName) free(pInst->pszClientName);
			pInst->pszClientName = strdup(pszCmd);
		}
		Send (pInst, "OK");
		return;
	}
	else 
	if (strcasecmp(pszCmd, "MINIMIZE") == 0 || strcasecmp(pszCmd, "FOCUS") == 0)
	{
		if (pszCmd[5]) pInst->pszWindowState = "MINIMIZED";
		Send (pInst, pszCmd);
		return;
	}
	else
	{
		Send (pInst, "ERROR 2 Not Implemented");
	}
}

// -----------------------------------------------------------------------------

