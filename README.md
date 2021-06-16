*THIS CODE DOESNT WORK IN NEW CHROME VERSION (90). Please refer https://github.com/0xfd3/Chrome-Password-Recovery* 

Dump Google Chrome database data and save url,username,decrypted password (plain text) in txt file. 
Tested on Google Chrome Version 67.0.3396.62 (Official Build) (64-bit).

!!!ATTENTION!!! Dumper MUST BE RUN on THE SAME computer where Chrome was installed and passwords were saved AND run under THE SAME user that saved passwords and whom passwords you want to steal. It means that you can not just steal SQLLite database file and decrypt it later. It means that you can not decrypt passwords of user TEST_USER having SYSTEM privileges.

Why? Because I use CryptUnprotectData() function.

All code is licensed under WTFPL. You just DO WHAT THE FUCK YOU WANT TO. You can even assign autorship to YOURSELF.
