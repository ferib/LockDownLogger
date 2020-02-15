# LockDownLogging
Small DLL hooking Respondus LockDownBrowser functions in order to obtain more information about what it is doing.
Grab a tinfoil hat and start looking at what kind of weird stuff LockDownBrowser is logging!

# Keyboard Bypass
LockDownBrowser prevents keyboard action like "alt+tab" to avoid cheating on a exam. A bypass has been added to counter this feature in order to alt+tab into our Console Window (and of course, to take screenshots as done below)

# Result
![img](https://github.com/ferib/LockDownLogger/blob/master/img/screenshot.png?raw=true)

# :warning: Warning :warning:
**if you thought you could use my tool in order to cheat on your exam, then you are wrong!**

There is a "TopMost" detection that checks if the current focused window is from LockDownBrowser. 
This detection will trigger from the moment you do alt+tab or any other keybind to change focus to any other window that isn't created by LockDownBrowser itself.
You will get a warning at first, after the first warning you get notified that LockDownBrowser will get shut down.

Good thing is that the detection is **only active when you are doing an exam**, so it's still possible to use my tool for logging & debugging LockDownBrowser.

*(Canvas has also a build in detection system for when focus is lost, these results gets send back to canvas to detect cheaters!)*

Of course it's still possible to bypass both "TopMost" detection and the Canvas detection.. but hey, fuck off.