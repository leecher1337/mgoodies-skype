#ifndef MVARS
#define MVARS

#define EGENERIC		99
#define EOK				0
#define OK				EOK
#define ENOWORD			1		/* $n does not exists */
#define EMEM			2		/* malloc error */
#define EVARARG			3		/* error in argument e.a. %lname()% */
#define EMULTC			4		/* multiple contacts found */
#define ENOC			5		/* no contacts found */
#define ELSEEN			6		/* error processing LASTSEEN variable */
#define EFOO			7		/* error processing FOOBARSONG variable */
#define EWAMP			8		/* error processing WINAMPSONG variable */
#define ETVPROG			9		/* error processing TVPROGRAM variable */
#define ETVCHAN			10		/* error processing TVCHANNEL variable */
#define ESTATUS			11		/* error processing CSTATUS variable */
#define EFILE			12		/* error opening file */
#define EDB				13		/* error processing DBSETTING variable */
#define EPARSE			14		/* parse error (probably % forgotten) */
#define ECTIME			15		/* error processing CURRENTTIME variable */
#define ESCROLL			16		/* error processing SCROLL variable */
#define EFDATE			17		/* error processing CFDATE variable */
#define EFTIME			18		/* error processing CFTIME variable */
#define EMYSTATUS		19		/* error processing MYSTATUS variable */
#define EMYSTSMSG		20		/* error processing STSMSG variable */
#define EREPLACE		21		/* error processing REPLACE variable */
#define EURLESC			22		/* error processing URLESC */
#define EURLUNESC		23		/* error processing URLUNESC */

typedef struct {
	int cbSize;
	char *szFormat;
	char *szSource;
	HANDLE hContact;
	int scroll;
	int err;
} FORMATINFO;
#define MS_VARS_FORMATSTRING			"Vars/FormatString"

#define CI_PROTOID		0x00000001
#define CI_NICK			0x00000002
#define CI_LISTNAME		0x00000004
#define CI_FIRSTNAME	0x00000008
#define CI_LASTNAME		0x00000010
#define CI_EMAIL		0x00000020
#define CI_UNIQUEID		0x00000040
typedef struct {
	int cbSize;
	char* szContact;
	HANDLE* hContacts;
	DWORD flags;
} CONTACTSINFO;
#define MS_VARS_GETCONTACTFROMSTRING	"Vars/GetContactFromString"

#define MS_VARS_FREEMEMORY				"Vars/FreeMemory"

#endif