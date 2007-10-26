Some plugins in this folder can be automatically loaded into players. This means that you don't have to do nothing and the plugin will be loaded into the player when it is first detected running.

For this to happen 3 things are needed:
1. The following option must be enabled: "Allow auto-loading plugins into players (affect players with *)" (Inside Options->Status->Listening info->Players)
2. The implementation allow this to happen (only Winamp by now)
3. The plugin isn't loaded previously (aka it wasn't installed into player by the user)

One warning: for the auto-loading to work, some inter-process messages have to happen (namelly code injection) and some anti-virus or firewalls can complain. Also, I don't know if this works if the user don't have admin rights.

You also can install this plugins into the player yourself (and avoid the above warning). Instructions in how to do that are based on the player:

- Winamp: copy the gen_mlt.dll to <WinampDir>\Plugins\gen_mlt.dll
- foobar2000: copy the foo_mlt.dll to <FoobarDir>\components\foo_mlt.dll

PS: Auto loading does not work on Win 9X