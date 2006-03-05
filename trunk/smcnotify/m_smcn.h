#ifndef __M_SMCN_H__
# define __M_SMCN_H__


/*
Return TRUE is smcn is enabled for this protocol
If is enabled, status message is kept under CList\StatusMsg db key in user data

wParam: protocol as string
*/
#define MS_SMCN_ENABLED_FOR_PROTOCOL "smcn/EnabledForProtocol"


/*
Return TRUE is smcn is enabled for this user and this protocol (smcn can be disabled per user,
if protocol is enabled)
If is enabled, status message is kept under CList\StatusMsg db key in user data

wParam: protocol as string
lParam: hContact
*/
#define MS_SMCN_ENABLED_FOR_USER "smcn/EnabledForUser"








#endif // __M_SMCN_H__
