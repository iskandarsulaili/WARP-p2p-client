$env:PATH = "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Tools\MSVC\14.44.35207\bin\Hostx86\x86;" + $env:PATH
& "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Tools\MSVC\14.44.35207\bin\Hostx86\x86\cl.exe" /std:c++17 /EHsc launcher.cpp /link /out:Launcher.exe /SUBSYSTEM:WINDOWS user32.lib
