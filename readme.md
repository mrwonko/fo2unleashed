# FlatOut 2 Unleashed

A WIP FlatOut 2 mod for improved scripting, akin to the Oblivion Script Extender.

## State

* Working dll injection at startup using a launcher
	* Does not support the steam version though, since that is self-modifying, so dll injection would have to be delayed until after decryption
* Code for signature matching and general hooking
* Hooks:
	* loading of files listed in mods.txt in addition to those listed in filesystem and patch
