TS3OBSPlugin
============
Plugin that edits a TS3 nickname to have a defined prefix when streaming.  
(Hopefully) Fixed the bug that renamed a user r or cted.  
As of 2013-12-20, the plugin has been updated to allow users to mute and/or deafen themselves as well as switch channels while streaming/recording. They will be unmuted/undeafened and moved back to the original channel when recording/streaming stops.  
  
Compiled using VS 2010  

INSTALLATION
============
Put TS3Plugin.dll into ../Program Files/OBS/plugins folder  
or  
Put TS3Plugin.dll into ../Program Files (x86)/OBS/64 bit/plugins folder  
depending on installation and if you are using x64 or x32(x86) OS.  
Before streaming, open the TS3 Recording Notifier config.  
  
First box allows you to set the IP of the PC running your TS3 client if you record and play on different computers.  
In the second box add your TS3 Unique ID (Settings -> Identities -> Unique ID)  
Change \*R\* in the third box to what ever you want the prefix to be (must be 10 characters or less and have NO spaces)  
The next two checkboxes are self explanatory.  
If the "Change Channel" checkbox is ticked, the "Set Channel" button and "Channel Password" boxes are activated. Move to the streaming/recording channel and press "Set Channel" to set that channel in OBS. Then add the channel password if necessary.  
