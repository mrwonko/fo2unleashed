# FlatOut 2 Unleashed

A WIP FlatOut 2 mod for improved scripting, akin to the Oblivion Script Extender.

## State

* Working dll injection at startup using a launcher
	* Does not support the steam version though, since that is self-modifying, so dll injection would have to be delayed until after decryption
* Efficient finding of substrings with wildcards (will be used for signature matching)
* However, there is no changed behaviour whatsoever so far aside from a message box at startup
