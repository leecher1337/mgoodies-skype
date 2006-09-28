Plugins in this folder can be automatically loaded into players. This means that you don't have to do nothing and the plugin will be loaded into the player when it is first detected running.

For this to happen 3 things are needed:
1. The following option must be enabled: "Allow auto-loading plugins into players (affect players with *)" (Inside Options->Status->Listening info->Players)
2. The dll name must be: mlt_<playername>.dll
3. The plugin isn't loaded previously (aka it wasn't installed into player by the user)

One warning: for the auto-loading work, some inter-process messages have to happen (namelly code injection) and some anti-virus or firewalls can complain. Also, I don't know if this works if the user don't have admin rights.

You also can install this plugins into the player yourself (and avoid the above warning). Instructions in how to do that are based on the player:

- Winamp: copy the mlt_winamp.dll to <WinampDir>\Plugins\gen_mlt_winamp.dll (Yes, you need to rename it)
